// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

const { Main, Widget, Button, ViewFlags } = require('./index');

console.log('Starting Lui Node.js Demo...');

// Create main context
const main = new Main();

// Create root widget
const root = new Widget();
root.setName('Root Widget');
root.setSize(550, 400);
root.setOpaque(true);
root.setVisible(true);

// Create some buttons
const button1 = new Button();
button1.setName('Button 1');
button1.setText('Click Me!');
button1.setBounds(20, 20, 120, 40);
button1.setVisible(true);
button1.onClick(() => {
    console.log('Button 1 clicked!');
});

const button2 = new Button();
button2.setName('Button 2');
button2.setText('Another Button');
button2.setBounds(20, 70, 120, 40);
button2.setVisible(true);
button2.onClick(() => {
    console.log('Button 2 clicked!');
});

const button3 = new Button();
button3.setName('Button 3');
button3.setText('Exit');
button3.setBounds(20, 120, 120, 40);
button3.setVisible(true);
button3.onClick(() => {
    console.log('Exit button clicked - closing window');
    main.setExitCode(0);
    process.exit(0);
});

// Add buttons to root
root.add(button1);
root.add(button2);
root.add(button3);

// Elevate root widget to a window
console.log('Creating window...');
const success = main.elevate(root, ViewFlags.RESIZABLE);

if (!success) {
    console.error('Failed to create window!');
    process.exit(1);
}

// Main event loop
console.log('Entering main loop...');
const fps = 60;
const frameTime = 1.0 / fps;

let frameCount = 0;
function runLoop() {
    // Process events
    main.loop(frameTime);
    
    frameCount++;
    if (frameCount % (fps * 2) === 0) {
        console.log('Still running... (close window to exit)');
    }
    
    // Check if still running
    if (!main.running()) {
        console.log('Exiting with code:', main.exitCode());
        process.exit(main.exitCode());
        return;
    }
    
    // Schedule next frame
    setImmediate(runLoop);
}

// Start the loop
console.log('Window created. Starting event loop...');
runLoop();
