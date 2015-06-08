#pragma once

#ifndef FLACPP_FLACDECODER
#define FLACPP_FLACDECODER

#include <memory>
#include "StreamInfo.h"
#include "FlacBuffer.h"
#include <vector>

namespace FlacPP{

	class temp_buffer_cache {
	private:
		std::vector<std::vector<std::int32_t>> _data;
	public:
		std::vector<std::int32_t> getOne() {
			std::vector<std::int32_t> result;
			if (_data.size() > 0) {
				result = std::move(_data[_data.size() - 1]);

				_data.resize(_data.size() - 1);
			}
			return std::move(result);
		}
		void release(std::vector<std::int32_t>&& data) {
			data.resize(0);
			_data.push_back(std::move(data));
		}
	};

	class FlacDecodingException : public std::exception {};
	class FlacMagicWordNotFoundException : public FlacDecodingException {};
	class IFlacStream;

	// seek point found in the metadata
	struct seekpoint {
		std::uint64_t firstSampleIndex;
		std::uint64_t bytesOffsetFromFirstFrameHeader;
		std::uint16_t sampleCount;
	};

	// main decoder
	class FlacDecoder{
	private:
		stream_info _streamInfo;
		std::unique_ptr<IFlacStream>_stream;
		std::unique_ptr<std::uint8_t[]> _outputBuffer;
		std::uint64_t _posOfFirstFrame;
		std::uint64_t _nextSample;
		std::vector<seekpoint> _seekpoints;
		std::uint64_t seekSample(std::uint64_t sample);
		temp_buffer_cache _residualBufferCache;
		temp_buffer_cache _outputBufferCache;
	public:
		// get information from the flac stream
		const stream_info& streamInfo() const {
			return _streamInfo;
		}
		// create an instance of the decoder
		FlacDecoder(std::unique_ptr<IFlacStream>&& stream);
		// decode the next frame in the stream. Returned buffer view remains valid until next call to decodeNextFrame, seek, or destruction of the decoder
		FlacBufferView decodeNextFrame(time_unit_100ns& frameTime);
		// decode the next frame in the stream directly to an externally allocated buffer
		void decodeNextFrame(time_unit_100ns& frameTime, FlacBufferView& buf);
		// seek inside the stream (returns the actual position in the stream after the seek operation)
		time_unit_100ns seek(const time_unit_100ns& ts);
		// always true (will be false when "unseekable Byte stream support" will be implemnted)
		bool canSeek() {
			return true;
		}
		// destroy the decoder
		~FlacDecoder();
	};

}
#endif