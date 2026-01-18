# Timer Implementation Plan for LUI

## Background

PUGL already provides native timer support via `puglStartTimer()` and `puglStopTimer()`. Timer events are delivered through the OS message queue and processed by `puglUpdate()` on the event loop thread - no separate timer thread needed.

Timer callbacks run on the same thread that calls `main.loop()`, making them safe for UI updates without synchronization.

## Architecture Decision

Use **per-View timer management** rather than global state:
- Avoids global initialization issues
- Each View manages its own timer namespace
- Automatic cleanup when View is destroyed
- Thread-safe by design (all on event loop thread)
- Plugin-safe: each instance has isolated timers

## API Design

### Widget Timer Interface (JUCE-style)

```cpp
// In include/lui/widget.hpp
class Widget {
public:
    /** Start a repeating timer. Calls timer_callback() at the specified interval.
        @param interval_ms Interval in milliseconds (minimum ~10ms on Windows)
    */
    void start_timer(int interval_ms);
    
    /** Stop the timer if running. */
    void stop_timer();
    
    /** Check if timer is currently running. */
    bool is_timer_running() const noexcept;

protected:
    /** Override this to handle timer ticks. Called on the event loop thread. */
    virtual void timer_callback() {}

private:
    bool timer_running{false};
};
```

### View Timer Management

```cpp
// In detail::View
class View {
    // Map widget pointers to PUGL timer IDs
    std::unordered_map<Widget*, uintptr_t> widget_timers;
    uintptr_t next_timer_id{1};
    
    void start_widget_timer(Widget* widget, int interval_ms);
    void stop_widget_timer(Widget* widget);
    void dispatch_timer_event(uintptr_t pugl_timer_id);
};
```

## Implementation Steps

### 1. Add Widget Timer Methods

**File**: `include/lui/widget.hpp`
- Add public methods: `start_timer()`, `stop_timer()`, `is_timer_running()`
- Add protected virtual: `timer_callback()`
- Add private member: `bool timer_running{false}`

**File**: `src/widget.cpp`
- Implement `start_timer()` - finds view, calls view timer manager
- Implement `stop_timer()` - finds view, stops timer
- Implement `is_timer_running()` - returns flag

### 2. Implement View Timer Manager

**File**: `src/detail/view.hpp`
- Add timer mapping: `std::unordered_map<Widget*, uintptr_t> widget_timers`
- Add reverse map: `std::unordered_map<uintptr_t, Widget*> timer_to_widget`
- Add counter: `uintptr_t next_timer_id{1}`
- Add methods: `start_widget_timer()`, `stop_widget_timer()`

**File**: `src/detail/view.hpp` (timer event handler)
Update existing `timer()` handler:
```cpp
static PuglStatus timer(View& view, const PuglTimerEvent& ev) {
    view.dispatch_timer_event(ev.id);
    return PUGL_SUCCESS;
}
```

### 3. Implement Timer Dispatch

**File**: `src/view.cpp`
```cpp
void View::start_widget_timer(Widget* widget, int interval_ms) {
    // Stop existing timer if any
    stop_widget_timer(widget);
    
    // Assign new PUGL timer ID
    uintptr_t timer_id = next_timer_id++;
    widget_timers[widget] = timer_id;
    timer_to_widget[timer_id] = widget;
    
    // Start PUGL timer
    puglStartTimer((PuglView*)view, timer_id, interval_ms / 1000.0);
}

void View::stop_widget_timer(Widget* widget) {
    auto it = widget_timers.find(widget);
    if (it != widget_timers.end()) {
        puglStopTimer((PuglView*)view, it->second);
        timer_to_widget.erase(it->second);
        widget_timers.erase(it);
    }
}

void View::dispatch_timer_event(uintptr_t pugl_timer_id) {
    auto it = timer_to_widget.find(pugl_timer_id);
    if (it != timer_to_widget.end()) {
        Widget* widget = it->second;
        if (widget) {
            widget->timer_callback();
        }
    }
}
```

### 4. Add Automatic Cleanup

**File**: `src/widget.cpp`
Update `Widget::~Widget()`:
```cpp
Widget::~Widget() {
    stop_timer();  // Ensure timer stops before destruction
    // ... existing cleanup
}
```

## Usage Example

```cpp
class AnimatedWidget : public Widget {
    int frame_count{0};
    
    void resized() override {
        start_timer(16);  // ~60fps
    }
    
    void timer_callback() override {
        frame_count++;
        repaint();  // Safe - on event loop thread
    }
    
    void paint(Graphics& g) override {
        // Draw animation frame
        float angle = frame_count * 0.1f;
        // ...
    }
};
```

## Error Handling

- `start_timer()` with no view: silently no-op (widget not attached yet)
- `stop_timer()` when not running: safe no-op
- Widget destroyed while timer active: automatically stopped in destructor
- View destroyed with active timers: PUGL automatically cleans up OS timers

## Platform Considerations

- **Minimum interval**: ~10ms on Windows (SetTimer limitation), ~1ms on macOS/Linux
- **Resolution**: Not guaranteed high precision, suitable for UI animation not audio timing
- **Thread**: Always runs on event loop thread (thread that calls `main.loop()`)

## Testing Strategy

1. Basic timer start/stop
2. Multiple timers on different widgets
3. Timer cleanup on widget destruction
4. Timer behavior when widget not attached to view
5. Timer interval accuracy measurement

## Future Enhancements

- High-resolution timers (microsecond precision)
- One-shot timers (non-repeating)
- Timer ID for multiple timers per widget
- Callable-based timers: `view->set_timeout([]() { ... }, 1000)`
