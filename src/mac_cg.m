// Copyright 2019-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: ISC

#include "pugl/src/internal.h"
#include "pugl/src/mac.h"
#include "pugl/src/stub.h"

// CoreGraphics is available via Cocoa.h

#import <Cocoa/Cocoa.h>

#include <assert.h>

@interface PuglCGView : NSView
@end

@implementation PuglCGView {
@public
  PuglView*    puglview;
  CGContextRef cgContext;
}

- (id)initWithFrame:(NSRect)frame
{
  self = [super initWithFrame:frame];

  return self;
}

- (void)resizeWithOldSuperviewSize:(NSSize)oldSize
{
  PuglWrapperView* wrapper = (PuglWrapperView*)[self superview];

  [super resizeWithOldSuperviewSize:oldSize];
  [wrapper setReshaped];
}

- (void)drawRect:(NSRect)rect
{
  PuglWrapperView* wrapper = (PuglWrapperView*)[self superview];
  [wrapper dispatchExpose:rect];
}

@end

static PuglStatus
puglMacCGCreate(PuglView* view)
{
  PuglInternals* impl  = view->impl;
  PuglCGView* drawView = [PuglCGView alloc];

  drawView->puglview = view;
  [drawView initWithFrame:[impl->wrapperView bounds]];
  if (view->hints[PUGL_RESIZABLE]) {
    [drawView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  } else {
    [drawView setAutoresizingMask:NSViewNotSizable];
  }

  impl->drawView = drawView;
  return PUGL_SUCCESS;
}

static void
puglMacCGDestroy(PuglView* view)
{
  PuglCGView* const drawView = (PuglCGView*)view->impl->drawView;

  [drawView removeFromSuperview];
  [drawView release];

  view->impl->drawView = nil;
}

static PuglStatus
puglMacCGEnter(PuglView* view, const PuglExposeEvent* expose)
{
  PuglCGView* const drawView = (PuglCGView*)view->impl->drawView;
  if (!expose) {
    return PUGL_SUCCESS;
  }

  assert(!drawView->cgContext);

  CGContextRef context =
    (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];

  const CGSize sizePx = {(CGFloat)view->lastConfigure.width,
                         (CGFloat)view->lastConfigure.height};

  // Transform to match flipped view coordinates (top-left origin)
  // Note: sizePx is already in pixel units, context is in points
  const double backingScale = [[NSScreen mainScreen] backingScaleFactor];
  CGContextTranslateCTM(context, 0.0, sizePx.height / backingScale);
  CGContextScaleCTM(context, 1.0, -1.0);

  drawView->cgContext = context;

  return PUGL_SUCCESS;
}

static PuglStatus
puglMacCGLeave(PuglView* view, const PuglExposeEvent* expose)
{
  PuglCGView* const drawView = (PuglCGView*)view->impl->drawView;
  if (!expose) {
    return PUGL_SUCCESS;
  }

  assert(drawView->cgContext);

  CGContextFlush(drawView->cgContext);
  drawView->cgContext = NULL;

  return PUGL_SUCCESS;
}

static void*
puglMacCGGetContext(PuglView* view)
{
  return ((PuglCGView*)view->impl->drawView)->cgContext;
}

const PuglBackend*
puglCGBackend(void)
{
  static const PuglBackend backend = {puglStubConfigure,
                                      puglMacCGCreate,
                                      puglMacCGDestroy,
                                      puglMacCGEnter,
                                      puglMacCGLeave,
                                      puglMacCGGetContext};

  return &backend;
}
