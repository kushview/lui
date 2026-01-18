// Copyright 2026 Kushview, LLC
// SPDX-License-Identifier: ISC

#include "tests.hpp"
#include <lui/signal.hpp>

using namespace lui;

TEST(SignalTest, BasicEmit) {
    Signal<void(int)> signal;
    int received = 0;

    signal.connect([&](int value) { received = value; });
    signal(42);

    EXPECT_EQ(received, 42);
}

TEST(SignalTest, MultipleSlots) {
    Signal<void(int)> signal;
    int sum = 0;

    signal.connect([&](int value) { sum += value; });
    signal.connect([&](int value) { sum += value * 2; });

    signal(10);

    EXPECT_EQ(sum, 30); // 10 + 20
}

TEST(SignalTest, ConnectionDisconnect) {
    Signal<void(int)> signal;
    int count = 0;

    Connection conn = signal.connect([&](int) { count++; });

    signal(1);
    EXPECT_EQ(count, 1);

    conn.disconnect();
    signal(2);
    EXPECT_EQ(count, 1); // Should not increment
}

TEST(SignalTest, ConnectionConnected) {
    Signal<void()> signal;
    Connection conn = signal.connect([]() {});

    EXPECT_TRUE(conn.connected());
    conn.disconnect();
    EXPECT_FALSE(conn.connected());
}

TEST(SignalTest, ScopedConnection) {
    Signal<void(int)> signal;
    int count = 0;

    {
        ScopedConnection scoped(signal.connect([&](int) { count++; }));
        signal(1);
        EXPECT_EQ(count, 1);
    } // scoped destroyed here

    signal(2);
    EXPECT_EQ(count, 1); // Should not increment
}

TEST(SignalTest, ScopedConnectionMove) {
    Signal<void(int)> signal;
    int count = 0;

    ScopedConnection scoped1(signal.connect([&](int) { count++; }));
    signal(1);
    EXPECT_EQ(count, 1);

    ScopedConnection scoped2 = std::move(scoped1);
    signal(2);
    EXPECT_EQ(count, 2);

    EXPECT_FALSE(scoped1.connected());
    EXPECT_TRUE(scoped2.connected());
}

TEST(SignalTest, ScopedConnectionRelease) {
    Signal<void(int)> signal;
    int count = 0;

    ScopedConnection scoped(signal.connect([&](int) { count++; }));
    Connection conn = scoped.release();

    signal(1);
    EXPECT_EQ(count, 1);

    // Destroying scoped should not disconnect
    scoped = ScopedConnection();
    signal(2);
    EXPECT_EQ(count, 2);

    conn.disconnect();
    signal(3);
    EXPECT_EQ(count, 2);
}

TEST(SignalTest, DisconnectAll) {
    Signal<void(int)> signal;
    int count = 0;

    signal.connect([&](int) { count++; });
    signal.connect([&](int) { count++; });

    signal(1);
    EXPECT_EQ(count, 2);

    signal.disconnect_all();
    signal(2);
    EXPECT_EQ(count, 2); // Should not increment
}

TEST(SignalTest, MultipleParameters) {
    Signal<void(int, float, const std::string&)> signal;
    int received_int = 0;
    float received_float = 0.0f;
    std::string received_str;

    signal.connect([&](int i, float f, const std::string& s) {
        received_int = i;
        received_float = f;
        received_str = s;
    });

    signal(42, 3.14f, "test");

    EXPECT_EQ(received_int, 42);
    EXPECT_FLOAT_EQ(received_float, 3.14f);
    EXPECT_EQ(received_str, "test");
}

TEST(SignalTest, DisconnectDuringEmit) {
    Signal<void()> signal;
    int count = 0;
    Connection conn1, conn2;

    conn1 = signal.connect([&]() {
        count++;
        conn2.disconnect(); // Disconnect other slot during emission
    });

    conn2 = signal.connect([&]() { count += 10; });

    signal();
    EXPECT_EQ(count, 1); // conn2 not called (disconnected before its turn)

    signal();
    EXPECT_EQ(count, 2); // Only conn1 called
}

TEST(SignalTest, SelfDisconnectDuringEmit) {
    Signal<void()> signal;
    int count = 0;
    Connection conn;

    conn = signal.connect([&]() {
        count++;
        conn.disconnect(); // Self-disconnect
    });

    signal();
    EXPECT_EQ(count, 1);

    signal();
    EXPECT_EQ(count, 1); // Should not increment
}

TEST(SignalTest, ReentrantEmission) {
    Signal<void(int)> signal;
    std::vector<int> order;

    signal.connect([&](int value) {
        order.push_back(value);
        if (value < 3) {
            signal(value + 1); // Reentrant emit
        }
    });

    signal(1);

    // Should get: 1, 2, 3 (depth-first)
    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

TEST(SignalTest, EmptySignal) {
    Signal<void(int)> signal;

    EXPECT_TRUE(signal.empty());
    EXPECT_EQ(signal.size(), 0u);

    signal(42); // Should not crash

    Connection conn = signal.connect([](int) {});
    EXPECT_FALSE(signal.empty());
    EXPECT_EQ(signal.size(), 1u);

    conn.disconnect();
    EXPECT_TRUE(signal.empty());
    EXPECT_EQ(signal.size(), 0u);
}

TEST(SignalTest, MoveSignal) {
    Signal<void(int)> signal1;
    int count = 0;

    signal1.connect([&](int) { count++; });
    signal1(1);
    EXPECT_EQ(count, 1);

    Signal<void(int)> signal2 = std::move(signal1);
    signal2(2);
    EXPECT_EQ(count, 2);
}

TEST(SignalTest, NoParameters) {
    Signal<void()> signal;
    bool called = false;

    signal.connect([&]() { called = true; });
    signal();

    EXPECT_TRUE(called);
}

TEST(SignalTest, ConnectionRelease) {
    Signal<void()> signal;
    int count = 0;

    Connection conn = signal.connect([&]() { count++; });
    signal();
    EXPECT_EQ(count, 1);

    conn.release(); // Release without disconnecting
    signal();
    EXPECT_EQ(count, 2); // Still connected

    EXPECT_FALSE(conn.connected()); // But connection handle is invalid
}
