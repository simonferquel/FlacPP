#pragma once

#ifndef FLACPP_STREAMINFO
#define FLACPP_STREAMINFO

#include <cstdint>
#include <chrono>
namespace FlacPP {
	typedef std::ratio<1, 10000000> nanox100;
	typedef std::chrono::duration<std::int64_t, nanox100> time_unit_100ns;

	// information about a flac decoded stream
	struct stream_info {
		std::uint16_t minBlockSizeInSamples;
		std::uint16_t maxBlockSizeInSamples;
		std::uint32_t minFrameSizeInBytes;
		std::uint32_t maxFrameSizeInBytes;
		std::uint32_t sampleRate;
		std::uint16_t channels;
		std::uint16_t bitsPerSample;
		std::uint64_t totalSamples;

		std::uint16_t outputBitsPerSample;

		time_unit_100ns duration()const {
			
			return time_unit_100ns( totalSamples * 10000000 / sampleRate);
		}

		bool isConstantBlockSize() const {
			return minBlockSizeInSamples == maxBlockSizeInSamples;
		}
	};
}

#endif