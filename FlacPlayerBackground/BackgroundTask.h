#pragma once
namespace FlacPlayerBackground {
	public ref class BackgroundTask sealed : public Windows::ApplicationModel::Background::IBackgroundTask
	{
	private:
		Windows::ApplicationModel::Background::BackgroundTaskDeferral^ _deferral;
		Windows::Foundation::EventRegistrationToken _smtcButtonPressed,
			_smtcPropertyChanged;
		Windows::Media::SystemMediaTransportControls^ _smtc;
		Windows::Media::Playback::MediaPlayer^ _player;
		Windows::Media::Core::MediaStreamSource^ _currentMSS;
	public:

		// Inherited via IBackgroundTask
		virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^taskInstance);
		void OnCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason);
		void OnMessageReceivedFromForeground(Platform::Object ^sender, Windows::Media::Playback::MediaPlayerDataReceivedEventArgs ^args);
		void OnButtonPressed(Windows::Media::SystemMediaTransportControls ^sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs ^args);
		void OnPropertyChanged(Windows::Media::SystemMediaTransportControls ^sender, Windows::Media::SystemMediaTransportControlsPropertyChangedEventArgs ^args);
		void OnCurrentStateChanged(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args);
		void OnMediaFailed(Windows::Media::Playback::MediaPlayer ^sender, Windows::Media::Playback::MediaPlayerFailedEventArgs ^args);
	};
}

