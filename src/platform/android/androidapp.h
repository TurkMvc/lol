//
// Lol Engine
//
// Copyright: (c) 2010-2011 Sam Hocevar <sam@hocevar.net>
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the Do What The Fuck You Want To
//   Public License, Version 2, as published by Sam Hocevar. See
//   http://www.wtfpl.net/ for more details.
//

#pragma once

//
// The AndroidApp class
// --------------------
//

#include <lol/math/vector.h>

namespace lol
{

class AndroidAppData;

class AndroidApp
{
public:
    AndroidApp(char const *title, ivec2 res, float fps);
    virtual ~AndroidApp();

    void ShowPointer(bool show);
    void Tick();

private:
    AndroidAppData *m_data;
};

} /* namespace lol */

