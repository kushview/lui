// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace lui {

class Connection;
class ScopedConnection;

namespace detail {

/// Base connection state shared between Signal and Connection handles
struct ConnectionState {
    bool connected = true;
    uint64_t id    = 0;
};

} // namespace detail

/// Handle to a signal connection. Can be used to disconnect.
class Connection {
public:
    Connection() = default;

    /// Check if this connection is still active
    [[nodiscard]] bool connected() const {
        return state && state->connected;
    }

    /// Disconnect this connection
    void disconnect() {
        if (state) {
            state->connected = false;
            state.reset();
        }
    }

    /// Release ownership without disconnecting
    void release() {
        state.reset();
    }

    /// Check if this connection is valid
    [[nodiscard]] explicit operator bool() const {
        return connected();
    }

private:
    friend class ScopedConnection;
    template <typename>
    friend class Signal;

    explicit Connection (std::shared_ptr<detail::ConnectionState> s)
        : state (std::move (s)) {}

    std::shared_ptr<detail::ConnectionState> state;
};

/// RAII connection handle that automatically disconnects on destruction
class ScopedConnection {
public:
    ScopedConnection() = default;

    explicit ScopedConnection (Connection&& conn)
        : connection (std::move (conn)) {}

    ~ScopedConnection() {
        disconnect();
    }

    // Non-copyable
    ScopedConnection (const ScopedConnection&)            = delete;
    ScopedConnection& operator= (const ScopedConnection&) = delete;

    // Movable
    ScopedConnection (ScopedConnection&& other) noexcept
        : connection (std::move (other.connection)) {}

    ScopedConnection& operator= (ScopedConnection&& other) noexcept {
        if (this != &other) {
            disconnect();
            connection = std::move (other.connection);
        }
        return *this;
    }

    /// Check if this connection is still active
    [[nodiscard]] bool connected() const {
        return connection.connected();
    }

    /// Disconnect this connection
    void disconnect() {
        connection.disconnect();
    }

    /// Release ownership without disconnecting
    [[nodiscard]] Connection release() {
        Connection conn = std::move (connection);
        connection      = Connection();
        return conn;
    }

    /// Check if this connection is valid
    [[nodiscard]] explicit operator bool() const {
        return connected();
    }

private:
    Connection connection;
};

/// Type-safe signal/slot implementation
template <typename Signature>
class Signal;

template <typename... Args>
class Signal<void (Args...)> {
public:
    using Slot = std::function<void (Args...)>;

    Signal() = default;
    ~Signal() {
        disconnect_all();
    }

    // Non-copyable, movable
    Signal (const Signal&)            = delete;
    Signal& operator= (const Signal&) = delete;

    Signal (Signal&& other) noexcept
        : _slots (std::move (other._slots)), _next_id (other._next_id), _emitting (other._emitting) {}

    Signal& operator= (Signal&& other) noexcept {
        if (this != &other) {
            disconnect_all();
            _slots    = std::move (other._slots);
            _next_id  = other._next_id;
            _emitting = other._emitting;
        }
        return *this;
    }

    /// Connect a slot to this signal
    Connection connect (Slot slot) {
        auto state = std::make_shared<detail::ConnectionState>();
        state->id  = _next_id++;
        _slots.push_back ({ state, std::move (slot) });
        return Connection (state);
    }

    /// Disconnect all slots
    void disconnect_all() {
        for (auto& entry : _slots) {
            if (entry.state) {
                entry.state->connected = false;
            }
        }
        if (! _emitting) {
            _slots.clear();
        }
    }

    /// Emit the signal, calling all connected slots
    void operator() (Args... args) const {
        // Prevent recursive modifications during emission
        _emitting = true;

        for (const auto& entry : _slots) {
            if (entry.state && entry.state->connected && entry.slot) {
                entry.slot (args...);
            }
        }

        _emitting = false;

        // Clean up disconnected slots after emission
        cleanup();
    }

    /// Get number of connected slots
    [[nodiscard]] size_t size() const {
        cleanup();
        return _slots.size();
    }

    /// Check if any slots are connected
    [[nodiscard]] bool empty() const {
        cleanup();
        return _slots.empty();
    }

private:
    struct SlotEntry {
        std::shared_ptr<detail::ConnectionState> state;
        Slot slot;
    };

    void cleanup() const {
        if (_emitting) {
            return; // Don't modify during emission
        }

        _slots.erase (
            std::remove_if (_slots.begin(),
                            _slots.end(),
                            [] (const SlotEntry& entry) {
                                return ! entry.state || ! entry.state->connected;
                            }),
            _slots.end());
    }

    mutable std::vector<SlotEntry> _slots;
    uint64_t _next_id      = 0;
    mutable bool _emitting = false;
};

} // namespace lui
