#pragma once
#include <memory>
#include "StreamInfo.h"
#include "FlacBuffer.h"
#include <vector>

namespace FlacPP{

	class FlacDecodingException : public std::exception {};
	class FlacMagicWordNotFoundException : public FlacDecodingException {};
	class IFlacStream;

	struct seekpoint {
		std::uint64_t firstSampleIndex;
		std::uint64_t bytesOffsetFromFirstFrameHeader;
		std::uint16_t sampleCount;
	};

	class FlacDecoder{
	private:
		stream_info _streamInfo;
		std::unique_ptr<IFlacStream>_stream;
		std::unique_ptr<std::int32_t[]> _outputBuffer;
		std::unique_ptr<std::uint8_t[]> _inputBuffer;
		std::uint64_t _posOfFirstFrame;
		std::uint64_t _nextSample;
		std::vector<seekpoint> _seekpoints;
		std::uint64_t seekSample(std::uint64_t sample);
	public:
		const stream_info& streamInfo() const {
			return _streamInfo;
		}
		FlacDecoder(std::unique_ptr<IFlacStream>&& stream);
		FlacBufferView decodeNextFrame(time_unit_100ns& frameTime);
		time_unit_100ns seek(const time_unit_100ns& ts);
		bool canSeek() {
			return true;
		}
		~FlacDecoder();
	};

}