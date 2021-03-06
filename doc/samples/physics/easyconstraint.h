//
//  Lol Engine
//
//  Copyright © 2010—2015 Sam Hocevar <sam@hocevar.net>
//            © 2009—2013 Benjamin “Touky” Huet <huet.benjamin@gmail.com>
//
//  This library is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#pragma once

//
// The EasyPhysic class
// ------------------
//

#include <lol/engine.h>

#include "easyphysics.h"

namespace lol
{

namespace phys
{

class EasyConstraint
{

    friend class Simulation;
    friend class EasyPhysic;

public:
    EasyConstraint() :
        m_typed_constraint(nullptr),
        m_p2p_constraint(nullptr),
        m_hinge_constraint(nullptr),
        m_slider_constraint(nullptr),
        m_cone_twist_constraint(nullptr),
        m_6dof_constraint(nullptr),
        m_owner_simulation(nullptr),
        m_a_physobj(nullptr),
        m_b_physobj(nullptr),
        m_a_transform(lol::mat4(1.f)),
        m_b_transform(lol::mat4(1.f)),
        m_using_ref_a(false),
        m_disable_a2b_collision(false)

    {
    }
    ~EasyConstraint()
    {
        delete m_typed_constraint;
        m_p2p_constraint = nullptr;
        m_hinge_constraint = nullptr;
        m_slider_constraint = nullptr;
        m_cone_twist_constraint = nullptr;
        m_6dof_constraint = nullptr;
    }

    void AddToSimulation(class Simulation* current_simulation);
    void RemoveFromSimulation(class Simulation* current_simulation);

private:

    //check if Init can be done
    bool CanProceedWithInit()
    {
        if (!m_a_physobj || !m_b_physobj)
            return false;

        if (!m_a_physobj->m_rigid_body || !m_b_physobj->m_rigid_body)
            return false;

        return true;
    }

    //-------------------------------------------------------------------------
    //Init constraint functions
    //--
    void CustomInitConstraintToPoint2Point()
    {
        m_p2p_constraint = new btPoint2PointConstraint(*m_a_physobj->m_rigid_body, *m_b_physobj->m_rigid_body,
                                                        LOL2BT_VEC3(m_a_transform[3].xyz * LOL2BT_UNIT), LOL2BT_VEC3(m_b_transform[3].xyz * LOL2BT_UNIT));
        m_typed_constraint = m_p2p_constraint;
    }

    void CustomInitConstraintToHinge()
    {
        m_hinge_constraint = new btHingeConstraint(*m_a_physobj->m_rigid_body, *m_b_physobj->m_rigid_body,
                                                        btTransform(LOL2BT_QUAT(quat(mat3(m_a_transform))), LOL2BT_VEC3(m_a_transform[3].xyz * LOL2BT_UNIT)),
                                                        btTransform(LOL2BT_QUAT(quat(mat3(m_b_transform))), LOL2BT_VEC3(m_b_transform[3].xyz * LOL2BT_UNIT)),
                                                        m_using_ref_a);
        m_typed_constraint = m_hinge_constraint;
    }

    void CustomInitConstraintToSlider()
    {
        m_slider_constraint = new btSliderConstraint(*m_a_physobj->m_rigid_body, *m_b_physobj->m_rigid_body,
                                                        btTransform(LOL2BT_QUAT(quat(mat3(m_a_transform))), LOL2BT_VEC3(m_a_transform[3].xyz * LOL2BT_UNIT)),
                                                        btTransform(LOL2BT_QUAT(quat(mat3(m_b_transform))), LOL2BT_VEC3(m_b_transform[3].xyz * LOL2BT_UNIT)),
                                                        m_using_ref_a);
        m_typed_constraint = m_slider_constraint;
    }

    void CustomInitConstraintToConeTwist()
    {
        m_cone_twist_constraint = new btConeTwistConstraint(*m_a_physobj->m_rigid_body, *m_b_physobj->m_rigid_body,
                                                        btTransform(LOL2BT_QUAT(quat(mat3(m_a_transform))), LOL2BT_VEC3(m_a_transform[3].xyz * LOL2BT_UNIT)),
                                                        btTransform(LOL2BT_QUAT(quat(mat3(m_b_transform))), LOL2BT_VEC3(m_b_transform[3].xyz * LOL2BT_UNIT)));
        m_typed_constraint = m_cone_twist_constraint;
    }

    void CustomInitConstraintTo6Dof()
    {
        m_6dof_constraint = new btGeneric6DofConstraint(*m_a_physobj->m_rigid_body, *m_b_physobj->m_rigid_body,
                                                        btTransform(LOL2BT_QUAT(quat(mat3(m_a_transform))), LOL2BT_VEC3(m_a_transform[3].xyz * LOL2BT_UNIT)),
                                                        btTransform(LOL2BT_QUAT(quat(mat3(m_b_transform))), LOL2BT_VEC3(m_b_transform[3].xyz * LOL2BT_UNIT)),
                                                        m_using_ref_a);
        m_typed_constraint = m_6dof_constraint;
    }

    btTypedConstraint*            m_typed_constraint;
    btPoint2PointConstraint*    m_p2p_constraint;
    btHingeConstraint*            m_hinge_constraint;
    btSliderConstraint*            m_slider_constraint;
    btConeTwistConstraint*        m_cone_twist_constraint;
    btGeneric6DofConstraint*    m_6dof_constraint;

public:
    void InitConstraintToPoint2Point()    { if (CanProceedWithInit()) CustomInitConstraintToPoint2Point(); }
    void InitConstraintToHinge()        { if (CanProceedWithInit()) CustomInitConstraintToHinge(); }
    void InitConstraintToSlider()        { if (CanProceedWithInit()) CustomInitConstraintToSlider(); }
    void InitConstraintToConeTwist()    { if (CanProceedWithInit()) CustomInitConstraintToConeTwist(); }
    void InitConstraintTo6Dof()            { if (CanProceedWithInit()) CustomInitConstraintTo6Dof(); }

    //Set given physic object to the proper slot.
    void SetPhysObjA(EasyPhysic* NewPhysObj, lol::mat4 NewTransform) { SetPhysObj(false, NewPhysObj, NewTransform); }
    void SetPhysObjB(EasyPhysic* NewPhysObj, lol::mat4 NewTransform) { SetPhysObj(true, NewPhysObj, NewTransform); }
    void SetPhysObj(bool SetToB, EasyPhysic* NewPhysObj, lol::mat4 NewTransform)
    {
        if (SetToB)
        {
            m_b_physobj = NewPhysObj;
            m_b_transform = NewTransform;
        }
        else
        {
            m_a_physobj = NewPhysObj;
            m_a_transform = NewTransform;
        }
    }

    //Set whether or not the physic engine should use the A object as the reference (most constraint transform are local).
    void SetRefAsA(bool NewUseRefA)
    {
        m_using_ref_a = NewUseRefA;
    }

    //Set whether or not to disable the collision between the bodies
    void DisableCollisionBetweenObjs(bool DisableCollision)
    {
        m_disable_a2b_collision = DisableCollision;
    }

private:
    Simulation*                    m_owner_simulation;
    EasyPhysic*                    m_a_physobj;
    EasyPhysic*                    m_b_physobj;
    lol::mat4                    m_a_transform;
    lol::mat4                    m_b_transform;
    bool                        m_using_ref_a;
    bool                        m_disable_a2b_collision;

};

} /* namespace phys */

} /* namespace lol */

