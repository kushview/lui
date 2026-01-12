########
Backends
########

LUI supports multiple graphics backends for rendering, allowing it to work across different platforms and use cases.

*****
Cairo
*****

Cairo is a 2D graphics library that provides a device-independent API for rendering vector graphics.

Building Cairo Statically on macOS
===================================

To build Cairo as a static library on macOS with minimal dependencies:

.. code-block:: bash

   meson setup --reconfigure \
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
     -Ddefault_library=shared \
     -Dspectre=disabled \
     -Dpng=disabled \
     -Dquartz=enabled \
     build

This configuration disables unnecessary features and dependencies, creating a minimal static build suitable for embedding in LUI on macOS.