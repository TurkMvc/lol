//
// Lol Engine
//
// Copyright: (c) 2010-2011 Sam Hocevar <sam@hocevar.net>
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the Do What The Fuck You Want To
//   Public License, Version 2, as published by Sam Hocevar. See
//   http://sam.zoy.org/projects/COPYING.WTFPL for more details.
//

#if defined HAVE_CONFIG_H
#   include "config.h"
#endif

#if defined USE_GDIPLUS

#include <cmath>

#include <algorithm>

using namespace std;

#include <windows.h>
#include <Gdiplus.h>

#include "core.h"
#include "../../image/image-private.h"

namespace lol
{

/*
 * Image implementation class
 */

DECLARE_IMAGE_LOADER(GdiPlusImageData, 100)
{
public:
    virtual bool Open(char const *);
    virtual bool Close();

    virtual void *GetData() const;

private:
    Gdiplus::Bitmap *bitmap;
    Gdiplus::BitmapData bdata;
};

/*
 * Public Image class
 */

bool GdiPlusImageData::Open(char const *path)
{
    size_t len;
    len = mbstowcs(NULL, path, 0);
    wchar_t *wpath = new wchar_t[len + 1];
    if (mbstowcs(wpath, path, len + 1) == (size_t)-1)
    {
#if !LOL_RELEASE
        Log::Error("invalid image name %s\n", path);
#endif
        delete[] wpath;
        return false;
    }

    ULONG_PTR token;
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&token, &input, NULL);

    for (wchar_t const *wname = wpath; *wname; wname++)
        if ((bitmap = Gdiplus::Bitmap::FromFile(wname, 0)))
            break;

    delete[] wpath;
    if (!bitmap)
    {
#if !LOL_RELEASE
        Log::Error("could not load %s\n", path);
#endif
        return false;
    }

    size = ivec2(bitmap->GetWidth(), bitmap->GetHeight());
    format = Image::FORMAT_RGBA;

    Gdiplus::Rect rect(0, 0, size.x, size.y);
    if(bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead,
                        PixelFormat32bppARGB, &bdata) != Gdiplus::Ok)
    {
#if !LOL_RELEASE
        Log::Error("could not lock bits in %s\n", path);
#endif
        delete bitmap;
        return false;
    }

    return true;
}

bool GdiPlusImageData::Close()
{
    bitmap->UnlockBits(&bdata);
    delete bitmap;

    return true;
}

void * GdiPlusImageData::GetData() const
{
    return bdata.Scan0;
}

} /* namespace lol */

#endif /* defined USE_GDIPLUS */
