//
//  Lol Engine
//
//  Copyright © 2010—2017 Sam Hocevar <sam@hocevar.net>
//
//  Lol Engine is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#include <lol/engine-internal.h>

#include <cstdio>
#include <cstdlib>

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   undef WIN32_LEAN_AND_MEAN
#endif

#if defined(__ANDROID__)
#   include <android/log.h>
#   include <unistd.h> /* for gettid() */
#else
#   include <cstdarg>
#endif

namespace lol
{

/*
 * Public log class
 */

void msg::debug(char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    helper(MessageType::Debug, fmt, ap);
    va_end(ap);
}

void msg::info(char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    helper(MessageType::Info, fmt, ap);
    va_end(ap);
}

void msg::warn(char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    helper(MessageType::Warning, fmt, ap);
    va_end(ap);
}

void msg::error(char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    helper(MessageType::Error, fmt, ap);
    va_end(ap);
}

/*
 * Private helper function
 */

void msg::helper(MessageType type, char const *fmt, va_list ap)
{
    /* Unless this is a debug build, ignore debug messages unless
     * the LOL_DEBUG environment variable is set. */
#if !defined LOL_BUILD_DEBUG
    if (type == MessageType::Debug)
    {
        static char const *var = getenv("LOL_DEBUG");
        static bool const disable_debug = !var || !var[0];
        if (disable_debug)
            return;
    }
#endif

#if defined __ANDROID__
    static int const prio[] =
    {
        ANDROID_LOG_DEBUG,
        ANDROID_LOG_INFO,
        ANDROID_LOG_WARN,
        ANDROID_LOG_ERROR
    };

    std::string buf = vformat(fmt, ap);
    __android_log_print(prio[(int)type], "LOL", "[%d] %s", (int)gettid(), &buf[0]);

#else
    static char const * const prefix[] =
    {
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
    };

#   if defined _WIN32
    std::string buf = std::string(prefix[(int)type]) + ": " + vformat(fmt, ap);

    array<WCHAR> widechar;
    widechar.resize(buf.length() + 1);
    MultiByteToWideChar(CP_UTF8, 0, buf.c_str(), buf.length() + 1, widechar.data(), widechar.count());
    OutputDebugStringW(widechar.data());
#   else
    fprintf(stderr, "%s: ", prefix[(int)type]);
    vfprintf(stderr, fmt, ap);
#   endif
#endif
}

} /* namespace lol */

