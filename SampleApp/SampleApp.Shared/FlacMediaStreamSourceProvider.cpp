#include "pch.h"
#include "FlacMediaStreamSourceProvider.h"

#include <ProducerConsumerQueue.h>
#include <ppltasks.h>
#include <thread>
#include <robuffer.h>
#include <windows.storage.streams.h>
#include <wrl.h>
using namespace Microsoft::WRL;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace Windows::Media;
using namespace Windows::Media::Core;

FlacMediaStreamSourceProvider::FlacMediaStreamSourceProvider(const std::shared_ptr<FlacPP::FlacDecoder>& decoder)
	: _decoder(decoder)
{
	
}

struct decoding_state {
	std::thread currentDecodingThread;
	concurrency::cancellation_token_source currentCts;
	producer_consumer_queue<MediaStreamSample^> sampleQueue;
	decoding_state():sampleQueue(5){}
};

std::thread createDecodingThread(const std::shared_ptr<FlacPP::FlacDecoder>& decoder, const std::shared_ptr<decoding_state>& decodingState, concurrency::cancellation_token ct) {
	return std::thread([decoder, decodingState, ct]() {
		auto bufferSizeInSamples = decoder->streamInfo().sampleRate;
		if (bufferSizeInSamples < decoder->streamInfo().maxBlockSizeInSamples) {
			bufferSizeInSamples = decoder->streamInfo().maxBlockSizeInSamples;
		}
		auto bufferSize = decoder->streamInfo().outputBitsPerSample / 8 * decoder->streamInfo().channels * bufferSizeInSamples;
		while (!ct.is_canceled()) {
			auto buffer = ref new Buffer(bufferSize);
			ComPtr<IBufferByteAccess> bufferBa;
			reinterpret_cast<IInspectable*>(buffer)->QueryInterface<IBufferByteAccess>(&bufferBa);
			byte* bytes;
			bufferBa->Buffer(&bytes);
			FlacPP::time_unit_100ns frameTime;
			bool isFirst = true;
			while (!ct.is_canceled()) {
				FlacPP::time_unit_100ns lastFrameTime;
				auto decoded = decoder->decodeNextFrame(isFirst ? frameTime : lastFrameTime);
				isFirst = false;
				if (decoded.length() == 0) {
					try {
						if (buffer->Length>0) {
							TimeSpan ts;
							ts.Duration = frameTime.count();
							decodingState->sampleQueue.enqueue(MediaStreamSample::CreateFromBuffer(buffer, ts), ct);
						}
						decodingState->sampleQueue.enqueue(nullptr, ct);
						return;
					}
					catch (concurrency::task_canceled&) {}
					return;
				}
				else {
					if (decoded.length() + buffer->Length > bufferSize) {
						try {
							TimeSpan ts;
							ts.Duration = frameTime.count();
							decodingState->sampleQueue.enqueue(MediaStreamSample::CreateFromBuffer(buffer, ts), ct);
							frameTime = lastFrameTime;
							buffer = ref new Buffer(bufferSize);
							reinterpret_cast<IInspectable*>(buffer)->QueryInterface<IBufferByteAccess>(&bufferBa);
							bytes;
							bufferBa->Buffer(&bytes);
							memcpy(bytes, decoded.begin(), decoded.length());
							buffer->Length = decoded.length();
						}
						catch (concurrency::task_canceled&) {
							return;
						}

					}
					else {
						memcpy(bytes + buffer->Length, decoded.begin(), decoded.length());
						buffer->Length += decoded.length();
					}
				}
			}
		}
	});
}

Windows::Media::Core::MediaStreamSource ^ FlacMediaStreamSourceProvider::CreateMediaStreamSource()
{
	auto decoder = _decoder;
	std::shared_ptr<decoding_state> decodingState = std::make_shared<decoding_state>();
	auto mss = ref new MediaStreamSource(ref new AudioStreamDescriptor(Windows::Media::MediaProperties::AudioEncodingProperties::CreatePcm(
		decoder->streamInfo().sampleRate,
		decoder->streamInfo().channels,
		decoder->streamInfo().outputBitsPerSample
		)));

	mss->CanSeek = true;
	auto duration = decoder->streamInfo().duration();
	TimeSpan durationTS;
	durationTS.Duration = duration.count();
	mss->Duration = durationTS;

	mss->Paused += ref new TypedEventHandler<MediaStreamSource^, Platform::Object^>([decodingState]
		(MediaStreamSource^ src, Platform::Object^ args) {
		decodingState->currentCts.cancel();
		if (decodingState->currentDecodingThread.joinable()) {
			decodingState->currentDecodingThread.join();
		}
		decodingState->sampleQueue.clear();
	});

	mss->Starting += ref new TypedEventHandler<MediaStreamSource^, MediaStreamSourceStartingEventArgs^>(
		[decoder, decodingState](MediaStreamSource^ src, MediaStreamSourceStartingEventArgs^ args) {

		if (args->Request->StartPosition) {
			auto p = decoder->seek(FlacPP::time_unit_100ns(args->Request->StartPosition->Value.Duration));
			TimeSpan ts;
			ts.Duration = p.count();
			args->Request->SetActualStartPosition(ts);
		}
		decodingState->currentCts = concurrency::cancellation_token_source();
		auto cancelToken = decodingState->currentCts.get_token();
		decodingState->currentDecodingThread = createDecodingThread(decoder, decodingState, cancelToken);
	});


	mss->SampleRequested += ref new TypedEventHandler<MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^>(
		[decodingState](MediaStreamSource^ src, MediaStreamSourceSampleRequestedEventArgs^ args) {

		args->Request->Sample = decodingState->sampleQueue.dequeue(concurrency::cancellation_token::none());
	});
	mss->Closed += ref new TypedEventHandler<MediaStreamSource^, MediaStreamSourceClosedEventArgs^>([decodingState](MediaStreamSource^ src, MediaStreamSourceClosedEventArgs^ args) {
		decodingState->currentCts.cancel();
	});
	return mss;
}
