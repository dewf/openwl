#include "pngloader.h"
#include <Ole2.h>
#include <objbase.h>
#include <wincodec.h>

#include "unicodestuff.h"

static bool initialized = false;
static IWICImagingFactory *wicFactory;
static HRESULT hr;

void initialize()
{
    // Create WIC factory
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory)
    );
    if (!SUCCEEDED(hr)) {
        printf("CoCreateInstance(CLSID_WICImagingFactory) failed\n");
        return;
    }
    initialized = true;
}

#include <assert.h>
void resizeToFit(UINT *width, UINT *height, UINT maxWidth, UINT maxHeight)
{
    double sourceAspect = (double)*width / (double)*height;
    double targetAspect = (double)maxWidth / (double)maxHeight;
    if (sourceAspect <= targetAspect) {
        // clamp height, width will be OK
        *height = maxHeight;
        *width = UINT(sourceAspect * maxHeight);
        assert(*width <= maxWidth);
    }
    else {
        // clamp width, height will be OK
        *width = maxWidth;
        *height = UINT(maxWidth / sourceAspect);
        assert(*height <= maxHeight);
    }
}

void printGuid(const char *format, GUID guid) {
    wchar_t szGUID[64] = { 0 };
    StringFromGUID2(guid, szGUID, 64);
    printf(format, wstring_to_utf8(szGUID).c_str());
}

void InitBitmapInfo(__out_bcount(cbInfo) BITMAPINFO *pbmi, ULONG cbInfo, LONG cx, LONG cy, WORD bpp)
{
    ZeroMemory(pbmi, cbInfo);
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biCompression = BI_RGB;

    pbmi->bmiHeader.biWidth = cx;
    pbmi->bmiHeader.biHeight = cy;
    pbmi->bmiHeader.biBitCount = bpp;
}

HRESULT Create32BitHBITMAP(HDC hdc, const SIZE *psize, __deref_opt_out void **ppvBits, __out HBITMAP* phBmp)
{
    *phBmp = NULL;

    BITMAPINFO bmi;
    InitBitmapInfo(&bmi, sizeof(bmi), psize->cx, psize->cy, 32);

    HDC hdcUsed = hdc ? hdc : GetDC(NULL);
    if (hdcUsed)
    {
        *phBmp = CreateDIBSection(hdcUsed, &bmi, DIB_RGB_COLORS, ppvBits, NULL, 0);
        if (hdc != hdcUsed)
        {
            ReleaseDC(NULL, hdcUsed);
        }
    }
    return (NULL == *phBmp) ? E_OUTOFMEMORY : S_OK;
}

HBITMAP loadPngAndResize(const char *filename, int maxWidth, int maxHeight)
{
    if (!initialized) initialize();

    HRESULT hr = S_OK;

    IWICBitmapDecoder *decoder = NULL;
    IWICBitmapFrameDecode *frameDecoder = NULL;
    IWICBitmapScaler *scaler = NULL;

    SIZE hbSize; // have to declare up here to avoid all the complaints thanks to gotos.

    auto fname_wide = utf8_to_wstring(filename);

	HBITMAP result = NULL;

    hr = wicFactory->CreateDecoderFromFilename(
        fname_wide.c_str(),                   // Image to be decoded
        NULL,                           // Do not prefer a particular vendor
        GENERIC_READ,                   // Desired read access to the file
        WICDecodeMetadataCacheOnDemand, // Cache metadata when needed
        &decoder                      // Pointer to the decoder
    );
	if (SUCCEEDED(hr)) {
		// get first (probably only) frame
		hr = decoder->GetFrame(0, &frameDecoder);
		if (SUCCEEDED(hr)) {
			// create scaler
			UINT width, height;
			frameDecoder->GetSize(&width, &height);
			resizeToFit(&width, &height, maxWidth, maxHeight);
			hr = wicFactory->CreateBitmapScaler(&scaler);
			if (SUCCEEDED(hr)) {
				// init scaler with a specific size
				hr = scaler->Initialize(
					frameDecoder,
					width,
					height,
					WICBitmapInterpolationModeFant);
				if (SUCCEEDED(hr)) {
					// create HBITMAP with premultiplied alpha
					IWICBitmapSource *premultiplied;
					hr = WICConvertBitmapSource(
						GUID_WICPixelFormat32bppPBGRA,
						scaler,
						&premultiplied);
					if (SUCCEEDED(hr)) {
						// alloc DIB and copy stuff
						hbSize = { (int)width, -(int)height };
						BYTE *pbBuffer;
						HBITMAP outBitmap;
						hr = Create32BitHBITMAP(NULL, &hbSize, (void **)&pbBuffer, &outBitmap);
						if (SUCCEEDED(hr)) {
							// finally, copy data
							const UINT cbStride = width * sizeof(DWORD);
							const UINT cbBuffer = height * cbStride;
							hr = premultiplied->CopyPixels(NULL, cbStride, cbBuffer, pbBuffer);
							if (SUCCEEDED(hr)) {
								// success!!
								result = outBitmap;
							}
							else {
								printf("CopyPixels failed\n");
								// have to delete in the failure branch (vs. one level out), because it needs to be preserved in the success branch
								DeleteObject(outBitmap);
							}
						}
						else {
							printf("failed creating HBitmap\n");
						}
						premultiplied->Release();
					}
					else {
						printf("WICConvertBitmapSource (to 32-PBGRA) failed\n");
						//goto ReleaseScaler;
					}
					// nothing to release for a successful init
				}
				else {
					printf("scaler init failed\n");
				}
				scaler->Release();
			}
			else {
				printf("failed to create scaler\n");
			}
			frameDecoder->Release();
		}
		else {
			printf("failed to get first frame from decoder\n");
		}
		decoder->Release();
	}
	else {
		printf("failed to create decoder for file [%s]\n", filename);
	}
	return result; // either NULL or HBITMAP (outBitmap)
}


