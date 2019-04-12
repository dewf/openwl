#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

HBITMAP loadPngAndResize(const char *filename, int maxWidth, int maxHeight);

