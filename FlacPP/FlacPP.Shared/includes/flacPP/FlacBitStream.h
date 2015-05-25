#pragma once
#include "FlacStream.h"
#include <assert.h>
#include <vector>
#include <cstdint>

namespace FlacPP {
	


	inline std::uint8_t subbyteMask(std::uint8_t bitcount) {
		if (bitcount == 8) {
			return 0xFF;
		}
		return (1u << bitcount) - 1u;
	}

	inline std::uint32_t subbyteMask32(std::uint8_t bitcount) {
		if (bitcount == 32) {
			return 0xFFFFFFFF;
		}
		return (1u << bitcount) - 1u;
	}

	

	class FlacBitStream {
	private:
		IFlacStream* _innerStream;
		std::uint8_t _currentByte;
		std::uint8_t _consumedBitsInCurrentByte;

		std::uint8_t readSubByteDataGeneric(std::uint8_t bitCount) {
			auto remainingBits = 8 - _consumedBitsInCurrentByte;
			if (remainingBits == bitCount) {
				_consumedBitsInCurrentByte = 8;
				return (_currentByte & subbyteMask(bitCount));
			}
			else if (remainingBits > bitCount) {
				auto remainingAfterThis = remainingBits - bitCount;
				_consumedBitsInCurrentByte += bitCount;
				return(_currentByte >> remainingAfterThis)&subbyteMask(bitCount);
			}
			else {

				if (remainingBits == 0) {
					_currentByte = _innerStream->readOneByte();
					_consumedBitsInCurrentByte = 0;
					return readSubByteDataGeneric(bitCount);
				}
				else {
					auto temp = _currentByte&subbyteMask(remainingBits);

					_currentByte = _innerStream->readOneByte();
					_consumedBitsInCurrentByte = 0;
					temp = temp << (bitCount - remainingBits);
					return temp | readSubByteDataGeneric(bitCount - remainingBits);
				}
			}
		}

		template<std::uint8_t bitCount>
		std::uint8_t readSubByteData()
		{
			auto remainingBits = 8 - _consumedBitsInCurrentByte;
			if (remainingBits == bitCount) {
				_consumedBitsInCurrentByte = 8;
				return (_currentByte & subbyteMask(bitCount));
			}
			else if (remainingBits > bitCount) {
				auto remainingAfterThis = remainingBits - bitCount;
				_consumedBitsInCurrentByte += bitCount;
				return(_currentByte >> remainingAfterThis)&subbyteMask(bitCount);
			}
			else {

				if (remainingBits == 0) {
					_currentByte = _innerStream->readOneByte();
					_consumedBitsInCurrentByte = 0;
					return readSubByteData<bitCount>();
				}
				else {
					auto temp = _currentByte&subbyteMask(remainingBits);

					_currentByte = _innerStream->readOneByte();
					_consumedBitsInCurrentByte = 0;
					temp = temp << (bitCount - remainingBits);
					return temp | readSubByteDataGeneric(bitCount - remainingBits);
				}
			}
		}

		
	public:
		FlacBitStream(IFlacStream* innerStream);

		template<std::uint8_t bits, typename returnType = std::enable_if_t<bits <= 32, std::uint32_t>>
		returnType readPartialUint32() {
			{
				if (bits <= 8) {
					return readSubByteData<bits>();
				}
				if (bits <= 16) {
					auto p0 = (std::uint32_t) readSubByteData<8>();
					auto p1 = (std::uint32_t) readSubByteData<bits - 8>();
					return (p0 << (bits - 8)) | p1;
				}

				if (bits <= 24) {
					auto p0 = (std::uint32_t) readSubByteData<8>();
					auto p1 = (std::uint32_t) readSubByteData<8>();
					auto p2 = (std::uint32_t) readSubByteData<bits - 16>();
					return (p0 << (bits - 8)) | (p1 << (bits - 16)) | p2;
				}


				auto p0 = (std::uint32_t) readSubByteData<8>();
				auto p1 = (std::uint32_t) readSubByteData<8>();
				auto p2 = (std::uint32_t) readSubByteData<8>();
				auto p3 = (std::uint32_t) readSubByteData<bits - 24>();
				return (p0 << (bits - 8)) | (p1 << (bits - 16)) | (p2 << (bits - 24)) | p3;

			}

		}

		std::uint32_t readPartialUint32(std::uint8_t bits) {
			{
				if (bits <= 8) {
					return readSubByteDataGeneric(bits);
				}
				if (bits <= 16) {
					auto p0 = (std::uint32_t) readSubByteData<8>();
					auto p1 = (std::uint32_t) readSubByteDataGeneric(bits - 8);
					return (p0 << (bits - 8)) | p1;
				}

				if (bits <= 24) {
					auto p0 = (std::uint32_t) readSubByteData<8>();
					auto p1 = (std::uint32_t) readSubByteData<8>();
					auto p2 = (std::uint32_t) readSubByteDataGeneric(bits - 16);
					return (p0 << (bits - 8)) | (p1 << (bits - 16)) | p2;
				}


				auto p0 = (std::uint32_t) readSubByteData<8>();
				auto p1 = (std::uint32_t) readSubByteData<8>();
				auto p2 = (std::uint32_t) readSubByteData<8>();
				auto p3 = (std::uint32_t) readSubByteDataGeneric(bits - 24);
				return (p0 << (bits - 8)) | (p1 << (bits - 16)) | (p2 << (bits - 24)) | p3;

			}
		}
		void skipBits(std::uint32_t bitCount);

		std::uint32_t readUnaryUnsigned();

		std::uint32_t readInverseUnaryUnsigned();
		template<std::uint8_t interestingBits>
		std::int32_t readPartialInt32() {
			auto res = readPartialUint32<interestingBits>();
			auto toShift = 32 - interestingBits;
			if (toShift != 0) {
				auto val = ((std::int32_t)res) << toShift;
				val >>= toShift;
				return val;
			}
			else {
				return static_cast<std::int32_t>(res);
			}
		}

		std::int32_t readPartialInt32(std::uint8_t interestingBits) {
			auto res = readPartialUint32(interestingBits);
			auto toShift = 32 - interestingBits;
			if (toShift != 0) {
				auto val = ((std::int32_t)res) << toShift;
				val >>= toShift;
				return val;
			}
			else {
				return static_cast<std::int32_t>(res);
			}
		}
		void readRiceSignedBlock(std::vector<std::int32_t>& output, std::uint32_t nvals, std::uint8_t parameter);



	};
}

