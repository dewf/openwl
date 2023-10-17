#include "../main.h"

#include <stdio.h>

void platformInit()
{
}

void platformDraw(wl_PlatformContext *platformContext)
{
    platformContext->view->SetHighColor(0, 0, 0);
    platformContext->view->FillRect(BRect(0, 0, 800, 600));
}

void platformDrawFrameless(wl_PlatformContext *platformContext)
{
}

void platformDrawModal(wl_PlatformContext* platformContext)
{
}

void platformDrawBox(RandomBox *box)
{
}

bool platformProvidesDragFormat(const char *formatMIME)
{
    return false;
}

void platformRenderDragFormat(wl_RenderPayloadRef payload, const char *formatMIME)
{
}


bool platformCheckDropFormats(wl_DropDataRef dropData)
{
    return false;
}

void platformHandleDrop(wl_DropDataRef dropData)
{
}

void platformCreateThreads(threadFunc_t threadFunc, int count)
{
}

void platformJoinThreads()
{
}

void platformShutdown()
{
}
