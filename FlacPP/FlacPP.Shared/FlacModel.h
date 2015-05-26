#pragma once

#ifndef FLACPP_FLACMODEL
#define FLACPP_FLACMODEL

#include <cstdint>
#include <BitHandling.h>

// type of flac metadata block
enum class metadata_block_type : std::uint8_t {

	streamInfo = 0,
	padding = 1,
	application = 2,
	seekTable = 3,
	vorbisComment = 4,
	cueSheet = 5,
	picture = 6

};


enum class blocking_strategy : std::uint8_t {
	encode_frame_number = 0,
	encode_sample_number = 1
};

enum class blocksize_strategy : std::uint8_t {
	direct,
	grab8bits,
	grab16bits
};

enum class samplerate_strategy : std::uint8_t {
	from_streaminfo,
	in_fixed_header,
	grab8BitsKHz,
	grab16BitsHz,
	grab16Bits10Hz,
	invalid
};
enum class samplesize_strategy :std::uint8_t {
	from_streaminfo,
	direct,
	invalid
};

enum class channel_assignment : std::uint8_t {
	independent,
	left_side_stereo,
	side_right_stereo,
	mid_side_stereo,
	invalid
};

#pragma pack(push,1)

struct metadata_block_header {
	union {
		std::uint32_t data_whole;
		struct {
			std::uint8_t data_firstbyte;
			std::uint8_t padding[3];
		} data_parts;
	};
	bool isLast() const {
		return 0x80 == (0x80 & data_parts.data_firstbyte);
	}
	metadata_block_type blockType()const {
		return static_cast<metadata_block_type>(data_parts.data_firstbyte & 0x7F);
	}
	std::uint32_t blockSizeInBytes()const {
		return FlacPP::swapEndiannes(data_whole) & 0x00FFFFFF;
	}
};

struct streaminfo_raw {
	std::uint16_t minBlockSizeRaw;
	std::uint16_t maxBlockSizeRaw;
	std::uint8_t minFrameSizeRaw[3];
	std::uint8_t maxFrameSizeRaw[3];
	std::uint8_t sampleRateChannelsBitsPerSampleAndTotalSamplesFirst4Bits[4];
	std::uint32_t totalSamplesLast4Bytes;
	std::uint8_t md5[16];

	std::uint16_t minBlockSize() const {
		return FlacPP::swapEndiannes(minBlockSizeRaw);
	}
	std::uint16_t maxBlockSize() const {
		return FlacPP::swapEndiannes(maxBlockSizeRaw);
	}
	std::uint32_t minFrameSize()const {
		return (minFrameSizeRaw[0] << 16) | (minFrameSizeRaw[1] << 8) | minFrameSizeRaw[2];
	}

	std::uint32_t maxFrameSize()const {
		return (maxFrameSizeRaw[0] << 16) | (maxFrameSizeRaw[1] << 8) | maxFrameSizeRaw[2];
	}
	std::uint32_t sampleRate()const {
		return (sampleRateChannelsBitsPerSampleAndTotalSamplesFirst4Bits[0] << 12) | (sampleRateChannelsBitsPerSampleAndTotalSamplesFirst4Bits[1] << 4) | (sampleRateChannelsBitsPerSampleAndTotalSamplesFirst4Bits[2] >> 4);
	}
	std::uint16_t channels()const {
		return 1 + ((sampleRateChannelsBitsPerSampleAndTotalSamplesFirst4Bits[2] >> 1) & 0x7);
	}
	std::uint16_t bitsPerSample()const {
		return 1 + ((sampleRateChannelsBitsPerSampleAndTotalSamplesFirst4Bits[2] & 1) << 4) | (sampleRateChannelsBitsPerSampleAndTotalSamplesFirst4Bits[3] >> 4);
	}
	std::uint64_t totalSamples()const {
		return (((std::uint64_t)(sampleRateChannelsBitsPerSampleAndTotalSamplesFirst4Bits[3] & 0xF)) << (std::uint64_t)32) | FlacPP::swapEndiannes(totalSamplesLast4Bytes);
	}
};


struct frame_header_fixed_part_raw {
	std::uint8_t syncCodeAndBlockingStrategyRaw[2];
	std::uint8_t blockSizeAndSampleRateRaw;
	std::uint8_t channelAssignmentAndSampleSizeRaw;

	samplesize_strategy sampleSizeStrategy() const {
		auto sampleSizeBits = ((channelAssignmentAndSampleSizeRaw >> 1) & 0x7);
		if (sampleSizeBits == 0) {
			return samplesize_strategy::from_streaminfo;
		}
		if (sampleSizeBits == 3) {
			return samplesize_strategy::invalid;
		}
		if (sampleSizeBits == 7) {
			return samplesize_strategy::invalid;
		}
		return samplesize_strategy::direct;
	}

	std::uint16_t sampleSizeInBits() const {
		auto sampleSizeBits = ((channelAssignmentAndSampleSizeRaw >> 1) & 0x7);
		switch (sampleSizeBits) {
		case 1:
			return 8;
		case 2:
			return 12;
		case 4:
			return 16;
		case 5:
			return 20;
		case 6:
			return 24;
		default:
			return 0;
		}
	}

	channel_assignment channelAssignment() const {
		std::uint8_t channelAssignmentBits = ((channelAssignmentAndSampleSizeRaw >> 4) & 0xf);
		if (channelAssignmentBits < 8) {
			return channel_assignment::independent;
		}
		if (channelAssignmentBits == 8) {
			return channel_assignment::left_side_stereo;
		}
		if (channelAssignmentBits == 9) {
			return channel_assignment::side_right_stereo;
		}
		if (channelAssignmentBits == 10) {
			return channel_assignment::mid_side_stereo;
		}
		return channel_assignment::invalid;
	}

	std::uint16_t independentChannelsCount() const {
		std::uint8_t channelAssignmentBits = ((channelAssignmentAndSampleSizeRaw >> 4) & 0xf);
		if (channelAssignmentBits >= 8) {
			return 0;
		}
		return channelAssignmentBits + 1;
	}

	blocking_strategy blockingStrategy() const {
		return static_cast<blocking_strategy>(1 == (1 & syncCodeAndBlockingStrategyRaw[1]));
	}
	blocksize_strategy getBlockSizeStrategy() const {
		std::uint8_t blockSizeBits = ((blockSizeAndSampleRateRaw >> 4) & 0xf);

		if (blockSizeBits == 6) {
			return blocksize_strategy::grab8bits;
		}
		if (blockSizeBits == 7) {
			return blocksize_strategy::grab16bits;
		}

		return blocksize_strategy::direct;
	}
	std::uint32_t directBlockSize() const {
		std::uint8_t blockSizeBits = ((blockSizeAndSampleRateRaw >> 4) & 0xf);
		if (blockSizeBits == 0) {
			return 0;
		}
		if (blockSizeBits == 1) {
			return 192;
		}
		if (blockSizeBits >= 2 && blockSizeBits <= 5) {
			return 576 * (1 << (blockSizeBits - 2));
		}
		if (blockSizeBits >= 8) {
			return 256 * (1 << (blockSizeBits - 8));
		}
		return 0;
	}

	samplerate_strategy sampleRateStrategy() const {
		std::uint8_t sampleRateBits = (blockSizeAndSampleRateRaw & 0xf);
		if (sampleRateBits == 0) {
			return samplerate_strategy::from_streaminfo;
		}
		if (sampleRateBits >= 1 && sampleRateBits <= 11) {
			return samplerate_strategy::in_fixed_header;
		}
		if (sampleRateBits == 12) {
			return samplerate_strategy::grab8BitsKHz;
		}
		if (sampleRateBits == 13) {
			return samplerate_strategy::grab16BitsHz;
		}
		if (sampleRateBits == 14) {
			return samplerate_strategy::grab16Bits10Hz;
		}
		return samplerate_strategy::invalid;
	}

	std::uint32_t sampleRate()const {
		std::uint8_t sampleRateBits = (blockSizeAndSampleRateRaw & 0xf);
		switch (sampleRateBits) {
		case 1:
			return 88200;
		case 2:
			return 176400;
		case 3:
			return 192000;
		case 4:
			return 8000;
		case 5:
			return 16000;
		case 6:
			return 22050;
		case 7:
			return 24000;
		case 8:
			return 32000;
		case 9:
			return 44100;
		case 10:
			return 48000;
		case 11:
			return 96000;
		default:
			return 0;
		}
	}
};

struct seek_point_raw {
	std::uint64_t _firstSampleIndexRaw;
	std::uint64_t _bytesOffsetFromFirstFrameHeaderRaw;
	std::uint16_t _sampleCountRaw;
	std::uint64_t firstSampleIndex() const {
		return FlacPP::swapEndiannes(_firstSampleIndexRaw);
	}
	std::uint64_t bytesOffsetFromFirstFrameHeader() const {
		return FlacPP::swapEndiannes(_bytesOffsetFromFirstFrameHeaderRaw);
	}
	std::uint16_t sampleCount() const {
		return FlacPP::swapEndiannes(_sampleCountRaw);
	}
};

#pragma pack(pop)

struct frame_header {
	std::uint64_t firstSampleIndex;
	std::uint32_t blockSize;
	std::uint32_t sampleRate;
	std::uint16_t channelCount;
	std::uint16_t sampleSizeInBits;
	channel_assignment channelAssignment;
};
#endif