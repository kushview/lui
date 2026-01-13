// Copyright 2012-2022 David Robillard <d@drobilla.net>
// Copyright 2026 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#include "pugl/src/stub.h"
#include "pugl/src/types.h"
#include "pugl/src/win.h"

#include <d2d1.h>
#include <dwrite.h>

#include <stdlib.h>

typedef struct {
    ID2D1Factory* d2dFactory;
    ID2D1HwndRenderTarget* renderTarget;
    IDWriteFactory* writeFactory;
} PuglWinDirect2DSurface;

static PuglStatus
    puglWinDirect2DCreateDrawContext (PuglView* view) {
    PuglInternals* const impl             = view->impl;
    PuglWinDirect2DSurface* const surface = (PuglWinDirect2DSurface*) impl->surface;

    if (! surface->d2dFactory) {
        return PUGL_CREATE_CONTEXT_FAILED;
    }

    // Create render target properties
    D2D1_RENDER_TARGET_PROPERTIES props = {
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
        0.0f,
        0.0f,
        D2D1_RENDER_TARGET_USAGE_NONE,
        D2D1_FEATURE_LEVEL_DEFAULT
    };

    // Create HWND render target properties
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = {
        impl->hwnd,
        { (UINT32) view->lastConfigure.width, (UINT32) view->lastConfigure.height },
        D2D1_PRESENT_OPTIONS_NONE
    };

    HRESULT hr = ID2D1Factory_CreateHwndRenderTarget (
        surface->d2dFactory,
        &props,
        &hwndProps,
        &surface->renderTarget);

    return (SUCCEEDED (hr)) ? PUGL_SUCCESS : PUGL_CREATE_CONTEXT_FAILED;
}

static PuglStatus
    puglWinDirect2DDestroyDrawContext (PuglView* view) {
    PuglInternals* const impl             = view->impl;
    PuglWinDirect2DSurface* const surface = (PuglWinDirect2DSurface*) impl->surface;

    if (surface->renderTarget) {
        ID2D1HwndRenderTarget_Release (surface->renderTarget);
        surface->renderTarget = NULL;
    }

    return PUGL_SUCCESS;
}

static PuglStatus
    puglWinDirect2DConfigure (PuglView* view) {
    const PuglStatus st = puglWinConfigure (view);

    if (! st) {
        PuglWinDirect2DSurface* surface =
            (PuglWinDirect2DSurface*) calloc (1, sizeof (PuglWinDirect2DSurface));

        view->impl->surface = surface;

        // Create D2D factory
        HRESULT hr = D2D1CreateFactory (
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            &IID_ID2D1Factory,
            NULL,
            (void**) &surface->d2dFactory);

        if (FAILED (hr)) {
            free (surface);
            view->impl->surface = NULL;
            return PUGL_CREATE_CONTEXT_FAILED;
        }

        // Create DWrite factory
        hr = DWriteCreateFactory (
            DWRITE_FACTORY_TYPE_SHARED,
            &IID_IDWriteFactory,
            (IUnknown**) &surface->writeFactory);

        if (FAILED (hr)) {
            ID2D1Factory_Release (surface->d2dFactory);
            free (surface);
            view->impl->surface = NULL;
            return PUGL_CREATE_CONTEXT_FAILED;
        }
    }

    return st;
}

static void
    puglWinDirect2DClose (PuglView* view) {
    // Direct2D doesn't need explicit close per frame
    (void) view;
}

static PuglStatus
    puglWinDirect2DOpen (PuglView* view) {
    // Direct2D doesn't need explicit open per frame
    (void) view;
    return PUGL_SUCCESS;
}

static void
    puglWinDirect2DDestroy (PuglView* view) {
    PuglInternals* const impl             = view->impl;
    PuglWinDirect2DSurface* const surface = (PuglWinDirect2DSurface*) impl->surface;

    if (surface) {
        puglWinDirect2DClose (view);
        puglWinDirect2DDestroyDrawContext (view);

        if (surface->writeFactory) {
            IDWriteFactory_Release (surface->writeFactory);
        }

        if (surface->d2dFactory) {
            ID2D1Factory_Release (surface->d2dFactory);
        }

        free (surface);
        impl->surface = NULL;
    }
}

static PuglStatus
    puglWinDirect2DEnter (PuglView* view, const PuglExposeEvent* expose) {
    PuglStatus st = PUGL_SUCCESS;

    if (expose) {
        if (! st) {
            st = puglWinDirect2DCreateDrawContext (view);
        }
        if (! st) {
            st = puglWinDirect2DOpen (view);
        }
        if (! st) {
            st = puglWinEnter (view, expose);
        }

        // Begin drawing
        if (! st) {
            PuglWinDirect2DSurface* const surface =
                (PuglWinDirect2DSurface*) view->impl->surface;
            if (surface->renderTarget) {
                ID2D1RenderTarget_BeginDraw ((ID2D1RenderTarget*) surface->renderTarget);
            }
        }
    }

    return st;
}

static PuglStatus
    puglWinDirect2DLeave (PuglView* view, const PuglExposeEvent* expose) {
    PuglInternals* const impl             = view->impl;
    PuglWinDirect2DSurface* const surface = (PuglWinDirect2DSurface*) impl->surface;

    if (expose && surface->renderTarget) {
        // End drawing
        HRESULT hr = ID2D1RenderTarget_EndDraw (
            (ID2D1RenderTarget*) surface->renderTarget,
            NULL,
            NULL);

        if (hr == D2DERR_RECREATE_TARGET) {
            // Need to recreate render target
            puglWinDirect2DDestroyDrawContext (view);
        }

        puglWinDirect2DClose (view);
    }

    return puglWinLeave (view, expose);
}

static void*
    puglWinDirect2DGetContext (PuglView* view) {
    return ((PuglWinDirect2DSurface*) view->impl->surface)->renderTarget;
}

const PuglBackend*
    puglDirect2DBackend (void) {
    static const PuglBackend backend = {
        puglWinDirect2DConfigure,
        puglStubCreate,
        puglWinDirect2DDestroy,
        puglWinDirect2DEnter,
        puglWinDirect2DLeave,
        puglWinDirect2DGetContext
    };

    return &backend;
}
