// Copyright 2012-2022 David Robillard <d@drobilla.net>
// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#include "pugl/src/types.h"

extern "C" {
#include "pugl/src/win.h"
}

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include <stdlib.h>

typedef struct {
    ID2D1Factory* d2dFactory;
    ID2D1HwndRenderTarget* renderTarget;
    IDWriteFactory* writeFactory;
} PuglWinDirect2DSurface;

static PuglStatus
puglWinDirect2DCreateDeviceResources (PuglView* view) {
    PuglInternals* const impl        = view->impl;
    PuglWinDirect2DSurface* const surface = (PuglWinDirect2DSurface*) impl->surface;

    if (surface->renderTarget) {
        // Just resize if it already exists
        D2D1_SIZE_U size = D2D1::SizeU (
            (UINT32) view->lastConfigure.width,
            (UINT32) view->lastConfigure.height);
        surface->renderTarget->Resize (size);
        return PUGL_SUCCESS;
    }

    D2D1_SIZE_U size = D2D1::SizeU (
        (UINT32) view->lastConfigure.width,
        (UINT32) view->lastConfigure.height);

    // Create render target with 96 DPI (identity DPI) so coordinates match 1:1 with pixels
    // This matches how pugl reports window size and mouse coordinates
    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
    rtProps.dpiX = 96.0f;
    rtProps.dpiY = 96.0f;
    rtProps.pixelFormat = D2D1::PixelFormat (
        DXGI_FORMAT_B8G8R8A8_UNORM,
        D2D1_ALPHA_MODE_PREMULTIPLIED);

    HRESULT hr = surface->d2dFactory->CreateHwndRenderTarget (
        rtProps,
        D2D1::HwndRenderTargetProperties (impl->hwnd, size),
        &surface->renderTarget);

    return SUCCEEDED (hr) ? PUGL_SUCCESS : PUGL_CREATE_CONTEXT_FAILED;
}

static void
puglWinDirect2DDiscardDeviceResources (PuglView* view) {
    PuglInternals* const impl        = view->impl;
    PuglWinDirect2DSurface* const surface = (PuglWinDirect2DSurface*) impl->surface;

    if (surface->renderTarget) {
        surface->renderTarget->Release();
        surface->renderTarget = nullptr;
    }
}

static PuglStatus
puglWinDirect2DConfigure (PuglView* view) {
    const PuglStatus st = puglWinConfigure (view);

    if (st != PUGL_SUCCESS) {
        return st;
    }

    // Allocate surface structure
    view->impl->surface = (PuglWinDirect2DSurface*) calloc (1, sizeof (PuglWinDirect2DSurface));
    if (! view->impl->surface) {
        return PUGL_FAILURE;
    }

    PuglWinDirect2DSurface* surface = (PuglWinDirect2DSurface*) view->impl->surface;

    // Create Direct2D factory (device-independent)
    HRESULT hr = D2D1CreateFactory (
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &surface->d2dFactory);

    if (FAILED (hr)) {
        free (view->impl->surface);
        view->impl->surface = nullptr;
        return PUGL_CREATE_CONTEXT_FAILED;
    }

    // Create DirectWrite factory (device-independent)
    hr = DWriteCreateFactory (
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**> (&surface->writeFactory));

    if (FAILED (hr)) {
        surface->d2dFactory->Release();
        free (view->impl->surface);
        view->impl->surface = nullptr;
        return PUGL_CREATE_CONTEXT_FAILED;
    }

    return PUGL_SUCCESS;
}

static PuglStatus
puglWinDirect2DCreate (PuglView* view) {
    (void) view;
    return PUGL_SUCCESS;
}

static void
puglWinDirect2DDestroy (PuglView* view) {
    PuglInternals* const impl        = view->impl;
    PuglWinDirect2DSurface* const surface = (PuglWinDirect2DSurface*) impl->surface;

    if (surface) {
        puglWinDirect2DDiscardDeviceResources (view);

        if (surface->writeFactory) {
            surface->writeFactory->Release();
            surface->writeFactory = nullptr;
        }

        if (surface->d2dFactory) {
            surface->d2dFactory->Release();
            surface->d2dFactory = nullptr;
        }

        free (surface);
        impl->surface = nullptr;
    }
}

static PuglStatus
puglWinDirect2DEnter (PuglView* view, const PuglExposeEvent* expose) {
    PuglStatus st = PUGL_SUCCESS;

    if (expose) {
        // Create device resources if needed
        st = puglWinDirect2DCreateDeviceResources (view);
        if (st != PUGL_SUCCESS) {
            return st;
        }

        PuglWinDirect2DSurface* surface = (PuglWinDirect2DSurface*) view->impl->surface;
        
        // Begin drawing
        surface->renderTarget->BeginDraw();
        surface->renderTarget->SetTransform (D2D1::Matrix3x2F::Identity());
    }

    return st;
}

static PuglStatus
puglWinDirect2DLeave (PuglView* view, const PuglExposeEvent* expose) {
    PuglWinDirect2DSurface* surface = (PuglWinDirect2DSurface*) view->impl->surface;

    if (expose && surface->renderTarget) {
        // End drawing
        HRESULT hr = surface->renderTarget->EndDraw();

        // Check if we need to recreate device resources
        if (hr == D2DERR_RECREATE_TARGET) {
            puglWinDirect2DDiscardDeviceResources (view);
        }
    }

    return PUGL_SUCCESS;
}

static void*
puglWinDirect2DGetContext (PuglView* view) {
    return view->impl->surface;
}

extern "C" {

const PuglBackend*
puglDirect2DBackend (void) {
    static const PuglBackend backend = {
        puglWinDirect2DConfigure,
        puglWinDirect2DCreate,
        puglWinDirect2DDestroy,
        puglWinDirect2DEnter,
        puglWinDirect2DLeave,
        puglWinDirect2DGetContext
    };

    return &backend;
}

} // extern "C"
