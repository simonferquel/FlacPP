#pragma once

#ifndef FLACPP_FLACSTREAM
#define FLACPP_FLACSTREAM
#include <cstdint>
#include <memory>
#include "FlacBuffer.h"
#include <istream>

namespace FlacPP{
	// byte stream interface used by the decoder
	class IFlacStream{
	public:
		virtual std::uint64_t size() const = 0;
		virtual std::uint64_t position() const = 0;
		virtual void seek(std::uint64_t absolutePos) = 0;
		virtual void readIntoBuffer(std::uint32_t size, FlacBufferView& buffer) = 0;
		virtual std::uint8_t readOneByte() = 0;
	};

	// implementation of IFlacStream over a std::istream (or derived)
	class FlacStreamOveristream : public IFlacStream {
	private:
		std::unique_ptr<std::istream> _innerStream;
	public:
		FlacStreamOveristream(std::unique_ptr<std::istream>&& innerStream) : _innerStream(std::move(innerStream)) {}

		virtual std::uint64_t size() const override {
			auto pos = _innerStream->tellg();
			_innerStream->seekg(0, std::ios_base::end);
			auto size = _innerStream->tellg();
			_innerStream->seekg(pos);
			return size;
		}

		virtual std::uint64_t position() const override {
			return _innerStream->tellg();
		}

		virtual void seek(std::uint64_t absolutePos) override {
			_innerStream->seekg(absolutePos);
		}


		virtual void readIntoBuffer(std::uint32_t size, FlacBufferView& buffer) override {
			_innerStream->read(reinterpret_cast<char*>(buffer.end()), size);
			auto actuallyRead = static_cast<std::uint32_t>(_innerStream->gcount());
			buffer.length( buffer.length()+ actuallyRead);
			while (actuallyRead < size) {
				_innerStream->read(reinterpret_cast<char*>(buffer.end()), size-actuallyRead);
				auto thisRead = static_cast<std::uint32_t>(_innerStream->gcount());
				if (thisRead == 0) {
					return;
				}
				buffer.length(buffer.length() + thisRead);
				actuallyRead += thisRead;
			}
		}

		virtual std::uint8_t readOneByte() override {
			char val;
			_innerStream->read(&val, 1);
			return (std::uint8_t)val;
		}
	};
#ifdef WINRT
	// implementation of IFlacStream over a WinRT IRandomAccessStream
	// uses an internal buffer to limit the creation of temporary winrt buffer objects 
	class BufferedInputStream : public IFlacStream
	{
	private:
		Windows::Storage::Streams::IRandomAccessStream^ _innerStream;
		std::uint64_t _innerStreamLength;
		std::uint64_t _positionInInnerStream;
		std::uint64_t _absolutePosition;
		std::uint32_t _positionInBuffer;
		std::uint32_t _bufferLength;
		std::uint32_t _loadedData;
		std::unique_ptr<std::uint8_t[]> _buffer;
		void seekForward(std::uint64_t amount);
		void seekBackward(std::uint64_t amount);

		std::uint32_t read(std::uint8_t buffer[], std::uint32_t bytes);
	public:
		BufferedInputStream(Windows::Storage::Streams::IRandomAccessStream^ innerStream, std::uint64_t streamLength, std::uint32_t bufferLength = 1024 * 1024);
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
#endif
}
#endif