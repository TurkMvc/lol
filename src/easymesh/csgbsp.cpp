//
// EasyMesh-Csg: The code belonging to CSG operations
//
// Copyright: (c) 2010-2013 Sam Hocevar <sam@hocevar.net>
//            (c) 2010-2013 Benjamin "Touky" Huet <huet.benjamin@gmail.com>
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the Do What The Fuck You Want To
//   Public License, Version 2, as published by Sam Hocevar. See
//   http://www.wtfpl.net/ for more details.
//

#include <lol/engine-internal.h>

namespace lol
{

//--
int CsgBsp::AddLeaf(int leaf_type, vec3 origin, vec3 normal, int above_idx)
{
    if (leaf_type > 2 && leaf_type < -1)
        return -1;

    if ((m_tree.count() == 0 && above_idx == -1) ||
        (above_idx >= 0 &&
            above_idx < m_tree.count() &&
            leaf_type > LEAF_CURRENT &&
            leaf_type < LEAF_ABOVE &&
            m_tree[above_idx].m_leaves[leaf_type] == -1))
    {
        if (m_tree.count() != 0)
            m_tree[above_idx].m_leaves[leaf_type] = (int32_t)m_tree.count();
        m_tree.push(CsgBspLeaf(origin, normal, above_idx));
        return m_tree.count() - 1;
    }

    return -1;
}

int CsgBsp::TestPoint(int leaf_idx, vec3 point)
{
    if (leaf_idx >= 0 && leaf_idx < m_tree.count())
    {
        vec3 p2o = point - m_tree[leaf_idx].m_origin;

        if (length(p2o) < TestEpsilon::Get())
            return LEAF_CURRENT;

        float p2o_dot = dot(normalize(p2o), m_tree[leaf_idx].m_normal);

        if (p2o_dot > TestEpsilon::Get())
            return LEAF_FRONT;
        else if (p2o_dot < -TestEpsilon::Get())
            return LEAF_BACK;
    }
    return LEAF_CURRENT;
}

void CsgBsp::AddTriangleToTree(int const &tri_idx, vec3 const &tri_p0, vec3 const &tri_p1, vec3 const &tri_p2)
{
    //<Leaf_Id, v0, v1, v2>
    array< int, vec3, vec3, vec3 > tri_to_process;
    //<FW/BW, Leaf_Id, v0, v1, v2, twin_leaf>
    array< int, int, vec3, vec3, vec3, int > Leaf_to_add;

    //Tree is empty, so this leaf is the first
    if (m_tree.count() == 0)
    {
        AddLeaf(LEAF_CURRENT, tri_p0, cross(normalize(tri_p1 - tri_p0), normalize(tri_p2 - tri_p1)), LEAF_CURRENT);
        m_tree.last().m_tri_list.push(tri_idx, tri_p0, tri_p1, tri_p2);
        return;
    }

    tri_to_process.reserve(20);
    tri_to_process.push(0, tri_p0, tri_p1, tri_p2);

    while (tri_to_process.count())
    {
        int leaf_idx = tri_to_process.last().m1;
        vec3 v[3] = { tri_to_process.last().m2, tri_to_process.last().m3, tri_to_process.last().m4 };

        int res_nb[3] = { 0, 0, 0 };
        int res_side[3] = { -1, -1, -1 };

        //Check where each point is located
        for (int i = 0; i < 3; i++)
        {
            int result = TestPoint(leaf_idx, v[i]);
            if (result != LEAF_CURRENT)
            {
                res_nb[result]++;
                res_side[i] = result;
            }
        }

        //Points are located on each sides, let's do the mumbo-jumbo
        if (res_nb[LEAF_BACK] && res_nb[LEAF_FRONT])
        {
            //there are two intersections, no more.
            vec3 isec_v[2] = { vec3(.0f), vec3(.0f) };
            int isec_base = 0;
            int isec_idx = 0;

            for (int i = 0; i < 3; i++)
            {
                vec3 ray = v[(i + 1) % 3] - v[i];

                if (TestRayVsPlane(v[i], v[(i + 1) % 3],
                                    m_tree[leaf_idx].m_origin, m_tree[leaf_idx].m_normal,
                                    isec_v[isec_idx]))
                    ++isec_idx;
                else
                    isec_base = i;
            }

            int v_idx0 = (isec_base == 1)?(1):(0);
            int v_idx1 = (isec_base == 1)?(0):(1);
            int leaf_type = res_side[(isec_base + 2) % 3];

            tri_to_process.pop();

#if 1
            if (m_tree[leaf_idx].m_leaves[leaf_type] == LEAF_CURRENT)
                Leaf_to_add.push(leaf_type, leaf_idx, v[((isec_base + 2) % 3)], isec_v[v_idx1], isec_v[v_idx0], -1);
            else
                tri_to_process.push(leaf_idx, v[((isec_base + 2) % 3)], isec_v[v_idx1], isec_v[v_idx0]);

            if (m_tree[leaf_idx].m_leaves[1 - leaf_type] == LEAF_CURRENT)
            {
                Leaf_to_add.push(1 - leaf_type, leaf_idx, v[isec_base], v[((isec_base + 1) % 3)], isec_v[v_idx0], -1);
                Leaf_to_add.push(1 - leaf_type, leaf_idx, v[isec_base], isec_v[v_idx0], isec_v[v_idx1], Leaf_to_add.count() - 1);
            }
            else
            {
                tri_to_process.push(m_tree[leaf_idx].m_leaves[1 - leaf_type], v[isec_base], v[((isec_base + 1) % 3)], isec_v[v_idx0]);
                tri_to_process.push(m_tree[leaf_idx].m_leaves[1 - leaf_type], v[isec_base], isec_v[v_idx0], isec_v[v_idx1]);
            }
#else
            vec3 new_v[9] = { v[((isec_base + 2) % 3)], isec_v[v_idx1],             isec_v[v_idx0],
                                v[isec_base],             v[((isec_base + 1) % 3)],   isec_v[v_idx0],
                                v[isec_base],             isec_v[v_idx0],             isec_v[v_idx1] };

            //Error check : Skip the triangle where two points are on the same location.
            //it fixes the problem of having an intersection with one of the isec-point being on one of the triangle vertices.
            //(the problem being a very funny infinite loop)
            for(int k = 0; k < 9; k += 3)
            {
                bool skip_tri = false;
                for(int l = 0; l < 3; l++)
                {
                    if (length(new_v[k + l] - new_v[k + (l + 1) % 3]) < TestEpsilon::Get())
                    {
                        skip_tri = true;
                        break;
                    }
                }

                if (skip_tri)
                    continue;

                tri_to_process.push(0, new_v[k], new_v[k + 1], new_v[k + 2]);
            }
#endif
        }
        //All points are on one side, transfer to the next leaf
        else if (res_nb[LEAF_BACK] || res_nb[LEAF_FRONT])
        {
            int new_leaf_type = ((res_nb[LEAF_FRONT])?(LEAF_FRONT):(LEAF_BACK));
            int new_leaf = m_tree[leaf_idx].m_leaves[new_leaf_type];

            //No leaf exist, so add a new one
            if (new_leaf == LEAF_CURRENT)
            {
                tri_to_process.pop();
                Leaf_to_add.push(new_leaf_type, leaf_idx, v[0], v[1], v[2], -1);
            }
            else
                tri_to_process.last().m1 = new_leaf;
        }
        //All points are on the current leaf, add the tri_idx to the list of this leaf.
        else
        {
            tri_to_process.pop();

            bool already_exist = false;
            for (int i = 0; !already_exist && i < m_tree[leaf_idx].m_tri_list.count(); i++)
                already_exist = (m_tree[leaf_idx].m_tri_list[i].m1 == tri_idx);
            if (!already_exist)
                m_tree[leaf_idx].m_tri_list.push(tri_idx, tri_p0, tri_p1, tri_p2);
        }
    }

    //Add all leaves to the tree.
    for (int i = 0; i < Leaf_to_add.count(); i++)
    {
        //If we had it to an already existing leaf.
        if (Leaf_to_add[i].m2 < m_tree.count() && m_tree[Leaf_to_add[i].m2].m_leaves[Leaf_to_add[i].m1] == LEAF_CURRENT)
        {
            AddLeaf(Leaf_to_add[i].m1, tri_p0, cross(normalize(tri_p1 - tri_p0), normalize(tri_p2 - tri_p1)), Leaf_to_add[i].m2);
            m_tree.last().m_tri_list.push(tri_idx, tri_p0, tri_p1, tri_p2);
        }

        /*
        if (Leaf_to_add[i].m6 == -1)
        {
            AddLeaf(Leaf_to_add[i].m1, tri_p0, cross(normalize(tri_p1 - tri_p0), normalize(tri_p2 - tri_p1)), Leaf_to_add[i].m2);
            m_tree.last().m_tri_list.push(tri_idx, tri_p0, tri_p1, tri_p2);
        }
        else
            m_tree[Leaf_to_add[i].m6].m_tri_list.push(tri_idx, tri_p0, tri_p1, tri_p2);
        */
    }
}

//return 0 when no split has been done.
//return 1 when split has been done.
//return -1 when error.
int CsgBsp::TestTriangleToTree(vec3 const &tri_p0, vec3 const &tri_p1, vec3 const &tri_p2,
                               //In order to easily build the actual vertices list afterward, this list stores each Vertices location and its source vertices & Alpha.
                               //<Point_Loc, Src_V0, Src_V1, Alpha> as { Point_Loc = Src_V0 + (Src_V1 - Src_V0) * Alpha; }
                               array< vec3, int, int, float > &vert_list,
                               //This is the final triangle list : If Side_Status is LEAF_CURRENT, a new test will be done point by point.
                               //<{IN|OUT}side_status, v0, v1, v2>
                               array< int, int, int, int > &tri_list)
{
    //This list stores the current triangles to process.
    //<Leaf_Id_List, v0, v1, v2, Should_Point_Test>
    array< array< int >, int, int, int, int > tri_to_process;

    //Tree is empty, ABORT !
    if (m_tree.count() == 0)
        return -1;

    //Let's push the source vertices in here.
    vert_list.push(tri_p0, -1, -1, .0f);
    vert_list.push(tri_p1, -1, -1, .0f);
    vert_list.push(tri_p2, -1, -1, .0f);

    //Let's push the triangle in here.
    tri_to_process.reserve(20);
    tri_to_process.push( array< int >(), 0, 1, 2, 0);
    tri_to_process.last().m1.push(0);

    while (tri_to_process.count())
    {
        while (tri_to_process.count())
        {
            int leaf_idx = tri_to_process.last().m1.last();
            int t[3] = { tri_to_process.last().m2,
                            tri_to_process.last().m3,
                            tri_to_process.last().m4 };
            vec3 v[3] = { vert_list[t[0]].m1,
                            vert_list[t[1]].m1,
                            vert_list[t[2]].m1 };

            int res_nb[3] = { 0, 0, 0 };
            int res_side[3] = { -1, -1, -1 };
            //Check where each point is located
            for (int i = 0; i < 3; i++)
            {
                int result = TestPoint(leaf_idx, v[i]);
                if (result != LEAF_CURRENT)
                {
                    res_nb[result]++;
                    res_side[i] = result;
                }
            }

            //Points are located on each sides, let's do the mumbo-jumbo
            if (res_nb[LEAF_BACK] && res_nb[LEAF_FRONT])
            {
                //there are two intersections, no more.
                vec3 isec_v[2] = { vec3(.0f), vec3(.0f) };
                int isec_i[2] = { 0, 0 };
                int new_v_idx[2] = { 0, 0 };
                int isec_base = 0;

                int i = 0;
                for (; i < m_tree[leaf_idx].m_tri_list.count(); i++)
                {
                    if (TestTriangleVsTriangle(v[0], v[1], v[2],
                                                m_tree[leaf_idx].m_tri_list[i].m2, m_tree[leaf_idx].m_tri_list[i].m3, m_tree[leaf_idx].m_tri_list[i].m4,
                                                isec_v[0], isec_v[1]))
                        break;
                }

                //There was no triangle intersection, the complex case.
                if (i == m_tree[leaf_idx].m_tri_list.count())
                {
                    if (m_tree[leaf_idx].m_leaves[LEAF_FRONT] == LEAF_CURRENT &&
                        m_tree[leaf_idx].m_leaves[LEAF_BACK] == LEAF_CURRENT &&
                        tri_to_process.last().m1.count() == 1)
                    {
                        tri_list.push(LEAF_CURRENT, tri_to_process.last().m2, tri_to_process.last().m3, tri_to_process.last().m4);
                        tri_to_process.pop();
                    }
                    else
                    {
                        tri_to_process.last().m1.pop();

                        //Register the triangle as needing to intersect with Front & back leaves.
                        if (m_tree[leaf_idx].m_leaves[LEAF_FRONT] != LEAF_CURRENT)
                            tri_to_process.last().m1.push(m_tree[leaf_idx].m_leaves[LEAF_FRONT]);
                        if (m_tree[leaf_idx].m_leaves[LEAF_BACK] != LEAF_CURRENT)
                            tri_to_process.last().m1.push(m_tree[leaf_idx].m_leaves[LEAF_BACK]);
                        //Mark the triangle as needing point by point test

                        tri_to_process.last().m5 = 1;
                    }
                }
                //there was an intersection, so let's split the triangle.
                else
                {
                    //Get intersection on actual triangle sides.
                    if (TestRayVsTriangleSide(v[0], v[1], v[2],
                                                isec_v[0], isec_v[1],
                                                isec_v[0], isec_i[0], isec_v[1], isec_i[1]))
                    {
                        {
                            for(int k = 0; k < 2; k++)
                            {
                                if (isec_base == isec_i[k])
                                    isec_base++;

#if 1 //Skip point creation if it's on the same location a one of the triangle.
                                bool skip_point = false;
                                int l = 0;
                                for(; l < 3; l++)
                                {
                                    if (length(v[l] - isec_v[k]) < TestEpsilon::Get())
                                    {
                                        skip_point = true;
                                        new_v_idx[k] = t[l];
                                        break;
                                    }
                                }

                                if (skip_point)
                                    continue;
#endif
                                new_v_idx[k] = vert_list.count();
                                vec3 PmV0 = (isec_v[k] - vert_list[t[isec_i[k]]].m1);
                                vec3 V1mV0 = (vert_list[t[(isec_i[k] + 1) % 3]].m1 - vert_list[t[isec_i[k]]].m1);
                                float alpha = length(PmV0) / length(V1mV0);
                                vert_list.push(isec_v[k],
                                                t[isec_i[k]], t[(isec_i[k] + 1) % 3],
                                                //Alpha = length((Point_Loc - Src_V0) / (Src_V1 - Src_V0));
                                                alpha);
                            }

                            int v_idx0 = (isec_base == 1)?(1):(0);
                            int v_idx1 = (isec_base == 1)?(0):(1);
                            int tri_to_remove = tri_to_process.count() - 1;
#if 0
                            //Leaf_type is the type for the triangle that is alone on its side.
                            int leaf_type = res_side[(isec_base + 2) % 3];

                            if (m_tree[leaf_idx].m_leaves[leaf_type] == LEAF_CURRENT && tri_to_process.last().m1.last() == 1)
                                tri_list.push(leaf_type,
                                                t[(isec_base + 2) % 3], new_v_idx[v_idx1], new_v_idx[v_idx0]);
                            else
                            {
                                tri_to_process.push(array< int >(), t[(isec_base + 2) % 3], new_v_idx[v_idx1], new_v_idx[v_idx0], 0);
                                tri_to_process.last().m1.push(0);
                            }

                            if (m_tree[leaf_idx].m_leaves[1 - leaf_type] == LEAF_CURRENT && tri_to_process.last().m1.last() == 1)
                            {
                                tri_list.push((tri_to_process.last().m5)?(LEAF_CURRENT):(1 - leaf_type),
                                                t[isec_base], new_v_idx[((isec_base + 1) % 3)], new_v_idx[v_idx0]);
                                tri_list.push((tri_to_process.last().m5)?(LEAF_CURRENT):(1 - leaf_type),
                                                t[isec_base], new_v_idx[v_idx0], new_v_idx[v_idx1]);
                            }
                            else
                            {
                                tri_to_process.push(array< int >(), t[isec_base], t[((isec_base + 1) % 3)], new_v_idx[v_idx0], 0);
                                tri_to_process.last().m1.push(0);
                                tri_to_process.push(array< int >(), t[isec_base], new_v_idx[v_idx0], new_v_idx[v_idx1], 0);
                                tri_to_process.last().m1.push(0);
                            }
#else
                            int new_t[9] = { t[(isec_base + 2) % 3], new_v_idx[v_idx1],         new_v_idx[v_idx0],
                                                t[isec_base],           t[((isec_base + 1) % 3)],  new_v_idx[v_idx0],
                                                t[isec_base],           new_v_idx[v_idx0],         new_v_idx[v_idx1] };
                            int new_side[3] = { res_side[(isec_base + 2) % 3],
                                                (res_side[isec_base] == LEAF_CURRENT)?(res_side[((isec_base + 1) % 3)]):(res_side[isec_base]),
                                                res_side[isec_base] };

                            //Error check : Skip the triangle where two points are on the same location.
                            //it fixes the problem of having an intersection with one of the isec-point being on one of the triangle vertices.
                            //(the problem being a very funny infinite loop)
                            for(int k = 0; k < 9; k += 3)
                            {
#if 1 //Error check
                                bool skip_tri = false;
                                for(int l = 0; l < 3; l++)
                                {
                                    if (length(vert_list[new_t[k + l]].m1 - vert_list[new_t[k + (l + 1) % 3]].m1) < TestEpsilon::Get())
                                    {
                                        skip_tri = true;
                                        break;
                                    }
                                }

                                if (skip_tri)
                                    continue;
#endif
#if 0 //Send the newly created triangle back to the beginning
                                tri_to_process.push(array< int >(), new_t[k], new_t[k + 1], new_t[k + 2], 0);
                                tri_to_process.last().m1.push(0);
#else //Inherit parent tree
                                if (m_tree[leaf_idx].m_leaves[new_side[k / 3]] == LEAF_CURRENT && tri_to_process[tri_to_remove].m1.count() == 1)
                                    tri_list.push(new_side[k / 3], new_t[k], new_t[k + 1], new_t[k + 2]);
                                else
                                {
                                    tri_to_process.push(array< int >(), new_t[k], new_t[k + 1], new_t[k + 2], 0);
                                    tri_to_process.last().m1 = tri_to_process[tri_to_remove].m1;
                                    if (m_tree[leaf_idx].m_leaves[new_side[k / 3]] == LEAF_CURRENT)
                                        tri_to_process.last().m1.pop();
                                    else
                                        tri_to_process.last().m1.last() = m_tree[leaf_idx].m_leaves[new_side[k / 3]];
                                }
#endif
                            }
#endif

                            tri_to_process.remove(tri_to_remove);
                        }
                    }
                }
            }
            //All points are on one side, transfer to the next leaf
            else if (res_nb[LEAF_BACK] || res_nb[LEAF_FRONT])
            {
                int new_leaf_type = ((res_nb[LEAF_FRONT])?(LEAF_FRONT):(LEAF_BACK));
                int new_leaf = m_tree[leaf_idx].m_leaves[new_leaf_type];

                //No leaf exist, we're at the end
                if (new_leaf == LEAF_CURRENT)
                {
                    //We still need to test other leaves.
                    if (tri_to_process.last().m1.count() > 1)
                        tri_to_process.last().m1.pop();
                    else
                    {
                        tri_list.push((tri_to_process.last().m5)?(LEAF_CURRENT):(new_leaf_type),
                                        tri_to_process.last().m2, tri_to_process.last().m3, tri_to_process.last().m4);
                        tri_to_process.pop();
                    }
                }
                else
                    tri_to_process.last().m1.last() = new_leaf;
            }
            //All points are on the current leaf, add the tri_idx to the list of this leaf.
            else
            {
                //TODO : Special case, handle coplanar cut.
                tri_list.push(LEAF_CURRENT, tri_to_process.last().m2, tri_to_process.last().m3, tri_to_process.last().m4);
                tri_to_process.pop();
            }
        }

        //Now that we have all the split points, let's double-check the results
        for (int i = 0; i < tri_list.count(); i++)
        {
#define TEST_MAX 4
            int t[3] = { tri_list[i].m2,
                            tri_list[i].m3,
                            tri_list[i].m4 };
            vec3 v[4] = { vert_list[t[0]].m1,
                            vert_list[t[1]].m1,
                            vert_list[t[2]].m1,
                            (vert_list[t[0]].m1 +
                            vert_list[t[1]].m1 +
                            vert_list[t[2]].m1) / 3.0f };

            int res_total = 0;
            int res_nb[3] = { 0, 0, 0 };

            int res_Leaf[4] = { 0, 0, 0, 0 };
            int res_side[4] = { -1, -1, -1, -1 };
            while (res_total < TEST_MAX)
            {
                for (int k = 0; k < TEST_MAX; k++)
                {
                    if (res_Leaf[k] != LEAF_CURRENT)
                    {
                        int result = TestPoint(res_Leaf[k], v[k]);
                        if (result != LEAF_CURRENT)
                        {
                            res_Leaf[k] = m_tree[res_Leaf[k]].m_leaves[result];
                            res_side[k] = result;
                            if (res_Leaf[k] == LEAF_CURRENT)
                            {
                                res_total++;
                                res_nb[result]++;
                            }
                        }
                        else
                        {
                            res_Leaf[k] = LEAF_CURRENT;
                            res_side[k] = LEAF_CURRENT;
                            res_total++;
                        }
                    }
                }
            }
            int k = 0;
            if (res_nb[LEAF_BACK] && res_nb[LEAF_FRONT])
            {
                res_total = res_total;
                tri_list[i].m1 = LEAF_BACK;
#if 0
                tri_to_process.push( array< int >(), tri_list[i].m2, tri_list[i].m3, tri_list[i].m4, 0);
                tri_to_process.last().m1.push(0);
                tri_list.remove(i--);
                break;
#endif
            }
            else
            {
                for (; k < TEST_MAX; k++)
                {
                    if (res_side[k] != LEAF_CURRENT)
                    {
                        tri_list[i].m1 = res_side[k];
                        break;
                    }
                }
                if (k == TEST_MAX)
                    tri_list[i].m1 = LEAF_FRONT;
            }
        }
    }

    if (tri_list.count() == 1)
        return 0;
    return 1;
}

} /* namespace lol */

