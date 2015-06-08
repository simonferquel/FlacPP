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





#if defined(_M_ARM)
#include <arm_neon.h>

template<std::uint32_t order>
void restoreLpcSignal(const std::vector<std::int32_t>& residual, std::vector<std::int32_t>& qlpCoeffs, std::int32_t lp_quantization, std::vector<std::int32_t>& output) {
	unsigned  j;
	std::reverse(qlpCoeffs.begin(), qlpCoeffs.end());
	for (auto ix = 0u; ix < residual.size();++ix) {
		std::int64_t res = 0;
		auto pairwiseCount = order / 2;
		for (j = 0; j < pairwiseCount; j++) {
			int32x2_t coeffs = vld1_s32((&qlpCoeffs[order - (j + 1) * 2]));
			int32x2_t hs = vld1_s32(&output[order + ix - (j + 1) * 2]);
			int64x2_t mul = vmull_s32(coeffs, hs);
			int64x1_t partialSum = vadd_s64(vget_high_s64(mul), vget_low_s64(mul));
			res += partialSum.n64_i64[0];
		}
		if (order % 2 == 1) {
			auto coeff = qlpCoeffs[0];
			auto h = output[ix];
			res += (std::int64_t)coeff*(std::int64_t)h;
		}
		
		output[order + ix] = residual[ix] + (std::int32_t)(res >> lp_quantization);
	}

}
void restoreLpcSignal(const std::vector<std::int32_t>& residual, std::vector<std::int32_t>& qlpCoeffs, std::int32_t lp_quantization, std::uint32_t order, std::vector<std::int32_t>& output) {
	/*switch (order)
	{
	case 0:
		restoreLpcSignal<0>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 1:
		restoreLpcSignal<1>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 2:
		restoreLpcSignal<2>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 3:
		restoreLpcSignal<3>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 4:
		restoreLpcSignal<4>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 5:
		restoreLpcSignal<5>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 6:
		restoreLpcSignal<6>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 7:
		restoreLpcSignal<7>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 8:
		restoreLpcSignal<8>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 9:
		restoreLpcSignal<9>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 10:
		restoreLpcSignal<10>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 11:
		restoreLpcSignal<11>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 12:
		restoreLpcSignal<12>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 13:
		restoreLpcSignal<13>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 14:
		restoreLpcSignal<14>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 15:
		restoreLpcSignal<15>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 16:
		restoreLpcSignal<16>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 17:
		restoreLpcSignal<17>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 18:
		restoreLpcSignal<18>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 19:
		restoreLpcSignal<19>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 20:
		restoreLpcSignal<20>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 21:
		restoreLpcSignal<21>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 22:
		restoreLpcSignal<22>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 23:
		restoreLpcSignal<23>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 24:
		restoreLpcSignal<24>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 25:
		restoreLpcSignal<25>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 26:
		restoreLpcSignal<26>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 27:
		restoreLpcSignal<27>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 28:
		restoreLpcSignal<28>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 29:
		restoreLpcSignal<29>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 30:
		restoreLpcSignal<30>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 31:
		restoreLpcSignal<31>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 32:
		restoreLpcSignal<32>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;

	default:
		break;
	}*/
unsigned  j;
std::reverse(qlpCoeffs.begin(), qlpCoeffs.end());
for (auto ix = 0u; ix < residual.size(); ++ix) {
	std::int64_t res = 0;
	auto pairwiseCount = order / 2;
	for (j = 0; j < pairwiseCount; j++) {
		int32x2_t coeffs = vld1_s32((&qlpCoeffs[order - (j + 1) * 2]));
		int32x2_t hs = vld1_s32(&output[order + ix - (j + 1) * 2]);
		int64x2_t mul = vmull_s32(coeffs, hs);
		int64x1_t partialSum = vadd_s64(vget_high_s64(mul), vget_low_s64(mul));
		res += partialSum.n64_i64[0];
	}
	if (order % 2 == 1) {
		auto coeff = qlpCoeffs[0];
		auto h = output[ix];
		res += (std::int64_t)coeff*(std::int64_t)h;
	}
	output[order + ix] = residual[ix] + (std::int32_t)(res >> lp_quantization);
}
}

template<std::uint32_t order>
void restoreLpcSignal_32bit(const std::vector<std::int32_t>& residual, std::vector<std::int32_t>& qlpCoeffs, std::int32_t lp_quantization, std::vector<std::int32_t>& output) {
	unsigned  j;
	std::reverse(qlpCoeffs.begin(), qlpCoeffs.end());
	for (auto ix = 0u; ix < residual.size();++ix) {
		int64x2_t sum = { 0,0 };
		auto pairwiseCount = order / 2;
		for (j = 0; j < pairwiseCount; j++) {
			int32x2_t coeffs = vld1_s32((&qlpCoeffs[order-(j+1)*2]));
			int32x2_t hs = vld1_s32(&output[order + ix - (j+1)*2]);
			sum = vmlal_s32(sum, coeffs, hs);			
		}
		int64x1_t sumAll = vadd_s64(vget_high_s64(sum), vget_low_s64(sum));
		std::int64_t res = sumAll.n64_i64[0];
		if (order % 2 == 1) {
			auto coeff = qlpCoeffs[0];
			auto h = output[ix];
			res += (std::int64_t)coeff*(std::int64_t)h;
		}
		output[order + ix] = residual[ix] + (std::int32_t)(res >> lp_quantization);
	}

}

void restoreLpcSignal_32bit(const std::vector<std::int32_t>& residual, std::vector<std::int32_t>& qlpCoeffs, std::int32_t lp_quantization, std::uint32_t order, std::vector<std::int32_t>& output) {
	/*switch (order) {
	case 0:
		restoreLpcSignal_32bit<0>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 1:
		restoreLpcSignal_32bit<1>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 2:
		restoreLpcSignal_32bit<2>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 3:
		restoreLpcSignal_32bit<3>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 4:
		restoreLpcSignal_32bit<4>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 5:
		restoreLpcSignal_32bit<5>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 6:
		restoreLpcSignal_32bit<6>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 7:
		restoreLpcSignal_32bit<7>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 8:
		restoreLpcSignal_32bit<8>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 9:
		restoreLpcSignal_32bit<9>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 10:
		restoreLpcSignal_32bit<10>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 11:
		restoreLpcSignal_32bit<11>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 12:
		restoreLpcSignal_32bit<12>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 13:
		restoreLpcSignal_32bit<13>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 14:
		restoreLpcSignal_32bit<14>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 15:
		restoreLpcSignal_32bit<15>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 16:
		restoreLpcSignal_32bit<16>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 17:
		restoreLpcSignal_32bit<17>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 18:
		restoreLpcSignal_32bit<18>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 19:
		restoreLpcSignal_32bit<19>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 20:
		restoreLpcSignal_32bit<20>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 21:
		restoreLpcSignal_32bit<21>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 22:
		restoreLpcSignal_32bit<22>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 23:
		restoreLpcSignal_32bit<23>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 24:
		restoreLpcSignal_32bit<24>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 25:
		restoreLpcSignal_32bit<25>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 26:
		restoreLpcSignal_32bit<26>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 27:
		restoreLpcSignal_32bit<27>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 28:
		restoreLpcSignal_32bit<28>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 29:
		restoreLpcSignal_32bit<29>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 30:
		restoreLpcSignal_32bit<30>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 31:
		restoreLpcSignal_32bit<31>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 32:
		restoreLpcSignal_32bit<32>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;

	}*/
unsigned  j;
std::reverse(qlpCoeffs.begin(), qlpCoeffs.end());
for (auto ix = 0u; ix < residual.size(); ++ix) {
	int64x2_t sum = { 0, 0 };
	auto pairwiseCount = order / 2;
	for (j = 0; j < pairwiseCount; j++) {
		int32x2_t coeffs = vld1_s32((&qlpCoeffs[order - (j + 1) * 2]));
		int32x2_t hs = vld1_s32(&output[order + ix - (j + 1) * 2]);
		sum = vmlal_s32(sum, coeffs, hs);
	}
	int64x1_t sumAll = vadd_s64(vget_high_s64(sum), vget_low_s64(sum));
	std::int64_t res = sumAll.n64_i64[0];
	if (order % 2 == 1) {
		auto coeff = qlpCoeffs[0];
		auto h = output[ix];
		res += (std::int64_t)coeff*(std::int64_t)h;
	}
	output[order + ix] = residual[ix] + (std::int32_t)(res >> lp_quantization);
}
}
template<std::uint32_t order>
void restoreLpcSignal_16bit(const std::vector<std::int32_t>& residual, std::vector<std::int32_t>& qlpCoeffs, std::int32_t lp_quantization, std::vector<std::int32_t>& output) {
	unsigned  j;
	std::reverse(qlpCoeffs.begin(), qlpCoeffs.end());
	for (auto ix = 0u; ix < residual.size();++ix) {
		int32x4_t sum = { 0,0};
		auto quadwiseCount = order / 4;
		for (j = 0; j < quadwiseCount; j++) {
			int32x4_t coeffs = vld1q_s32((&qlpCoeffs[order - (j+1) * 4]));
			int32x4_t hs = vld1q_s32(&output[order + ix - (j + 1) * 4]);
			sum = vmlaq_s32(sum, coeffs, hs);
		}
		std::int32_t quad[4];
		vst1q_s32(quad, sum);
		auto res = quad[0] + quad[1] + quad[2] + quad[3];
		if (order % 2 == 1) {
			auto coeff = qlpCoeffs[0];
			auto h = output[ix];
			res += (std::int64_t)coeff*(std::int64_t)h;
		}
		auto remaining = order % 4;
		if (remaining >= 2) {
			auto j = quadwiseCount * 2;
			int32x2_t coeffs = vld1_s32((&qlpCoeffs[order - (j+1) * 2]));
			int32x2_t hs = vld1_s32(&output[order + ix - (j + 1) * 2]);
			int32x2_t m = vmul_s32( coeffs, hs);
			res += (m.n64_i32[0] + m.n64_i32[1]);
		}
		
		output[order + ix] = residual[ix] + (std::int32_t)(res >> lp_quantization);
	}

}
void restoreLpcSignal_16bit(const std::vector<std::int32_t>& residual, std::vector<std::int32_t>& qlpCoeffs, std::int32_t lp_quantization, std::uint32_t order, std::vector<std::int32_t>& output) {
	/*switch (order) {
	case 0:
		restoreLpcSignal_16bit<0>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 1:
		restoreLpcSignal_16bit<1>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 2:
		restoreLpcSignal_16bit<2>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 3:
		restoreLpcSignal_16bit<3>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 4:
		restoreLpcSignal_16bit<4>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 5:
		restoreLpcSignal_16bit<5>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 6:
		restoreLpcSignal_16bit<6>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 7:
		restoreLpcSignal_16bit<7>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 8:
		restoreLpcSignal_16bit<8>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 9:
		restoreLpcSignal_16bit<9>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 10:
		restoreLpcSignal_16bit<10>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 11:
		restoreLpcSignal_16bit<11>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 12:
		restoreLpcSignal_16bit<12>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 13:
		restoreLpcSignal_16bit<13>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 14:
		restoreLpcSignal_16bit<14>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 15:
		restoreLpcSignal_16bit<15>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 16:
		restoreLpcSignal_16bit<16>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 17:
		restoreLpcSignal_16bit<17>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 18:
		restoreLpcSignal_16bit<18>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 19:
		restoreLpcSignal_16bit<19>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 20:
		restoreLpcSignal_16bit<20>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 21:
		restoreLpcSignal_16bit<21>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 22:
		restoreLpcSignal_16bit<22>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 23:
		restoreLpcSignal_16bit<23>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 24:
		restoreLpcSignal_16bit<24>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 25:
		restoreLpcSignal_16bit<25>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 26:
		restoreLpcSignal_16bit<26>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 27:
		restoreLpcSignal_16bit<27>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 28:
		restoreLpcSignal_16bit<28>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 29:
		restoreLpcSignal_16bit<29>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 30:
		restoreLpcSignal_16bit<30>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 31:
		restoreLpcSignal_16bit<31>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;
	case 32:
		restoreLpcSignal_16bit<32>(residual, qlpCoeffs, lp_quantization, bps, output, 0);
		break;

	}*/
unsigned  j;
std::reverse(qlpCoeffs.begin(), qlpCoeffs.end());
for (auto ix = 0u; ix < residual.size(); ++ix) {
	int32x4_t sum = { 0, 0 };
	auto quadwiseCount = order / 4;
	for (j = 0; j < quadwiseCount; j++) {
		int32x4_t coeffs = vld1q_s32((&qlpCoeffs[order - (j + 1) * 4]));
		int32x4_t hs = vld1q_s32(&output[order + ix - (j + 1) * 4]);
		sum = vmlaq_s32(sum, coeffs, hs);
	}
	std::int32_t quad[4];
	vst1q_s32(quad, sum);
	auto res = quad[0] + quad[1] + quad[2] + quad[3];
	if (order % 2 == 1) {
		auto coeff = qlpCoeffs[0];
		auto h = output[ix];
		res += (std::int64_t)coeff*(std::int64_t)h;
	}
	auto remaining = order % 4;
	if (remaining >= 2) {
		auto j = quadwiseCount * 2;
		int32x2_t coeffs = vld1_s32((&qlpCoeffs[order - (j + 1) * 2]));
		int32x2_t hs = vld1_s32(&output[order + ix - (j + 1) * 2]);
		int32x2_t m = vmul_s32(coeffs, hs);
		res += (m.n64_i32[0] + m.n64_i32[1]);
	}

	output[order + ix] = residual[ix] + (std::int32_t)(res >> lp_quantization);
}
}
#else

void restoreLpcSignal(const std::vector<std::int32_t>& residual, const std::vector<std::int32_t>& qlpCoeffs, std::int32_t lp_quantization, std::uint32_t order, std::vector<std::int32_t>& output) {
	unsigned  j;

	for (auto ix = 0u; ix < residual.size();++ix) {
		std::int64_t sum = 0;
		for (j = 0; j < order; j++) {
			auto h = output[order + ix - j - 1];
			sum += (int64_t)qlpCoeffs[j] * (int64_t)(h);
		}
		output[order + ix] = residual[ix] + (std::int32_t)(sum >> lp_quantization);
	}

}
void restoreLpcSignal_32bit(const std::vector<std::int32_t>& residual, const std::vector<std::int32_t>& qlpCoeffs, std::int32_t lp_quantization, std::uint32_t order, std::vector<std::int32_t>& output) {
	unsigned  j;

	for (auto ix = 0u; ix < residual.size();++ix) {
		std::int64_t sum = 0;
		for (j = 0; j < order; j++) {
			auto h = output[order + ix - j - 1];
			sum += (int64_t)(qlpCoeffs[j] * h);
		}
		output[order + ix] = residual[ix] + (std::int32_t)(sum >> lp_quantization);
	}

}
void restoreLpcSignal_16bit(const std::vector<std::int32_t>& residual, const std::vector<std::int32_t>& qlpCoeffs, std::int32_t lp_quantization, std::uint32_t order, std::vector<std::int32_t>& output) {
	unsigned  j;

	for (auto ix = 0u; ix < residual.size();++ix) {
		std::int32_t sum = 0;
		for (j = 0; j < order; j++) {
			auto h = output[order + ix - j - 1];
			sum += (qlpCoeffs[j] * h);
		}
		output[order + ix] = residual[ix] + (sum >> lp_quantization);
	}

}
#endif





void restoreFixedSignal(const std::vector<std::int32_t>& residual, const std::uint32_t order, std::vector<std::int32_t>& output)
{
	switch (order) {
	case 0:
		for (auto ix = 0u;ix < residual.size();++ix) {
			output[ix] = residual[ix];
		}
		
		break;
	case 1:
		for (auto ix = 0u;ix < residual.size();++ix) {
			output[ix+order] = residual[ix] + output[ix +order - 1];
		}
		break;
	case 2:
		for (auto ix = 0u;ix < residual.size();++ix) {
			output[ix + order] = residual[ix] + (output[(ix + order - 1)] << 1) - output[(ix + order - 2)];
		}
		break;

	case 3:
		for (auto ix = 0u;ix < residual.size();++ix) {
			output[ix + order] = residual[ix] + (((output[(ix + order - 1)] - output[(ix + order - 2)]) << 1) + (output[(ix + order - 1)] - output[(ix + order - 2)])) + output[(ix + order - 3)];
		}
		break;
	case 4:
		for (auto ix = 0u;ix < residual.size();++ix) {
			output[ix + order] = residual[ix] + 4 * output[(ix + order - 1)] - 6 * output[(ix + order - 2)] + 4 * output[(ix + order - 3)] - output[(ix + order - 4)];
		}
		break;
	default:
		throw FlacDecodingException();

	}
}

void readConstantSubFrameContent(FlacBitStream* stream, std::vector<std::int32_t>& outputBuffer, std::uint8_t usefulBps, const std::uint32_t blockSize,  bool validateOnly) {
	outputBuffer.resize(blockSize);
	auto constant = stream->readPartialInt32(usefulBps);
	if (!validateOnly) {
		for (auto ix = 0u;ix < blockSize;ix++) {
			outputBuffer[ix] = constant;
		}
	}
}
template<std::uint8_t usefulBps>
void readVerbatimSubFrameContent(FlacBitStream* stream, std::vector<std::int32_t>& outputBuffer, std::uint32_t blockSize, bool validateOnly) {
	outputBuffer.resize(blockSize);
	if (!validateOnly) {
		for (auto ix = 0u;ix < blockSize;ix++) {
			outputBuffer[ix] = stream->readPartialInt32<usefulBps>();
		}
	}
	else {
		for (auto ix = 0u;ix < blockSize;++ix) {
			stream->readPartialInt32(usefulBps);
		}
	}
}
void readVerbatimSubFrameContent(FlacBitStream* stream, std::vector<std::int32_t>& outputBuffer, std::uint8_t usefulBps, std::uint32_t blockSize, bool validateOnly) {
	switch (usefulBps) {
	case 1:
		readVerbatimSubFrameContent<1>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 2:
		readVerbatimSubFrameContent<2>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 3:
		readVerbatimSubFrameContent<3>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 4:
		readVerbatimSubFrameContent<4>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 5:
		readVerbatimSubFrameContent<5>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 6:
		readVerbatimSubFrameContent<6>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 7:
		readVerbatimSubFrameContent<7>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 8:
		readVerbatimSubFrameContent<8>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 9:
		readVerbatimSubFrameContent<9>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 10:
		readVerbatimSubFrameContent<10>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 11:
		readVerbatimSubFrameContent<11>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 12:
		readVerbatimSubFrameContent<12>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 13:
		readVerbatimSubFrameContent<13>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 14:
		readVerbatimSubFrameContent<14>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 15:
		readVerbatimSubFrameContent<15>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 16:
		readVerbatimSubFrameContent<16>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 17:
		readVerbatimSubFrameContent<17>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 18:
		readVerbatimSubFrameContent<18>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 19:
		readVerbatimSubFrameContent<19>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 20:
		readVerbatimSubFrameContent<20>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 21:
		readVerbatimSubFrameContent<21>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 22:
		readVerbatimSubFrameContent<22>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 23:
		readVerbatimSubFrameContent<23>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 24:
		readVerbatimSubFrameContent<24>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 25:
		readVerbatimSubFrameContent<25>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 26:
		readVerbatimSubFrameContent<26>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 27:
		readVerbatimSubFrameContent<27>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 28:
		readVerbatimSubFrameContent<28>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 29:
		readVerbatimSubFrameContent<29>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 30:
		readVerbatimSubFrameContent<30>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 31:
		readVerbatimSubFrameContent<31>(stream, outputBuffer, blockSize, validateOnly);
		break;
	case 32:
		readVerbatimSubFrameContent<32>(stream, outputBuffer, blockSize, validateOnly);
		break;

	}
}
template<std::uint8_t usefulBps>
bool readFixedSubFrameContent(std::uint32_t order, FlacBitStream* stream, std::vector<std::int32_t>& outputBuffer, std::uint32_t blockSize, bool validateOnly, FlacPP::temp_buffer_cache& residualCache) {

	outputBuffer.resize(blockSize);
	if (!validateOnly) {
		for (auto ix = 0u; ix < order;++ix) {
			auto val = stream->readPartialInt32<usefulBps>();

			outputBuffer[ix] = val;
		}
	}
	else {

		for (auto ix = 0u; ix < order;++ix) {
			auto val = stream->readPartialInt32<usefulBps>();
		}
	}



	auto residualCodingType = stream->readPartialUint32(2);
	std::vector<std::int32_t> residual = residualCache.getOne();
	if (residualCodingType == 0) {
		// partitioned rice with 4-bits parameter
		if (!extractResidualPartitionedRice4bits(residual, order, stream, usefulBps, blockSize)) {
			residualCache.release(std::move(residual));
			if (validateOnly) {
				return false;
			}
			throw FlacDecodingException();
		}
	}
	else if (residualCodingType == 1) {
		// partitioned rice with 5-bits parameter
		if (!extractResidualPartitionedRice5bits(residual, order, stream, usefulBps, blockSize)) {
			residualCache.release(std::move(residual));
			if (validateOnly) {
				return false;
			}
			throw FlacDecodingException();
		}
	}
	else {
		residualCache.release(std::move(residual));
		if (validateOnly) {
			return false;
		}
		throw FlacDecodingException();
	}
	if (!validateOnly) {
		restoreFixedSignal(residual, order, outputBuffer);
	}
	residualCache.release(std::move(residual));
	return true;
}
bool readFixedSubFrameContent(std::uint32_t order, FlacBitStream* stream, std::vector<std::int32_t>& outputBuffer, std::uint8_t usefulBps, std::uint32_t blockSize, bool validateOnly, FlacPP::temp_buffer_cache& residualCache) {
	
	switch (usefulBps) {
	case 1:
		return readFixedSubFrameContent<1>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 2:
		return readFixedSubFrameContent<2>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 3:
		return readFixedSubFrameContent<3>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 4:
		return readFixedSubFrameContent<4>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 5:
		return readFixedSubFrameContent<5>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 6:
		return readFixedSubFrameContent<6>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 7:
		return readFixedSubFrameContent<7>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 8:
		return readFixedSubFrameContent<8>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 9:
		return readFixedSubFrameContent<9>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 10:
		return readFixedSubFrameContent<10>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 11:
		return readFixedSubFrameContent<11>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 12:
		return readFixedSubFrameContent<12>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 13:
		return readFixedSubFrameContent<13>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 14:
		return readFixedSubFrameContent<14>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 15:
		return readFixedSubFrameContent<15>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 16:
		return readFixedSubFrameContent<16>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 17:
		return readFixedSubFrameContent<17>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 18:
		return readFixedSubFrameContent<18>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 19:
		return readFixedSubFrameContent<19>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 20:
		return readFixedSubFrameContent<20>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 21:
		return readFixedSubFrameContent<21>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 22:
		return readFixedSubFrameContent<22>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 23:
		return readFixedSubFrameContent<23>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 24:
		return readFixedSubFrameContent<24>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 25:
		return readFixedSubFrameContent<25>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 26:
		return readFixedSubFrameContent<26>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 27:
		return readFixedSubFrameContent<27>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 28:
		return readFixedSubFrameContent<28>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 29:
		return readFixedSubFrameContent<29>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 30:
		return readFixedSubFrameContent<30>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
		
	case 31:
		return readFixedSubFrameContent<31>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);
	case 32:
		return readFixedSubFrameContent<32>(order, stream, outputBuffer, blockSize, validateOnly, residualCache);

	default:
		return false;

	}
}


static inline std::uint32_t FLAC__bitmath_ilog2(std::uint32_t v)
{
#ifdef _MSC_VER

		unsigned long idx;
		_BitScanReverse(&idx, v);
		return idx;
#else
	return 31 - __builtin_clz(v);
#endif
}

template<std::uint8_t usefulBps>
bool readLpcSubFrameContent(std::uint32_t predictorOrder, FlacBitStream* stream, std::vector<std::int32_t>& outputBuffer, std::uint32_t blockSize, bool validateOnly, FlacPP::temp_buffer_cache& residualCache) {

	outputBuffer.resize(blockSize);
	if (!validateOnly) {
		for (auto ix = 0u; ix < predictorOrder;++ix) {
			auto val = stream->readPartialInt32<usefulBps>();

			outputBuffer[ix] = val;
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

	std::vector<std::int32_t> residual = residualCache.getOne();
	if (residualCodingType == 0) {
		// partitioned rice with 4-bits parameter
		if (!extractResidualPartitionedRice4bits(residual, predictorOrder, stream, usefulBps, blockSize)) {
			residualCache.release(std::move(residual));
			if (validateOnly) {
				return false;
			}
			throw FlacDecodingException();
		}
	}
	else if (residualCodingType == 1) {
		// partitioned rice with 5-bits parameter
		if (!extractResidualPartitionedRice5bits(residual, predictorOrder, stream, usefulBps, blockSize)) {
			residualCache.release(std::move(residual));
			if (validateOnly) {
				return false;
			}
			throw FlacDecodingException();
		}
	}
	else {
		residualCache.release(std::move(residual));
		if (validateOnly) {
			return false;
		}
		throw FlacDecodingException();
	}
	if (!validateOnly) {
		if (usefulBps + quantizedLinearPredictorCoeffPrecision + FLAC__bitmath_ilog2(predictorOrder) <= 32) {
			if (usefulBps <= 16 && quantizedLinearPredictorCoeffPrecision <= 16)
			{
				restoreLpcSignal_16bit(residual, qlpCoeffs, quantizedLinearPredictorCoeffShift, predictorOrder, outputBuffer);
			}
			else {
				restoreLpcSignal_32bit(residual, qlpCoeffs, quantizedLinearPredictorCoeffShift, predictorOrder, outputBuffer);
			}
		}
		else {
			restoreLpcSignal(residual, qlpCoeffs, quantizedLinearPredictorCoeffShift, predictorOrder, outputBuffer);
		}
	}
	residualCache.release(std::move(residual));
	return true;
}

bool readLpcSubFrameContent(std::uint32_t predictorOrder,FlacBitStream* stream, std::vector<std::int32_t>& outputBuffer, std::uint8_t usefulBps, std::uint32_t blockSize, bool validateOnly, FlacPP::temp_buffer_cache& residualCache) {
	switch (usefulBps) {
	case 1:
		return readLpcSubFrameContent<1>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 2:
		return readLpcSubFrameContent<2>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 3:
		return readLpcSubFrameContent<3>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 4:
		return readLpcSubFrameContent<4>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 5:
		return readLpcSubFrameContent<5>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 6:
		return readLpcSubFrameContent<6>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 7:
		return readLpcSubFrameContent<7>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 8:
		return readLpcSubFrameContent<8>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 9:
		return readLpcSubFrameContent<9>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 10:
		return readLpcSubFrameContent<10>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 11:
		return readLpcSubFrameContent<11>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 12:
		return readLpcSubFrameContent<12>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 13:
		return readLpcSubFrameContent<13>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 14:
		return readLpcSubFrameContent<14>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 15:
		return readLpcSubFrameContent<15>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 16:
		return readLpcSubFrameContent<16>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 17:
		return readLpcSubFrameContent<17>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 18:
		return readLpcSubFrameContent<18>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 19:
		return readLpcSubFrameContent<19>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 20:
		return readLpcSubFrameContent<20>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 21:
		return readLpcSubFrameContent<21>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 22:
		return readLpcSubFrameContent<22>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 23:
		return readLpcSubFrameContent<23>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 24:
		return readLpcSubFrameContent<24>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 25:
		return readLpcSubFrameContent<25>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 26:
		return readLpcSubFrameContent<26>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 27:
		return readLpcSubFrameContent<27>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 28:
		return readLpcSubFrameContent<28>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 29:
		return readLpcSubFrameContent<29>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 30:
		return readLpcSubFrameContent<30>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 31:
		return readLpcSubFrameContent<31>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	case 32:
		return readLpcSubFrameContent<32>(predictorOrder, stream, outputBuffer, blockSize, validateOnly, residualCache);

	default:
		return false;
	}
}

bool readSubframe(std::uint16_t channelIndex, FlacBitStream* stream, std::vector<std::int32_t>& outputBuffer, const frame_header& frameHeader, bool validateOnly, FlacPP::temp_buffer_cache& residualCache) {
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
		if (!readLpcSubFrameContent(order, stream, outputBuffer, bps, frameHeader.blockSize, validateOnly, residualCache)) {
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
		if (!readFixedSubFrameContent( order, stream, outputBuffer, bps, frameHeader.blockSize, validateOnly, residualCache)) {
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
		readVerbatimSubFrameContent(stream, outputBuffer, bps, frameHeader.blockSize, validateOnly);
	}
	else {
		subframeType = subframe_type::constant;
		readConstantSubFrameContent(stream, outputBuffer, bps, frameHeader.blockSize, validateOnly);
	}

	if (wastedBits != 0 && !validateOnly) {
		for (auto ix = 0u; ix < frameHeader.blockSize; ix++) {
			auto val =  (std::uint32_t)outputBuffer[ix];
			outputBuffer[ix] = (std::int32_t)( val << wastedBits);
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
			if (_streamInfo.bitsPerSample <= 16) {
				_streamInfo.outputBitsPerSample = 16;
			}
			else if (_streamInfo.bitsPerSample <= 24) {
				_streamInfo.outputBitsPerSample = 24;
			}
			else {

				_streamInfo.outputBitsPerSample = 32;
			}
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
		header.blockSize = FlacPP::swapEndiannes(temp) + 1;
	}

	if (sampleRateStrategy == samplerate_strategy::grab8BitsKHz) {
		std::uint8_t temp;
		readFromStream(stream, temp);
		header.sampleRate = static_cast<std::uint32_t>(temp) * 1000;
	}
	else if (sampleRateStrategy == samplerate_strategy::grab16BitsHz) {
		std::uint16_t temp;
		readFromStream(stream, temp);
		header.sampleRate = static_cast<std::uint32_t>(FlacPP::swapEndiannes(temp));
	}
	else if (sampleRateStrategy == samplerate_strategy::grab16Bits10Hz) {

		std::uint16_t temp;
		readFromStream(stream, temp);
		header.sampleRate = static_cast<std::uint32_t>(FlacPP::swapEndiannes(temp)) * 10;
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

bool findNextFrameHeader(FlacPP::IFlacStream* stream, frame_header& header, const stream_info& streamInfo, std::uint64_t sampleToFind, FlacPP::temp_buffer_cache& outputBufCache, FlacPP::temp_buffer_cache& residualCache) {
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
					auto tempV = outputBufCache.getOne();
					if (!readSubframe(ix, &fbs, tempV, header, true, residualCache)) {
						outputBufCache.release(std::move(tempV));
						allValid = false;
						break;
					}
					else {
						outputBufCache.release(std::move(tempV));

					}
				}
			}
			if (allValid) {
				// validate crc
				std::uint16_t readCrc;
				auto sizeOfCrcedData = stream->position() - headPos;
				readFromStream(stream, readCrc);
				readCrc = FlacPP::swapEndiannes(readCrc);
				
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
	if (!_outputBuffer) {

		_outputBuffer.reset(new std::uint8_t[this->_streamInfo.maxBlockSizeInSamples*_streamInfo.channels*(_streamInfo.outputBitsPerSample / 8u)]);
	}
	FlacBufferView buf(_outputBuffer.get(), this->_streamInfo.maxBlockSizeInSamples*_streamInfo.channels*(_streamInfo.outputBitsPerSample / 8u));
	decodeNextFrame(frameTime, buf);
	return buf;
}
void FlacPP::FlacDecoder::decodeNextFrame(time_unit_100ns & frameTime, FlacBufferView & buf)
{
	if (_stream->position() == _stream->size()) {
		return;
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
	auto frameOrSampleIndex = read_utf8_uint64(_stream.get(), 64 / 8);
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
		header.blockSize = FlacPP::swapEndiannes(temp) + 1;
	}

	if (sampleRateStrategy == samplerate_strategy::grab8BitsKHz) {
		std::uint8_t temp;
		readFromStream(_stream.get(), temp);
		header.sampleRate = static_cast<std::uint32_t>(temp) * 1000;
	}
	else if (sampleRateStrategy == samplerate_strategy::grab16BitsHz) {
		std::uint16_t temp;
		readFromStream(_stream.get(), temp);
		header.sampleRate = static_cast<std::uint32_t>(FlacPP::swapEndiannes(temp));
	}
	else if (sampleRateStrategy == samplerate_strategy::grab16Bits10Hz) {

		std::uint16_t temp;
		readFromStream(_stream.get(), temp);
		header.sampleRate = static_cast<std::uint32_t>(FlacPP::swapEndiannes(temp)) * 10;
	}

	std::uint8_t frameHeaderCRC;
	readFromStream(_stream.get(), frameHeaderCRC);

	frameTime = time_unit_100ns(header.firstSampleIndex * 10000000 / header.sampleRate);
	std::vector<std::vector<int32_t>> channelsData;

	{
		FlacBitStream fbs(this->_stream.get());
		for (std::uint16_t ix = 0;ix < header.channelCount;++ix) {
			channelsData.push_back(_outputBufferCache.getOne());
			readSubframe(ix, &fbs, channelsData[channelsData.size() - 1], header, false, _residualBufferCache);
		}
	}

	std::uint16_t frameContentCRC;
	readFromStream(_stream.get(), frameContentCRC);
	auto bytesPerSample = _streamInfo.outputBitsPerSample / 8u;
	if (buf.remainingCapacity() < bytesPerSample*_streamInfo.channels * header.blockSize) {
		throw FlacDecodingException();
	}
	auto output = buf.end();
	buf.length(buf.length() + bytesPerSample*_streamInfo.channels * header.blockSize);
	auto skippedBytesPerSample = 0;
	switch (header.channelAssignment) {
	case channel_assignment::independent:

		for (auto i = 0u; i < header.blockSize; i++)
		{
			for (auto j = 0u;j < header.channelCount;++j) {
				auto ptr = reinterpret_cast<std::uint8_t*>(&channelsData[j][i]);
				for (auto byteix = 0u;byteix < bytesPerSample;++byteix) {
					output[i*header.channelCount*bytesPerSample + j*bytesPerSample + byteix] = *(ptr + skippedBytesPerSample + byteix);
				}
			}
		}
		/* do nothing */
		break;
	case channel_assignment::left_side_stereo:
		for (auto i = 0u; i < header.blockSize; i++) {
			auto ptr0 = reinterpret_cast<std::uint8_t*>(&channelsData[0][i]);
			auto val1 = channelsData[0][i] - channelsData[1][i];
			auto ptr1 = reinterpret_cast<std::uint8_t*>(&val1);
			for (auto byteix = 0u;byteix < bytesPerSample;++byteix) {
				output[i * 2 * bytesPerSample + byteix] = *(ptr0 + skippedBytesPerSample + byteix);
				output[i * 2 * bytesPerSample + bytesPerSample + byteix] = *(ptr1 + skippedBytesPerSample + byteix);
			}
		}
		break;
	case channel_assignment::side_right_stereo:
		for (auto i = 0u; i < header.blockSize; i++) {
			auto ptr1 = reinterpret_cast<std::uint8_t*>(&channelsData[1][i]);
			auto val0 = channelsData[1][i] + channelsData[0][i];
			auto ptr0 = reinterpret_cast<std::uint8_t*>(&val0);
			for (auto byteix = 0u;byteix < bytesPerSample;++byteix) {
				output[i * 2 * bytesPerSample + byteix] = *(ptr0 + skippedBytesPerSample + byteix);
				output[i * 2 * bytesPerSample + bytesPerSample + byteix] = *(ptr1 + skippedBytesPerSample + byteix);
			}
		}
		break;
	case channel_assignment::mid_side_stereo:
		for (auto i = 0u; i < header.blockSize; i++) {

			auto mid = channelsData[0][i];
			auto side = channelsData[1][i];
			mid <<= 1;
			mid |= (side & 1); /* i.e. if 'side' is odd... */

			auto val0 = (mid + side) >> 1;
			auto val1 = (mid - side) >> 1;
			auto ptr0 = reinterpret_cast<std::uint8_t*>(&val0);
			auto ptr1 = reinterpret_cast<std::uint8_t*>(&val1);
			for (auto byteix = 0u;byteix < bytesPerSample;++byteix) {
				output[i * 2 * bytesPerSample + byteix] = *(ptr0 + skippedBytesPerSample + byteix);
				output[i * 2 * bytesPerSample + bytesPerSample + byteix] = *(ptr1 + skippedBytesPerSample + byteix);
			}
		}
		break;
	}


	_nextSample = header.firstSampleIndex + header.blockSize;
	for (auto&& item : channelsData) {
		_outputBufferCache.release(std::move(item));
	}
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
	
	auto startSeekOffset = static_cast<std::uint64_t>((minFrameSearchPos + (maxFrameSearchPos - minFrameSearchPos)*relativeSeekPos));
	_stream->seek(startSeekOffset);
	for (;;) {
		
		
		if (_stream->position() < minFrameSearchPos) {
			_stream->seek(minFrameSearchPos);
		}
		if (_stream->position() > maxFrameSearchPos) {
			_stream->seek(maxFrameSearchPos);
		}
		
			if (!findNextFrameHeader(_stream.get(), header, _streamInfo, sample, _outputBufferCache, _residualBufferCache)) {
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
