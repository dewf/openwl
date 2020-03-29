#include "messagebox.h"

#include <map>
#include "unicodestuff.h"
#include "window.h"

static std::map<wl_MessageBoxParams::Buttons, UINT> mbButtonMap;
static std::map<wl_MessageBoxParams::Icon, UINT> mbIconMap;
//static std::map<wl_MessageBoxParams::DefButton, UINT> mbDefButtonMap;
//static std::map<wl_MessageBoxParams::ModalType, UINT> mbModalMap;
static std::map<int, wl_MessageBoxParams::Result> mbResultMap;

void messagebox_init() {
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

	//// default buttons
	//mbDefButtonMap[wl_MessageBoxParams::kDefButtonDefault] = 0;
	//mbDefButtonMap[wl_MessageBoxParams::kDefButton1] = MB_DEFBUTTON1;
	//mbDefButtonMap[wl_MessageBoxParams::kDefButton2] = MB_DEFBUTTON2;
	//mbDefButtonMap[wl_MessageBoxParams::kDefButton3] = MB_DEFBUTTON3;
	//mbDefButtonMap[wl_MessageBoxParams::kDefButton4] = MB_DEFBUTTON4;

	//// modality
	//mbModalMap[wl_MessageBoxParams::kModalDefault] = 0;
	//mbModalMap[wl_MessageBoxParams::kModalApp] = MB_APPLMODAL;
	//mbModalMap[wl_MessageBoxParams::kModalSystem] = MB_SYSTEMMODAL;
	//mbModalMap[wl_MessageBoxParams::kModalTask] = MB_TASKMODAL;

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
	//mbDefButtonMap[params->defButton] |
	//mbModalMap[params->modalType];

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
