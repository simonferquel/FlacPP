//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include <FlacPP/FlacDecoder.h>
#include <FlacPP/FlacStream.h>
#include <robuffer.h>
#include <windows.storage.streams.h>
#include <wrl.h>
#include "FlacMediaStreamSourceProvider.h"
#include <sstream>
#include "ProducerConsumerQueue.h"
#include <thread>

using namespace SampleApp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Media::Core;
using namespace Microsoft::WRL;
// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

MainPage::MainPage()
{
	InitializeComponent();
}




void SampleApp::MainPage::OnLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	concurrency::create_task(Windows::ApplicationModel::Package::Current->InstalledLocation->GetFolderAsync(L"Samples"))
		.then([this](StorageFolder^ sampleFolder) {
		concurrency::create_task(sampleFolder->GetFilesAsync())
			.then([this](IVectorView<StorageFile^>^ files) {
			for (auto file : files) {
				auto fi = ref new FlacFileInfo();
				fi->File = file;
				fi->FileName = file->Name;
				this->lv->Items->Append(fi);
			}
		});
	});
}





void SampleApp::MainPage::OnItemClick(Platform::Object^ sender, Windows::UI::Xaml::Controls::ItemClickEventArgs^ e)
{
	auto fi = dynamic_cast<FlacFileInfo^>(e->ClickedItem);
	if (!fi) {
		return;
	}

	/*auto picker = ref new Windows::Storage::Pickers::FileSavePicker();
	picker->FileTypeChoices->Insert(L"raw", ref new Platform::Collections::Vector<Platform::String^>{ L".raw" });
	concurrency::create_task(picker->PickSaveFileAsync())
		.then([this](Windows::Storage::StorageFile^ file) {
		return concurrency::create_task(file->OpenAsync(FileAccessMode::ReadWrite));
	}).then([this](IRandomAccessStream^ ras) {
		this->_debugStream = ras->GetOutputStreamAt(0);
	}).then([this,fi]() {*/

	
	concurrency::create_task(fi->File->OpenReadAsync())
		.then([](IRandomAccessStreamWithContentType^ ras) {
		return concurrency::create_task([ras]() {
			std::unique_ptr<FlacPP::IFlacStream> stream = std::make_unique<FlacPP::BufferedInputStream>(ras, ras->Size);

			return std::make_shared<FlacPP::FlacDecoder>(std::move(stream));

		});
	}).then([this](const std::shared_ptr<FlacPP::FlacDecoder>& decoder) {
		FlacMediaStreamSourceProvider mssp(decoder);
		return mssp.CreateMediaStreamSource();
	}).then([this](MediaStreamSource^ mss) {
		this->me->SetMediaStreamSource(mss);
	/*}).then([this](std::wstring message) {
		auto dlg = ref new Windows::UI::Popups::MessageDialog(ref new Platform::String(message.c_str()));
		dlg->ShowAsync();*/
	}, concurrency::task_continuation_context::use_current());

	/*});*/
}
