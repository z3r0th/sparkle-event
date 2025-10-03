#include <catch2/catch_all.hpp>
#include <Sparkle/Event.h>
#include <memory>
#include <string>

using namespace Sparkle;

struct TestObject {
    int counter = 0;

    void Increment() { counter++; }

    void Add(int value) { counter += value; }
};

TEST_CASE("Lambda binding works", "[event]") {
    Event<int> onValue("OnValue");
    int result = 0;

    onValue.Bind([&](int v) { result = v; });
    onValue(123);

    REQUIRE(result == 123);
}

TEST_CASE("BindOnce executes only once", "[event]") {
    Event onPing("OnPing");
    int count = 0;

    onPing.BindOnce([&]() { count++; });

    onPing();
    REQUIRE(count == 1);

    onPing();
    REQUIRE(count == 1); // should not be called again
}

TEST_CASE("Member function binding with raw pointer", "[event]") {
    Event<> onIncrement("OnIncrement");
    TestObject obj;

    onIncrement.Bind(&TestObject::Increment, &obj);
    onIncrement();
    onIncrement();

    REQUIRE(obj.counter == 2);
}

TEST_CASE("Member function binding with weak_ptr auto expires", "[event]") {
    Event<> onIncrement("OnIncrement");

    std::weak_ptr<TestObject> weak;
    {
        auto strong = std::make_shared<TestObject>();
        weak = strong;

        onIncrement.Bind(&TestObject::Increment, weak);

        onIncrement();
        REQUIRE(strong->counter == 1);
    }

// strong is gone, weak expired
    REQUIRE_FALSE(weak.lock());
    onIncrement();
    REQUIRE(onIncrement.Size() == 0); // expired callback removed on Raise
}

TEST_CASE("BindOnce with member function", "[event]") {
    Event<int> onAdd("OnAdd");
    TestObject obj;

    onAdd.BindOnce(&TestObject::Add, &obj);

    onAdd(5);
    REQUIRE(obj.counter == 5);

    onAdd(10);
    REQUIRE(obj.counter == 5); // not called again
}

TEST_CASE("Multiple callbacks are invoked", "[event]") {
    Event<int> onMulti("OnMulti");

    int a = 0, b = 0;
    onMulti.Bind([&](int v) { a = v; });
    onMulti.Bind([&](int v) { b = v * 2; });

    onMulti(7);
    REQUIRE(a == 7);
    REQUIRE(b == 14);

    REQUIRE(onMulti.CallbackCount() == 2);
}

TEST_CASE("Clear removes all callbacks", "[event]") {
    Event<int> onClear("OnClear");

    int called = 0;
    onClear.Bind([&](int v) { called = v; });

    REQUIRE(onClear.CallbackCount() == 1);

    onClear.Clear();
    REQUIRE(onClear.CallbackCount() == 0);

    onClear(42);
    REQUIRE(called == 0); // not called
}

TEST_CASE("Remove by raw pointer works", "[event]") {
    Event<> onIncrement("OnIncrement");
    TestObject obj;

    onIncrement.Bind(&TestObject::Increment, &obj);
    REQUIRE(onIncrement.Size() == 1);

    bool removed = onIncrement.Remove(&obj);
    REQUIRE(removed);
    REQUIRE(onIncrement.Size() == 0);
}

TEST_CASE("Remove by weak_ptr works", "[event]") {
    Event<> onIncrement("OnIncrement");

    auto strong = std::make_shared<TestObject>();
    auto weak = std::weak_ptr<TestObject>(strong);

    onIncrement.Bind(&TestObject::Increment, weak);
    REQUIRE(onIncrement.Size() == 1);

    bool removed = onIncrement.Remove(weak);
    REQUIRE(removed);
    REQUIRE(onIncrement.Size() == 0);
}

TEST_CASE("IsBound detects bindings", "[event]") {
    Event<> onEvent("OnEvent");
    TestObject obj;

    REQUIRE_FALSE(onEvent.GetBinder().IsBound(&obj));
    onEvent.Bind(&TestObject::Increment, &obj);
    REQUIRE(onEvent.GetBinder().IsBound(&obj));
}