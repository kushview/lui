<!-- .github/prompt/signal-vs-boost-signals2-gap-analysis.md -->

# Prompt: Compare `lui::Signal` with `boost::signals2` and Identify Gaps

You are reviewing a lightweight custom C++ signal/slot library (`lui::Signal`) and comparing it against `boost::signals2`.

The goal is **not** to fully re-implement Boost.Signals2, but to:
- Avoid missing *important, easy-to-overlook features*
- Decide which gaps matter for safety, usability, and long-term maintenance
- Evaluate whether any features should be added intentionally or explicitly excluded

## Context

Current `lui::Signal` characteristics:

- Header-only, minimal dependencies
- `Signal<void(Args...)>` only
- Slots stored in `std::vector`
- `connect()` returns `Connection`
- `ScopedConnection` RAII disconnect
- No built-in thread safety
- No slot blocking
- No lifetime tracking
- Limited re-entrancy protection
- Cleanup deferred via `connected` state

`boost::signals2` characteristics:

- Fully featured signal/slot system
- Thread-safe by default
- Safe re-entrancy and modification during emit
- Slot lifetime tracking
- Slot blocking
- Grouping, ordering, and combiners
- Large dependency and higher complexity

## Primary Question

> What are the *meaningful behavioral and safety differences* between `lui::Signal` and `boost::signals2`, and which ones should we intentionally close or explicitly document as unsupported?

## Focus Areas (Must Review)

### 1. Connection Blocking (High Priority)
Boost provides:
- `shared_connection_block`
- Temporarily disables a slot **without disconnecting**
- Scope-based enable/disable
- Safe during emission

Evaluate:
- Why connection blocking matters in real-world usage (UI state changes, recursive updates, guard patterns)
- Whether blocking can be implemented by:
  - a flag in `ConnectionState`
  - a `ScopedBlocker` RAII object
- Interaction with re-entrant emits and nested blockers
- Whether blocked slots should:
  - be skipped silently
  - count as “connected”
  - participate in `size()` / introspection

### 2. Re-entrancy Guarantees
Boost.Signals2 allows:
- Emitting the same signal from inside a slot
- Connecting/disconnecting during emission safely

Review:
- What re-entrancy guarantees Boost actually provides
- What guarantees `lui::Signal` currently lacks
- Whether re-entrancy should be:
  - fully supported
  - partially supported (with restrictions)
  - explicitly forbidden and asserted against

### 3. Thread Safety Expectations
Boost.Signals2:
- Thread-safe by default (mutex-based)

Evaluate:
- Whether `lui::Signal` should:
  - remain explicitly single-threaded
  - optionally support locking via policy
- What documentation is required if thread-safety is intentionally excluded
- Whether connection blocking interacts with threading assumptions

### 4. Lifetime Tracking / Auto-Disconnect
Boost.Signals2 supports:
- Automatic slot disconnection when tracked objects are destroyed

Evaluate:
- Whether `lui::Signal` should:
  - remain manual-only
  - provide optional helpers (e.g. weak_ptr-based connect helpers)
- What common bugs this prevents
- Whether documentation alone is sufficient

### 5. Slot Ordering and Grouping
Boost supports:
- Grouped slots
- Ordered execution

Evaluate:
- Whether predictable ordering is enough
- Whether grouping is worth the API cost
- How blocking and grouping interact

### 6. Exception Safety
Boost:
- Protects internal state even if slots throw

Evaluate:
- Whether `lui::Signal` should:
  - allow exceptions to propagate
  - catch and continue
- How to ensure emit guards (depth counters, cleanup) are exception-safe

## Drop-in Replacement Reality Check

Explicitly assess:

- Can `lui::Signal` ever be a drop-in replacement for `boost::signals2`?
- Which Boost assumptions *will break user code* if replaced?
- What subset of Boost usage patterns could be supported safely?

## Deliverables Requested

1. A **gap table** listing:
   - Feature
   - Present in Boost.Signals2
   - Present in `lui::Signal`
   - Risk if omitted
   - Recommendation (Add / Document / Ignore)

2. A **short recommendation section**:
   - “Must-have for correctness”
   - “Nice-to-have ergonomics”
   - “Intentionally excluded features”

3. A **proposal for connection blocking**, including:
   - API sketch
   - Internal state changes
   - Semantics during emit
   - RAII usage example

4. Clear guidance on:
   - Whether `lui::Signal` should aim to be “Boost-like” or “intentionally smaller”
   - What guarantees users can rely on

## Constraints

- Keep API minimal and readable
- Prefer explicit behavior over implicit magic
- Avoid hidden thread-safety or lifetime assumptions
- Do not introduce Boost as a dependency

## Tone

- Engineering-focused
- Honest about tradeoffs
- Prefer correctness and clarity over feature parity
