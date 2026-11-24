// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "core/Kernel.hpp"
#include "core/Log.hpp"

#include <algorithm>

namespace engine
{

    void Kernel::add (Task & task)
    {
        if (contains (task))
        {
            log_warning ("Kernel: task '%s' is already added", task.get_name ().c_str ());
            return;
        }

        if (running) pending_add.push_back (&task);
        else         insert_sorted (task);
    }

    void Kernel::remove (Task & task)
    {
        if (running)
        {
            pending_remove.push_back (&task);
        }
        else
        {
            std::erase (tasks, &task);
        }
    }

    void Kernel::run ()
    {
        running        = true;
        stop_requested = false;

        for (Task * task : tasks) task->initialize ();

        log_info ("Kernel: running %zu tasks", tasks.size ());

        for (Task * task : tasks)
        {
            log_info ("  %4d  %s", task->get_priority (), task->get_name ().c_str ());
        }

        frame_timer.reset ();
        fps_timer  .reset ();

        while (!stop_requested)
        {
            apply_pending ();
            update_timing ();

            for (Task * task : tasks)
            {
                task->execute (delta_time);
            }

            ++frame_count;
        }

        // Finalize in reverse order: a task never outlives the ones it depends on.

        for (auto task = tasks.rbegin (); task != tasks.rend (); ++task)
        {
            (*task)->finalize ();
        }

        running = false;

        log_info ("Kernel: stopped after %llu frames", static_cast< unsigned long long >(frame_count));
    }

    void Kernel::insert_sorted (Task & task)
    {
        // upper_bound keeps tasks with the same priority in the order they were added.

        auto position = std::upper_bound
        (
            tasks.begin (), tasks.end (), task.get_priority (),
            [] (int priority, const Task * other) { return priority < other->get_priority (); }
        );

        tasks.insert (position, &task);
    }

    bool Kernel::contains (const Task & task) const
    {
        auto same = [&task] (const Task * other) { return other == &task; };

        return std::ranges::any_of (tasks, same) || std::ranges::any_of (pending_add, same);
    }

    void Kernel::apply_pending ()
    {
        for (Task * task : pending_remove)
        {
            if (std::erase (tasks, task) > 0) task->finalize ();

            std::erase (pending_add, task);
        }

        pending_remove.clear ();

        for (Task * task : pending_add)
        {
            insert_sorted (*task);
            task->initialize ();
        }

        pending_add.clear ();
    }

    void Kernel::update_timing ()
    {
        // The best estimate of the coming frame's duration is the last one.

        delta_time = std::min (frame_timer.elapsed_seconds (), max_delta_time);
        frame_timer.reset ();

        // Frames per second averaged over half a second, so the number is readable.

        ++fps_frames;

        const float elapsed = fps_timer.elapsed_seconds ();

        if (elapsed >= 0.5f)
        {
            fps        = float(fps_frames) / elapsed;
            fps_frames = 0;
            fps_timer.reset ();
        }
    }

}
