//
//  Lol Engine — Bullet physics test
//
//  Copyright © 2009—2013 Benjamin “Touky” Huet <huet.benjamin@gmail.com>
//            © 2012—2018 Sam Hocevar <sam@hocevar.net>
//
//  Lol Engine is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#pragma once

#include <string>
#include <map>

class CatShaderData : public GpuShaderData
{
    friend class CRenderer;

public:
    //---
    CatShaderData(uint32_t vert_decl_flags, Shader* shader);
    //---
    void SetupDefaultData();
    virtual void SetupShaderDatas(mat4 const &model);
    //--
    virtual std::string GetInVertexName()   { return "in_vertex";   }
    virtual std::string GetInNormalName()   { return "in_normal";   }
    virtual std::string GetInColorName()    { return "in_color";    }
    virtual std::string GetInTexCoordName() { return "in_texcoord"; }

    TextureUniform m_tex_uniform;
    float m_sprite_orientation;
    float m_sprite_flip;
};

class BtPhysTest : public WorldEntity
{
public:
    BtPhysTest(bool editor = false);
    virtual ~BtPhysTest();

    std::string GetName() const { return "<BtPhysTest>"; }

protected:
    virtual void TickGame(float seconds);
    virtual void TickDraw(float seconds, Scene &scene);

    void InitApp();

private:
    //MeshViewerInput ---------------------------------------------------------
    struct BtPhysTestKeyInputBase : public StructSafeEnum
    {
        enum Type
        {
            KEY_START = 0,

            KEY_MOVE_FORWARD = KEY_START,
            KEY_MOVE_BACK,
            KEY_MOVE_LEFT,
            KEY_MOVE_RIGHT,
            KEY_MOVE_UP,
            KEY_MOVE_DOWN,
            KEY_MOVE_JUMP,
            KEY_QUIT,

            KEY_MAX
        };
    protected:
        virtual bool BuildEnumMap(std::map<int64_t, std::string>& enum_map)
        {
            enum_map[KEY_MOVE_FORWARD] = g_name_key_Up;
            enum_map[KEY_MOVE_BACK] = g_name_key_Down;
            enum_map[KEY_MOVE_LEFT] = g_name_key_Left;
            enum_map[KEY_MOVE_RIGHT] = g_name_key_Right;
            enum_map[KEY_MOVE_UP] = g_name_key_PageUp;
            enum_map[KEY_MOVE_DOWN] = g_name_key_PageDown;
            enum_map[KEY_MOVE_JUMP] = g_name_key_Space;
            enum_map[KEY_QUIT] = g_name_key_Escape;

            return true;
        }
    };
    typedef SafeEnum<BtPhysTestKeyInputBase> BtPhysTestKeyInput;


    TileSet*                        m_cat_texture;
    Shader*                         m_cat_shader;
    CatShaderData*                  m_cat_sdata;
    Camera*                         m_camera;
    Controller*                     m_controller;
    InputProfile                    m_profile;
    Light*                          m_light1;
    Light*                          m_light2;
    int                             m_init_status;
    bool                            m_ready;

    lol::phys::Simulation*          m_simulation;
    array<EasyConstraint*>          m_constraint_list;
    array<PhysicsObject*, float>    m_physobj_list;
    array<PhysicsObject*>           m_ground_list;
    array<PhysicsObject*>           m_platform_list;
    array<PhysicsObject*>           m_character_list;
    array<PhysicsObject*>           m_stairs_list;

    float                           m_loop_value;
    float                           m_target_timer;
    int                             m_cam_target;
    float                           m_fov_dp;
    float                           m_loc_dp;
};

