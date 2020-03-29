#include "dialogs.h"

#include <map>
#include "unicodestuff.h"
#include "window.h"

// file open/save stuff
#include <shobjidl_core.h>
#include "comstuff.h"

static std::map<wl_MessageBoxParams::Buttons, UINT> mbButtonMap;
static std::map<wl_MessageBoxParams::Icon, UINT> mbIconMap;
static std::map<int, wl_MessageBoxParams::Result> mbResultMap;

void dialogs_init() {
	// buttons
	mbButtonMap[wl_MessageBoxParams::kButtonsDefault] = MB_OK;
	mbButtonMap[wl_MessageBoxParams::kButtonsAbortRetryIgnore] = MB_ABORTRETRYIGNORE;
	mbButtonMap[wl_MessageBoxParams::kButtonsCancelTryContinue] = MB_CANCELTRYCONTINUE;
	mbButtonMap[wl_MessageBoxParams::kButtonsOk] = MB_OK;
	mbButtonMap[wl_MessageBoxParams::kButtonsOkCancel] = MB_OKCANCEL;
	mbButtonMap[wl_MessageBoxParams::kButtonsRetryCancel] = MB_RETRYCANCEL;
	mbButtonMap[wl_MessageBoxParams::kButtonsYesNo] = MB_YESNO;
	mbButtonMap[wl_MessageBoxParams::kButtonsYesNoCancel] = MB_YESNOCANCEL;

	// icons
	mbIconMap[wl_MessageBoxParams::kIconDefault] = MB_ICONINFORMATION;
	mbIconMap[wl_MessageBoxParams::kIconWarning] = MB_ICONWARNING;
	mbIconMap[wl_MessageBoxParams::kIconInformation] = MB_ICONINFORMATION;
	mbIconMap[wl_MessageBoxParams::kIconQuestion] = MB_ICONQUESTION;
	mbIconMap[wl_MessageBoxParams::kIconError] = MB_ICONERROR;

	// TODO maybe someday: button order, modal options

	// results
	mbResultMap[IDABORT] = wl_MessageBoxParams::kResultAbort;
	mbResultMap[IDCANCEL] = wl_MessageBoxParams::kResultCancel;
	mbResultMap[IDCONTINUE] = wl_MessageBoxParams::kResultContinue;
	mbResultMap[IDIGNORE] = wl_MessageBoxParams::kResultIgnore;
	mbResultMap[IDNO] = wl_MessageBoxParams::kResultNo;
	mbResultMap[IDOK] = wl_MessageBoxParams::kResultOk;
	mbResultMap[IDRETRY] = wl_MessageBoxParams::kResultRetry;
	mbResultMap[IDTRYAGAIN] = wl_MessageBoxParams::kResultTryAgain;
	mbResultMap[IDYES] = wl_MessageBoxParams::kResultYes;
}

OPENWL_API wl_MessageBoxParams::Result CDECL wl_MessageBox(wl_WindowRef window, wl_MessageBoxParams* params)
{
	UINT mbType =
		mbButtonMap[params->buttons] |
		mbIconMap[params->icon];

	if (params->withHelpButton) {
		mbType |= MB_HELP;
	}

	auto wideMessage = utf8_to_wstring(params->message);
	auto wideTitle = utf8_to_wstring(params->title);

	auto rawResult =
		MessageBox(
			window ? window->getHWND() : NULL,
			wideMessage.c_str(),
			wideTitle.c_str(),
			mbType
			);

	return mbResultMap[rawResult];
}

OPENWL_API bool CDECL wl_FileOpenDialog(wl_WindowRef owner)
{
	IFileOpenDialog* dialog = NULL;
	HR(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))); // IID_PPV_ARGS is a macro that handles final 2 args

	// get existing options
	DWORD dwFlags;
	HR(dialog->GetOptions(&dwFlags));

	// multi select if wanted
	dwFlags |= FOS_ALLOWMULTISELECT;

	// file system items only
	HR(dialog->SetOptions(dwFlags | FOS_FORCEFILESYSTEM));

	// file types
	COMDLG_FILTERSPEC rgSpec[] = {
		{ L"JPEG Images", L"*.jpg;*.jpeg" },
		{ L"PNG Images", L"*.png" },
		{ L"All Files", L"*.*" }
	};
	HR(dialog->SetFileTypes(3, rgSpec));
	HR(dialog->SetFileTypeIndex(1)); // 1-based index
	HR(dialog->SetDefaultExtension(L"jpg;jpeg"));

	auto hwnd = owner ? owner->getHWND() : NULL;

	auto hr = dialog->Show(hwnd);
	if (SUCCEEDED(hr)) {
		IShellItemArray* results;
		HR(dialog->GetResults(&results));

		DWORD numItems;
		HR(results->GetCount(&numItems));

		for (auto i = 0; i < numItems; i++) {
			IShellItem* item;
			HR(results->GetItemAt(i, &item));

			PWSTR filePath = NULL;
			HR(item->GetDisplayName(SIGDN_FILESYSPATH, &filePath));
			printf("you opened: [%ls]\n", filePath);

			CoTaskMemFree(filePath);
			SafeRelease(&item);
		}

		SafeRelease(&results);
	}
	else if (hr == 0x800704C7) {
		printf("file open canceled by user\n");
	}
	else {
		printf("file open dialog - some unknown error %08X\n", hr);
	}

	SafeRelease(&dialog);
	return false;
}

OPENWL_API bool CDECL wl_FileSaveDialog()
{
	return false;
}
