// SPDX-FileCopyrightText: 2022 Michael Fisher <mfisher@lvtk.org>m
// SPDX-License-Identifier: ISC

// NanoVG C amalgamation - include nanovg C sources directly

#define NVG_NO_STB
#define GL_SILENCE_DEPRECATION

#include "gl.hpp"

// Include the nanovg C sources directly
#include "../nanovg/nanovg.c"
#include "../nanovg/nanovg_gl.c"
