#!/bin/sh
# SPDX-License-Identifier: ISC
# Copyright Kushview, LLC

export PKG_CONFIG_PATH=""
export PKG_CONFIG_LIBDIR=""

rm -rf build-cairo

set -e

meson setup -Dprefix=/opt/lui \
    -Ddefault_library=static \
    -Dtee=disabled \
    -Dxcb=disabled \
    -Dxlib=disabled \
    -Dxlib-xcb=disabled \
    -Dzlib=disabled \
    -Dglib=disabled \
    -Dfreetype=disabled \
    -Dfontconfig=disabled \
    -Ddwrite=disabled \
    -Dlzo=disabled \
    -Dtests=disabled \
    -Dspectre=disabled \
    -Dpng=disabled \
    -Dquartz=enabled \
    -Dpixman:tests=disabled \
    "$(pwd)/build-cairo" \
    "$(pwd)/cairo"

meson compile -C build-cairo
sudo rm -rf /opt/lui
sudo ninja install -C build-cairo
sudo rm -rf build-cairo
