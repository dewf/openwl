#pragma once

// DPI macros
#define DECLSF(dpi) double scaleFactor = (dpi) / 96.0;
#define INT(x) ((int)(x))
#define DPIUP(x) INT((x) * scaleFactor)                // from device-independent pixels to physical res
#define DPIDOWN(x) INT((x) / scaleFactor)              // from physical res to DIPs
#define DPIUP_INPLACE(x) x = DPIUP(x);
#define DPIDOWN_INPLACE(x) x = DPIDOWN(x);
