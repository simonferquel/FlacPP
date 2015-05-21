#include "pch.h"
#include "FlacMediaStreamSourcePreBufferizer.h"
#include <wrl.h>
#include <robuffer.h>
#include <windows.storage.streams.h>
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Microsoft::WRL;
using namespace Windows::Media::Core;

void FlacMediaStreamSourcePreBufferizer::start()
{
	auto that = shared_from_this();
	_nextSampleTask = concurrency::create_task([that]() ->MediaStreamSample ^{
		FlacPP::time_unit_100ns frameTime;
		auto decoded = that->_decoder->decodeNextFrame(frameTime);
		if (decoded.length() == 0) {
			if (that->_debugStream) {
				concurrency::create_task(that->_debugStream->FlushAsync()).get();
			}
			return nullptr;
		}
		auto buffer = ref new Buffer(decoded.length());
		ComPtr<IBufferByteAccess> bufferBa;
		reinterpret_cast<IInspectable*>(buffer)->QueryInterface<IBufferByteAccess>(&bufferBa);
		byte* bytes;
		bufferBa->Buffer(&bytes);
		memcpy(bytes, decoded.begin(), decoded.length());
		buffer->Length = decoded.length();
		TimeSpan ts;
		ts.Duration = frameTime.count();
		if (that->_debugStream) {
			concurrency::create_task(that->_debugStream->WriteAsync(buffer)).get();
		}
		return MediaStreamSample::CreateFromBuffer(buffer, ts);
	});
}

void FlacMediaStreamSourcePreBufferizer::stop()
{
	_nextSampleTask.get();
}

Windows::Media::Core::MediaStreamSample ^ FlacMediaStreamSourcePreBufferizer::getSampleAndFetchNext()
{
	auto res = _nextSampleTask.get();
	if (res) {
		start();
	}
	return res;
}
