#pragma once
namespace FlacPlayer {

	inline Platform::String^ MessageType() {
		return ref new Platform::String( L"MessageType");
	}
	inline Platform::String^ MessageType_ForeGroundToBackgroundRequest() {
		return ref new Platform::String(L"ForeGroundToBackgroundRequest");
	}

	inline Platform::String^ MessageType_ForeGroundToBackgroundResponse() {
		return ref new Platform::String(L"ForeGroundToBackgroundResponse");
	}

	inline Platform::String^ MessageAction() {
		return ref new Platform::String(L"Action");
	}
	inline Platform::String^ MessageAction_PlayFile() {
		return ref new Platform::String(L"PlayFile");
	}

	inline Platform::String^ MessageAction_Ping() {
		return ref new Platform::String(L"Ping");
	}
	inline Platform::String^ MessageArg0() {
		return ref new Platform::String(L"Arg0");
	}
}