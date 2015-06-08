//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "Constants.h"

using namespace FlacPlayer;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::AccessCache;
using namespace Windows::Storage::Pickers;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Media::Playback;
using namespace concurrency;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

concurrency::task<bool> FlacPlayer::MainPage::WakeupBackgroundTask()
{
	_currentPingEvent.set(false);
	_currentPingEvent = concurrency::task_completion_event<bool>();

	auto t = create_task(_currentPingEvent);
	// wakes the task by accessing the BackgroundMediaPlayer::Current property
	// then wait for ping
	auto player = BackgroundMediaPlayer::Current;
	create_task([t]() {
		for (;;) {
			if (t.is_done()) {
				return;
			}

			auto valueSet = ref new Windows::Foundation::Collections::ValueSet();
			valueSet->Insert(MessageType(), MessageType_ForeGroundToBackgroundRequest());
			valueSet->Insert(MessageAction(), MessageAction_Ping());

			BackgroundMediaPlayer::SendMessageToBackground(valueSet);
			WaitForSingleObjectEx(GetCurrentThread(), 100, FALSE);
		}
	});

	return t;
}

MainPage::MainPage()
{
	InitializeComponent();
	WeakReference weakThis(this);
	Loaded += ref new RoutedEventHandler([weakThis](Object^, RoutedEventArgs^) {
		auto that = weakThis.Resolve<MainPage>();
		that->_bgMediaPlayerStateChangedToken = BackgroundMediaPlayer::Current->CurrentStateChanged += ref new TypedEventHandler<MediaPlayer^, Object^>(that, &MainPage::OnBackgroundMediaPlayerStateChanged);
		that->_bgMediaPlayerMessageReceivedToken = BackgroundMediaPlayer::MessageReceivedFromBackground += ref new EventHandler<MediaPlayerDataReceivedEventArgs^>(that, &MainPage::OnBackgroundMediaPlayerMessageReceiver);
	});
	Unloaded += ref new RoutedEventHandler([weakThis](Object^, RoutedEventArgs^) {
		auto that = weakThis.Resolve<MainPage>();
		BackgroundMediaPlayer::Current->CurrentStateChanged -= that->_bgMediaPlayerStateChangedToken;
		BackgroundMediaPlayer::MessageReceivedFromBackground -= that->_bgMediaPlayerMessageReceivedToken;
	});
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	auto asPickerContinuation = dynamic_cast<Windows::ApplicationModel::Activation::FileOpenPickerContinuationEventArgs^>(e->Parameter);
	if (asPickerContinuation) {
		String ^ context = dynamic_cast<String^>( asPickerContinuation->ContinuationData->Lookup(L"OpenFileContext"));
		if (asPickerContinuation->Files->Size > 0) {
			if (context == L"OpenForPlay") {
				OpenFile(asPickerContinuation->Files->First()->Current);
			}
		}
		return;
	}
	auto asFileActivated = dynamic_cast<Windows::ApplicationModel::Activation::FileActivatedEventArgs^>(e->Parameter);
	if (asFileActivated) {
		OpenFile(dynamic_cast<IStorageFile^>(asFileActivated->Files->First()->Current));
	}
}


void FlacPlayer::MainPage::OnOpenClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto picker = ref new FileOpenPicker();
	picker->SuggestedStartLocation = PickerLocationId::MusicLibrary;
	picker->FileTypeFilter->Append(L".flac");
	picker->ContinuationData->Insert(L"OpenFileContext", ref new String(L"OpenForPlay"));
	picker->PickSingleFileAndContinue();
}

void FlacPlayer::MainPage::OpenFile(IStorageFile ^ file)
{
	auto futureAccessList = StorageApplicationPermissions::FutureAccessList;
	auto token = futureAccessList->Add(file);
	WakeupBackgroundTask()
		.then([token](bool succeeded) {
		if (succeeded) {
			auto valueSet = ref new Windows::Foundation::Collections::ValueSet();
			valueSet->Insert(MessageType(), MessageType_ForeGroundToBackgroundRequest());
			valueSet->Insert(MessageAction(), MessageAction_PlayFile());
			valueSet->Insert(MessageArg0(), token);

			BackgroundMediaPlayer::SendMessageToBackground(valueSet);
		}
	});

	

}

void FlacPlayer::MainPage::OnBackgroundMediaPlayerStateChanged(Windows::Media::Playback::MediaPlayer ^ mediaPlayer, Platform::Object ^ args)
{
	auto state = mediaPlayer->CurrentState;
	
}

void FlacPlayer::MainPage::OnBackgroundMediaPlayerMessageReceiver(Object ^ mediaPlayer, Windows::Media::Playback::MediaPlayerDataReceivedEventArgs ^ args)
{
	auto messageType = dynamic_cast<String^>(args->Data->Lookup(MessageType()));
	if (messageType == MessageType_ForeGroundToBackgroundResponse()) {
		auto messageAction =dynamic_cast<String^>(args->Data->Lookup(MessageAction()));
		if (messageAction == MessageAction_Ping()) {
			_currentPingEvent.set(true);
		}
	}
}
