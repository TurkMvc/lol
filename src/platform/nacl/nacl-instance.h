// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <ppapi/cpp/instance.h>
#include <ppapi/c/ppb_gamepad.h>
#include <ppapi/cpp/input_event.h>

#include "platform/nacl/opengl_context.h"
#include "platform/nacl/opengl_context_ptrs.h"

#include "input/input.h"
#include "input/input_internal.h"

namespace lol {

class NaClInputData
{
    friend class NaClInstance;

private:
    void Tick(float seconds);
    bool IsViewportSizeValid() { return (m_app.x > 0.f && m_app.y > 0.f && m_screen.x > 0.f && m_screen.y > 0.f); }
    void InitViewportSize();

    static void SetMousePos(ivec2 position);

    NaClInputData()
        : m_mousecapture(false)
    {
        InitViewportSize();
    }

    array<class pp::InputEvent>     m_input_events;
    class InputDeviceInternal*      m_mouse;
    class InputDeviceInternal*      m_keyboard;

    vec2 m_app;
    vec2 m_screen;
    bool m_mousecapture;
};

class NaClInstance : public pp::Instance
{
public:
    explicit NaClInstance(PP_Instance instance);

    // The dtor makes the 3D context current before deleting the cube view, then
    // destroys the 3D context both in the module and in the browser.
    virtual ~NaClInstance();

    // Called by the browser when the NaCl module is loaded and all ready to go.
    virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);

    // Called whenever the in-browser window changes size.
    virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip);

    // Called by the browser to handle the postMessage() call in Javascript.
    virtual void HandleMessage(const pp::Var& message);

    // Bind and publish the module's methods to JavaScript.
    //void InitializeMethods(ScriptingBridge* bridge);

    // Called to draw the contents of the module's browser area.
    virtual bool HandleInputEvent(const pp::InputEvent& event);

    void DrawSelf();

    /* Communication with the application object */
    static void MainSignal();

private:
    SharedOpenGLContext m_opengl_ctx;

    ivec2 m_size;

    static void TickCallback(void* data, int32_t result);
    static void * MainRun(void *data);

    /* Gamepad support */
    PPB_Gamepad const *m_pad_interface;

    //12/09/2013 : Should use new system.
    NaClInputData *m_input_data;

    /* Communication with the application object */
    struct Args
    {
        Args(int argc, char **argv, char **env)
          : m_argc(argc), m_argv(argv), m_env(env) {}
        Args() {}

        int m_argc;
        char **m_argv;
        char **m_env;
    };
    static mutex main_mutex;
    static queue<Args *, 1> main_queue;

    Thread *m_main_thread;
};

}  // namespace lol

