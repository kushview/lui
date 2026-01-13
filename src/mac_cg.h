// Copyright 2026 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#pragma once

#include "pugl/attributes.h"
#include "pugl/pugl.h"

PUGL_BEGIN_DECLS

/**
   CoreGraphics backend accessor.

   Pass the returned value to puglSetBackend() to draw to a view with 
   native CoreGraphics on macOS.
*/
const PuglBackend*
puglCGBackend(void);

PUGL_END_DECLS
