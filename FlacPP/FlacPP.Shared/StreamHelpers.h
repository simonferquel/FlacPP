#pragma once

#ifndef FLACPP_STREAMHELPERS
#define FLACPP_STREAMHELPERS
#include <cstdint>
#include "includes\flacPP\FlacStream.h"

// read arbitrary type from a IFlacStream object
template <typename T>
void readFromStream(FlacPP::IFlacStream* stream, T& value) {
	FlacPP::FlacBufferView buf(reinterpret_cast<std::uint8_t*>(&value), sizeof(T));
	stream->readIntoBuffer(sizeof(T), buf);
	if (buf.length() != sizeof(T)) {
		throw FlacPP::FlacDecodingException();
	}
}

// read utf8 encoded uint64 from a flac stream
std::uint64_t read_utf8_uint64(FlacPP::IFlacStream* stream, std::int32_t maxBytes) {
	uint64_t value = 0;
	uint8_t x;
	std::uint32_t remainingBytes;

	readFromStream(stream, x);
	if (--maxBytes < 0) {
		return 0;
	}

	if (!(x & 0x80)) { /* 0xxxxxxx */
		value = x;
		remainingBytes = 0;
	}
	else if (x & 0xC0 && !(x & 0x20)) { /* 110xxxxx */
		value = x & 0x1F;
		remainingBytes = 1;
	}
	else if (x & 0xE0 && !(x & 0x10)) { /* 1110xxxx */
		value = x & 0x0F;
		remainingBytes = 2;
	}
	else if (x & 0xF0 && !(x & 0x08)) { /* 11110xxx */
		value = x & 0x07;
		remainingBytes = 3;
	}
	else if (x & 0xF8 && !(x & 0x04)) { /* 111110xx */
		value = x & 0x03;
		remainingBytes = 4;
	}
	else if (x & 0xFC && !(x & 0x02)) { /* 1111110x */
		value = x & 0x01;
		remainingBytes = 5;
	}
	else if (x & 0xFE && !(x & 0x01)) { /* 11111110 */
		value = 0;
		remainingBytes = 6;
	}
	else {
		return std::uint64_t(0xffffffffffffffff);
	}

	for (; remainingBytes; remainingBytes--) {
		readFromStream(stream, x);
		if (--maxBytes < 0) {
			return 0;
		}
		if (!(x & 0x80) || (x & 0x40)) { /* 10xxxxxx */
			return std::uint64_t(0xffffffffffffffff);
		}
		value <<= 6;
		value |= (x & 0x3F);
	}
	return value;
}

#endif