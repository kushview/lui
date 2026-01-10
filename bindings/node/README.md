# Lui Node.js Bindings

Native cross-platform GUI library for Node.js with OpenGL rendering.

## Installation (End Users)

```bash
npm install lui
```

Prebuilt binaries are provided for macOS (x64, arm64), Linux (x64), and Windows (x64).

## Development

### Prerequisites

- Node.js 20+
- CMake 3.20+
- Ninja
- C++20 compiler
- Boost (via Homebrew on macOS: `brew install boost`)

### Setup

```bash
git clone https://github.com/lvtk/lui.git
cd lui/bindings/node
npm install
```

### Building

Full rebuild (CMake + Node addon):
```bash
npm run rebuild
```

Incremental (Node addon only):
```bash
npm run build
```

### Testing

```bash
npm run demo    # Opens GUI window with interactive demo
```

### Creating Prebuilds

Generate prebuilt binaries for distribution:

```bash
npm run prebuildify
```

Creates `prebuilds/{platform}-{arch}/lui.node`. Used for npm publishing.

## API

```javascript
const { Main, Widget, Button, ViewFlags } = require('lui');

const main = new Main();
const root = new Widget();
root.setSize(400, 300);
root.setVisible(true);

const button = new Button();
button.setText('Click Me!');
button.setBounds(20, 20, 120, 40);
button.setVisible(true);
button.onClick(() => console.log('Clicked!'));

root.add(button);
main.elevate(root, ViewFlags.RESIZABLE);

// Event loop
const loop = () => {
    if (!main.running()) return;
    main.loop(1.0 / 60.0);
    setImmediate(loop);
};
loop();
```

## Publishing (Maintainers)

1. Update version in `package.json`
2. Tag: `git tag node-v0.0.2 && git push --tags`
3. GitHub Actions builds prebuilds for all platforms
4. Download artifacts, add to `prebuilds/`
5. Publish: `npm publish`

## Build Structure

- `build-node/` - CMake build for Node.js (isolated from main project)
- `build/` - node-gyp build output
- `prebuilds/` - Prebuild binaries for distribution

