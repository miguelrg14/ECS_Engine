// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#include "Test_Framework.hpp"

#include <chrono>
#include <cstdio>
#include <exception>

int main ()
{
    int failures = 0;

    const auto start = std::chrono::steady_clock::now ();

    for (const tests::Test_Case & test : tests::all_tests ())
    {
        try
        {
            test.function ();
            std::printf ("[  OK  ] %s\n", test.name);
        }
        catch (const tests::Check_Failure & failure)
        {
            ++failures;
            std::printf ("[ FAIL ] %s\n         %s\n", test.name, failure.message.c_str ());
        }
        catch (const std::exception & exception)
        {
            ++failures;
            std::printf ("[ FAIL ] %s\n         exception: %s\n", test.name, exception.what ());
        }
    }

    const double milliseconds = std::chrono::duration< double, std::milli >(std::chrono::steady_clock::now () - start).count ();

    std::printf
    (
        "\n%zu tests, %d failed (%.1f ms)\n",
        tests::all_tests ().size (), failures, milliseconds
    );

    return failures;
}
