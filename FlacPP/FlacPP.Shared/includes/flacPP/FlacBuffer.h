#pragma once
#ifndef FLACPP_FLACBUFFER
#define FLACPP_FLACBUFFER

#include <memory>
#include <cstdint>
#include <exception>

namespace FlacPP{

	class FlacBufferOverflow : public std::exception{

	};

	// represents a view over an existing native buffer
	class FlacBufferView
	{
	private:
		std::uint32_t _capacity;
		std::uint8_t* _data;
		std::uint32_t _length;
	public:
		// initialize the buffer view
		// data: raw buffer
		// capacity: allocated size of the buffer
		// length: commited data in the buffer
		FlacBufferView(std::uint8_t* data, std::uint32_t capacity, std::uint32_t length=0)
			: _capacity(capacity),
			_data(data),
			_length(length){
		}

		// size of commited data in the buffer
		std::uint32_t length() const{
			return _length;
		}
		// pointer to the first byte of the buffer
		std::uint8_t* begin(){
			return _data;
		}
		// capacity of the buffer (allocated size from begin())
		std::uint32_t capacity() const{
			return _capacity;
		}
		// set the commited size
		void length(std::uint32_t value){
			_length = value;
		}
		// remaining size in the buffer (capacity - length)
		std::uint32_t remainingCapacity() const{
			return _capacity - _length;
		}
		// pointer to the end of the committed data
		std::uint8_t* end(){
			return _data + _length;
		}

		// append bytes into the buffer
		void append(std::uint8_t* bytes, std::uint32_t size){
			if (size > remainingCapacity()){
				throw FlacBufferOverflow();
			}
			memcpy(end(), bytes, size);
			_length += size;
		}


	};
}

#endif