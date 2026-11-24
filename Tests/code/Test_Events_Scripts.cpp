// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Test_Framework.hpp"

#include <core/Input.hpp>
#include <ecs/Registry.hpp>
#include <events/Event_Bus.hpp>
#include <scripting/Script.hpp>
#include <scripting/Script_Component.hpp>
#include <systems/Script_System.hpp>

#include <string>
#include <vector>

using namespace engine;

namespace
{

    struct Hit   { int damage; };
    struct Shout { std::string text; };

}

// ---------------------------------------------------------------------------
// Event_Bus

TEST (published_events_wait_for_dispatch)
{
    Event_Bus events;
    int total = 0;

    events.subscribe< Hit >([&total] (const Hit & hit) { total += hit.damage; });

    events.publish (Hit{ 3 });
    events.publish (Hit{ 4 });

    CHECK (total == 0);
    CHECK (events.pending< Hit >() == 2);

    events.dispatch ();

    CHECK (total == 7);
    CHECK (events.pending< Hit >() == 0);
}

TEST (every_subscriber_receives_the_event_once)
{
    Event_Bus events;
    int a = 0, b = 0;

    events.subscribe< Hit >([&a] (const Hit & ) { ++a; });
    events.subscribe< Hit >([&b] (const Hit & ) { ++b; });

    events.publish (Hit{ 1 });
    events.dispatch ();
    events.dispatch ();                     // nothing left to deliver

    CHECK (a == 1);
    CHECK (b == 1);
}

TEST (event_types_are_independent)
{
    Event_Bus events;
    int hits = 0;
    std::string heard;

    events.subscribe< Hit   >([&hits ] (const Hit   & ) { ++hits; });
    events.subscribe< Shout >([&heard] (const Shout & shout) { heard = shout.text; });

    events.publish (Shout{ "hello" });
    events.dispatch ();

    CHECK (hits == 0);
    CHECK (heard == "hello");
}

TEST (emit_calls_handlers_immediately)
{
    Event_Bus events;
    int total = 0;

    events.subscribe< Hit >([&total] (const Hit & hit) { total += hit.damage; });
    events.emit (Hit{ 5 });

    CHECK (total == 5);
}

TEST (unsubscribed_handler_is_not_called)
{
    Event_Bus events;
    int total = 0;

    Subscription subscription = events.subscribe< Hit >([&total] (const Hit & ) { ++total; });
    events.unsubscribe (subscription);

    events.publish (Hit{ 1 });
    events.dispatch ();

    CHECK (total == 0);
    CHECK (!subscription.is_valid ());
    CHECK (events.subscriber_count< Hit >() == 0);
}

TEST (handler_can_unsubscribe_itself_while_running)
{
    Event_Bus events;
    int calls = 0;
    Subscription subscription;

    subscription = events.subscribe< Hit >([&] (const Hit & )
    {
        ++calls;
        events.unsubscribe (subscription);
    });

    events.publish (Hit{ 1 });
    events.publish (Hit{ 1 });
    events.dispatch ();

    CHECK (calls == 1);
}

TEST (handler_subscribed_while_dispatching_starts_next_event)
{
    Event_Bus events;
    int late_calls = 0;
    bool added = false;

    events.subscribe< Hit >([&] (const Hit & )
    {
        if (!added)
        {
            added = true;
            events.subscribe< Hit >([&late_calls] (const Hit & ) { ++late_calls; });
        }
    });

    events.publish (Hit{ 1 });
    events.dispatch ();
    CHECK (late_calls == 0);

    events.publish (Hit{ 1 });
    events.dispatch ();
    CHECK (late_calls == 1);
}

TEST (events_published_while_dispatching_wait_for_next_dispatch)
{
    Event_Bus events;
    int shouts = 0;

    events.subscribe< Hit   >([&events] (const Hit & ) { events.publish (Hit{ 0 }); });
    events.subscribe< Shout >([&shouts] (const Shout & ) { ++shouts; });

    events.publish (Hit{ 1 });
    events.dispatch ();

    // The Hit published from the handler must not be delivered in the same dispatch.
    CHECK (events.pending< Hit >() == 1);
    CHECK (shouts == 0);
}

// ---------------------------------------------------------------------------
// Scripts

namespace
{

    struct Counter { int value = 0; };

    struct Log
    {
        std::vector< std::string > lines;
    };

    class Recorder final : public Script
    {
        Log & log;

    public:

        explicit Recorder (Log & given_log) : log(given_log) {}

        void on_create  ()         override { log.lines.push_back ("create");  }
        void on_update  (float dt) override { log.lines.push_back ("update " + std::to_string (int(dt * 10))); }
        void on_destroy ()         override { log.lines.push_back ("destroy"); }
    };

    class Incrementer final : public Script
    {
    public:
        void on_update (float ) override { ++get< Counter >().value; }
    };

    class Self_Destruct final : public Script
    {
        int frames_left;
    public:
        explicit Self_Destruct (int frames) : frames_left(frames) {}
        void on_update (float ) override { if (--frames_left == 0) destroy (); }
    };

    class Listener final : public Script
    {
    public:
        int heard = 0;
        void on_create () override { subscribe< Hit >([this] (const Hit & ) { ++heard; }); }
    };

    class Spawner final : public Script
    {
    public:
        void on_update (float ) override
        {
            const Entity child = registry ().create ();
            registry ().add< Counter >(child);
            registry ().add< Script_Component >(child).bind< Incrementer >();
        }
    };

}

TEST (script_lifecycle_order)
{
    Registry registry;
    Event_Bus events;
    Input input;
    Script_System system (registry, events, input);

    Log log;
    const Entity entity = registry.create ();
    registry.add< Script_Component >(entity).bind< Recorder >(log);

    system.execute (0.1f);
    system.execute (0.2f);
    system.finalize ();

    CHECK (log.lines == std::vector< std::string >{ "create", "update 1", "update 2", "destroy" });
}

TEST (scripts_reach_their_own_components)
{
    Registry registry;
    Event_Bus events;
    Input input;
    Script_System system (registry, events, input);

    const Entity entity = registry.create ();
    registry.add< Counter >(entity);
    registry.add< Script_Component >(entity).bind< Incrementer >();

    for (int i = 0; i < 3; ++i) system.execute (0.016f);

    CHECK (registry.get< Counter >(entity).value == 3);
}

TEST (several_scripts_on_one_entity_run_in_order)
{
    Registry registry;
    Event_Bus events;
    Input input;
    Script_System system (registry, events, input);

    Log log;
    const Entity entity = registry.create ();
    registry.add< Counter >(entity);

    Script_Component & scripts = registry.add< Script_Component >(entity);
    scripts.bind< Incrementer >();
    scripts.bind< Recorder    >(log);

    system.execute (0.1f);

    CHECK (registry.get< Counter >(entity).value == 1);
    CHECK (log.lines.size () == 2);
}

TEST (destroy_from_a_script_is_deferred_and_calls_on_destroy)
{
    Registry registry;
    Event_Bus events;
    Input input;
    Script_System system (registry, events, input);

    Log log;
    const Entity entity = registry.create ();

    Script_Component & scripts = registry.add< Script_Component >(entity);
    scripts.bind< Recorder      >(log);
    scripts.bind< Self_Destruct >(2);

    system.execute (0.1f);
    CHECK (registry.valid (entity));

    system.execute (0.1f);
    CHECK (!registry.valid (entity));
    CHECK (log.lines.back () == "destroy");

    system.execute (0.1f);                  // nothing left to run
    CHECK (registry.alive_count () == 0);
}

TEST (script_subscriptions_end_with_the_script)
{
    Registry registry;
    Event_Bus events;
    Input input;
    Script_System system (registry, events, input);

    const Entity entity = registry.create ();
    Listener & listener = registry.add< Script_Component >(entity).bind< Listener >();

    system.execute (0.1f);

    events.publish (Hit{ 1 });
    events.dispatch ();
    CHECK (listener.heard == 1);
    CHECK (events.subscriber_count< Hit >() == 1);

    registry.destroy (entity);              // deletes the script

    CHECK (events.subscriber_count< Hit >() == 0);

    events.publish (Hit{ 1 });
    events.dispatch ();                     // would crash if the handler were still there
}

TEST (scripts_created_during_update_start_next_frame)
{
    Registry registry;
    Event_Bus events;
    Input input;
    Script_System system (registry, events, input);

    const Entity spawner = registry.create ();
    registry.add< Script_Component >(spawner).bind< Spawner >();

    system.execute (0.1f);                  // spawns child 1
    CHECK (registry.alive_count () == 2);
    CHECK (registry.view< Counter >().count () == 1);

    system.execute (0.1f);                  // child 1 runs once, spawns child 2

    int total = 0;
    registry.view< Counter >().each ([&total] (Counter & counter) { total += counter.value; });

    CHECK (registry.alive_count () == 3);
    CHECK (total == 1);
}
