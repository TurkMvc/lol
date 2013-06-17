//
// Lol Engine
//
// Copyright: (c) 2010-2013 Sam Hocevar <sam@hocevar.net>
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the Do What The Fuck You Want To
//   Public License, Version 2, as published by Sam Hocevar. See
//   http://www.wtfpl.net/ for more details.
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
// The FramebufferData class
// -------------------------
//

class FramebufferData
{
    friend class Framebuffer;

    ivec2 m_size;

#if defined USE_D3D9
    IDirect3DTexture9 *m_texture;
    IDirect3DSurface9 *m_surface, *m_back_surface;
#elif defined _XBOX
    D3DTexture *m_texture;
    D3DSurface *m_surface, *m_back_surface;
#else
    GLuint m_fbo, m_texture, m_depth;
#endif
};

//
// The FramebufferFormat struct
// ----------------------
//

uint32_t FramebufferFormat::GetFormat()
{
    switch (m_format)
    {
#if defined USE_D3D9 || defined _XBOX
    case R_16_F:        return D3DFMT_R16F;
    case R_32_F:        return D3DFMT_R32F;
    case RG_16:
    case RG_16_I:
    case RG_16_UI:      return D3DFMT_G16R16;
    case RG_16_F:       return D3DFMT_G16R16F;
    case RG_32_F:       return D3DFMT_G32R32F;
    case RGB_8:
    case RGB_8_I:
    case RGB_8_UI:      return D3DFMT_R8G8B8;
    case RGBA_8:
    case RGBA_8_I:
    case RGBA_8_UI:     return D3DFMT_A8R8G8B8;
    case RGBA_16:
    case RGBA_16_I:
    case RGBA_16_UI:    return D3DFMT_A16B16G16R16;
    case RGBA_16_F:     return D3DFMT_A16B16G16R16F;
    case RGBA_32_F:     return D3DFMT_A32B32G32R32F;
#elif defined __CELLOS_LV2__
    /* Supported drawable formats on the PS3: GL_ARGB_SCE, GL_RGB16F_ARB,
     * GL_RGBA16F_ARB, GL_RGB32F_ARB, GL_RGBA32F_ARB, GL_LUMINANCE32F_ARB. */
    case RGB_16_F:      return GL_RGB16F_ARB;
    case RGB_32_F:      return GL_RGB32F_ARB;
    case RGBA_8:        return GL_ARGB_SCE;
    case RGBA_16_F:     return GL_RGBA16F_ARB;
    case RGBA_32_F:     return GL_RGBA32F_ARB;
#elif defined HAVE_GLES_2X
    /* FIXME: not implemented at all */

#elif defined __APPLE__ && defined __MACH__
    case R_8:
    case R_8_I:
    case R_8_UI:
    case R_8_F:

    case R_16:
    case R_16_I:
    case R_16_UI:
    case R_16_F:

    case R_32_I:
    case R_32:
    case R_32_UI:
    case R_32_F:        return GL_RED;

    case RG_8:
    case RG_8_I:
    case RG_8_UI:
    case RG_8_F:

    case RG_16:
    case RG_16_I:
    case RG_16_UI:
    case RG_16_F:

    case RG_32:
    case RG_32_I:
    case RG_32_UI:
    case RG_32_F:       return GL_RG;

    case RGB_8:
    case RGB_8_I:
    case RGB_8_UI:
    case RGB_8_F:

    case RGB_16:
    case RGB_16_I:
    case RGB_16_UI:
    case RGB_16_F:

    case RGB_32:
    case RGB_32_I:
    case RGB_32_UI:
    case RGB_32_F:      return (m_invert_rgb)?(GL_BGR):(GL_RGB);

    case RGBA_8:
    case RGBA_8_I:
    case RGBA_8_UI:
    case RGBA_8_F:

    case RGBA_16:
    case RGBA_16_I:
    case RGBA_16_UI:
    case RGBA_16_F:

    case RGBA_32:
    case RGBA_32_I:
    case RGBA_32_UI:
    case RGBA_32_F:     return (m_invert_rgb)?(GL_BGRA):(GL_RGBA);
#else
    case R_8:           return GL_R8;
    case R_8_I:         return GL_R8I;
    case R_8_UI:        return GL_R8UI;

    case R_16:          return GL_R16;
    case R_16_I:        return GL_R16I;
    case R_16_UI:       return GL_R16UI;
    case R_16_F:        return GL_R16F;

    case R_32_I:        return GL_R32I;
    case R_32_UI:       return GL_R32UI;
    case R_32_F:        return GL_R32F;

    case RG_8:          return GL_RG8;
    case RG_8_I:        return GL_RG8I;
    case RG_8_UI:       return GL_RG8UI;

    case RG_16:         return GL_RG16;
    case RG_16_I:       return GL_RG16I;
    case RG_16_UI:      return GL_RG16UI;
    case RG_16_F:       return GL_RG16F;

    case RG_32_I:       return GL_RG32I;
    case RG_32_UI:      return GL_RG32UI;
    case RG_32_F:       return GL_RG32F;

    case RGB_8:         return GL_RGB8;
    case RGB_8_I:       return GL_RGB8I;
    case RGB_8_UI:      return GL_RGB8UI;

    case RGB_16:        return GL_RGB16;
    case RGB_16_I:      return GL_RGB16I;
    case RGB_16_UI:     return GL_RGB16UI;
    case RGB_16_F:      return GL_RGB16F;

    case RGB_32_I:      return GL_RGB32I;
    case RGB_32_UI:     return GL_RGB32UI;
    case RGB_32_F:      return GL_RGB32F;

    case RGBA_8:        return GL_RGBA8;
    case RGBA_8_I:      return GL_RGBA8I;
    case RGBA_8_UI:     return GL_RGBA8UI;

    case RGBA_16:       return GL_RGBA16;
    case RGBA_16_I:     return GL_RGBA16I;
    case RGBA_16_UI:    return GL_RGBA16UI;
    case RGBA_16_F:     return GL_RGBA16F;

    case RGBA_32_I:     return GL_RGBA32I;
    case RGBA_32_UI:    return GL_RGBA32UI;
    case RGBA_32_F:     return GL_RGBA32F;
#endif
    default:
        ASSERT(false, "unknown framebuffer format %d", m_format);
        return 0;
    }
}

uint32_t FramebufferFormat::GetFormatOrder()
{
    switch (m_format)
    {
#if defined USE_D3D9 || defined _XBOX
    /* FIXME: not implemented at all */
#elif defined __CELLOS_LV2__
    /* FIXME: not implemented at all */
#elif defined HAVE_GLES_2X
    /* FIXME: not implemented at all */
#elif defined __APPLE__ && defined __MACH__
    case R_8:   case RG_8:   case RGB_8:   case RGBA_8:
    case R_8_I: case RG_8_I: case RGB_8_I: case RGBA_8_I:
        return GL_BYTE;
    case R_8_UI: case RG_8_UI: case RGB_8_UI: case RGBA_8_UI:
        return GL_UNSIGNED_BYTE;

    case R_16:   case RG_16:   case RGB_16:   case RGBA_16:
    case R_16_I: case RG_16_I: case RGB_16_I: case RGBA_16_I:
        return GL_SHORT;
    case R_16_UI: case RG_16_UI: case RGB_16_UI: case RGBA_16_UI:
        return GL_UNSIGNED_SHORT;

    case R_16_F: case RG_16_F: case RGB_16_F: case RGBA_16_F:
        ASSERT(false, "unsupported framebuffer format order %d", m_format);
        return 0;

    case R_32_I: case RG_32_I: case RGB_32_I: case RGBA_32_I:
        return GL_INT;
    case R_32_UI: case RG_32_UI: case RGB_32_UI: case RGBA_32_UI:
        return GL_UNSIGNED_INT;
    case R_32_F: case RG_32_F: case RGB_32_F: case RGBA_32_F:
        return GL_FLOAT;
#else
    case R_8:  case R_8_I:  case R_8_UI:  case R_8_F:
    case R_16: case R_16_I: case R_16_UI: case R_16_F:
    case R_32: case R_32_I: case R_32_UI: case R_32_F:
        return GL_RED;

    case RG_8:  case RG_8_I:  case RG_8_UI:  case RG_8_F:
    case RG_16: case RG_16_I: case RG_16_UI: case RG_16_F:
    case RG_32: case RG_32_I: case RG_32_UI: case RG_32_F:
        return GL_RG;

    case RGB_8:  case RGB_8_I:  case RGB_8_UI:  case RGB_8_F:
    case RGB_16: case RGB_16_I: case RGB_16_UI: case RGB_16_F:
    case RGB_32: case RGB_32_I: case RGB_32_UI: case RGB_32_F:
        return m_invert_rgb ? GL_BGR : GL_RGB;

    case RGBA_8:  case RGBA_8_I:  case RGBA_8_UI:  case RGBA_8_F:
    case RGBA_16: case RGBA_16_I: case RGBA_16_UI: case RGBA_16_F:
    case RGBA_32: case RGBA_32_I: case RGBA_32_UI: case RGBA_32_F:
        return (m_invert_rgb)?(GL_BGRA):(GL_RGBA);
#endif
    default:
        ASSERT(false, "unknown framebuffer format order %d", m_format);
        return 0;
    }
}

//
// The Framebuffer class
// ----------------------
//

Framebuffer::Framebuffer(ivec2 size, FramebufferFormat fbo_format)
  : m_data(new FramebufferData)
{
    m_data->m_size = size;
#if defined USE_D3D9
    if (FAILED(g_d3ddevice->CreateTexture(size.x, size.y, 1,
                                          D3DUSAGE_RENDERTARGET,
                                          (D3DFORMAT)fbo_format.GetFormat(), D3DPOOL_DEFAULT,
                                          &m_data->m_texture, nullptr)))
        Abort();
    if (FAILED(m_data->m_texture->GetSurfaceLevel(0, &m_data->m_surface)))
        Abort();
#elif defined _XBOX
    if (FAILED(g_d3ddevice->CreateTexture(size.x, size.y, 1, 0,
                                          fbo_format.GetFormat(), D3DPOOL_DEFAULT,
                                          &m_data->m_texture, nullptr)))
        Abort();
    if (FAILED(g_d3ddevice->CreateRenderTarget(size.x, size.y,
                                               fbo_format.GetFormat(),
                                               D3DMULTISAMPLE_NONE, 0, 0,
                                               &m_data->m_surface, nullptr)))
        Abort();
#else
#   if GL_VERSION_1_1
    GLenum internal_format = fbo_format.GetFormat();
    GLenum format = fbo_format.GetFormatOrder();
    GLenum depth = GL_DEPTH_COMPONENT;
#   elif defined __CELLOS_LV2__
    /* Supported drawable formats on the PS3: GL_ARGB_SCE, GL_RGB16F_ARB,
     * GL_RGBA16F_ARB, GL_RGB32F_ARB, GL_RGBA32F_ARB, GL_LUMINANCE32F_ARB. */
    GLenum internal_format = fbo_format.GetFormat();
    GLenum format = fbo_format.GetFormatOrder();
#   else
    GLenum internal_format = fbo_format.GetFormat();
    GLenum format = fbo_format.GetFormatOrder();
#   endif
    GLenum wrapmode = GL_REPEAT;
    GLenum filtering = GL_NEAREST;

#   if GL_VERSION_1_1 || GL_ES_VERSION_2_0
    glGenFramebuffers(1, &m_data->m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_data->m_fbo);
#   else
    glGenFramebuffersOES(1, &m_data->m_fbo);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_data->m_fbo);
#   endif

    glGenTextures(1, &m_data->m_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_data->m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLenum)wrapmode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLenum)wrapmode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLenum)filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLenum)filtering);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size.x, size.y, 0,
                 format, GL_UNSIGNED_BYTE, nullptr);

#   if GL_VERSION_1_1 || GL_ES_VERSION_2_0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_data->m_texture, 0);
#   else
    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_EXT,
                              GL_TEXTURE_2D, m_data->m_texture, 0);
#   endif

    m_data->m_depth = GL_INVALID_ENUM;
#   if GL_VERSION_1_1
    /* FIXME: not implemented on GL ES, see
     * http://stackoverflow.com/q/4041682/111461 */
    if (depth != GL_INVALID_ENUM)
    {
        glGenRenderbuffers(1, &m_data->m_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, m_data->m_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, depth, size.x, size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, m_data->m_depth);
    }
#   endif

#   if GL_VERSION_1_1 || GL_ES_VERSION_2_0
    glCheckFramebufferStatus(GL_FRAMEBUFFER);
#   endif

    Unbind();
#endif
}

Framebuffer::~Framebuffer()
{
#if defined USE_D3D9 || defined _XBOX
    m_data->m_surface->Release();
    m_data->m_texture->Release();
#else
#   if GL_VERSION_1_1 || GL_ES_VERSION_2_0
    glDeleteFramebuffers(1, &m_data->m_fbo);
#   else
    glDeleteFramebuffersOES(1, &m_data->m_fbo);
#   endif
    glDeleteTextures(1, &m_data->m_texture);
#   if GL_VERSION_1_1
    if (m_data->m_depth != GL_INVALID_ENUM)
        glDeleteRenderbuffers(1, &m_data->m_depth);
#   endif
#endif
    delete m_data;
}

ShaderTexture Framebuffer::GetTexture() const
{
    ShaderTexture ret;
#if defined USE_D3D9 || defined _XBOX
    ret.m_flags = (uint64_t)(uintptr_t)m_data->m_texture;
#else
    ret.m_flags = m_data->m_texture;
#endif
    return ret;
}

ivec2 Framebuffer::GetSize() const
{
    return m_data->m_size;
}

void Framebuffer::Bind()
{
#if defined USE_D3D9 || defined _XBOX
    if (FAILED(g_d3ddevice->GetRenderTarget(0, &m_data->m_back_surface)))
        Abort();
    if (FAILED(g_d3ddevice->SetRenderTarget(0, m_data->m_surface)))
        Abort();
#else
#   if GL_VERSION_1_1 || GL_ES_VERSION_2_0
    glBindFramebuffer(GL_FRAMEBUFFER, m_data->m_fbo);
#   else
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_data->m_fbo);
#   endif
    //change viewport draw size
    Video::SetCustomSize(m_data->m_size);
#endif
}

void Framebuffer::Unbind()
{
#if defined USE_D3D9
    if (FAILED(g_d3ddevice->SetRenderTarget(0, m_data->m_back_surface)))
        Abort();
    m_data->m_back_surface->Release();
#elif defined _XBOX
    if (FAILED(g_d3ddevice->Resolve(D3DRESOLVE_RENDERTARGET0, nullptr,
                                    m_data->m_texture, nullptr, 0, 0, nullptr,
                                    0, 0, nullptr)))
        Abort();
    if (FAILED(g_d3ddevice->SetRenderTarget(0, m_data->m_back_surface)))
        Abort();
    m_data->m_back_surface->Release();
#else
    //Restore viewport draw size
    Video::RestoreSize();
#   if GL_VERSION_1_1 || GL_ES_VERSION_2_0
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#   else
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
#   endif
#endif
}

} /* namespace lol */

