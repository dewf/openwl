#include "dialogs.h"

#include <stdio.h>
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include "WLWindowObject.h"

#include <stdlib.h> // for malloc/free

static bool fileDialogCommon(NSSavePanel *panel, struct wl_FileDialogOpts* opts, struct wl_FileResults **results, bool isSave)
{
    if (!isSave) {
        auto openPanel = (NSOpenPanel *)panel;
        switch (opts->mode) {
            case wl_FileDialogOpts::kModeFile:
                // already the default
                break;
            case wl_FileDialogOpts::kModeMultiFile:
                [openPanel setAllowsMultipleSelection:YES];
                break;
            case wl_FileDialogOpts::kModeFolder:
                [openPanel setCanChooseFiles:NO];
                [openPanel setCanChooseDirectories:YES];
                break;
        }
    }
    
    if (opts->mode != wl_FileDialogOpts::kModeFolder) {
        // file-only stuff:
        NSMutableArray<NSString*> *allowed = [NSMutableArray array];
        for (int i=0; i< opts->numFilters; i++) {
            auto extString = [NSString stringWithUTF8String:opts->filters[i].exts];
            auto items = [extString componentsSeparatedByString:@";"];
            for (NSString *ext in items) {
                auto ext2 = [ext stringByReplacingOccurrencesOfString:@"*." withString:@""]; // remove *. prefix
                [allowed addObject:ext2];
            }
        }
        [panel setAllowedFileTypes:allowed];
        [panel setAllowsOtherFileTypes:YES];
    }
    
    // default return values
    __block bool retval = false;
    *results = nullptr;
    
    auto handler = ^(NSModalResponse result) {
        if (result == NSFileHandlingPanelOKButton) {
            
            NSArray *urls;
            int numItems;
            if (isSave) {
                urls = [NSArray arrayWithObject:[panel URL]];
                numItems = 1;
            } else {
                urls = [(NSOpenPanel *)panel URLs];
                numItems = (int)[urls count];
            }
            
            // allocate results
            *results = new wl_FileResults;
            (*results)->numResults = numItems;
            (*results)->results = new const char *[numItems];

            int i = 0;
            for (NSURL *url in urls) {
                auto cstr = [[[url absoluteString] stringByReplacingOccurrencesOfString:@"file://" withString:@""] UTF8String];
                (*results)->results[i++] = strdup(cstr);
            }
            
            retval = true;
            
            [NSApp stopModalWithCode:0];
        }
    };
    
    if (opts->owner) {
        auto obj = (WLWindowObject *)opts->owner;
        // window sheet
        [panel beginSheetModalForWindow:obj.nsWindow completionHandler:handler];
        [NSApp runModalForWindow:obj.nsWindow];
    } else {
        // app modal
        handler([panel runModal]);
    }

    return retval;
}

OPENWL_API bool CDECL wl_FileOpenDialog(struct wl_FileDialogOpts* opts, struct wl_FileResults** results)
{
    auto panel = [NSOpenPanel openPanel];
    return fileDialogCommon(panel, opts, results, false);
}

OPENWL_API bool CDECL wl_FileSaveDialog(struct wl_FileDialogOpts* opts, struct wl_FileResults** results)
{
    auto panel = [NSSavePanel savePanel];
    return fileDialogCommon(panel, opts, results, true);
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

OPENWL_API wl_MessageBoxParams::Result CDECL wl_MessageBox(wl_WindowRef window, struct wl_MessageBoxParams* params)
{
    NSAlert *alert = [[NSAlert alloc] init];
    
    alert.informativeText = [NSString stringWithUTF8String:params->message]; // autorelease seemed to crash
    alert.messageText = [NSString stringWithUTF8String:params->title];
    alert.showsHelp = params->withHelpButton;
    
    wl_MessageBoxParams::Result buttonRetVals[3];
    switch (params->buttons) {
        case wl_MessageBoxParams::kButtonsAbortRetryIgnore:
            [alert addButtonWithTitle:@"Ignore"];
            [alert addButtonWithTitle:@"Retry"];
            [alert addButtonWithTitle:@"Abort"];
            buttonRetVals[0] = wl_MessageBoxParams::Result::kResultIgnore;
            buttonRetVals[1] = wl_MessageBoxParams::Result::kResultRetry;
            buttonRetVals[2] = wl_MessageBoxParams::Result::kResultAbort;
            break;
        case wl_MessageBoxParams::kButtonsCancelTryContinue:
            [alert addButtonWithTitle:@"Continue"];
            [alert addButtonWithTitle:@"Try Again"];
            [alert addButtonWithTitle:@"Cancel"];
            buttonRetVals[0] = wl_MessageBoxParams::Result::kResultContinue;
            buttonRetVals[1] = wl_MessageBoxParams::Result::kResultTryAgain;
            buttonRetVals[2] = wl_MessageBoxParams::Result::kResultCancel;
            break;
        case wl_MessageBoxParams::kButtonsOk:
            [alert addButtonWithTitle:@"OK"]; // even though it's default, if we don't add it, we don't get the positional value (ends up being 'Cancel' by default)
            buttonRetVals[0] = wl_MessageBoxParams::Result::kResultOk;
            break;
        case wl_MessageBoxParams::kButtonsOkCancel:
            [alert addButtonWithTitle:@"Cancel"];
            [alert addButtonWithTitle:@"OK"];
            buttonRetVals[0] = wl_MessageBoxParams::Result::kResultCancel;
            buttonRetVals[1] = wl_MessageBoxParams::Result::kResultOk;
            break;
        case wl_MessageBoxParams::kButtonsRetryCancel:
            [alert addButtonWithTitle:@"Cancel"];
            [alert addButtonWithTitle:@"Retry"];
            buttonRetVals[0] = wl_MessageBoxParams::Result::kResultCancel;
            buttonRetVals[1] = wl_MessageBoxParams::Result::kResultRetry;
            break;
        case wl_MessageBoxParams::kButtonsYesNo:
            [alert addButtonWithTitle:@"No"];
            [alert addButtonWithTitle:@"Yes"];
            buttonRetVals[0] = wl_MessageBoxParams::Result::kResultNo;
            buttonRetVals[1] = wl_MessageBoxParams::Result::kResultYes;
            break;
        case wl_MessageBoxParams::kButtonsYesNoCancel:
            [alert addButtonWithTitle:@"Cancel"];
            [alert addButtonWithTitle:@"No"];
            [alert addButtonWithTitle:@"Yes"];
            buttonRetVals[0] = wl_MessageBoxParams::Result::kResultCancel;
            buttonRetVals[1] = wl_MessageBoxParams::Result::kResultNo;
            buttonRetVals[2] = wl_MessageBoxParams::Result::kResultYes;
            break;
        default:
            break;
    }
    
    switch (params->icon) {
        case wl_MessageBoxParams::kIconError:
            alert.alertStyle = NSAlertStyleCritical;
            break;
        case wl_MessageBoxParams::kIconWarning:
            alert.alertStyle = NSAlertStyleWarning;
            break;
        default:
            // info + question
            alert.alertStyle = NSAlertStyleInformational;
    }
    
    __block NSModalResponse resp;
    if (window) {
        auto obj = (WLWindowObject *)window;
        [alert beginSheetModalForWindow:obj.nsWindow completionHandler:^(NSModalResponse returnCode) {
            [NSApp stopModalWithCode:returnCode];
        }];
        resp = [NSApp runModalForWindow:obj.nsWindow];
    } else {
        resp = [alert runModal];
    }
    [alert release];
    
    // convert native 'resp' value to OpenWL messagebox result
    wl_MessageBoxParams::Result retval;
    switch (resp) {
        case NSAlertFirstButtonReturn:
        case NSAlertSecondButtonReturn:
        case NSAlertThirdButtonReturn:
            retval = buttonRetVals[resp - NSAlertFirstButtonReturn];
            break;
        case NSModalResponseContinue:
            retval = wl_MessageBoxParams::Result::kResultContinue;
            break;
        case NSModalResponseStop:
        case NSModalResponseAbort:
            retval = wl_MessageBoxParams::Result::kResultAbort;
            break;
        case NSModalResponseCancel:
            retval = wl_MessageBoxParams::Result::kResultCancel;
            break;
        case NSModalResponseOK:
        default:
            retval = wl_MessageBoxParams::Result::kResultOk;
            break;
    }
    return retval;
}
