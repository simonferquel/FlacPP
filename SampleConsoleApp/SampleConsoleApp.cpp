// SampleConsoleApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <wrl.h>
#include <xaudio2.h>
#include <flacPP\FlacStream.h>
#include <flacPP\FlacDecoder.h>
#include <fstream>
#include <iostream>

using namespace Microsoft::WRL;

class cb : public IXAudio2VoiceCallback{
private:
	HANDLE _ev;
public:
	cb(HANDLE ev):_ev(ev){}

	// Inherited via IXAudio2VoiceCallback
	virtual void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) override
	{
	}
	virtual void __stdcall OnVoiceProcessingPassEnd(void) override
	{
	}
	virtual void __stdcall OnStreamEnd(void) override
	{
	}
	virtual void __stdcall OnBufferStart(void * pBufferContext) override
	{
		SetEvent(_ev);
	}
	virtual void __stdcall OnBufferEnd(void * pBufferContext) override
	{
	}
	virtual void __stdcall OnLoopEnd(void * pBufferContext) override
	{
	}
	virtual void __stdcall OnVoiceError(void * pBufferContext, HRESULT Error) override
	{
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	std::unique_ptr<std::ifstream> s = std::make_unique<std::ifstream>(argv[1], std::ifstream::binary);
	auto stream = std::make_unique<FlacPP::FlacStreamOveristream>(std::move(s));
	FlacPP::FlacDecoder decoder(std::move(stream));
	CoInitialize(nullptr);
	ComPtr<IXAudio2> xaudio2;
	auto hr = XAudio2Create(&xaudio2);
	
	hr = xaudio2->StartEngine();
	IXAudio2MasteringVoice* masteringVoice;

	hr = xaudio2->CreateMasteringVoice(&masteringVoice, decoder.streamInfo().channels, decoder.streamInfo().sampleRate, 0,0, nullptr, AudioCategory_Media);
	
	IXAudio2SourceVoice* srcVoice;
	WAVEFORMATEX wfx;
	ZeroMemory(&wfx, sizeof(wfx));
	wfx.cbSize = sizeof(WAVEFORMATEX);
	wfx.nAvgBytesPerSec = decoder.streamInfo().channels * decoder.streamInfo().sampleRate * decoder.streamInfo().outputBitsPerSample / 8;
	wfx.nChannels = decoder.streamInfo().channels;
	wfx.nSamplesPerSec = decoder.streamInfo().sampleRate;
	wfx.wBitsPerSample = decoder.streamInfo().outputBitsPerSample;
	wfx.nBlockAlign = decoder.streamInfo().channels * decoder.streamInfo().outputBitsPerSample / 8;
	wfx.wFormatTag = WAVE_FORMAT_PCM;

	auto ev = CreateEvent(nullptr, TRUE, TRUE, nullptr);
	cb callback(ev);

	hr = xaudio2->CreateSourceVoice(&srcVoice, &wfx, 0, 1.0f, &callback);
	XAUDIO2_VOICE_SENDS os;
	os.SendCount = 1;
	XAUDIO2_SEND_DESCRIPTOR o;
	o.Flags = 0;
	o.pOutputVoice = masteringVoice;
	os.pSends = &o;
	srcVoice->SetOutputVoices(&os);
	bool first = true;
	std::vector<std::uint8_t> frontbuffer, backbuffer;
	for (;;) {
		FlacPP::time_unit_100ns unused;
		WaitForSingleObject(ev, INFINITE);
		ResetEvent(ev);
		auto decoded = decoder.decodeNextFrame(unused);
		if (decoded.length() == 0) {
			break;
		}
		backbuffer.resize(decoded.length());
		memcpy(&backbuffer[0], decoded.begin(), decoded.length());
		std::swap(backbuffer, frontbuffer);
		XAUDIO2_BUFFER buf;
		ZeroMemory(&buf, sizeof(buf));
			
		buf.AudioBytes = frontbuffer.size();
		buf.pAudioData = &frontbuffer[0];
		
		srcVoice->SubmitSourceBuffer(&buf);
		if (first) {

			srcVoice->Start();
			first = false;
		}
	}
	char c;
	std::cin >> c;
	srcVoice->Stop();
	CloseHandle(ev);
	return 0;
}

