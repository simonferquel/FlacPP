#pragma once
#include <memory>
#include <ppltasks.h>
#include <FlacPP/FlacDecoder.h>
class FlacMediaStreamSourcePreBufferizer : public std::enable_shared_from_this<FlacMediaStreamSourcePreBufferizer>
{
private:
	concurrency::task<Windows::Media::Core::MediaStreamSample^> _nextSampleTask;
	std::shared_ptr<FlacPP::FlacDecoder> _decoder;
	Windows::Storage::Streams::IOutputStream^ _debugStream;
public:
	FlacMediaStreamSourcePreBufferizer(const std::shared_ptr<FlacPP::FlacDecoder>& decoder, Windows::Storage::Streams::IOutputStream^ debugStream) : _decoder(decoder), _debugStream (debugStream) {

	}
	void start();
	void stop();
	Windows::Media::Core::MediaStreamSample^ getSampleAndFetchNext();
};

