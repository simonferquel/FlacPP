#pragma once
#include <cstdint>
#include <memory>
#include "FlacBuffer.h"

namespace FlacPP{
	class IFlacStream{
	public:
		virtual std::uint64_t size() const = 0;
		virtual std::uint64_t position() const = 0;
		virtual void seek(std::uint64_t absolutePos) = 0;
		virtual void readIntoBuffer(std::uint32_t size, FlacBufferView& buffer) = 0;
		virtual std::uint8_t readOneByte() = 0;
	};

	class BufferedInputStream : public IFlacStream
	{
	private:
		Windows::Storage::Streams::IRandomAccessStream^ _innerStream;
		std::uint64_t _innerStreamLength;
		std::uint64_t _positionInInnerStream;
		std::uint64_t _absolutePosition;
		std::uint32_t _positionInBuffer;
		const std::uint32_t _bufferLength = 1024 * 1024;
		std::uint32_t _loadedData;
		std::unique_ptr<std::uint8_t[]> _buffer;
		void seekForward(std::uint64_t amount);
		void seekBackward(std::uint64_t amount);

		std::uint32_t read(std::uint8_t buffer[], std::uint32_t bytes);
	public:
		BufferedInputStream(Windows::Storage::Streams::IRandomAccessStream^ innerStream, std::uint64_t streamLength);
		bool isEof() {
			return _absolutePosition == _innerStreamLength;
		}

		virtual std::uint64_t size() const override {
			return _innerStreamLength;
		}
		virtual std::uint64_t position() const override {
			return _absolutePosition;
		}
		virtual void seek(std::uint64_t absolutePos) override;
		virtual void readIntoBuffer(std::uint32_t size, FlacPP::FlacBufferView& buffer) override;
		virtual std::uint8_t readOneByte() override;
	};
}