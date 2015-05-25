#pragma once

#ifndef FLACPP_RICE
#define FLACPP_RICE

#include <cstdint>
#include <vector>
#include "includes/flacPP/FlacBitStream.h"


bool extractResidualPartitionedRice4bits(std::vector<std::int32_t>& output, std::uint32_t predictorOrder, FlacPP::FlacBitStream* stream, std::uint8_t usefulBps, std::uint32_t blockSize) {
	auto partitionOrder = stream->readPartialUint32<4>();
	auto partitionCount = 1u << (partitionOrder);
	if (partitionOrder == 0) {
		if (blockSize < predictorOrder) {

			/* We have received a potentially malicious bit stream. All we can do is error out to avoid a heap overflow. */
			return false;
		}
	}
	else {
		auto partition_samples = partitionOrder > 0 ? blockSize >> partitionOrder : blockSize - predictorOrder;
		if (partition_samples < predictorOrder) {

			return false;
		}
	}

	for (auto partitionIx = 0u;partitionIx < partitionCount;++partitionIx) {
		std::uint32_t nvalues = 0;
		if (partitionOrder == 0) {
			nvalues = blockSize - predictorOrder;
		}
		else if (partitionIx != 0) {
			nvalues = blockSize >> partitionOrder;
		}
		else {
			nvalues = (blockSize >> partitionOrder) - predictorOrder;
		}
		auto riceParameter = stream->readPartialUint32<4>();
		if (riceParameter < 0xF) {

			stream->readRiceSignedBlock(output, nvalues, riceParameter);
		}
		else {
			riceParameter = stream->readPartialUint32<5>();
			switch (riceParameter)
			{
			case 0:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<0>());
					output.push_back(i);
				}
				break;
			case 1:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<1>());
					output.push_back(i);
				}
				break;
			case 2:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<2>());
					output.push_back(i);
				}
				break;
			case 3:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<3>());
					output.push_back(i);
				}
				break;
			case 4:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<4>());
					output.push_back(i);
				}
				break;
			case 5:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<5>());
					output.push_back(i);
				}
				break;
			case 6:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<6>());
					output.push_back(i);
				}
				break;
			case 7:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<7>());
					output.push_back(i);
				}
				break;
			case 8:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<8>());
					output.push_back(i);
				}
				break;
			case 9:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<9>());
					output.push_back(i);
				}
				break;
			case 10:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<10>());
					output.push_back(i);
				}
				break;
			case 11:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<11>());
					output.push_back(i);
				}
				break;
			case 12:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<12>());
					output.push_back(i);
				}
				break;
			case 13:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<13>());
					output.push_back(i);
				}
				break;
			case 14:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<14>());
					output.push_back(i);
				}
				break;
			case 15:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<15>());
					output.push_back(i);
				}
				break;
			case 16:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<16>());
					output.push_back(i);
				}
				break;
			case 17:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<17>());
					output.push_back(i);
				}
				break;
			case 18:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<18>());
					output.push_back(i);
				}
				break;
			case 19:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<19>());
					output.push_back(i);
				}
				break;
			case 20:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<20>());
					output.push_back(i);
				}
				break;
			case 21:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<21>());
					output.push_back(i);
				}
				break;
			case 22:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<22>());
					output.push_back(i);
				}
				break;
			case 23:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<23>());
					output.push_back(i);
				}
				break;
			case 24:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<24>());
					output.push_back(i);
				}
				break;
			case 25:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<25>());
					output.push_back(i);
				}
				break;
			case 26:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<26>());
					output.push_back(i);
				}
				break;
			case 27:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<27>());
					output.push_back(i);
				}
				break;
			case 28:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<28>());
					output.push_back(i);
				}
				break;
			case 29:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<29>());
					output.push_back(i);
				}
				break;
			case 30:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<30>());
					output.push_back(i);
				}
				break;
			case 31:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<31>());
					output.push_back(i);
				}
				break;
			case 32:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<32>());
					output.push_back(i);
				}
				break;

			default:
				break;
			}
			
		}
	}
	return true;

}


bool extractResidualPartitionedRice5bits(std::vector<std::int32_t>& output, std::uint32_t predictorOrder, FlacPP::FlacBitStream* stream, std::uint8_t usefulBps, std::uint32_t blockSize) {
	auto partitionOrder = stream->readPartialUint32<4>();
	auto partitionCount = 1u << (partitionOrder);

	if (partitionOrder == 0) {
		if (blockSize < predictorOrder) {
			
			/* We have received a potentially malicious bit stream. All we can do is error out to avoid a heap overflow. */
			return false;
		}
	}
	else {
		auto partition_samples = partitionOrder > 0 ? blockSize >> partitionOrder : blockSize - predictorOrder;
		if (partition_samples < predictorOrder) {
			
			return false;
		}
	}

	for (auto partitionIx = 0u;partitionIx < partitionCount;++partitionIx) {
		std::uint32_t nvalues = 0;
		if (partitionOrder == 0) {
			nvalues = blockSize - predictorOrder;
		}
		else if (partitionIx != 0) {
			nvalues = blockSize >> partitionOrder;
		}
		else {
			nvalues = (blockSize >> partitionOrder) - predictorOrder;
		}
		auto riceParameter = stream->readPartialUint32<5>();
		if (riceParameter < 0x1F) {
			stream->readRiceSignedBlock(output, nvalues, riceParameter);
		}
		else {
			riceParameter = stream->readPartialUint32<5>();
			switch (riceParameter)
			{
			case 0:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<0>());
					output.push_back(i);
				}
				break;
			case 1:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<1>());
					output.push_back(i);
				}
				break;
			case 2:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<2>());
					output.push_back(i);
				}
				break;
			case 3:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<3>());
					output.push_back(i);
				}
				break;
			case 4:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<4>());
					output.push_back(i);
				}
				break;
			case 5:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<5>());
					output.push_back(i);
				}
				break;
			case 6:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<6>());
					output.push_back(i);
				}
				break;
			case 7:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<7>());
					output.push_back(i);
				}
				break;
			case 8:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<8>());
					output.push_back(i);
				}
				break;
			case 9:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<9>());
					output.push_back(i);
				}
				break;
			case 10:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<10>());
					output.push_back(i);
				}
				break;
			case 11:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<11>());
					output.push_back(i);
				}
				break;
			case 12:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<12>());
					output.push_back(i);
				}
				break;
			case 13:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<13>());
					output.push_back(i);
				}
				break;
			case 14:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<14>());
					output.push_back(i);
				}
				break;
			case 15:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<15>());
					output.push_back(i);
				}
				break;
			case 16:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<16>());
					output.push_back(i);
				}
				break;
			case 17:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<17>());
					output.push_back(i);
				}
				break;
			case 18:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<18>());
					output.push_back(i);
				}
				break;
			case 19:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<19>());
					output.push_back(i);
				}
				break;
			case 20:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<20>());
					output.push_back(i);
				}
				break;
			case 21:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<21>());
					output.push_back(i);
				}
				break;
			case 22:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<22>());
					output.push_back(i);
				}
				break;
			case 23:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<23>());
					output.push_back(i);
				}
				break;
			case 24:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<24>());
					output.push_back(i);
				}
				break;
			case 25:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<25>());
					output.push_back(i);
				}
				break;
			case 26:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<26>());
					output.push_back(i);
				}
				break;
			case 27:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<27>());
					output.push_back(i);
				}
				break;
			case 28:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<28>());
					output.push_back(i);
				}
				break;
			case 29:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<29>());
					output.push_back(i);
				}
				break;
			case 30:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<30>());
					output.push_back(i);
				}
				break;
			case 31:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<31>());
					output.push_back(i);
				}
				break;
			case 32:
				for (auto u = (partitionOrder == 0 || partitionIx > 0) ? 0 : predictorOrder; u < nvalues; u++) {
					auto i = static_cast<std::int32_t>(stream->readPartialInt32<32>());
					output.push_back(i);
				}
				break;

			default:
				break;
			}
		}
	}
	return true;

}

#endif