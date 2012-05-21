//
// Lol Engine
//
// Copyright: (c) 2010-2012 Sam Hocevar <sam@hocevar.net>
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the Do What The Fuck You Want To
//   Public License, Version 2, as published by Sam Hocevar. See
//   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
//

#if defined HAVE_CONFIG_H
#   include "config.h"
#endif

#include "core.h"
#include "lolgl.h"

#if defined _WIN32 && defined USE_D3D9
#   define FAR
#   define NEAR
#   include <d3d9.h>
#endif

using namespace std;

#if defined USE_D3D9
extern IDirect3DDevice9 *g_d3ddevice;
#elif defined _XBOX
extern D3DDevice *g_d3ddevice;
#endif

namespace lol
{

//
// The IndexBufferData class
// --------------------------
//

class IndexBufferData
{
    friend class IndexBuffer;

#if defined USE_D3D9
    IDirect3DIndexBuffer9 *m_ibo;
#elif defined _XBOX
    D3DIndexBuffer *m_ibo;
#elif !defined __CELLOS_LV2__ && !defined __ANDROID__
    GLuint m_ibo;
    uint8_t *m_memory;
    size_t m_size;
#endif
};

//
// The IndexBuffer class
// ----------------------
//

IndexBuffer::IndexBuffer(size_t size)
  : m_data(new IndexBufferData)
{
#if defined USE_D3D9 || defined _XBOX
    if (FAILED(g_d3ddevice->CreateIndexBuffer(size, D3DUSAGE_WRITEONLY,
                                              D3DFMT_INDEX16, D3DPOOL_MANAGED,
                                              &m_data->m_ibo, NULL)))
        Abort();
#elif !defined __CELLOS_LV2__ && !defined __ANDROID__
    glGenBuffers(1, &m_data->m_ibo);
    m_data->m_memory = new uint8_t[size];
    m_data->m_size = size;
#endif
}

IndexBuffer::~IndexBuffer()
{
#if defined USE_D3D9 || defined _XBOX
    if (FAILED(m_data->m_ibo->Release()))
        Abort();
#elif !defined __CELLOS_LV2__ && !defined __ANDROID__
    glDeleteBuffers(1, &m_data->m_ibo);
    delete[] m_data->m_memory;
#endif
    delete m_data;
}

void *IndexBuffer::Lock(size_t offset, size_t size)
{
#if defined USE_D3D9 || defined _XBOX
    void *ret;
    if (FAILED(m_data->m_ibo->Lock(offset, size, (void **)&ret, 0)))
        Abort();
    return ret;
#elif !defined __CELLOS_LV2__ && !defined __ANDROID__
    return m_data->m_memory + offset;
#endif
}

void IndexBuffer::Unlock()
{
#if defined USE_D3D9 || defined _XBOX
    if (FAILED(m_data->m_ibo->Unlock()))
        Abort();
#elif !defined __CELLOS_LV2__ && !defined __ANDROID__
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_data->m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_data->m_size, m_data->m_memory,
                 GL_STATIC_DRAW);
#endif
}

void IndexBuffer::Bind()
{
#if defined USE_D3D9 || defined _XBOX
    if (FAILED(g_d3ddevice->SetIndices(m_data->m_ibo)))
        Abort();
#elif !defined __CELLOS_LV2__ && !defined __ANDROID__
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_data->m_ibo);
    /* XXX: not necessary because we kept track of the size */
    //int size;
    //glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
#endif
}

void IndexBuffer::Unbind()
{
#if defined USE_D3D9 || defined _XBOX
    if (FAILED(g_d3ddevice->SetIndices(NULL)))
        Abort();
#elif !defined __CELLOS_LV2__ && !defined __ANDROID__
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}

} /* namespace lol */
