#include "dialogs.h"

#include <map>
#include <string>
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

struct filterSpecTemp {
	std::wstring name;
	std::wstring spec;
};

static void fileDialogCommon(IFileDialog* dialog, struct wl_FileDialogOpts* opts, bool isSave)
{
	// get default options
	DWORD dwFlags;
	HR(dialog->GetOptions(&dwFlags));

	if (!isSave) {
		if (opts->mode == wl_FileDialogOpts::kModeMultiFile) {
			dwFlags |= FOS_ALLOWMULTISELECT;
		}
		else if (opts->mode == wl_FileDialogOpts::kModeFolder) {
			dwFlags |= FOS_PICKFOLDERS;
		}
	}

	// file system items only
	dwFlags |= FOS_FORCEFILESYSTEM;

	HR(dialog->SetOptions(dwFlags));

	if (opts->mode != wl_FileDialogOpts::kModeFolder) {
		// file stuff only:

		// file types
		filterSpecTemp* temps = new filterSpecTemp[opts->numFilters];
		COMDLG_FILTERSPEC* specs = new COMDLG_FILTERSPEC[opts->numFilters];
		for (int i = 0; i < opts->numFilters; i++) {
			temps[i].name = utf8_to_wstring(opts->filters[i].desc);
			temps[i].spec = utf8_to_wstring(opts->filters[i].exts);
			specs[i].pszName = temps[i].name.c_str();
			specs[i].pszSpec = temps[i].spec.c_str();
		}
		HR(dialog->SetFileTypes(opts->numFilters, specs));
		HR(dialog->SetFileTypeIndex(1)); // 1-based index

		if (opts->defaultExt) {
			std::wstring defExts;
			defExts = utf8_to_wstring(opts->defaultExt);
			HR(dialog->SetDefaultExtension(defExts.c_str()));
		}

		// safe to do this before showing? guess we'll find out ...
		delete[] specs;
		delete[] temps;
	}
}

OPENWL_API bool CDECL wl_FileOpenDialog(struct wl_FileDialogOpts* opts, struct wl_FileResults** results)
{
	IFileOpenDialog* dialog = NULL;
	HR(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))); // IID_PPV_ARGS is a macro that handles final 2 args

	fileDialogCommon(dialog, opts, false);

	// zero out the results in case of failure
	*results = nullptr;
	bool retval = false;

	auto hwnd = opts->owner ? opts->owner->getHWND() : NULL;
	auto hr = dialog->Show(hwnd);
	if (SUCCEEDED(hr)) {
		IShellItemArray* itemArr;
		HR(dialog->GetResults(&itemArr));

		DWORD numItems;
		HR(itemArr->GetCount(&numItems));

		// allocate results
		*results = new wl_FileResults;
		(*results)->numResults = numItems;
		(*results)->results = new const char* [numItems];

		for (DWORD i = 0; i < numItems; i++) {
			IShellItem* item;
			HR(itemArr->GetItemAt(i, &item));

			PWSTR filePath = NULL;
			HR(item->GetDisplayName(SIGDN_FILESYSPATH, &filePath));

			auto utf8 = wstring_to_utf8(filePath);
			(*results)->results[i] = _strdup(utf8.c_str());

			CoTaskMemFree(filePath);
			SafeRelease(&item);
		}

		retval = true;

		SafeRelease(&itemArr);
	}
	else if (hr == 0x800704C7) {
		printf("file open canceled by user\n");
	}
	else {
		printf("file open dialog - some unknown error %08X\n", hr);
	}

	SafeRelease(&dialog);
	return retval;
}

OPENWL_API bool CDECL wl_FileSaveDialog(struct wl_FileDialogOpts* opts, struct wl_FileResults** results)
{
	IFileSaveDialog* dialog = NULL;
	HR(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))); // IID_PPV_ARGS is a macro that handles final 2 args

	fileDialogCommon(dialog, opts, true);

	// zero out the results in case of failure
	*results = nullptr;
	bool retval = false;

	auto hwnd = opts->owner ? opts->owner->getHWND() : NULL;
	auto hr = dialog->Show(hwnd);
	if (SUCCEEDED(hr)) {
		IShellItem* item;
		HR(dialog->GetResult(&item));

		PWSTR filePath = NULL;
		HR(item->GetDisplayName(SIGDN_FILESYSPATH, &filePath));

		// allocate results
		*results = new wl_FileResults;
		(*results)->numResults = 1;
		(*results)->results = new const char* [1];

		auto utf8 = wstring_to_utf8(filePath);
		(*results)->results[0] = _strdup(utf8.c_str());

		retval = true;

		CoTaskMemFree(filePath);
		SafeRelease(&item);
	}
	else if (hr == 0x800704C7) {
		printf("file open/save canceled by user\n");
	}
	else {
		printf("file dialog - some unknown error %08X\n", hr);
	}

	SafeRelease(&dialog);
	return retval;
}

OPENWL_API void CDECL wl_FileResultsFree(struct wl_FileResults** results)
{
	auto x = *results;
	if (x) {
		for (int i = 0; i < x->numResults; i++) {
			free((void *)x->results[i]); // allocated by strdup
		}
		delete[] x->results;
		delete x;
	}
	*results = nullptr;
}
