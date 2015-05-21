//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include <FlacPP/FlacDecoder.h>

namespace SampleApp
{

	[Windows::UI::Xaml::Data::Bindable]
	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class FlacFileInfo sealed
	{
	public:
		property Windows::Storage::StorageFile^ File;
		property Platform::String^ FileName;
	};
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	private:
		std::shared_ptr<FlacPP::FlacDecoder> _decoder;
		Windows::Storage::Streams::IOutputStream^ _debugStream;
	public:
		MainPage();

	private:
		void OnOpenClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e);
	};
}
