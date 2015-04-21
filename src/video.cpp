//
// Lol Engine
//
// Copyright: (c) 2010-2013 Sam Hocevar <sam@hocevar.net>
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the Do What The Fuck You Want To
//   Public License, Version 2, as published by Sam Hocevar. See
//   http://www.wtfpl.net/ for more details.
//

#include <lol/engine-internal.h>

#if defined _XBOX
#   include <xtl.h>
#   undef near /* Fuck Microsoft */
#   undef far /* Fuck Microsoft again */
#elif defined _WIN32
#   if defined USE_D3D9
#      include <d3d9.h>
#   endif
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   undef near /* Fuck Microsoft */
#   undef far /* Fuck Microsoft again */
#endif

#include "lolgl.h"

namespace lol
{

class VideoData
{
    friend class Video;

private:
    static DebugRenderMode render_mode;
};

DebugRenderMode VideoData::render_mode = DebugRenderMode::Default;

/*
 * Public Video class
 */

void Video::Setup(ivec2 size)
{
    g_renderer = new Renderer(size);
    Scene::g_scene = new Scene(size);

    /* Initialise reasonable scene default properties */
    SetDebugRenderMode(DebugRenderMode::Default);
}

void Video::SetDebugRenderMode(DebugRenderMode d)
{
    switch(d)
    {
        //All these modes are handled in the shaders.
        case DebugRenderMode::Default:
        case DebugRenderMode::Flat:
        case DebugRenderMode::Lighting:
        case DebugRenderMode::Normal:
        case DebugRenderMode::UV:
        {
#if defined USE_D3D9 || defined _XBOX
#elif defined HAVE_GLES_2X
#elif defined USE_GLEW || defined HAVE_GL_2X
            glEnable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
            break;
        }
        case DebugRenderMode::Wireframe:
        {
#if defined USE_D3D9 || defined _XBOX
#elif defined HAVE_GLES_2X
#elif defined USE_GLEW || defined HAVE_GL_2X
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
            break;
        }
        default:
            return; /* Unknown render mode */
    }
    VideoData::render_mode = d;
}

DebugRenderMode Video::GetDebugRenderMode()
{
    return VideoData::render_mode;
}

void Video::Destroy()
{
    delete Scene::g_scene;
    Scene::g_scene = nullptr;

    delete g_renderer;
    g_renderer = nullptr;
}

void Video::Capture(uint32_t *buffer)
{
#if defined USE_D3D9 || defined _XBOX
    /* TODO */
#elif defined USE_GLEW || defined HAVE_GL_2X || defined HAVE_GLEX_2X
    GLint v[4];
    glGetIntegerv(GL_VIEWPORT, v);
    int width = v[2], height = v[3];

#   if defined HAVE_GL_2X
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
#   endif
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

#   if defined GL_BGRA
    glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
#   else
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
#   endif

    for (int j = 0; j < height / 2; j++)
        for (int i = 0; i < width; i++)
        {
            uint32_t tmp = buffer[j * width + i];
            buffer[j * width + i] = buffer[(height - j - 1) * width + i];
            buffer[(height - j - 1) * width + i] = tmp;
        }
#endif
}

ivec2 Video::GetSize()
{
    ibox2 viewport = g_renderer->GetViewport();

    return viewport.B - viewport.A;
}

} /* namespace lol */

