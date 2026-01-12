// Copyright 2022 Kushview, LLC
// SPDX-License-Identifier: ISC

#ifndef LUI_BINDINGS_H
#define LUI_BINDINGS_H

#define LUI_MT_BYTE_ARRAY "lvtk.ByteArray"

#if __cplusplus
#    define LUI_EXTERN extern "C"
#else
#    define LUI_EXTERN
#endif

#ifdef _WIN32
#    define LUI_LUALIB LUI_EXTERN __declspec(dllexport)
#else
#    define LUI_LUALIB LUI_EXTERN __attribute__ ((visibility ("default")))
#endif

#endif
