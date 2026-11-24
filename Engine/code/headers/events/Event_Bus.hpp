// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace engine
{

    namespace detail
    {
        inline std::size_t next_event_id ()
        {
            static std::size_t counter = 0;
            return counter++;
        }

        /// Small consecutive number per event type, like component ids.
        template< typename T >
        std::size_t event_id ()
        {
            static const std::size_t id = next_event_id ();
            return id;
        }
    }

    /// Identifies one subscription so it can be cancelled.
    struct Subscription
    {
        std::size_t   type = static_cast< std::size_t >(-1);
        std::uint32_t id   = 0;

        bool is_valid () const { return id != 0; }
    };

    /// Typed publish / subscribe. Any struct can be an event:
    ///
    ///     struct Landed { Entity entity; float impact_speed; };
    ///
    ///     auto subscription = events.subscribe< Landed >([] (const Landed & event) { ... });
    ///     events.publish (Landed{ player, 9.f });       // queued
    ///     events.dispatch ();                             // Event_System does this once per frame
    ///
    /// - publish() queues the event; handlers run at the next dispatch(), so the
    ///   sender never runs other code in the middle of its own update.
    /// - Events published while dispatching wait for the following dispatch.
    /// - emit() runs the handlers immediately, for the rare case that needs it.
    /// - Handlers may subscribe and unsubscribe while being called.
    class Event_Bus
    {
        class IQueue
        {
        public:

            virtual ~IQueue () = default;
            virtual void dispatch    ()                 = 0;
            virtual void unsubscribe (std::uint32_t id) = 0;
            virtual void clear       ()                 = 0;
            virtual std::size_t pending () const        = 0;
        };

        template< typename T >
        class Queue final : public IQueue
        {
            struct Handler
            {
                std::uint32_t                        id;
                std::function< void (const T & ) >   function;
                bool                                 active = true;
            };

            std::vector< T       > events;
            std::vector< Handler > handlers;
            std::vector< Handler > added_while_dispatching;
            int                    dispatch_depth = 0;
            bool                   has_removed    = false;

        public:

            void add_handler (std::uint32_t id, std::function< void (const T & ) > function)
            {
                // Never grow `handlers` while one of them runs: it would move the running function.
                if (dispatch_depth > 0) added_while_dispatching.push_back ({ id, std::move(function) });
                else                    handlers               .push_back ({ id, std::move(function) });
            }

            void push (T event)
            {
                events.push_back (std::move(event));
            }

            void emit (const T & event)
            {
                ++dispatch_depth;

                for (Handler & handler : handlers)
                {
                    if (handler.active) handler.function (event);
                }

                --dispatch_depth;
                tidy ();
            }

            void dispatch () override
            {
                // Take the queue first: events published from a handler go to the next dispatch.
                std::vector< T > current;
                current.swap (events);

                for (const T & event : current) emit (event);
            }

            void unsubscribe (std::uint32_t id) override
            {
                // Only marked here: the handler may be the one running right now.
                for (Handler & handler : handlers)               if (handler.id == id) handler.active = false;
                for (Handler & handler : added_while_dispatching) if (handler.id == id) handler.active = false;

                has_removed = true;
                tidy ();
            }

            void clear () override
            {
                events.clear ();
            }

            std::size_t pending () const override { return events.size (); }

            std::size_t handler_count () const
            {
                std::size_t total = 0;
                for (const Handler & handler : handlers)               if (handler.active) ++total;
                for (const Handler & handler : added_while_dispatching) if (handler.active) ++total;
                return total;
            }

        private:

            /// Applies the changes made while dispatching, once no handler is running.
            void tidy ()
            {
                if (dispatch_depth > 0) return;

                if (has_removed)
                {
                    std::erase_if (handlers,                [] (const Handler & handler) { return !handler.active; });
                    std::erase_if (added_while_dispatching, [] (const Handler & handler) { return !handler.active; });
                    has_removed = false;
                }

                for (Handler & handler : added_while_dispatching) handlers.push_back (std::move(handler));

                added_while_dispatching.clear ();
            }
        };

        std::vector< std::unique_ptr< IQueue > > queues;
        std::uint32_t                            next_id = 1;

    public:

        Event_Bus () = default;

        Event_Bus (const Event_Bus & ) = delete;
        Event_Bus & operator = (const Event_Bus & ) = delete;

    public:

        template< typename T, typename Function >
        Subscription subscribe (Function && function)
        {
            const std::uint32_t id = next_id++;
            queue< T >().add_handler (id, std::function< void (const T & ) >(std::forward< Function >(function)));
            return { detail::event_id< T >(), id };
        }

        /// Cancels a subscription. Unknown or already cancelled ones are ignored.
        void unsubscribe (Subscription & subscription)
        {
            if (subscription.is_valid () && subscription.type < queues.size () && queues[subscription.type])
            {
                queues[subscription.type]->unsubscribe (subscription.id);
            }

            subscription = {};
        }

        /// Queues the event until the next dispatch().
        template< typename T >
        void publish (T event)
        {
            queue< std::decay_t< T > >().push (std::move(event));
        }

        /// Calls the handlers of this event now.
        template< typename T >
        void emit (const T & event)
        {
            queue< T >().emit (event);
        }

        /// Delivers every queued event, type by type, in the order they were published.
        void dispatch ()
        {
            // Index loop: a handler may use a new event type and grow `queues`.
            for (std::size_t i = 0; i < queues.size (); ++i)
            {
                if (queues[i]) queues[i]->dispatch ();
            }
        }

        /// Drops every queued event (subscriptions stay).
        void clear ()
        {
            for (auto & q : queues) if (q) q->clear ();
        }

        template< typename T >
        std::size_t pending () const
        {
            const std::size_t id = detail::event_id< T >();
            return id < queues.size () && queues[id] ? queues[id]->pending () : 0;
        }

        template< typename T >
        std::size_t subscriber_count ()
        {
            return queue< T >().handler_count ();
        }

    private:

        template< typename T >
        Queue< T > & queue ()
        {
            const std::size_t id = detail::event_id< T >();

            if (id >= queues.size ()) queues.resize (id + 1);
            if (!queues[id]) queues[id] = std::make_unique< Queue< T > >();

            return static_cast< Queue< T > & >(*queues[id]);
        }
    };

}
