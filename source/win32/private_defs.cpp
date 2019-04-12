#include "private_defs.h"

#include "unicodestuff.h"

static bool formatEtcFromDragFormat(const char *dragFormatMIME, FORMATETC *fmtetc) {
	if (!strcmp(dragFormatMIME, kWLDragFormatUTF8)) {
		*fmtetc = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	}
	else if (!strcmp(dragFormatMIME, kWLDragFormatFiles)) {
		*fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	}
	else {
		printf("wlDragHasFormat: unknown format [%s]\n", dragFormatMIME);
		return false;
	}
	return true;
}

struct _wlFilesInternal : public WLFiles
{
	_wlFilesInternal(int numFiles)
	{
		this->numFiles = numFiles;
		filenames = new const char *[numFiles];
		for (int i = 0; i < numFiles; i++) {
			filenames[i] = nullptr;
		}
	}
	~_wlFilesInternal() {
		for (int i = 0; i < numFiles; i++) {
			free(const_cast<char *>(filenames[i])); // created with strdup
		}
		delete[] filenames;
	}
};

_wlDropData::~_wlDropData()
{
	printf("wlDropData dtor\n");

	if (recvObject) recvObject->Release();
	if (data) {
		free(const_cast<void *>(data)); // we created it, so OK to free
	}
	if (files) {
		delete files;
	}
}

bool _wlDropData::hasFormat(const char *dragFormatMIME)
{
	// testing for existence, without triggering a render (or whatever might be required on the other end)
	FORMATETC fmtetc;
	if (formatEtcFromDragFormat(dragFormatMIME, &fmtetc)) {
		return recvObject->QueryGetData(&fmtetc) == S_OK ? true : false;
	}
	// else
	printf("### _wlDropData::hasFormat() returning false? format: [%s]\n", dragFormatMIME);
	return false;
}

bool _wlDropData::getFormat(const char *dropFormatMIME, const void ** outData, size_t * outSize)
{
	// force other end to generate data
	if (!strcmp(dropFormatMIME, kWLDragFormatFiles)) {
		printf("wlDropData::getFormat() - must use ::getFiles() for file drops\n");
		return false;
	}

	FORMATETC fmtetc;
	if (!formatEtcFromDragFormat(dropFormatMIME, &fmtetc)) {
		// unknown format
		*outData = nullptr;
		*outSize = 0;
		return false;
	}

	STGMEDIUM stgmed;
	if (recvObject->GetData(&fmtetc, &stgmed) == S_OK) {
		auto ptr = GlobalLock(stgmed.hGlobal);

		void *tempData;
		size_t tempSize;
		if (!strcmp(dropFormatMIME, kWLDragFormatUTF8)) {
			// special handling for strings
			auto utf8 = wstring_to_utf8((wchar_t *)ptr);
			tempSize = strlen(utf8.c_str()) + 1;
			tempData = malloc(tempSize);
			memcpy(tempData, utf8.c_str(), tempSize);
		}
		else {
			tempSize = GlobalSize(stgmed.hGlobal);
			tempData = malloc(tempSize);
			memcpy(tempData, ptr, tempSize);
		}
		*outData = tempData;
		*outSize = tempSize;

		GlobalUnlock(stgmed.hGlobal);
		ReleaseStgMedium(&stgmed);

		return true;
	}
	return false;
}

bool _wlDropData::getFiles(const WLFiles **outFiles)
{
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;
	if (recvObject->GetData(&fmtetc, &stgmed) == S_OK) {
		HDROP dropfiles = (HDROP)GlobalLock(stgmed.hGlobal);

		auto numFiles = DragQueryFile(dropfiles, -1, 0, 0);
		files = new _wlFilesInternal(numFiles);

		for (int i = 0; i < files->numFiles; i++) {
			auto numChars = DragQueryFile(dropfiles, i, nullptr, 0);

			auto buffer = new wchar_t[numChars + 1];
			DragQueryFile(dropfiles, i, buffer, numChars + 1); // not sure if the +1 is needed or not ...

			auto fname_utf8 = wstring_to_utf8(buffer);
			files->filenames[i] = _strdup(fname_utf8.c_str()); // will be freed by the _wlFilesInternal dtor
			//auto utf8_len = fname_utf8.size();
			//files->filenames[i] = new char[utf8_len + 1]; // size indicates number of chars without null
			//strcpy_s(files->filenames[i], utf8_len + 1, fname_utf8.c_str()); // c_str() includes null

			delete buffer;
		}
		GlobalUnlock(stgmed.hGlobal);
		ReleaseStgMedium(&stgmed);

		*outFiles = (const WLFiles *) files;

		return true;
	}
	return false;
}
