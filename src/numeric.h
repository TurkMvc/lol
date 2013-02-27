//
// Lol Engine
//
// Copyright: (c) 2010-2013 Sam Hocevar <sam@hocevar.net>
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the Do What The Fuck You Want To
//   Public License, Version 2, as published by Sam Hocevar. See
//   http://www.wtfpl.net/ for more details.
//

//
// The Matrix classes
// ------------------
//

#if !defined __LOL_NUMERIC_H__
#define __LOL_NUMERIC_H__

#include <cstdlib>
#include <stdint.h>

namespace lol
{

/* Next power of two. */
template <typename T> static inline T PotUp(T val)
{
    val = val - 1;
    if (sizeof(val) > 4) val = val | ((uint64_t)val >> 32);
    if (sizeof(val) > 2) val = val | ((uint64_t)val >> 16);
    if (sizeof(val) > 1) val = val | ((uint64_t)val >> 8);
    val = val | ((uint64_t)val >> 4);
    val = val | ((uint64_t)val >> 2);
    val = val | ((uint64_t)val >> 1);
    return val + 1;
}

//Damp for float
template <typename T1, typename T2, typename Tf> static inline T1 damp(const T1 &a, const T2 &b, const Tf &x, const Tf &dt)
{
    if (dt <= .0f)
        return a;
    return lol::lerp(a, b, dt / (dt + x));
}

} /* namespace lol */

#endif // __LOL_NUMERIC_H__

