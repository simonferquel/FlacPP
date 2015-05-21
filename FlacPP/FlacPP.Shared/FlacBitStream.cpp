#include "pch.h"
#include "includes\flacPP\FlacBitStream.h"
#include <vector>





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



