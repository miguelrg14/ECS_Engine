// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <string>
#include <utility>

namespace engine
{

    /// Execution order of the tasks in a frame: lower values run first.
    /// Gaps between values leave room for tasks added later.
    namespace Priority
    {
        inline constexpr int input     =   10;
        inline constexpr int scripts   =   20;
        inline constexpr int movement  =   30;
        inline constexpr int transform =   50;
        inline constexpr int collision =   55;     // needs this frame's world matrices
        inline constexpr int events    =   60;
        inline constexpr int render    =   90;
        inline constexpr int debug_ui  =  100;
        inline constexpr int present   = 1000;
    }

    /// Unit of work run by the Kernel once per frame.
    /// The Kernel only holds a pointer: whoever creates a task keeps it alive while it is added.
    class Task
    {
        std::string name;
        int         priority;

    public:

        Task (std::string given_name, int given_priority)
        :
            name    (std::move(given_name)),
            priority(given_priority)
        {
        }

        virtual ~Task () = default;

        Task (const Task & ) = delete;
        Task & operator = (const Task & ) = delete;

    public:

        /// Called once, before the first execute (or when added to a running kernel).
        virtual void initialize () {}

        /// Called every frame with the duration of the previous frame in seconds.
        virtual void execute (float delta_time) = 0;

        /// Called once, after the last execute (or when removed from a running kernel).
        virtual void finalize () {}

    public:

        const std::string & get_name     () const { return name;     }
        int                 get_priority () const { return priority; }
    };

}
