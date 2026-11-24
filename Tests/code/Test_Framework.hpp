// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

// Minimal test runner: TEST(name) { ... CHECK(condition); ... }
// CHECK is variadic so conditions with template commas need no extra parentheses.
// Each test registers itself before main runs; main runs them all and returns
// the number of failures, so a failing test also fails the build step.

#pragma once

#include <string>
#include <vector>

namespace tests
{

    struct Test_Case
    {
        const char * name;
        void      (* function) ();
    };

    struct Check_Failure
    {
        std::string message;
    };

    inline std::vector< Test_Case > & all_tests ()
    {
        static std::vector< Test_Case > list;
        return list;
    }

    struct Registrar
    {
        Registrar (const char * name, void (* function) ())
        {
            all_tests ().push_back ({ name, function });
        }
    };

}

#define TEST(name)                                                          \
    static void name ();                                                    \
    static const tests::Registrar name##_registrar (#name, &name);         \
    static void name ()

#define CHECK(...)                                                          \
    do                                                                      \
    {                                                                       \
        if (!(__VA_ARGS__))                                                 \
        {                                                                   \
            throw tests::Check_Failure                                      \
            {                                                               \
                std::string(__FILE__) + "(" + std::to_string(__LINE__) +   \
                "): CHECK(" #__VA_ARGS__ ") failed"                         \
            };                                                              \
        }                                                                   \
    }                                                                       \
    while (false)
