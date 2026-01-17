// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#include "pugl/src/stub.h"
#include "pugl/src/types.h"
#include "pugl/src/win.h"

#include <stdlib.h>
#include <windows.h>

typedef struct {
    HDC drawDc;
    HBITMAP drawBitmap;
    HBITMAP oldBitmap;
} PuglWinGdiSurface;

static PuglStatus
    puglWinGdiCreateDrawContext (PuglView* view) {
    PuglInternals* const impl        = view->impl;
    PuglWinGdiSurface* const surface = (PuglWinGdiSurface*) impl->surface;

    surface->drawDc     = CreateCompatibleDC (impl->hdc);
    surface->drawBitmap = CreateCompatibleBitmap (
        impl->hdc, (int) view->lastConfigure.width, (int) view->lastConfigure.height);

    surface->oldBitmap = (HBITMAP) SelectObject (surface->drawDc, surface->drawBitmap);

    return PUGL_SUCCESS;
}

static PuglStatus
    puglWinGdiDestroyDrawContext (PuglView* view) {
    PuglInternals* const impl        = view->impl;
    PuglWinGdiSurface* const surface = (PuglWinGdiSurface*) impl->surface;

    if (surface->oldBitmap) {
        SelectObject (surface->drawDc, surface->oldBitmap);
        surface->oldBitmap = NULL;
    }

    if (surface->drawBitmap) {
        DeleteObject (surface->drawBitmap);
        surface->drawBitmap = NULL;
    }

    if (surface->drawDc) {
        DeleteDC (surface->drawDc);
        surface->drawDc = NULL;
    }

    return PUGL_SUCCESS;
}

static PuglStatus
    puglWinGdiConfigure (PuglView* view) {
    const PuglStatus st = puglWinConfigure (view);

    if (! st) {
        view->impl->surface =
            (PuglWinGdiSurface*) calloc (1, sizeof (PuglWinGdiSurface));
    }

    return st;
}

static PuglStatus
    puglWinGdiCreate (PuglView* view) {
    (void) view;
    return PUGL_SUCCESS;
}

static void
    puglWinGdiDestroy (PuglView* view) {
    PuglInternals* const impl        = view->impl;
    PuglWinGdiSurface* const surface = (PuglWinGdiSurface*) impl->surface;

    if (surface) {
        puglWinGdiDestroyDrawContext (view);
        free (surface);
        impl->surface = NULL;
    }
}

static PuglStatus
    puglWinGdiEnter (PuglView* view, const PuglExposeEvent* expose) {
    PuglStatus st = PUGL_SUCCESS;

    if (expose && ! (st = puglWinGdiCreateDrawContext (view))) {
        st = puglWinEnter (view, expose);
    }

    return st;
}

static PuglStatus
    puglWinGdiLeave (PuglView* view, const PuglExposeEvent* expose) {
    PuglInternals* const impl        = view->impl;
    PuglWinGdiSurface* const surface = (PuglWinGdiSurface*) impl->surface;

    if (expose) {
        // Blit the offscreen buffer to the window
        BitBlt (impl->hdc,
                0,
                0,
                (int) view->lastConfigure.width,
                (int) view->lastConfigure.height,
                surface->drawDc,
                0,
                0,
                SRCCOPY);

        puglWinGdiDestroyDrawContext (view);
    }

    return puglWinLeave (view, expose);
}

static void*
    puglWinGdiGetContext (PuglView* view) {
    return ((PuglWinGdiSurface*) view->impl->surface)->drawDc;
}

const PuglBackend*
    puglGdiBackend (void) {
    static const PuglBackend backend = { puglWinGdiConfigure,
                                         puglWinGdiCreate,
                                         puglWinGdiDestroy,
                                         puglWinGdiEnter,
                                         puglWinGdiLeave,
                                         puglWinGdiGetContext };

    return &backend;
}
