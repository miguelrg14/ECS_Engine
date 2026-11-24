// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "ecs/Registry.hpp"
#include "events/Event_Bus.hpp"

#include <vector>

namespace engine
{

    class Input;
    class Resource_Manager;
    class Script_System;
    class Window;

    /// Base class of gameplay logic written in C++ and attached to an entity
    /// through a Script_Component. Override the callbacks you need:
    ///
    ///     class Spinner : public Script
    ///     {
    ///         void on_update (float dt) override
    ///         {
    ///             get< Transform >().position.y += dt;
    ///         }
    ///     };
    ///
    ///     registry.add< Script_Component >(entity).bind< Spinner >();
    ///
    /// - on_create runs once, the first frame the script is updated.
    /// - on_update runs every frame (Script_System, priority 20).
    /// - on_destroy runs when the entity is destroyed with destroy(), or when the game ends.
    ///
    /// Subscriptions made with subscribe() are cancelled when the script is deleted.
    class Script
    {
        friend class Script_System;

        Entity             self             = null_entity;
        Registry         * registry_pointer = nullptr;
        Event_Bus        * event_bus        = nullptr;
        const Input      * input_pointer    = nullptr;
        Window           * window_pointer   = nullptr;
        Resource_Manager * resources_pointer= nullptr;
        Script_System    * owner            = nullptr;
        bool               created          = false;

        std::vector< Subscription > subscriptions;

    public:

        Script () = default;
        virtual ~Script ();

        Script (const Script & ) = delete;
        Script & operator = (const Script & ) = delete;

    public:

        virtual void on_create  ()                 {}
        virtual void on_update  (float delta_time) { (void)delta_time; }
        virtual void on_destroy ()                 {}

    protected:

        Entity             entity    () const { return self; }
        Registry         & registry  () const { return *registry_pointer;  }
        Event_Bus        & events    () const { return *event_bus;         }
        const Input      & input     () const { return *input_pointer;     }

        /// Null when the engine runs without a window (unit tests).
        Window           * window    () const { return window_pointer;     }
        Resource_Manager * resources () const { return resources_pointer;  }

        template< typename T > T  & get     () const { return registry ().get     < T >(self); }
        template< typename T > T  * try_get () const { return registry ().try_get < T >(self); }
        template< typename T > bool has     () const { return registry ().has     < T >(self); }

        template< typename T >
        void publish (T event) const { events ().publish (std::move(event)); }

        /// Subscribes for as long as this script lives.
        template< typename T, typename Function >
        void subscribe (Function && function)
        {
            subscriptions.push_back (events ().template subscribe< T >(std::forward< Function >(function)));
        }

        /// Destroys an entity (this one by default) at the end of the script pass,
        /// after calling on_destroy on its scripts. Safe to call from on_update.
        void destroy (Entity target = null_entity) const;
    };

}
