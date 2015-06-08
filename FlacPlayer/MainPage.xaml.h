//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include <ppltasks.h>

namespace FlacPlayer
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	private:
		Windows::Foundation::EventRegistrationToken _bgMediaPlayerStateChangedToken,
			_bgMediaPlayerMessageReceivedToken;

		concurrency::task_completion_event<bool> _currentPingEvent;
		concurrency::task<bool> WakeupBackgroundTask();
		
	public:
		MainPage();
		void OnBackgroundMediaPlayerStateChanged(Windows::Media::Playback::MediaPlayer^ mediaPlayer, Platform::Object^ args);
		void OnBackgroundMediaPlayerMessageReceiver(Object^ mediaPlayer, Windows::Media::Playback::MediaPlayerDataReceivedEventArgs^ args);
	protected:
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
	private:
		void OnOpenClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OpenFile(Windows::Storage::IStorageFile^ file);
		
	};
}
