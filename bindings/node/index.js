// Copyright 2026 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

const { existsSync } = require('fs');
const { join } = require('path');

let addon;

// Try to load prebuilt binary first
try {
    addon = require('node-gyp-build')(__dirname);
} catch (err) {
    // Fall back to local build (for development)
    const buildPath = join(__dirname, 'build/Release/lui_node.node');
    if (existsSync(buildPath)) {
        addon = require(buildPath);
    } else {
        throw new Error(
            'Lui native addon not found. Please run: npm run rebuild\n' +
            'Original error: ' + err.message
        );
    }
}

module.exports = {
    Main: addon.Main,
    Widget: addon.Widget,
    Button: addon.Button,
    ViewFlags: addon.ViewFlags
};
