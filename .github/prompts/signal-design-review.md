<!-- .github/prompt/signal-design-review.md -->

# Prompt: Review + Improve `lui::Signal` Design (C++)

You are reviewing a small C++ signal/slot library:

- `lui::Signal<void(Args...)>` stores slots in `std::vector`
- `connect()` returns a `Connection` handle holding `shared_ptr<ConnectionState>`
- `ScopedConnection` disconnects in destructor
- Emission uses a guard flag `_emitting` and calls `cleanup()` after emitting
- `size()` / `empty()` call `cleanup()` even though they are `const` (via `mutable` storage)

## Goals

1. Keep the library **small and readable**.
2. Make behavior **well-defined** for:
   - re-entrant emits (emit the same signal from inside a slot)
   - connect/disconnect during emit
   - exceptions thrown by slots
3. Explicitly decide/document **thread-safety** expectations.

## Key Problems to Address

### A) Re-entrancy tracking with a bool is incorrect
Current design uses a single `bool _emitting`. With nested emission:

- outer emit sets `_emitting = true`
- inner emit sets `_emitting = true`
- inner returns and sets `_emitting = false` while outer is still iterating

This can cause cleanup or modifications during the still-active outer emission.

**Fix:** replace `_emitting` with an `int _emit_depth` counter (or `size_t`) and only allow cleanup when depth returns to 0.

### B) Modifying a `std::vector` while iterating can be UB
If `connect()` happens inside a slot, `push_back` may reallocate and invalidate iterators used in the range-for loop over `_slots`. That is undefined behavior.

Similarly, erasing during iteration is unsafe (you currently avoid erase during emit, but connect can still be a problem).

**Fix options (choose one):**
1. **Disallow connect/disconnect structural changes during emit** (assert in debug, document in API), and defer requested changes.
2. **Defer changes**: queue pending connections/disconnections during emit, apply after emission ends.
3. **Stable iteration strategy**:
   - iterate by index over a snapshot size (still risky if reallocation occurs unless you prevent reallocation)
   - snapshot pointers/copies of slots before iterating
   - use a stable container (e.g., `std::list`) (tradeoffs: perf, cache)

### C) Exception safety can poison internal state
If a slot throws and you don’t reset emission guards, `_emitting` / `_emit_depth` may remain “emitting” forever and cleanup never runs.

**Fix:** use an RAII guard to increment/decrement emit depth reliably, even on exceptions.

### D) `size()` / `empty()` mutating internal storage in `const`
Calling `cleanup()` from `size()`/`empty()` means “observer” methods mutate internal state, which can surprise users and complicate thread-safety.

**Decide:**
- Keep it (and document: “queries may compact/discard disconnected slots”), or
- Stop doing it and provide an explicit `compact()` / `cleanup()` method.

### E) Thread-safety is currently undefined
`connected()` reads non-atomic state; `_slots` mutations aren’t synchronized. This is fine if the library is single-threaded (UI thread), but should be stated.

**Decide:**
- “Not thread-safe; all ops on same thread” (document clearly), or
- Add locking / atomics (more complexity).

## Recommended Minimal Patch Set (Small + Correct)

Implement the following with minimal API changes:

1. Replace `_emitting` with `_emit_depth` and an RAII guard:
   - increment at start of `operator()`
   - decrement on scope exit
   - only cleanup when `_emit_depth == 0`

2. Prevent vector invalidation during emit:
   - Maintain a `std::vector<SlotEntry> _pending_add;`
   - In `connect()`:
     - if emitting: push to `_pending_add` and return the connection
     - else: push directly to `_slots`
   - After outermost emit ends (depth becomes 0):
     - append `_pending_add` to `_slots`
     - clear `_pending_add`
     - run cleanup

3. Ensure disconnects during emit are safe:
   - `Connection::disconnect()` already flips `state->connected = false`
   - That is OK during emit; just avoid erasing until after emission ends

4. Decide behavior for `disconnect_all()` during emit:
   - allow it: mark states disconnected; defer erase
   - if you add “pending actions”, avoid clearing `_slots` while emitting

5. Decide and document exception policy:
   - Option A: Let exceptions propagate, but state remains consistent due to RAII guard
   - Option B: Catch exceptions, store/report, continue calling remaining slots (document)

6. Address `size()` / `empty()` const-mutation:
   - Recommended: don’t compact automatically in these methods
   - Provide `compact()` for deterministic cleanup points
   - Alternatively, keep but document clearly

## Acceptance Tests / Scenarios to Validate

Ask Copilot to implement tests or small examples for:

1. Re-entrant emission:
   - a slot calls the same signal again
   - ensure no UB, no missed cleanup, correct call ordering policy documented

2. Connect during emit:
   - slot calls `connect()`
   - ensure it does not crash / UB
   - define whether newly-connected slot fires in the same emission or next one
     - recommended: **next emission**

3. Disconnect during emit:
   - slot disconnects itself or another connection
   - ensure the disconnected slot is not called again in the same emission if iteration reaches it later

4. `disconnect_all()` during emit:
   - ensure no UB
   - ensure after emission finishes, no slots remain connected

5. Exception thrown by slot:
   - verify emit-depth resets properly
   - verify cleanup still runs when allowed

6. Move semantics:
   - moving a `Signal` while connections exist: define behavior
     - connections typically refer to `ConnectionState` only, so they remain “connected” but the signal moved-from should be inert
   - consider whether to invalidate states on move assignment

## Documentation Notes (Add to Header Comments)

- Thread-safety: “All operations must occur on the same thread” (if chosen)
- Re-entrancy: allowed; connect/disconnect during emit behavior defined
- Delivery semantics for connects during emit: “takes effect next emit”
- Exception semantics: propagate vs swallow

## Output Requested from Copilot

1. A revised header-only implementation following the minimal patch set above.
2. Short comments explaining any behavioral choices (connect-during-emit semantics, exception handling).
3. (Optional) A tiny test/demo snippet showing safe re-entrant emit and connect/disconnect during emit.
