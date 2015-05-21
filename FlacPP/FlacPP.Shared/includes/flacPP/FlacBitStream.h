#pragma once
#include "FlacStream.h"
#include <assert.h>
#include <vector>
namespace FlacPP {
	inline std::uint8_t subbyteMask(std::uint8_t bitcount) {
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

		template<std::uint8_t parameter, typename returnType = std::enable_if_t<parameter <= 32>>
		returnType readRiceSignedBlock(std::vector<std::int32_t>& output, std::uint32_t nvals) {
			if (parameter == 0) {
				for (auto ix = 0u;ix < nvals;++ix) {
					/* read the unary MSBs and end bit */
					auto msbs = readUnaryUnsigned();
					if (msbs & 1) {
						output.push_back(-((int)(msbs >> 1)) - 1);
					}
					else {
						output.push_back((int)(msbs >> 1));
					}
				}

				return;
			}


			for (auto ix = 0u;ix < nvals;++ix) {
				auto msbs = readUnaryUnsigned();
				auto lsbs = readPartialUint32<parameter>();

				auto unsignedVal = (msbs << parameter) | lsbs;

				if (unsignedVal & 1) {
					output.push_back(-((int)(unsignedVal >> 1)) - 1);
				}
				else {
					output.push_back((int)(unsignedVal >> 1));
				}
			}
		}
	public:
		FlacBitStream(IFlacStream* innerStream);

		template<std::uint8_t bits, typename returnType = std::enable_if_t<bits<=32, std::uint32_t>>
		returnType readPartialUint32() {
			{
				if (bits <= 8) {
					return readSubByteData<bits>();
				}
				if (bits <= 16) {
					auto p0 = (std::uint32_t) readSubByteData<8>();
					auto p1 = (std::uint32_t) readSubByteData<bits-8>();
					return (p0 << (bits - 8)) | p1;
				}

				if (bits <= 24) {
					auto p0 = (std::uint32_t) readSubByteData<8>();
					auto p1 = (std::uint32_t) readSubByteData<8>();
					auto p2 = (std::uint32_t) readSubByteData<bits-16>();
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
		void readRiceSignedBlock(std::vector<std::int32_t>& output, std::uint32_t nvals, std::uint8_t parameter)
		{
			switch (parameter)
			{
			case 0:
				readRiceSignedBlock<0>(output, nvals);
				break;
			case 1:
				readRiceSignedBlock<1>(output, nvals);
				break;
			case 2:
				readRiceSignedBlock<2>(output, nvals);
				break;
			case 3:
				readRiceSignedBlock<3>(output, nvals);
				break;
			case 4:
				readRiceSignedBlock<4>(output, nvals);
				break;
			case 5:
				readRiceSignedBlock<5>(output, nvals);
				break;
			case 6:
				readRiceSignedBlock<6>(output, nvals);
				break;
			case 7:
				readRiceSignedBlock<7>(output, nvals);
				break;
			case 8:
				readRiceSignedBlock<8>(output, nvals);
				break;
			case 9:
				readRiceSignedBlock<9>(output, nvals);
				break;
			case 10:
				readRiceSignedBlock<10>(output, nvals);
				break;
			case 11:
				readRiceSignedBlock<11>(output, nvals);
				break;
			case 12:
				readRiceSignedBlock<12>(output, nvals);
				break;
			case 13:
				readRiceSignedBlock<13>(output, nvals);
				break;
			case 14:
				readRiceSignedBlock<14>(output, nvals);
				break;
			case 15:
				readRiceSignedBlock<15>(output, nvals);
				break;
			case 16:
				readRiceSignedBlock<16>(output, nvals);
				break;
			case 17:
				readRiceSignedBlock<17>(output, nvals);
				break;
			case 18:
				readRiceSignedBlock<18>(output, nvals);
				break;
			case 19:
				readRiceSignedBlock<19>(output, nvals);
				break;
			case 20:
				readRiceSignedBlock<20>(output, nvals);
				break;
			case 21:
				readRiceSignedBlock<21>(output, nvals);
				break;
			case 22:
				readRiceSignedBlock<22>(output, nvals);
				break;
			case 23:
				readRiceSignedBlock<23>(output, nvals);
				break;
			case 24:
				readRiceSignedBlock<24>(output, nvals);
				break;
			case 25:
				readRiceSignedBlock<25>(output, nvals);
				break;
			case 26:
				readRiceSignedBlock<26>(output, nvals);
				break;
			case 27:
				readRiceSignedBlock<27>(output, nvals);
				break;
			case 28:
				readRiceSignedBlock<28>(output, nvals);
				break;
			case 29:
				readRiceSignedBlock<29>(output, nvals);
				break;
			case 30:
				readRiceSignedBlock<30>(output, nvals);
				break;
			case 31:
				readRiceSignedBlock<31>(output, nvals);
				break;
			case 32:
				readRiceSignedBlock<32>(output, nvals);
				break;
			

			

			default:
				break;
			}
		}

		

	};
}

