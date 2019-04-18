#include "MyDataObject.h"

#include "../openwl.h"
#include <map>
#include "globals.h"

#include "private_defs.h"

#include <cstring>

bool MyDataObject::fmtMatch(FORMATETC *fmt1, FORMATETC *fmt2) {
    return (fmt1->tymed & fmt2->tymed) &&
        (fmt1->cfFormat == fmt2->cfFormat) &&
        (fmt1->dwAspect == fmt2->dwAspect);
}

bool MyDataObject::canRenderData(FORMATETC *pFormatEtc) {
    for (auto kv : mFormats) {
        if (fmtMatch(pFormatEtc, &kv.second)) {
            //printf("can render: %s\n", kv.first.c_str());
            return true;
        }
    }
    return false;
}

bool MyDataObject::renderFormat(FORMATETC *pFormatEtc, STGMEDIUM *pMedium) {
    for (auto kv : mFormats) {
        auto fmtetc = kv.second;
        if (fmtMatch(pFormatEtc, &fmtetc)) {
            auto dragFormat = kv.first;
            //

			wl_RenderPayload payload; // used as render target

            wl_Event event;
            event._private = nullptr;
            event.eventType = wl_kEventTypeDragRender;
            event.dragRenderEvent.dragFormat = dragFormat.c_str();
            event.dragRenderEvent.payload = &payload;
            eventCallback(mWindow, &event, mWindow->userData);
            //
			if (event.handled) {
				pMedium->tymed = fmtetc.tymed;
				pMedium->pUnkForRelease = 0;
				if (pMedium->tymed == TYMED_HGLOBAL) {
					// pMedium->hGlobal = ...
					pMedium->hGlobal = GlobalAlloc(GHND, payload.size);
					auto ptr = GlobalLock(pMedium->hGlobal);
					memcpy(ptr, payload.data, payload.size);
					GlobalUnlock(pMedium->hGlobal);

					return true;
				}
				// else not supported yet
				// TODO: free dragRenderEvent.data ... do create a wlGlobalAlloc/Lock/Unlock() for clipboard data
			}
            break;
        }
    }
    return false;
}


void MyDataObject::enumFormats(FORMATETC **formats, LONG *numFormats) {
    *numFormats = (LONG)mFormats.size();
    *formats = new FORMATETC[mFormats.size()];
    int i = 0;
    for (auto kv : mFormats) {
        (*formats)[i++] = kv.second;
    }
}

MyDataObject::MyDataObject(wl_WindowRef window)
    : CDataObject()
{
    mWindow = window;
}

MyDataObject::~MyDataObject() {
    printf("MyDataObject destructor called\n");
}

void MyDataObject::addDragFormat(const char *dragFormat) {
    FORMATETC fmtetc;
	if (!strcmp(dragFormat, wl_kDragFormatUTF8)) {
		fmtetc = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	}
	else {
		printf("unknown format for MyDataObject::addDragFormat\n");
		return;
	}
	mFormats[dragFormat] = fmtetc;
}
