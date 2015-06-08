#include "pch.h"
#ifdef WINRT
#include "includes/flacPP/FlacStream.h"
#include <robuffer.h>
#include <wrl.h>
#include <windows.storage.streams.h>
#include <ppltasks.h>
using namespace Microsoft::WRL;
using namespace FlacPP;

namespace ABI
{
	namespace FlacPP {
		class NativeBufferWrapper :
			public Microsoft::WRL::RuntimeClass < Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>,
			ABI::Windows::Storage::Streams::IBuffer,
			::Windows::Storage::Streams::IBufferByteAccess >
		{
			InspectableClass(L"FlacPP.NativeBufferWrapper", BaseTrust)

		private:
			UINT32 m_length;
			byte *m_buffer;
		public:
			virtual ~NativeBufferWrapper()
			{
			}

			STDMETHODIMP RuntimeClassInitialize(byte *buffer, UINT32 totalSize)
			{
				m_length = totalSize;
				m_buffer = buffer;

				return S_OK;
			}

			STDMETHODIMP RuntimeClassInitialize()
			{
				m_length = 0;
				m_buffer = nullptr;

				return S_OK;
			}

			STDMETHODIMP Buffer(byte **value)
			{
				*value = m_buffer;

				return S_OK;
			}

			STDMETHODIMP get_Capacity(UINT32 *value)
			{
				*value = m_length;

				return S_OK;
			}

			STDMETHODIMP get_Length(UINT32 *value)
			{
				*value = m_length;

				return S_OK;
			}

			STDMETHODIMP put_Length(UINT32 value)
			{
				m_length = value;
				return S_OK;
			}


		};
		ActivatableClass(NativeBufferWrapper);
	}
}

inline Windows::Storage::Streams::IBuffer^ WrapNativeBuffer(void* lpBuffer, uint32 nNumberOfBytes)
{
	Microsoft::WRL::ComPtr<ABI::FlacPP::NativeBufferWrapper> nativeBuffer;
	Microsoft::WRL::Details::MakeAndInitialize<ABI::FlacPP::NativeBufferWrapper>(&nativeBuffer, (byte *)lpBuffer, nNumberOfBytes);
	auto iinspectable = (IInspectable *)reinterpret_cast<IInspectable *>(nativeBuffer.Get());
	Windows::Storage::Streams::IBuffer ^buffer = reinterpret_cast<Windows::Storage::Streams::IBuffer ^>(iinspectable);

	return buffer;
}

void BufferedInputStream::seekForward(std::uint64_t amount)
{
	if (amount < static_cast<std::uint64_t>(_loadedData - _positionInBuffer)) {
		_positionInBuffer += static_cast<std::uint32_t>(amount);
		_absolutePosition += static_cast<std::uint32_t>(amount);
	}
	else {
		_loadedData = 0;
		_positionInBuffer = 0;
		_innerStream->Seek(_absolutePosition + amount);
		_positionInInnerStream = _absolutePosition + amount;
		_absolutePosition = _absolutePosition + amount;
	}
}

void BufferedInputStream::seekBackward(std::uint64_t amount)
{
	if (amount < static_cast<std::uint64_t>(_positionInBuffer)) {
		_positionInBuffer -= static_cast<std::uint32_t>(amount);
		_absolutePosition -= static_cast<std::uint32_t>(amount);
	}
	else {

		_loadedData = 0;
		_positionInBuffer = 0;
		_innerStream->Seek(_absolutePosition - amount);
		_positionInInnerStream = _absolutePosition - amount;
		_absolutePosition = _absolutePosition - amount;
	}
}

BufferedInputStream::BufferedInputStream(Windows::Storage::Streams::IRandomAccessStream ^ innerStream, std::uint64_t streamLength, std::uint32_t bufferLength)
	: _innerStream(innerStream),
	_innerStreamLength(streamLength),
	_positionInInnerStream(0),
	_absolutePosition(0),
	_positionInBuffer(0),
	_loadedData(0),
	_bufferLength(bufferLength),
	_buffer(new std::uint8_t[bufferLength])
{
}

std::uint32_t BufferedInputStream::read(std::uint8_t buffer[], std::uint32_t length)
{
	if (isEof()){
		return 0;
	}
	if (length < _loadedData - _positionInBuffer) {
		memcpy(buffer, _buffer.get() + _positionInBuffer, length);
		_positionInBuffer += length;
		_absolutePosition += length;
		return length;
	}
	if (_loadedData - _positionInBuffer >0) {
		std::uint32_t thisStepRead = _loadedData - _positionInBuffer;
		memcpy(buffer, _buffer.get() + _positionInBuffer, thisStepRead);
		_absolutePosition += thisStepRead;
		_positionInBuffer += thisStepRead;
		if (isEof()) {
			return thisStepRead;
		}
		size_t nextStepBytes = length - thisStepRead;		
		return thisStepRead + read(buffer + thisStepRead, nextStepBytes);
	}

	_positionInBuffer = 0;
	_loadedData = 0;
	auto readBuffer = concurrency::create_task(_innerStream->ReadAsync(WrapNativeBuffer(_buffer.get(), _bufferLength), _bufferLength, Windows::Storage::Streams::InputStreamOptions::None)).get();
	_loadedData = readBuffer->Length;
	_positionInInnerStream += _loadedData;
	return read(buffer, length);
}

void BufferedInputStream::seek(std::uint64_t absolutePos)
{
	if (absolutePos > _absolutePosition)
	{
		seekForward(absolutePos - _absolutePosition);
	}
	else if (absolutePos < _absolutePosition) {
		seekBackward(_absolutePosition - absolutePos);
	}
}

void BufferedInputStream::readIntoBuffer(std::uint32_t size, FlacPP::FlacBufferView & buffer)
{
	auto oldLength = buffer.length();
	auto effectiveRead = read(buffer.end(), size);
	buffer.length(oldLength+effectiveRead);
}

std::uint8_t FlacPP::BufferedInputStream::readOneByte()

{
	if (isEof()) {
		return 0;
	}
	if (_loadedData == _positionInBuffer) {
		_positionInBuffer = 0;
		_loadedData = 0;
		auto readBuffer = concurrency::create_task(_innerStream->ReadAsync(WrapNativeBuffer(_buffer.get(), _bufferLength), _bufferLength, Windows::Storage::Streams::InputStreamOptions::None)).get();
		_loadedData = readBuffer->Length;
		_positionInInnerStream += _loadedData;
	}
	if (_loadedData == _positionInBuffer) {
		return 0;
	}
	++_absolutePosition;
	return _buffer[_positionInBuffer++];
}

#endif