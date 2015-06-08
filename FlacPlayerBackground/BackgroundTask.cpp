#include "pch.h"
#include "BackgroundTask.h"
#include "..\FlacPlayer\Constants.h"
#include "FlacMediaStreamSourceProvider.h"
#include <flacPP\FlacStream.h>
#include <ppltasks.h>
using namespace Windows::Media;
using namespace Windows::Media::Playback;
using namespace Platform;
using namespace concurrency;

void FlacPlayerBackground::BackgroundTask::Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^taskInstance)
{
	auto details = taskInstance->TriggerDetails;
	_deferral = taskInstance->GetDeferral();

	
	auto def = _deferral;
	taskInstance->Task->Completed += ref new Windows::ApplicationModel::Background::BackgroundTaskCompletedEventHandler([def](Windows::ApplicationModel::Background::BackgroundTaskRegistration^ registration, Windows::ApplicationModel::Background::BackgroundTaskCompletedEventArgs^ args) {

		def->Complete();
	});
	
	taskInstance->Canceled += ref new Windows::ApplicationModel::Background::BackgroundTaskCanceledEventHandler(this, &FlacPlayerBackground::BackgroundTask::OnCanceled);
	_smtc = SystemMediaTransportControls::GetForCurrentView();
	_smtcButtonPressed = _smtc->ButtonPressed += ref new Windows::Foundation::TypedEventHandler<Windows::Media::SystemMediaTransportControls ^, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs ^>(this, &FlacPlayerBackground::BackgroundTask::OnButtonPressed);
	_smtcPropertyChanged = _smtc->PropertyChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Media::SystemMediaTransportControls ^, Windows::Media::SystemMediaTransportControlsPropertyChangedEventArgs ^>(this, &FlacPlayerBackground::BackgroundTask::OnPropertyChanged);
	_smtc->IsEnabled = true;
	_smtc->IsPauseEnabled = true;
	_smtc->IsPlayEnabled = true;
	_smtc->IsNextEnabled = true;
	_smtc->IsPreviousEnabled = true;
	_smtc->PlaybackStatus = MediaPlaybackStatus::Stopped;
	_smtc->DisplayUpdater->ClearAll();
	_smtc->DisplayUpdater->Type = MediaPlaybackType::Music;
	_smtc->DisplayUpdater->Update();
	_player = BackgroundMediaPlayer::Current;
	_player->CurrentStateChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlayer ^, Platform::Object ^>(this, &FlacPlayerBackground::BackgroundTask::OnCurrentStateChanged);
	_player->MediaFailed += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlayer ^, Windows::Media::Playback::MediaPlayerFailedEventArgs ^>(this, &FlacPlayerBackground::BackgroundTask::OnMediaFailed);
	BackgroundMediaPlayer::MessageReceivedFromForeground += ref new Windows::Foundation::EventHandler<Windows::Media::Playback::MediaPlayerDataReceivedEventArgs ^>(this, &FlacPlayerBackground::BackgroundTask::OnMessageReceivedFromForeground);
}


void FlacPlayerBackground::BackgroundTask::OnCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason)
{
	auto r = reason;
	_smtc->ButtonPressed -= _smtcButtonPressed;
	_smtc->PropertyChanged -= _smtcPropertyChanged;
	_deferral->Complete();
}


void FlacPlayerBackground::BackgroundTask::OnMessageReceivedFromForeground(Platform::Object ^sender, Windows::Media::Playback::MediaPlayerDataReceivedEventArgs ^args)
{
	auto messageType = dynamic_cast<String^>(args->Data->Lookup(FlacPlayer::MessageType()));
	if (messageType == FlacPlayer::MessageType_ForeGroundToBackgroundRequest()) {
		auto action = dynamic_cast<String^>(args->Data->Lookup(FlacPlayer::MessageAction()));
		if (action == FlacPlayer::MessageAction_PlayFile()) {
			auto fileToken = dynamic_cast<String^>(args->Data->Lookup(FlacPlayer::MessageArg0()));
			auto smtc = _smtc;
			auto player = _player;
			auto file = create_task(Windows::Storage::AccessCache::StorageApplicationPermissions::FutureAccessList->GetFileAsync(fileToken)).get();
			auto stream = create_task(file->OpenAsync(Windows::Storage::FileAccessMode::Read)).get();

			auto decoder = std::make_shared<FlacPP::FlacDecoder>(std::make_unique<FlacPP::BufferedInputStream>(stream, stream->Size, 128*1024));
			FlacMediaStreamSourceProvider provider(decoder);
			player->AutoPlay = true;
			auto mss = provider.CreateMediaStreamSource();
			_currentMSS = mss;
			smtc->IsEnabled = true;
			smtc->DisplayUpdater->Type = MediaPlaybackType::Music;
			smtc->DisplayUpdater->MusicProperties->Title = L"Flac player";
			smtc->DisplayUpdater->Update();
			player->SetMediaSource(mss);
			player->Play();
			

		}
		else if (action == FlacPlayer::MessageAction_Ping()) {
			auto valueSet = ref new Windows::Foundation::Collections::ValueSet();
			valueSet->Insert(FlacPlayer::MessageType(), FlacPlayer::MessageType_ForeGroundToBackgroundResponse());
			valueSet->Insert(FlacPlayer::MessageAction(), FlacPlayer::MessageAction_Ping());
			BackgroundMediaPlayer::SendMessageToForeground(valueSet);
		}
	}
}


void FlacPlayerBackground::BackgroundTask::OnButtonPressed(Windows::Media::SystemMediaTransportControls ^sender, Windows::Media::SystemMediaTransportControlsButtonPressedEventArgs ^args)
{
	if (args->Button == SystemMediaTransportControlsButton::Pause) {
		_player->Pause();
	}
	else if (args->Button == SystemMediaTransportControlsButton::Play) {
		_player->Play();
	}
}


void FlacPlayerBackground::BackgroundTask::OnPropertyChanged(Windows::Media::SystemMediaTransportControls ^sender, Windows::Media::SystemMediaTransportControlsPropertyChangedEventArgs ^args)
{

}


void FlacPlayerBackground::BackgroundTask::OnCurrentStateChanged(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args)
{
	auto state = sender->CurrentState;
	if (state == MediaPlayerState::Opening) {
		//sender->Play();
	}
	switch (state)
	{
	
	case Windows::Media::Playback::MediaPlayerState::Playing:
		_smtc->PlaybackStatus = MediaPlaybackStatus::Playing;
		break;
	case Windows::Media::Playback::MediaPlayerState::Paused:
		_smtc->PlaybackStatus = MediaPlaybackStatus::Paused;
		break;
	case Windows::Media::Playback::MediaPlayerState::Stopped:
		_smtc->PlaybackStatus = MediaPlaybackStatus::Stopped;
		break;
	default:
		break;
	}
}


void FlacPlayerBackground::BackgroundTask::OnMediaFailed(Windows::Media::Playback::MediaPlayer ^sender, Windows::Media::Playback::MediaPlayerFailedEventArgs ^args)
{
	auto error = args->Error;
	auto errorMsg = args->ErrorMessage;
	auto extCode = args->ExtendedErrorCode;

}
