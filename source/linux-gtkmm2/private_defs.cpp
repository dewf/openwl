//
// Created by dang on 2/11/18.
//

#include "private_defs.h"

#include <cstring>

std::map<wl_CursorStyle, wl_CursorRef> cursorMap;


// ugly but can't be arsed to deal with the boost/std regex docs
std::vector<std::string> split_items(char *uri_chars, size_t dataSize) {
    char *last_start = uri_chars;
    char last_char = 0;
    std::vector<std::string> items;
    for (int i=0; i< dataSize; i++) {
        if (uri_chars[i] != '\0') {
            if (uri_chars[i] != '\r' && uri_chars[i] != '\n') {
                // append to existing string (or start a new one)
                if (last_char == 0) {
                    last_start = &uri_chars[i];
                } else {
                    // continue with current
                }
            } else {
                // end of current -- \r or \n
                uri_chars[i] = '\0';
                if (last_char != '\0') { // only do once (on \r if it's there, or \r\n)
                    items.emplace_back(std::string(last_start));
                }
            }
        } else {
            break;
        }
        last_char = uri_chars[i];
    }
    return items;
}

bool wl_DropData::getFiles(const struct wl_Files **outFiles)
{
    const void *uri_list;
    size_t dataSize;
    getFormat(wl_kDragFormatFiles, &uri_list, &dataSize);
//    dropData->getData(targetFromFormat(WLDragFormat_Files), &uri_list, &dataSize);

    auto uri_chars = (char *) uri_list;
    uri_chars[dataSize-1] = 0; // verify null term -- sometimes it seems like a bigger block was allocated?

    auto items = split_items(uri_chars, dataSize);
    auto numFiles = (int)items.size();

    files = new wl_FilesInternal(numFiles);
    for (int i=0; i< numFiles; i++) {
        files->filenames[i] = strdup(items[i].c_str());
    }

    // not really necessary, but we can release the internal data since we're not using it
    free(data);
    data = nullptr;

    // export (temporarily, for the life of this object)
    *outFiles = files;

//    files->numFiles = (int)items.size();
//    files->filenames = new char *[files->numFiles];
//    for (int i=0; i< files->numFiles; i++) {
//        auto item = items[i].c_str();
//        auto len = strlen(item);
//        files->filenames[i] = new char[len+1];
//        strcpy(files->filenames[i], item);
//    }
//
//    dropData->releaseData(&uri_list);

    return true; // ?? where to check if it succeeded or not ?
}



