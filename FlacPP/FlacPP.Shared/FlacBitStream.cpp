#include "pch.h"
#include "includes\flacPP\FlacBitStream.h"
#include <vector>
#include "BitHandling.h"


inline void mergeMsbsAndLsbs(std::int32_t* output, const std::uint32_t* msbs, const std::uint32_t* lsbs, std::uint32_t shift, std::uint32_t nvals) {
	for (auto ix = 0u; ix < nvals; ++ix) {
		auto unsignedVal = (msbs[ix] << shift) | lsbs[ix];
		output[ix] = (std::int32_t) (unsignedVal >> 1 ^ -(std::int32_t) (unsignedVal & 1));
	}
}
const std::uint32_t allOnes = 0xFFFFFFFF;

#ifdef _MSC_VER
#include <Windows.h>
inline std::uint32_t countZeros(std::uint32_t word) {
	DWORD index;
	_BitScanReverse(&index, word);
	return 31 - index;
	/*return (word) <= 0xffff ?

	((word) <= 0xff ? byte_to_unary_table[word] + 24 : byte_to_unary_table[(word) >> 8] + 16) :

	((word) <= 0xffffff ? byte_to_unary_table[word >> 16] + 8 : byte_to_unary_table[(word) >> 24]);*/
}
#else
inline std::uint32_t countZeros(std::uint32_t v) {
	return __builtin_clz(v);
}

#endif


FlacPP::FlacBitStream::FlacBitStream(IFlacStream * innerStream)
	:_innerStream(innerStream),
	_consumedBitsInCurrentByte(8)
{
}


void FlacPP::FlacBitStream::skipBits(std::uint32_t bitCount)
{
	while (bitCount + _consumedBitsInCurrentByte > 8) {

		_currentByte = _innerStream->readOneByte();
		bitCount -= 8;
	}
	_consumedBitsInCurrentByte += bitCount;
}

std::uint32_t FlacPP::FlacBitStream::readUnaryUnsigned()
{
	std::uint32_t result = 0;
	while (readPartialUint32<1>() == 0) {
		++result;
	}
	return result;
}

std::uint32_t FlacPP::FlacBitStream::readInverseUnaryUnsigned()
{
	std::uint32_t result = 0;
	while (readPartialUint32<1>() == 1) {
		++result;
	}
	return result;
}

void FlacPP::FlacBitStream::readRiceSignedBlock(std::vector<std::int32_t>& output, std::uint32_t nvals, std::uint8_t parameter)
{

	auto baseIndex = output.size();
	output.resize(baseIndex + nvals);
	std::uint32_t localData[1024];
	std::uint32_t consumedBitsInCurrentWord = 0;
	std::uint32_t currentWordIndex = 0;
	std::uint32_t loadedWords = 0;
	std::uint32_t currentWord = 0;
	std::uint32_t loadedBytes = 0;

	// load initial state in local variables (to help them stay in registers
	if (_consumedBitsInCurrentByte != 8) {
		localData[0] = currentWord = _currentByte;
		loadedWords = 1;
		consumedBitsInCurrentWord = 24 + _consumedBitsInCurrentByte;
	}
	else {
		// load data
		FlacBufferView buf(reinterpret_cast<std::uint8_t*>(localData), 1024 * sizeof(std::uint32_t));
		_innerStream->readIntoBuffer(1024 * sizeof(std::uint32_t), buf);
		loadedBytes = buf.length();
		loadedWords = loadedBytes / 4;
		if (loadedBytes % 4 != 0) {
			loadedWords += 1;
		}
		currentWord = FlacPP::swapEndiannes(localData[0]);
	}

	if (parameter == 0) {
		for (auto ix = 0u; ix < nvals; ++ix) {
			/* read the unary MSBs and end bit */
			std::uint32_t msbs = 0;
			for (;;) {
				if (consumedBitsInCurrentWord == 32) {
					// go to next word and continue
					++currentWordIndex;
					if (currentWordIndex == loadedWords) {
						// fetch data
						FlacBufferView buf(reinterpret_cast<std::uint8_t*>(localData), 1024 * sizeof(std::uint32_t));
						_innerStream->readIntoBuffer(1024 * sizeof(std::uint32_t), buf);
						loadedBytes = buf.length();
						loadedWords = loadedBytes / 4;
						if (loadedBytes % 4 != 0) {
							loadedWords += 1;
						}
						currentWordIndex = 0;
					}
					currentWord = FlacPP::swapEndiannes(localData[currentWordIndex]);
					consumedBitsInCurrentWord = 0;
				}
				auto testWord = currentWord << consumedBitsInCurrentWord;
				if (testWord) {
					auto zeroNB = countZeros(testWord);
					msbs += zeroNB;
					consumedBitsInCurrentWord += zeroNB + 1;
					break;
				}
				else {
					msbs += 32 - consumedBitsInCurrentWord;
					consumedBitsInCurrentWord = 32;
				}
			}

			if (msbs & 1) {
				output[ix + baseIndex] = (-((int)(msbs >> 1)) - 1);
			}
			else {
				output[ix + baseIndex] = ((int)(msbs >> 1));
			}

		}

	}
	else {

		std::unique_ptr<std::uint32_t[]> msbsList(new std::uint32_t[nvals + 4]);
		std::unique_ptr<std::uint32_t[]> lsbsList(new std::uint32_t[nvals + 4]);
		for (auto ix = 0u; ix < nvals; ++ix) {
			std::uint32_t msbs = 0;
			for (;;) {
				if (consumedBitsInCurrentWord == 32) {
					// go to next word and continue
					++currentWordIndex;
					if (currentWordIndex == loadedWords) {
						// fetch data
						FlacBufferView buf(reinterpret_cast<std::uint8_t*>(localData), 1024 * sizeof(std::uint32_t));
						_innerStream->readIntoBuffer(1024 * sizeof(std::uint32_t), buf);
						loadedBytes = buf.length();
						loadedWords = loadedBytes / 4;
						if (loadedBytes % 4 != 0) {
							loadedWords += 1;
						}
						currentWordIndex = 0;
					}
					currentWord = FlacPP::swapEndiannes(localData[currentWordIndex]);
					consumedBitsInCurrentWord = 0;
				}
				auto testWord = currentWord << consumedBitsInCurrentWord;
				if (testWord) {
					auto zeroNB = countZeros(testWord);
					msbs += zeroNB;
					consumedBitsInCurrentWord += zeroNB + 1;
					break;
				}
				else {
					msbs += 32 - consumedBitsInCurrentWord;
					consumedBitsInCurrentWord = 32;
				}
			}
			std::uint32_t lsbs = 0;
			auto bitsToRead = parameter;
			for (;;) {
				if (consumedBitsInCurrentWord == 32) {
					// go to next word and continue
					++currentWordIndex;
					if (currentWordIndex == loadedWords) {
						// fetch data
						FlacBufferView buf(reinterpret_cast<std::uint8_t*>(localData), 1024 * sizeof(std::uint32_t));
						_innerStream->readIntoBuffer(1024 * sizeof(std::uint32_t), buf);
						loadedBytes = buf.length();
						loadedWords = loadedBytes / 4;
						if (loadedBytes % 4 != 0) {
							loadedWords += 1;
						}
						currentWordIndex = 0;
					}
					currentWord = FlacPP::swapEndiannes(localData[currentWordIndex]);
					consumedBitsInCurrentWord = 0;
				}
				if (consumedBitsInCurrentWord == 0) {
					consumedBitsInCurrentWord = bitsToRead;
					lsbs += currentWord >> (32 - bitsToRead);
					break;
				}
				else if (consumedBitsInCurrentWord + bitsToRead <= 32) {
					auto bitsAfter = 32 - consumedBitsInCurrentWord - bitsToRead;
					lsbs += ((allOnes >> consumedBitsInCurrentWord)& currentWord) >> bitsAfter;
					consumedBitsInCurrentWord += bitsToRead;
					break;
				}
				else {
					auto temp = (allOnes >> consumedBitsInCurrentWord)& currentWord;
					bitsToRead -= 32 - consumedBitsInCurrentWord;
					lsbs += temp << bitsToRead;
					consumedBitsInCurrentWord = 32;
				}
			}

			msbsList[ix] = msbs;
			lsbsList[ix] = lsbs;

			auto unsignedVal = (msbs << parameter) | lsbs;

			output[ix + baseIndex] = (std::int32_t) (unsignedVal >> 1 ^ -(std::int32_t) (unsignedVal & 1));


		}
		mergeMsbsAndLsbs(&output[baseIndex], &msbsList[0], &lsbsList[0], parameter, nvals);

	}

	// restore state
	std::int32_t wastedBytesInWord = 0;
	if (consumedBitsInCurrentWord == 0) {
		//fail
	}
	else if (consumedBitsInCurrentWord <= 8) {
		_currentByte = static_cast<std::uint8_t>(currentWord >> 24);
		_consumedBitsInCurrentByte = consumedBitsInCurrentWord;
		wastedBytesInWord = 3;
	}
	else if (consumedBitsInCurrentWord <= 16) {
		_currentByte = static_cast<std::uint8_t>((currentWord >> 16) & 0xFF);
		_consumedBitsInCurrentByte = consumedBitsInCurrentWord - 8;
		wastedBytesInWord = 2;
	}
	else if (consumedBitsInCurrentWord <= 24) {
		_currentByte = static_cast<std::uint8_t>((currentWord >> 8) & 0xFF);
		_consumedBitsInCurrentByte = consumedBitsInCurrentWord - 16;
		wastedBytesInWord = 1;
	}
	else {
		_currentByte = static_cast<std::uint8_t>(currentWord & 0xFF);
		_consumedBitsInCurrentByte = consumedBitsInCurrentWord - 24;
		wastedBytesInWord = 0;
	}
	auto bytesToRewind = loadedBytes - 4 - currentWordIndex*sizeof(std::uint32_t) + wastedBytesInWord;
	_innerStream->seek(_innerStream->position() - bytesToRewind);
}



