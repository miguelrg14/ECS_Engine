// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Task.hpp"
#include "Timer.hpp"

#include <cstdint>
#include <vector>

namespace engine
{

    /// Runs the main loop: every frame it executes its tasks in priority order,
    /// passing them the duration of the previous frame.
    ///
    /// Tasks added or removed while the loop runs are applied at the start of the
    /// next frame, so the list never changes while it is being iterated.
    class Kernel
    {
        std::vector< Task * > tasks;            // sorted by priority, stable for equal values
        std::vector< Task * > pending_add;
        std::vector< Task * > pending_remove;

        bool running        = false;
        bool stop_requested = false;

        Timer frame_timer;
        float delta_time     = 0.f;
        float max_delta_time = 0.25f;           // avoids huge steps after a breakpoint or a window drag

        Timer         fps_timer;
        unsigned      fps_frames  = 0;
        float         fps         = 0.f;
        std::uint64_t frame_count = 0;

    public:

        void add    (Task & task);
        void remove (Task & task);

        /// Blocks until stop() is called. Calls initialize on every task first
        /// and finalize on every task, in reverse order, at the end.
        void run  ();
        void stop () { stop_requested = true; }

    public:

        bool          is_running      () const { return running;     }
        float         get_delta_time  () const { return delta_time;  }
        float         get_fps         () const { return fps;         }
        std::uint64_t get_frame_count () const { return frame_count; }
        std::size_t   get_task_count  () const { return tasks.size (); }

        void set_max_delta_time (float seconds) { max_delta_time = seconds; }

    private:

        void insert_sorted  (Task & task);
        bool contains       (const Task & task) const;
        void apply_pending  ();
        void update_timing  ();
    };

}
