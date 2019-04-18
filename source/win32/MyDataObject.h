#pragma once

#include "dragdrop/dataobject.h"
#include "../openwl.h"

#include <map>
#include <string>

// custom IDataObject for using with a drag source
class MyDataObject : public CDataObject {
private:
    std::map<std::string, FORMATETC> mFormats;
    wl_Window mWindow = 0;
protected:
    bool fmtMatch(FORMATETC *fmt1, FORMATETC *fmt2);
    bool canRenderData(FORMATETC *pFormatEtc) override;
    bool renderFormat(FORMATETC *pFormatEtc, STGMEDIUM *pMedium) override;
    void enumFormats(FORMATETC **formats, LONG *numFormats) override;
public:
    MyDataObject(wl_Window window);
    ~MyDataObject();
    // outgoing formats
    void addDragFormat(const char *dragFormat);
};
