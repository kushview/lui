// SPDX-FileCopyrightText: 2022 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

// Pugl Objective-C amalgamation - compile pugl Objective-C files

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "pugl/src/mac.m"
// #include "pugl/src/mac_cairo.m"    // Requires cairo
#include "pugl/src/mac_gl.m"
#include "pugl/src/mac_stub.m"
// #include "pugl/src/mac_vulkan.m"   // Requires vulkan

#pragma clang diagnostic pop
