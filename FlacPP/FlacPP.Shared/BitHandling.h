#pragma once
#ifndef FLACPP_BITHANDLING
#define FLACPP_BITHANDLING
#include <cstdint>
#ifdef _MSC_VER
#include <Windows.h>
#endif
namespace FlacPP {
	template<typename T>
	T swapEndiannes(const T& source) {
		T result;
		const std::uint8_t* srcPtr = reinterpret_cast<const std::uint8_t*>(&source);
		std::uint8_t* destPtr = reinterpret_cast<std::uint8_t*>(&result);

		for (auto ix = 0u; ix < sizeof(T); ++ix) {
			destPtr[ix] = srcPtr[sizeof(T) - 1 - ix];
		}
		return result;
	}

#ifdef _MSC_VER
	template<>
	inline std::uint32_t swapEndiannes(const std::uint32_t& source) {
		return _byteswap_ulong(source);
	}

	template<>
	inline std::int32_t swapEndiannes(const std::int32_t& source) {
		return static_cast<std::int32_t>( _byteswap_ulong(static_cast<std::uint32_t>(source)));
	}

	template<>
	inline std::uint16_t swapEndiannes(const std::uint16_t& source) {
		return _byteswap_ushort(source);
	}

	template<>
	inline std::uint64_t swapEndiannes(const std::uint64_t& source) {
		return _byteswap_uint64(source);
	}


	template<>
	inline std::int16_t swapEndiannes(const std::int16_t& source) {
		return static_cast<std::int16_t>(_byteswap_ushort(static_cast<std::uint16_t>(source)));
	}

	template<>
	inline std::int64_t swapEndiannes(const std::int64_t& source) {
		return static_cast<std::int64_t>(_byteswap_uint64(static_cast<std::uint64_t>(source)));
	}
#endif
}



#endif