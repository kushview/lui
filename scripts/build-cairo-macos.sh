#!/bin/sh
# SPDX-License-Identifier: ISC
# Copyright Kushview, LLC

export PKG_CONFIG_PATH=""
export PKG_CONFIG_LIBDIR=""

cairogit="https://gitlab.freedesktop.org/cairo/cairo.git"
cairorev="200a02286bfe9a39839b9fc8d715b852ccf25d71"

workdir="/tmp/cairo-build"
cairodir="${workdir}/cairo"
builddir="${workdir}/build"
prefix="$HOME/SDKs/cairo"

set -e

rm -rf "${workdir}"
mkdir -p "${workdir}"

trap 'rm -rf "${workdir}"' EXIT

git clone --depth=1 "${cairogit}" "${cairodir}"
git -C "${cairodir}" checkout "${cairorev}"

meson setup -Dprefix="${prefix}" \
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
    "${builddir}" "${cairodir}"

meson compile -C "${builddir}"
ninja install -C "${builddir}"
