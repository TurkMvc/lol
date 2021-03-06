//
// Lol Engine
//
// Copyright: (c) 2010-2013 Sam Hocevar <sam@hocevar.net>
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the Do What The Fuck You Want To
//   Public License, Version 2, as published by Sam Hocevar. See
//   http://www.wtfpl.net/ for more details.
//

#pragma once

//
// The D3d9Input class
// -------------------
//

#include "engine/entity.h"

namespace lol
{

class D3d9InputData;

class D3d9Input : public Entity
{
public:
    D3d9Input();
    virtual ~D3d9Input();

protected:
    virtual void TickGame(float seconds);
    virtual void TickDraw(float seconds, Scene &scene);

private:
    D3d9InputData *m_data;
};

} /* namespace lol */

