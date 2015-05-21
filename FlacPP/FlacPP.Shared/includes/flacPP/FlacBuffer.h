#pragma once

#include <memory>
#include <cstdint>
#include <exception>

namespace FlacPP{

	class FlacBufferOverflow : public std::exception{

	};


	class FlacBufferView
	{
	private:
		std::uint32_t _capacity;
		std::uint8_t* _data;
		std::uint32_t _length;
	public:
		FlacBufferView(std::uint8_t* data, std::uint32_t capacity, std::uint32_t length=0)
			: _capacity(capacity),
			_data(data),
			_length(length){
		}


		std::uint32_t length() const{
			return _length;
		}
		std::uint8_t* begin(){
			return _data;
		}
		std::uint32_t capacity() const{
			return _capacity;
		}
		void length(std::uint32_t value){
			_length = value;
		}

		std::uint32_t remainingCapacity() const{
			return _capacity - _length;
		}

		std::uint8_t* end(){
			return _data + _length;
		}

		void append(std::uint8_t* bytes, std::uint32_t size){
			if (size > remainingCapacity()){
				throw FlacBufferOverflow();
			}
			memcpy(end(), bytes, size);
			_length += size;
		}


	};
}

