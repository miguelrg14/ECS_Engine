// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <chrono>

namespace engine
{

    /// Measures elapsed time with a monotonic clock (never goes backwards).
    class Timer
    {
        using Clock = std::chrono::steady_clock;

        Clock::time_point start = Clock::now ();

    public:

        void reset ()
        {
            start = Clock::now ();
        }

        template< typename T = float >
        T elapsed_seconds () const
        {
            return std::chrono::duration< T >(Clock::now () - start).count ();
        }
    };

}
