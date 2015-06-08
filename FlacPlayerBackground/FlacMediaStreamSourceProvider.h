#pragma once
#include <flacPP\FlacDecoder.h>
#include <memory>

class FlacMediaStreamSourceProvider
{
private:
	std::shared_ptr<FlacPP::FlacDecoder> _decoder;
public:
	FlacMediaStreamSourceProvider(const std::shared_ptr<FlacPP::FlacDecoder>& decoder);
	Windows::Media::Core::MediaStreamSource^ CreateMediaStreamSource();
};

