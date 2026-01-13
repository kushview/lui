// Copyright 2022-2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#ifndef LUI_H_INCLUDED
#define LUI_H_INCLUDED

#ifdef __cplusplus
#    define LUI_EXTERN extern "C"
#else
#    define LUI_EXTERN
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#    if ! defined(LUI_STATIC) && defined(LUI_BUILD)
#        define LUI_API __declspec (dllexport)
#    elif ! defined(LUI_STATIC)
#        define LUI_API __declspec (dllimport)
#    endif
#else
#    if ! defined(LUI_STATIC)
#        define LUI_API __attribute__ ((visibility ("default")))
#    endif
#endif

#define LUI_EXPORT LUI_EXTERN LUI_API

#ifndef LUI_API
#    define LUI_API
#endif

#ifndef LUI_EXPORT
#    define LUI_EXPORT
#endif

#define LUI_ORG "https://lvtk.org"

#define LUI_PLUGINS_URI       LUI_ORG "/plugins"
#define LUI_PLUGINS_PREFIX    LUI_PLUGINS_URI "/"
#define LUI_PLUGINS__Volume   LUI_PLUGINS_PREFIX "volume"
#define LUI_PLUGINS__VolumeUI LUI_PLUGINS__Volume "#ui"

#define LUI_UI_URI     LUI_ORG "/ns/ui"
#define LUI_UI_PREFIX  LUI_UI_URI "#"
#define LUI_UI__Widget LUI_UI_PREFIX "Widget"

#ifdef __linux__
#    define LUI_UI__NativeUI LV2_UI__X11UI
#elif defined(__APPLE__)
#    define LUI_UI__NativeUI LV2_UI__CocoaUI
#else
#    define LUI_UI__NativeUI LV2_UI__WindowsUI
#endif

#define LUI_HOST_URI           LUI_ORG "/ns/host"
#define LUI_HOST_PREFIX        LUI_HOST_URI "#"
#define LUI_HOST__Main         LUI_HOST_PREFIX "World"
#define LUI_HOST__View         LUI_HOST_PREFIX "Instance"
#define LUI_HOST__Event        LUI_HOST_PREFIX "InstanceUI"
#define LUI_HOST__SupportedUI  LUI_HOST_PREFIX "SupportedUI"
#define LUI_HOST__SupportedUIs LUI_HOST_PREFIX "SupportedUIs"

#if __cplusplus
} // extern "C"

// clang-format off
#define LUI_DISABLE_COPY(ClassName)                   \
    ClassName (const ClassName&)            = delete;  \
    ClassName& operator= (const ClassName&) = delete;
#define LUI_DISABLE_MOVE(ClassName)                   \
    ClassName (const ClassName&&)            = delete; \
    ClassName& operator= (const ClassName&&) = delete;
// clang-format on
#endif

namespace lui {
template <typename... T>
inline static void ignore (T&&...) noexcept {}
} // namespace lui

#endif
