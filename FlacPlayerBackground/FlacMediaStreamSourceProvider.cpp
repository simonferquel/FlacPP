#include "pch.h"
#include "FlacMediaStreamSourceProvider.h"

#include <ProducerConsumerQueue.h>
#include <ppltasks.h>
#include <thread>
#include <robuffer.h>
#include <windows.storage.streams.h>
#include <wrl.h>
#include <mutex>
using namespace Microsoft::WRL;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace Windows::Media;
using namespace Windows::Media::Core;

class buffer_pool {
private:
	std::uint32_t _bufferSize;
	std::mutex _mut;
	std::vector<IBuffer^> _buffers;
public:
	buffer_pool(std::uint32_t size) : _bufferSize(size) {

	}

	IBuffer^ get() {
		std::unique_lock<std::mutex> lg(_mut);
		if (_buffers.size() == 0) {
			return ref new Buffer(_bufferSize);
		}
		auto bufIt = _buffers.begin() + (_buffers.size() - 1);
		auto buf = *bufIt;
		_buffers.erase(bufIt);
		return buf;
	}
	void release(IBuffer^ buf) {
		buf->Length = 0;
		std::unique_lock<std::mutex> lg(_mut);
		_buffers.push_back(buf);
	}
};

FlacMediaStreamSourceProvider::FlacMediaStreamSourceProvider(const std::shared_ptr<FlacPP::FlacDecoder>& decoder)
	: _decoder(decoder)
{

}

struct decoding_state {
	std::thread currentDecodingThread;
	concurrency::cancellation_token_source currentCts;
	producer_consumer_queue<MediaStreamSample^> sampleQueue;
	decoding_state() :sampleQueue(3) {}
};

std::thread createDecodingThread(const std::shared_ptr<FlacPP::FlacDecoder>& decoder, const std::shared_ptr<decoding_state>& decodingState, concurrency::cancellation_token ct) {
	return std::thread([decoder, decodingState, ct]() {
		auto bufferSizeInSamples = decoder->streamInfo().maxBlockSizeInSamples;

		auto bufferSize = decoder->streamInfo().outputBitsPerSample / 8 * decoder->streamInfo().channels * bufferSizeInSamples;
		auto bufPool = std::make_shared<buffer_pool>(bufferSize);
		while (!ct.is_canceled()) {
			auto buffer = bufPool->get();
			ComPtr<IBufferByteAccess> bufferBa;
			reinterpret_cast<IInspectable*>(buffer)->QueryInterface<IBufferByteAccess>(&bufferBa);
			byte* bytes;
			bufferBa->Buffer(&bytes);
			while (!ct.is_canceled()) {
				FlacPP::FlacBufferView buf(bytes, buffer->Capacity);
				FlacPP::time_unit_100ns frameTime;
				decoder->decodeNextFrame(frameTime, buf);
				if (buf.length() == 0) {
					try {

						decodingState->sampleQueue.enqueue(nullptr, ct);
						return;
					}
					catch (concurrency::task_canceled&) {}
					return;
				}
				else {
					try {
						buffer->Length = buf.length();
						TimeSpan ts;
						ts.Duration = frameTime.count();
						auto sample = MediaStreamSample::CreateFromBuffer(buffer, ts);
						auto weakBufPool = std::weak_ptr<buffer_pool>(bufPool);
						sample->Processed += ref new TypedEventHandler<MediaStreamSample^, Platform::Object^>([weakBufPool](MediaStreamSample^ sample, Platform::Object^) {
							auto bufPool = weakBufPool.lock();
							if (bufPool) {
								bufPool->release(sample->Buffer);
							}
						});
						decodingState->sampleQueue.enqueue(sample, ct);

						buffer = bufPool->get();
						reinterpret_cast<IInspectable*>(buffer)->QueryInterface<IBufferByteAccess>(&bufferBa);
						bytes;
						bufferBa->Buffer(&bytes);
					}
					catch (concurrency::task_canceled&) {
						return;
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
