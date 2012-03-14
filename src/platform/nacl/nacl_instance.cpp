// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined HAVE_CONFIG_H
#   include "config.h"
#endif

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <ppapi/cpp/rect.h>
#include <ppapi/cpp/size.h>
#include <ppapi/cpp/var.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/input_event.h>

#include "core.h"
#include "debug/quad.h"

#include "platform/nacl/nacl_instance.h"
#include "platform/nacl/opengl_context.h"

namespace lol
{

NaClInstance::NaClInstance(PP_Instance instance)
    : pp::Instance(instance),
      m_size(0, 0)
{
    RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE);
}

NaClInstance::~NaClInstance()
{
    // Destroy the cube view while GL context is current.
    opengl_context_->MakeContextCurrent(this);
}

static double const DELTA_MS = 1000.0 / 60.0;

void TickCallback(void* data, int32_t result)
{
    NaClInstance *instance = (NaClInstance *)data;
    instance->DrawSelf();

    /* FIXME: only set if if Ticker isn't finished */
    pp::Module::Get()->core()->CallOnMainThread(
            DELTA_MS, pp::CompletionCallback(&TickCallback, data), PP_OK);
}

}
#define main OLDMAIN
#include "../test/tutorial/tut03.cpp"
#undef main
namespace lol {

bool NaClInstance::Init(uint32_t /* argc */,
                        const char* /* argn */[],
                        const char* /* argv */[])
{
    Ticker::Setup(60.0f);

    //new Kub();
    //new DebugQuad();
    new Fractal(ivec2(640, 480));

    // My timer callback
    pp::Module::Get()->core()->CallOnMainThread(
            DELTA_MS, pp::CompletionCallback(&TickCallback, this), PP_OK);

    return true;
}

void NaClInstance::HandleMessage(const pp::Var& message)
{
    if (!message.is_string())
        return;
    /* FIXME: do some shit here */
}

void NaClInstance::DidChangeView(const pp::Rect& position, const pp::Rect& clip)
{
    if (position.size().width() == m_size.x &&
        position.size().height() == m_size.y)
        return;  // Size didn't change, no need to update anything.

    m_size = ivec2(position.size().width(), position.size().height());

    if (opengl_context_ == NULL)
        opengl_context_.reset(new OpenGLContext(this));
    opengl_context_->InvalidateContext(this);
    opengl_context_->ResizeContext(position.size());
    if (!opengl_context_->MakeContextCurrent(this))
        return;

    Video::Setup(m_size);
    DrawSelf();
}

bool NaClInstance::HandleInputEvent(const pp::InputEvent& event)
{
    switch (event.GetType())
    {
    case PP_INPUTEVENT_TYPE_MOUSEDOWN:
        Input::SetMouseButton(pp::MouseInputEvent(event).GetButton());
        break;
    case PP_INPUTEVENT_TYPE_MOUSEUP:
        Input::UnsetMouseButton(pp::MouseInputEvent(event).GetButton());
        break;
    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
        Input::SetMousePos(ivec2(pp::MouseInputEvent(event).GetPosition().x(), opengl_context_->GetSize().height() - 1 - pp::MouseInputEvent(event).GetPosition().y()));
        break;
    default:
        break;
    }
    return true;
}

void NaClInstance::DrawSelf()
{
    if (opengl_context_ == NULL)
        return;

    opengl_context_->MakeContextCurrent(this);
    Ticker::TickDraw();
    opengl_context_->FlushContext();
}

}  // namespace lol
