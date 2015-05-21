#include "pch.h"
#include <string>
#include "includes\flacPP\FlacDecoder.h"
#include "includes\flacPP\FlacStream.h"
#include "includes\flacPP\FlacBitStream.h"
#include <vector>
#include <sstream>
#include "StreamHelpers.h"
#include "FlacModel.h"
#include "rice.h"
#include <algorithm> 
#include <mutex>

using namespace FlacPP;

class residualCache {
private:
	std::vector<std::vector<std::int32_t>> _data;
	std::mutex _mut;
public:
	std::vector<std::int32_t> getOne() {
		std::lock_guard<std::mutex> lg(_mut);
		std::vector<std::int32_t> result;
		if (_data.size() > 0) {
			result = std::move(_data[_data.size() - 1]);
			
			_data.resize(_data.size() - 1);
		}
		return std::move(result);
	}
	void release(std::vector<std::int32_t>&& data) {
		data.resize(0);
		std::lock_guard<std::mutex> lg(_mut);
		_data.push_back(std::move(data));
	}
};
residualCache g_residualCache;

void ConsumeFlacMagicWord(IFlacStream* stream) {
	std::uint32_t first32Bits;
	readFromStream(stream, first32Bits);
	if (first32Bits != 'CaLf') {
		throw FlacMagicWordNotFoundException();
	}
}

enum class subframe_type {
	constant,
	verbatim,
	fixed,
	lpc,
	invalid
};






void restoreLpcSignal(const std::vector<std::int32_t>& residual, const std::vector<std::int32_t>& qlpCoeffs, std::int32_t lp_quantization, std::uint32_t order, std::uint8_t bps, std::int32_t* data, std::uint16_t channelCount) {
	unsigned  j;



	for (auto r : residual) {
		std::int64_t sum = 0;
		std::int32_t * history = data;
		for (j = 0; j < order; j++) {
			history -= channelCount;
			sum += (int64_t)qlpCoeffs[j] * (int64_t)(*(history));
		}
		*data = r + (std::int32_t)(sum >> lp_quantization);
		data += channelCount;
	}
}


void restoreFixedSignal(const std::vector<std::int32_t>& residual, std::uint32_t order, std::int32_t* output, std::uint16_t channelCount)
{
	switch (order) {
	case 0:
		for (auto&& val : residual) {
			*(output) = val;
			output += channelCount;
		}
		break;
	case 1:
		for (auto ix = 0u;ix < residual.size();++ix) {
			output[ix*channelCount] = residual[ix] + output[channelCount* (ix - 1)];
		}
		break;
	case 2:
		for (auto ix = 0u;ix < residual.size();++ix) {
			output[ix*channelCount] = residual[ix] + (output[channelCount* (ix - 1)] << 1) - output[channelCount* (ix - 2)];
		}
		break;

	case 3:
		for (auto ix = 0u;ix < residual.size();++ix) {
			output[ix*channelCount] = residual[ix] + (((output[channelCount*(ix - 1)] - output[channelCount*(ix - 2)]) << 1) + (output[channelCount*(ix - 1)] - output[channelCount*(ix - 2)])) + output[channelCount*(ix - 3)];
		}
		break;
	case 4:
		for (auto ix = 0u;ix < residual.size();++ix) {
			output[ix*channelCount] = residual[ix] + 4 * output[channelCount*(ix - 1)] - 6 * output[channelCount*(ix - 2)] + 4 * output[channelCount*(ix - 3)] - output[channelCount*(ix - 4)];
		}
		break;
	default:
		throw FlacDecodingException();

	}
}

void readConstantSubFrameContent(std::uint8_t wastedBits, const std::uint16_t channelIndex, FlacBitStream* stream, std::int32_t* outputBuffer, std::uint8_t usefulBps, const std::uint32_t blockSize, const std::uint16_t channelCount, bool validateOnly) {

	auto constant = stream->readPartialInt32(usefulBps);
	if (!validateOnly) {
		for (auto ix = 0u;ix < blockSize*channelCount;ix+=channelCount) {
			outputBuffer[ix + channelIndex] = constant;
		}
	}
}
template<std::uint8_t usefulBps>
void readVerbatimSubFrameContent(std::uint8_t wastedBits, std::uint16_t channelIndex, FlacBitStream* stream, std::int32_t* outputBuffer, std::uint32_t blockSize, std::uint16_t channelCount, bool validateOnly) {
	if (!validateOnly) {
		for (auto ix = 0u;ix < blockSize;++ix) {
			outputBuffer[ix*channelCount + channelIndex] = stream->readPartialInt32<usefulBps>();
		}
	}
	else {
		for (auto ix = 0u;ix < blockSize;++ix) {
			stream->readPartialInt32(usefulBps);
		}
	}
}
void readVerbatimSubFrameContent(std::uint8_t wastedBits, std::uint16_t channelIndex, FlacBitStream* stream, std::int32_t* outputBuffer, std::uint8_t usefulBps, std::uint32_t blockSize, std::uint16_t channelCount, bool validateOnly) {
	switch (usefulBps) {
	case 1:
		readVerbatimSubFrameContent<1>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 2:
		readVerbatimSubFrameContent<2>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 3:
		readVerbatimSubFrameContent<3>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 4:
		readVerbatimSubFrameContent<4>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 5:
		readVerbatimSubFrameContent<5>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 6:
		readVerbatimSubFrameContent<6>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 7:
		readVerbatimSubFrameContent<7>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 8:
		readVerbatimSubFrameContent<8>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 9:
		readVerbatimSubFrameContent<9>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 10:
		readVerbatimSubFrameContent<10>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 11:
		readVerbatimSubFrameContent<11>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 12:
		readVerbatimSubFrameContent<12>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 13:
		readVerbatimSubFrameContent<13>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 14:
		readVerbatimSubFrameContent<14>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 15:
		readVerbatimSubFrameContent<15>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 16:
		readVerbatimSubFrameContent<16>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 17:
		readVerbatimSubFrameContent<17>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 18:
		readVerbatimSubFrameContent<18>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 19:
		readVerbatimSubFrameContent<19>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 20:
		readVerbatimSubFrameContent<20>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 21:
		readVerbatimSubFrameContent<21>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 22:
		readVerbatimSubFrameContent<22>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 23:
		readVerbatimSubFrameContent<23>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 24:
		readVerbatimSubFrameContent<24>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 25:
		readVerbatimSubFrameContent<25>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 26:
		readVerbatimSubFrameContent<26>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 27:
		readVerbatimSubFrameContent<27>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 28:
		readVerbatimSubFrameContent<28>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 29:
		readVerbatimSubFrameContent<29>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 30:
		readVerbatimSubFrameContent<30>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 31:
		readVerbatimSubFrameContent<31>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;
	case 32:
		readVerbatimSubFrameContent<32>(wastedBits, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		break;

	}
}
template<std::uint8_t usefulBps>
bool readFixedSubFrameContent(std::uint8_t wastedBits, std::uint32_t order, std::uint16_t channelIndex, FlacBitStream* stream, std::int32_t* outputBuffer, std::uint32_t blockSize, std::uint16_t channelCount, bool validateOnly) {


	if (!validateOnly) {
		for (auto ix = 0u; ix < order;++ix) {
			auto val = stream->readPartialInt32<usefulBps>();

			outputBuffer[ix*channelCount + channelIndex] = val;
		}
	}
	else {

		for (auto ix = 0u; ix < order;++ix) {
			auto val = stream->readPartialInt32<usefulBps>();
		}
	}



	auto residualCodingType = stream->readPartialUint32(2);
	std::vector<std::int32_t> residual = g_residualCache.getOne();
	if (residualCodingType == 0) {
		// partitioned rice with 4-bits parameter
		if (!extractResidualPartitionedRice4bits(residual, wastedBits, order, stream, usefulBps, blockSize)) {
			if (validateOnly) {
				g_residualCache.release(std::move(residual));
				return false;
			}
			throw FlacDecodingException();
		}
	}
	else if (residualCodingType == 1) {
		// partitioned rice with 5-bits parameter
		if (!extractResidualPartitionedRice5bits(residual, wastedBits, order, stream, usefulBps, blockSize)) {
			if (validateOnly) {
				g_residualCache.release(std::move(residual));
				return false;
			}
			throw FlacDecodingException();
		}
	}
	else {
		if (validateOnly) {
			g_residualCache.release(std::move(residual));
			return false;
		}
		throw FlacDecodingException();
	}
	if (!validateOnly) {
		restoreFixedSignal(residual, order, outputBuffer + (order*channelCount) + channelIndex, channelCount);
	}
	g_residualCache.release(std::move(residual));
	return true;
}
bool readFixedSubFrameContent(std::uint8_t wastedBits, std::uint32_t order, std::uint16_t channelIndex, FlacBitStream* stream, std::int32_t* outputBuffer, std::uint8_t usefulBps, std::uint32_t blockSize, std::uint16_t channelCount, bool validateOnly) {
	
	switch (usefulBps) {
	case 1:
		return readFixedSubFrameContent<1>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 2:
		return readFixedSubFrameContent<2>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 3:
		return readFixedSubFrameContent<3>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 4:
		return readFixedSubFrameContent<4>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 5:
		return readFixedSubFrameContent<5>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 6:
		return readFixedSubFrameContent<6>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 7:
		return readFixedSubFrameContent<7>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 8:
		return readFixedSubFrameContent<8>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 9:
		return readFixedSubFrameContent<9>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 10:
		return readFixedSubFrameContent<10>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 11:
		return readFixedSubFrameContent<11>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 12:
		return readFixedSubFrameContent<12>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 13:
		return readFixedSubFrameContent<13>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 14:
		return readFixedSubFrameContent<14>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 15:
		return readFixedSubFrameContent<15>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 16:
		return readFixedSubFrameContent<16>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 17:
		return readFixedSubFrameContent<17>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 18:
		return readFixedSubFrameContent<18>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 19:
		return readFixedSubFrameContent<19>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 20:
		return readFixedSubFrameContent<20>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 21:
		return readFixedSubFrameContent<21>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 22:
		return readFixedSubFrameContent<22>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 23:
		return readFixedSubFrameContent<23>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 24:
		return readFixedSubFrameContent<24>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 25:
		return readFixedSubFrameContent<25>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 26:
		return readFixedSubFrameContent<26>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 27:
		return readFixedSubFrameContent<27>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 28:
		return readFixedSubFrameContent<28>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 29:
		return readFixedSubFrameContent<29>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 30:
		return readFixedSubFrameContent<30>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
		
	case 31:
		return readFixedSubFrameContent<31>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);
	case 32:
		return readFixedSubFrameContent<32>(wastedBits, order, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	default:
		return false;

	}
}

template<std::uint8_t usefulBps>
bool readLpcSubFrameContent(std::uint8_t wastedBits, std::uint32_t predictorOrder, std::uint16_t channelIndex, FlacBitStream* stream, std::int32_t* outputBuffer, std::uint32_t blockSize, std::uint16_t channelCount, bool validateOnly) {


	if (!validateOnly) {
		for (auto ix = 0u; ix < predictorOrder;++ix) {
			auto val = stream->readPartialInt32<usefulBps>();

			outputBuffer[ix*channelCount + channelIndex] = val;
		}
	}
	else {

		for (auto ix = 0u; ix < predictorOrder;++ix) {
			auto val = stream->readPartialInt32<usefulBps>();
		}
	}



	auto quantizedLinearPredictorCoeffPrecision = stream->readPartialUint32<4>();
	if (quantizedLinearPredictorCoeffPrecision == 0xF) {
		if (validateOnly) {
			return false;
		}
		throw FlacDecodingException();
	}
	quantizedLinearPredictorCoeffPrecision += 1;
	auto quantizedLinearPredictorCoeffShift = stream->readPartialInt32<5>();
	std::vector<int32_t> qlpCoeffs;
	for (auto ix = 0u;ix < predictorOrder;++ix) {
		auto val = stream->readPartialInt32(quantizedLinearPredictorCoeffPrecision);
		qlpCoeffs.push_back(val);
	}
	auto residualCodingType = stream->readPartialUint32<2>();

	std::vector<std::int32_t> residual = g_residualCache.getOne();
	if (residualCodingType == 0) {
		// partitioned rice with 4-bits parameter
		if (!extractResidualPartitionedRice4bits(residual, wastedBits, predictorOrder, stream, usefulBps, blockSize)) {
			if (validateOnly) {
				g_residualCache.release(std::move(residual));
				return false;
			}
			throw FlacDecodingException();
		}
	}
	else if (residualCodingType == 1) {
		// partitioned rice with 5-bits parameter
		if (!extractResidualPartitionedRice5bits(residual, wastedBits, predictorOrder, stream, usefulBps, blockSize)) {
			if (validateOnly) {
				g_residualCache.release(std::move(residual));
				return false;
			}
			throw FlacDecodingException();
		}
	}
	else {
		if (validateOnly) {
			g_residualCache.release(std::move(residual));
			return false;
		}
		throw FlacDecodingException();
	}
	if (!validateOnly) {
		restoreLpcSignal(residual, qlpCoeffs, quantizedLinearPredictorCoeffShift, predictorOrder, usefulBps, outputBuffer + (predictorOrder*channelCount) + channelIndex, channelCount);
	}
	g_residualCache.release(std::move(residual));
	return true;
}

bool readLpcSubFrameContent(std::uint8_t wastedBits, std::uint32_t predictorOrder, std::uint16_t channelIndex, FlacBitStream* stream, std::int32_t* outputBuffer, std::uint8_t usefulBps, std::uint32_t blockSize, std::uint16_t channelCount, bool validateOnly) {
	switch (usefulBps) {
	case 1:
		return readLpcSubFrameContent<1>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 2:
		return readLpcSubFrameContent<2>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 3:
		return readLpcSubFrameContent<3>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 4:
		return readLpcSubFrameContent<4>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 5:
		return readLpcSubFrameContent<5>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 6:
		return readLpcSubFrameContent<6>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 7:
		return readLpcSubFrameContent<7>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 8:
		return readLpcSubFrameContent<8>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 9:
		return readLpcSubFrameContent<9>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 10:
		return readLpcSubFrameContent<10>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 11:
		return readLpcSubFrameContent<11>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 12:
		return readLpcSubFrameContent<12>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 13:
		return readLpcSubFrameContent<13>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 14:
		return readLpcSubFrameContent<14>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 15:
		return readLpcSubFrameContent<15>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 16:
		return readLpcSubFrameContent<16>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 17:
		return readLpcSubFrameContent<17>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 18:
		return readLpcSubFrameContent<18>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 19:
		return readLpcSubFrameContent<19>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 20:
		return readLpcSubFrameContent<20>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 21:
		return readLpcSubFrameContent<21>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 22:
		return readLpcSubFrameContent<22>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 23:
		return readLpcSubFrameContent<23>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 24:
		return readLpcSubFrameContent<24>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 25:
		return readLpcSubFrameContent<25>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 26:
		return readLpcSubFrameContent<26>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 27:
		return readLpcSubFrameContent<27>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 28:
		return readLpcSubFrameContent<28>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 29:
		return readLpcSubFrameContent<29>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 30:
		return readLpcSubFrameContent<30>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 31:
		return readLpcSubFrameContent<31>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	case 32:
		return readLpcSubFrameContent<32>(wastedBits, predictorOrder, channelIndex, stream, outputBuffer, blockSize, channelCount, validateOnly);

	default:
		return false;
	}
}

bool readSubframe(std::uint16_t channelIndex, FlacBitStream* stream, std::int32_t* outputBuffer, const frame_header& frameHeader, bool validateOnly = false) {
	auto headerPadding = stream->readPartialUint32<1>();
	auto subFrameTypeRaw = stream->readPartialUint32<6>();
	auto wastedBits = stream->readPartialUint32<1>();
	auto subframeType = subframe_type::invalid;
	if (headerPadding != 0) {
		if (validateOnly) {
			return false;
		}
		throw FlacDecodingException();
	}

	// adjust bps for special channel assignments

	
	std::uint32_t order = 0;
	
	if (wastedBits == 1) {
		wastedBits = stream->readUnaryUnsigned() + 1;
	}
	auto bps = frameHeader.sampleSizeInBits - wastedBits;
	switch (frameHeader.channelAssignment) {
	case channel_assignment::independent:
		/* no adjustment needed */
		break;
	case channel_assignment::left_side_stereo:
		if (channelIndex == 1)
			bps++;
		break;
	case channel_assignment::side_right_stereo:
		if (channelIndex == 0)
			bps++;
		break;
	case channel_assignment::mid_side_stereo:
		if (channelIndex == 1)
			bps++;
		break;
	}
	if (subFrameTypeRaw >= 0x20) {
		subframeType = subframe_type::lpc;
		order = (subFrameTypeRaw - 0x20) + 1;
		if (!readLpcSubFrameContent(wastedBits, order, channelIndex, stream, outputBuffer, bps, frameHeader.blockSize, frameHeader.channelCount, validateOnly)) {
			if (validateOnly) {
				return false;
			}
			throw FlacDecodingException();

		};
	}
	else if (subFrameTypeRaw >= 0x10) {
		subframeType = subframe_type::invalid;
		if (validateOnly) {
			return false;
		}
		throw FlacDecodingException();
	}
	else if (subFrameTypeRaw >= 0x8) {
		subframeType = subframe_type::fixed;
		order = (subFrameTypeRaw - 0x8);
		if (order > 4) {
			if (validateOnly) {
				return false;
			}
			throw FlacDecodingException();
		}
		if (!readFixedSubFrameContent(wastedBits, order, channelIndex, stream, outputBuffer, bps, frameHeader.blockSize, frameHeader.channelCount, validateOnly)) {
			if (validateOnly) {
				return false;
			}
			throw FlacDecodingException();
		}
	}
	else if (subFrameTypeRaw >= 0x2) {
		subframeType = subframe_type::invalid;
		if (validateOnly) {
			return false;
		}
		throw FlacDecodingException();
	}
	else if (subFrameTypeRaw == 0x1) {
		subframeType = subframe_type::verbatim;
		readVerbatimSubFrameContent(wastedBits, channelIndex, stream, outputBuffer, bps, frameHeader.blockSize, frameHeader.channelCount,validateOnly);
	}
	else {
		subframeType = subframe_type::constant;
		readConstantSubFrameContent(wastedBits, channelIndex, stream, outputBuffer, bps, frameHeader.blockSize, frameHeader.channelCount,validateOnly);
	}

	if (wastedBits != 0 && !validateOnly) {
		auto base = outputBuffer + channelIndex;
		for (auto ix = 0u; ix < frameHeader.blockSize*frameHeader.channelCount; ix+= frameHeader.channelCount) {
			auto val = outputBuffer[ix];
			outputBuffer[ix] = val << wastedBits;
		}
	}

	return true;
}



FlacPP::FlacDecoder::FlacDecoder(std::unique_ptr<IFlacStream>&& stream)
	: _stream(std::move(stream)), _nextSample(0)
{
	ConsumeFlacMagicWord(_stream.get());
	for (;;) {
		metadata_block_header header;

		readFromStream(_stream.get(), header);

		auto metadataSize = header.blockSizeInBytes();
		auto metadataType = header.blockType();
		if (metadataType == metadata_block_type::streamInfo) {
			FlacBitStream bitStream(_stream.get());


			_streamInfo.minBlockSizeInSamples = (uint16_t)bitStream.readPartialUint32<16>();
			_streamInfo.maxBlockSizeInSamples = (uint16_t)bitStream.readPartialUint32<16>();
			_streamInfo.minFrameSizeInBytes = bitStream.readPartialUint32<24>();
			_streamInfo.maxFrameSizeInBytes = bitStream.readPartialUint32<24>();
			_streamInfo.sampleRate = bitStream.readPartialUint32<20>();
			_streamInfo.channels = (uint16_t)(1 + bitStream.readPartialUint32<3>());
			_streamInfo.bitsPerSample = (uint16_t)(1 + bitStream.readPartialUint32<5>());
			std::uint64_t totalSamples = bitStream.readPartialUint32<4>();
			totalSamples <<= 32;
			totalSamples |= bitStream.readPartialUint32<32>();
			_streamInfo.totalSamples = totalSamples;
			bitStream.readPartialUint32<32>();
			bitStream.readPartialUint32<32>();
			bitStream.readPartialUint32<32>();
			bitStream.readPartialUint32<32>();
		}
		else if (metadataType == metadata_block_type::seekTable) {
			auto itemCount = metadataSize / 18;
			for (auto ix = 0u;ix < itemCount;++ix) {
				seek_point_raw seekPoint;
				readFromStream(_stream.get(), seekPoint);
				if (seekPoint.firstSampleIndex() != 0xFFFFFFFFFFFFFFFF) {
					seekpoint sp;
					sp.firstSampleIndex = seekPoint.firstSampleIndex();
					sp.bytesOffsetFromFirstFrameHeader = seekPoint.bytesOffsetFromFirstFrameHeader();
					sp.sampleCount = seekPoint.sampleCount();
					_seekpoints.push_back(sp);
				}
			}
		}
		else {
			_stream->seek(_stream->position() + metadataSize);
		}
		auto isLast = header.isLast();
		if (isLast) {
			break;
		}
	}
	_posOfFirstFrame = _stream->position();
	_inputBuffer.reset(new std::uint8_t[this->_streamInfo.maxFrameSizeInBytes]);
	_outputBuffer.reset(new std::int32_t[this->_streamInfo.maxBlockSizeInSamples*_streamInfo.channels]);
}


time_unit_100ns FlacPP::FlacDecoder::seek(const time_unit_100ns& ts) {
	auto sample = static_cast<std::uint64_t>(ts.count())*_streamInfo.sampleRate / 10000000;
	

	return time_unit_100ns(seekSample(sample) * 10000000 / _streamInfo.sampleRate);
}


bool peekCurrentFrameHeader(FlacPP::IFlacStream* stream, frame_header& header, std::uint64_t& headerSizeWithoutCRC, const stream_info& streamInfo) {
	if (stream->position() == stream->size()) {
		return false;
	}
	auto posToRestore = stream->position();
	frame_header_fixed_part_raw fixedPartOfHeader;
	readFromStream(stream, fixedPartOfHeader);


	header.channelAssignment = fixedPartOfHeader.channelAssignment();
	header.blockSize = fixedPartOfHeader.directBlockSize();
	auto blockSizeStrategy = fixedPartOfHeader.getBlockSizeStrategy();
	if (header.channelAssignment == channel_assignment::independent) {
		header.channelCount = fixedPartOfHeader.independentChannelsCount();
	}
	else {
		header.channelCount = 2;
	}
	auto blockingStrategy = fixedPartOfHeader.blockingStrategy();
	header.sampleRate = fixedPartOfHeader.sampleRate();
	auto sampleRateStrategy = fixedPartOfHeader.sampleRateStrategy();
	header.sampleSizeInBits = fixedPartOfHeader.sampleSizeInBits();
	auto sampleSizeStrategy = fixedPartOfHeader.sampleSizeStrategy();

	if (sampleRateStrategy == samplerate_strategy::from_streaminfo) {
		header.sampleRate = streamInfo.sampleRate;
	}
	if (sampleSizeStrategy == samplesize_strategy::from_streaminfo) {
		header.sampleSizeInBits = streamInfo.bitsPerSample;
	}
	auto frameOrSampleIndex = read_utf8_uint64(stream, 64 / 8);
	if (blockingStrategy == blocking_strategy::encode_sample_number) {
		header.firstSampleIndex = frameOrSampleIndex;
	}
	else {
		header.firstSampleIndex = frameOrSampleIndex * streamInfo.minBlockSizeInSamples;
	}


	if (blockSizeStrategy == blocksize_strategy::grab8bits) {
		std::uint8_t temp;
		readFromStream(stream, temp);
		header.blockSize = temp + 1;
	}
	else if (blockSizeStrategy == blocksize_strategy::grab16bits) {
		std::uint16_t temp;
		readFromStream(stream, temp);
		header.blockSize = _byteswap_ushort(temp) + 1;
	}

	if (sampleRateStrategy == samplerate_strategy::grab8BitsKHz) {
		std::uint8_t temp;
		readFromStream(stream, temp);
		header.sampleRate = static_cast<std::uint32_t>(temp) * 1000;
	}
	else if (sampleRateStrategy == samplerate_strategy::grab16BitsHz) {
		std::uint16_t temp;
		readFromStream(stream, temp);
		header.sampleRate = static_cast<std::uint32_t>(_byteswap_ushort(temp));
	}
	else if (sampleRateStrategy == samplerate_strategy::grab16Bits10Hz) {

		std::uint16_t temp;
		readFromStream(stream, temp);
		header.sampleRate = static_cast<std::uint32_t>(_byteswap_ushort(temp)) * 10;
	}

	std::uint8_t frameHeaderCRC;
	readFromStream(stream, frameHeaderCRC);

	headerSizeWithoutCRC = stream->position() - posToRestore - 1;
	stream->seek(posToRestore);
	return true;
}


std::uint8_t const crc8_table[256] = {
	0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
	0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
	0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
	0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
	0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
	0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
	0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
	0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
	0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
	0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
	0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
	0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
	0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
	0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
	0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
	0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
	0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
	0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
	0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
	0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
	0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
	0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
	0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
	0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
	0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
	0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
	0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
	0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
	0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
	0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
	0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
	0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};


std::uint16_t const crc16_table[256] = {
	0x0000,  0x8005,  0x800f,  0x000a,  0x801b,  0x001e,  0x0014,  0x8011,
	0x8033,  0x0036,  0x003c,  0x8039,  0x0028,  0x802d,  0x8027,  0x0022,
	0x8063,  0x0066,  0x006c,  0x8069,  0x0078,  0x807d,  0x8077,  0x0072,
	0x0050,  0x8055,  0x805f,  0x005a,  0x804b,  0x004e,  0x0044,  0x8041,
	0x80c3,  0x00c6,  0x00cc,  0x80c9,  0x00d8,  0x80dd,  0x80d7,  0x00d2,
	0x00f0,  0x80f5,  0x80ff,  0x00fa,  0x80eb,  0x00ee,  0x00e4,  0x80e1,
	0x00a0,  0x80a5,  0x80af,  0x00aa,  0x80bb,  0x00be,  0x00b4,  0x80b1,
	0x8093,  0x0096,  0x009c,  0x8099,  0x0088,  0x808d,  0x8087,  0x0082,
	0x8183,  0x0186,  0x018c,  0x8189,  0x0198,  0x819d,  0x8197,  0x0192,
	0x01b0,  0x81b5,  0x81bf,  0x01ba,  0x81ab,  0x01ae,  0x01a4,  0x81a1,
	0x01e0,  0x81e5,  0x81ef,  0x01ea,  0x81fb,  0x01fe,  0x01f4,  0x81f1,
	0x81d3,  0x01d6,  0x01dc,  0x81d9,  0x01c8,  0x81cd,  0x81c7,  0x01c2,
	0x0140,  0x8145,  0x814f,  0x014a,  0x815b,  0x015e,  0x0154,  0x8151,
	0x8173,  0x0176,  0x017c,  0x8179,  0x0168,  0x816d,  0x8167,  0x0162,
	0x8123,  0x0126,  0x012c,  0x8129,  0x0138,  0x813d,  0x8137,  0x0132,
	0x0110,  0x8115,  0x811f,  0x011a,  0x810b,  0x010e,  0x0104,  0x8101,
	0x8303,  0x0306,  0x030c,  0x8309,  0x0318,  0x831d,  0x8317,  0x0312,
	0x0330,  0x8335,  0x833f,  0x033a,  0x832b,  0x032e,  0x0324,  0x8321,
	0x0360,  0x8365,  0x836f,  0x036a,  0x837b,  0x037e,  0x0374,  0x8371,
	0x8353,  0x0356,  0x035c,  0x8359,  0x0348,  0x834d,  0x8347,  0x0342,
	0x03c0,  0x83c5,  0x83cf,  0x03ca,  0x83db,  0x03de,  0x03d4,  0x83d1,
	0x83f3,  0x03f6,  0x03fc,  0x83f9,  0x03e8,  0x83ed,  0x83e7,  0x03e2,
	0x83a3,  0x03a6,  0x03ac,  0x83a9,  0x03b8,  0x83bd,  0x83b7,  0x03b2,
	0x0390,  0x8395,  0x839f,  0x039a,  0x838b,  0x038e,  0x0384,  0x8381,
	0x0280,  0x8285,  0x828f,  0x028a,  0x829b,  0x029e,  0x0294,  0x8291,
	0x82b3,  0x02b6,  0x02bc,  0x82b9,  0x02a8,  0x82ad,  0x82a7,  0x02a2,
	0x82e3,  0x02e6,  0x02ec,  0x82e9,  0x02f8,  0x82fd,  0x82f7,  0x02f2,
	0x02d0,  0x82d5,  0x82df,  0x02da,  0x82cb,  0x02ce,  0x02c4,  0x82c1,
	0x8243,  0x0246,  0x024c,  0x8249,  0x0258,  0x825d,  0x8257,  0x0252,
	0x0270,  0x8275,  0x827f,  0x027a,  0x826b,  0x026e,  0x0264,  0x8261,
	0x0220,  0x8225,  0x822f,  0x022a,  0x823b,  0x023e,  0x0234,  0x8231,
	0x8213,  0x0216,  0x021c,  0x8219,  0x0208,  0x820d,  0x8207,  0x0202
};

bool findNextFrameHeader(FlacPP::IFlacStream* stream, frame_header& header, const stream_info& streamInfo, std::uint64_t sampleToFind) {
	auto headPos = stream->position();
	for (;;) {
		if (headPos + 2 >= stream->size()) {
			return false;
		}
		std::uint8_t headHigh;
		std::uint8_t headLow;
		readFromStream(stream, headHigh);
		readFromStream(stream, headLow);
		if (headHigh != 0xff || (headLow >> 1) != 0x7c) {
			// sync code not found
			stream->seek(stream->position() - 1);
			headPos = stream->position();
			continue;
		}

		

		// sync code found.
		// peek header
		stream->seek(stream->position() - 2);

		headPos = stream->position();
		std::uint64_t headerSizeWithoutCRC;
		peekCurrentFrameHeader(stream, header, headerSizeWithoutCRC, streamInfo);
		if (header.firstSampleIndex >= streamInfo.totalSamples) {

			headPos += 1;
			stream->seek(headPos);
			continue;
		}
		std::uint8_t b;
		std::uint8_t computedCrc = 0;
		for (auto ix = 0ull;ix < headerSizeWithoutCRC;++ix) {
			readFromStream(stream, b);
			computedCrc = crc8_table[computedCrc ^ b];
		}
		readFromStream(stream, b);
		if (b == computedCrc) {
			//crc checked ! 
			// check subFramesCrc
			
			bool allValid = true;
			{
				FlacBitStream fbs(stream);
				for (auto ix = 0u; ix < header.channelCount;++ix) {
					if (!readSubframe(ix, &fbs, nullptr, header, true)) {
						allValid = false;
						break;
					}
				}
			}
			if (allValid) {
				// validate crc
				std::uint16_t readCrc;
				auto sizeOfCrcedData = stream->position() - headPos;
				readFromStream(stream, readCrc);
				readCrc = _byteswap_ushort(readCrc);
				
				stream->seek(headPos);
				std::uint16_t computedContentCrc = 0;
				for (auto ix = 0ull;ix < sizeOfCrcedData;++ix) {
					readFromStream(stream, b);
					computedContentCrc = ((computedContentCrc << 8) ^ crc16_table[(computedContentCrc >> 8) ^ b]) & 0xffff;
				}
				stream->seek(headPos);
				if (computedContentCrc == readCrc) {
					if (header.firstSampleIndex + header.blockSize > sampleToFind) {
						return true;
					}
					else {
						headPos += streamInfo.minFrameSizeInBytes;
						stream->seek(headPos);
						continue;
					}
				}
			}
			
		}
		headPos += 1;
		stream->seek(headPos);


	}
}



FlacPP::FlacBufferView FlacPP::FlacDecoder::decodeNextFrame(time_unit_100ns & frameTime)
{
	if (_stream->position() == _stream->size()) {
		return FlacBufferView(nullptr, 0, 0);
	}
	frame_header_fixed_part_raw fixedPartOfHeader;
	readFromStream(_stream.get(), fixedPartOfHeader);

	frame_header header;

	header.channelAssignment = fixedPartOfHeader.channelAssignment();
	header.blockSize = fixedPartOfHeader.directBlockSize();
	auto blockSizeStrategy = fixedPartOfHeader.getBlockSizeStrategy();
	if (header.channelAssignment == channel_assignment::independent) {
		header.channelCount = fixedPartOfHeader.independentChannelsCount();
	}
	else {
		header.channelCount = 2;
	}
	auto blockingStrategy = fixedPartOfHeader.blockingStrategy();
	header.sampleRate = fixedPartOfHeader.sampleRate();
	auto sampleRateStrategy = fixedPartOfHeader.sampleRateStrategy();
	header.sampleSizeInBits = fixedPartOfHeader.sampleSizeInBits();
	auto sampleSizeStrategy = fixedPartOfHeader.sampleSizeStrategy();

	if (sampleRateStrategy == samplerate_strategy::from_streaminfo) {
		header.sampleRate = this->_streamInfo.sampleRate;
	}
	if (sampleSizeStrategy == samplesize_strategy::from_streaminfo) {
		header.sampleSizeInBits = this->_streamInfo.bitsPerSample;
	}
	auto frameOrSampleIndex = read_utf8_uint64(_stream.get(), 64/8);
	if (blockingStrategy == blocking_strategy::encode_sample_number) {
		header.firstSampleIndex = frameOrSampleIndex;
	}
	else {
		header.firstSampleIndex = frameOrSampleIndex * this->_streamInfo.minBlockSizeInSamples;
	}


	if (blockSizeStrategy == blocksize_strategy::grab8bits) {
		std::uint8_t temp;
		readFromStream(_stream.get(), temp);
		header.blockSize = temp + 1;
	}
	else if (blockSizeStrategy == blocksize_strategy::grab16bits) {
		std::uint16_t temp;
		readFromStream(_stream.get(), temp);
		header.blockSize = _byteswap_ushort(temp) + 1;
	}

	if (sampleRateStrategy == samplerate_strategy::grab8BitsKHz) {
		std::uint8_t temp;
		readFromStream(_stream.get(), temp);
		header.sampleRate = static_cast<std::uint32_t>(temp) * 1000;
	}
	else if (sampleRateStrategy == samplerate_strategy::grab16BitsHz) {
		std::uint16_t temp;
		readFromStream(_stream.get(), temp);
		header.sampleRate = static_cast<std::uint32_t>(_byteswap_ushort(temp));
	}
	else if (sampleRateStrategy == samplerate_strategy::grab16Bits10Hz) {

		std::uint16_t temp;
		readFromStream(_stream.get(), temp);
		header.sampleRate = static_cast<std::uint32_t>(_byteswap_ushort(temp)) * 10;
	}

	std::uint8_t frameHeaderCRC;
	readFromStream(_stream.get(), frameHeaderCRC);

	frameTime = time_unit_100ns(header.firstSampleIndex * 10000000 / header.sampleRate);

	{
		FlacBitStream fbs(this->_stream.get());
		for (std::uint16_t ix = 0;ix < header.channelCount;++ix) {
			readSubframe(ix, &fbs, this->_outputBuffer.get(), header);
		}
	}

	std::uint16_t frameContentCRC;
	readFromStream(_stream.get(), frameContentCRC);

	switch (header.channelAssignment) {
	case channel_assignment::independent:
		/* do nothing */
		break;
	case channel_assignment::left_side_stereo:
		for (auto i = 0u; i < header.blockSize; i++)
			_outputBuffer[i * 2 + 1] = _outputBuffer[i * 2] - _outputBuffer[i * 2 + 1];
		break;
	case channel_assignment::side_right_stereo:
		for (auto i = 0u; i < header.blockSize; i++)
			_outputBuffer[i * 2] += _outputBuffer[i * 2 + 1];
		break;
	case channel_assignment::mid_side_stereo:
		for (auto i = 0u; i < header.blockSize; i++) {

			auto mid = _outputBuffer[i * 2];
			auto side = _outputBuffer[i * 2 + 1];
			mid <<= 1;
			mid |= (side & 1); /* i.e. if 'side' is odd... */
			_outputBuffer[i * 2] = (mid + side) >> 1;
			_outputBuffer[i * 2 + 1] = (mid - side) >> 1;

		}
		break;
	}

	if (header.sampleSizeInBits < 32) {
		const auto toShift = 32 - header.sampleSizeInBits;
		for (auto i = 0u; i < header.blockSize*header.channelCount; i++) {
			auto val = _outputBuffer[i];
			_outputBuffer[i] = (val << toShift);
		}
	}
	_nextSample = header.firstSampleIndex + header.blockSize;
	return FlacBufferView(reinterpret_cast<std::uint8_t*>(_outputBuffer.get()), sizeof(std::int32_t)*header.channelCount*header.blockSize, sizeof(std::int32_t)*header.channelCount*header.blockSize);

}
#undef max
#undef min

std::uint64_t FlacPP::FlacDecoder::seekSample(std::uint64_t sample)
{
	frame_header header;
	if (sample >= _streamInfo.totalSamples) {
		sample = _streamInfo.totalSamples - 1;
	}
	std::uint64_t headerSizeWithoutCRC;
	if (!peekCurrentFrameHeader(_stream.get(), header, headerSizeWithoutCRC, _streamInfo)) {
		_stream->seek(_posOfFirstFrame);
		_nextSample = 0;
		peekCurrentFrameHeader(_stream.get(), header, headerSizeWithoutCRC, _streamInfo);
	}
	if (sample >= header.firstSampleIndex && sample < header.firstSampleIndex + header.blockSize) {
		return header.firstSampleIndex;
	}
	auto oldHeader = header;
	auto oldPos = _stream->position();
	// relativePos
	auto relativeSeekPos = static_cast<double>(sample) / static_cast<double>(_streamInfo.totalSamples);
	auto minFrameSearchPos = _posOfFirstFrame;
	auto maxFrameSearchPos = _stream->size()- _streamInfo.maxFrameSizeInBytes;
	
	auto startSeekOffset = static_cast<std::uint64_t>(std::floor(minFrameSearchPos + (maxFrameSearchPos - minFrameSearchPos)*relativeSeekPos));
	_stream->seek(startSeekOffset);
	for (;;) {
		
		
		if (_stream->position() < minFrameSearchPos) {
			_stream->seek(minFrameSearchPos);
		}
		if (_stream->position() > maxFrameSearchPos) {
			_stream->seek(maxFrameSearchPos);
		}
		
			if (!findNextFrameHeader(_stream.get(), header, _streamInfo, sample)) {
				_stream->seek(oldPos);
				return header.firstSampleIndex;
			}
			
					
		
		if (sample >= header.firstSampleIndex && sample < header.firstSampleIndex + header.blockSize) {
			return header.firstSampleIndex;
		}
		else if (sample >= header.firstSampleIndex + header.blockSize) {

			minFrameSearchPos = std::max(minFrameSearchPos, _stream->position() + _streamInfo.minFrameSizeInBytes);
			
			_stream->seek(minFrameSearchPos);
		}
		else if (sample < header.firstSampleIndex) {

			maxFrameSearchPos = std::min(maxFrameSearchPos, _stream->position() - 1);
			if (_stream->position() < _streamInfo.maxFrameSizeInBytes) {
				_stream->seek(0);
			}
			else {
				_stream->seek(_stream->position() - _streamInfo.maxFrameSizeInBytes);
			}
		}
	}
	
}

FlacPP::FlacDecoder::~FlacDecoder()
{
}
