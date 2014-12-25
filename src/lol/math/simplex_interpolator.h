//
// Lol Engine
//
// Copyright: (c) 2010-2014 Sam Hocevar <sam@hocevar.net>
//            (c) 2013-2014 Benjamin "Touky" Huet <huet.benjamin@gmail.com>
//            (c) 2013-2014 Guillaume Bittoun <guillaume.bittoun@gmail.com>
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the Do What The Fuck You Want To
//   Public License, Version 2, as published by Sam Hocevar. See
//   http://www.wtfpl.net/ for more details.
//

#pragma once

#include <functional>

namespace lol
{

/*
 * Simplex noise in dimension N
 * ----------------------------
 *
 *  The N-dimensional regular hypercube can be split into N! simplices that
 * all have the main diagonal as a shared edge.
 *  - number of simplices: N!
 *  - number of vertices per simplex: N+1
 *  - number of edges: N(N+1)/2
 *  - minimum edge length: 1           (hypercube edges, e.g. [1,0,0,…,0])
 *  - maximum edge length: sqrt(N)     (hypercube diagonal, i.e. [1,1,1,…,1])
 *
 *  We skew the simplicial grid along the main diagonal by a factor f =
 * sqrt(N+1), which means the diagonal of the initial parallelotope has
 * length sqrt(N/(N+1)). The edges of that parallelotope have length
 * sqrt(N/(N+1)), too. A formula for the maximum edge length was found
 * empirically:
 *  - minimum edge length: sqrt(N/(N+1))   (parallelotope edges and diagonal)
 *  - maximum edge length: sqrt(floor((N+1)²/4)/(N+1))
 */

template<int N>
class simplex_interpolator
{
public:
    simplex_interpolator(int seed = 0)
      : m_seed(seed)
    {
#if 0
        debugprint();
#endif
    }

    /* Single interpolation */
    inline float Interp(vec_t<float, N> position) const
    {
        // Retrieve the containing hypercube origin and associated decimals
        vec_t<int, N> origin;
        vec_t<float, N> pos;
        get_origin(skew(position), origin, pos);

        return get_noise(origin, pos);
    }

    /* Only for debug purposes: return the gradient vector */
    inline vec_t<float, N> GetGradient(vec_t<float, N> position) const
    {
        vec_t<int, N> origin;
        vec_t<float, N> pos;
        get_origin(skew(position), origin, pos);

        return get_gradient(origin);
    }

protected:
    inline float get_noise(vec_t<int, N> origin,
                           vec_t<float, N> const & pos) const
    {
        /* For a given position [0…1]^N inside a regular N-hypercube, find
         * the N-simplex which contains that position, and return a path
         * along the hypercube edges from (0,0,…,0) to (1,1,…,1) which
         * uniquely describes that simplex. */
        vec_t<int, N> traversal_order;
        for (int i = 0; i < N; ++i)
            traversal_order[i] = i;

        /* Naïve bubble sort — enough for now */
        for (int i = 0; i < N; ++i)
            for (int j = i + 1; j < N; ++j)
                if (pos[traversal_order[i]] < pos[traversal_order[j]])
                    std::swap(traversal_order[i], traversal_order[j]);


        /* Get the position in world coordinates, too */
        vec_t<float, N> world_pos = unskew(pos);

        /* “corner” will traverse the simplex along its edges in world
         * coordinates. */
        vec_t<float, N> world_corner(0.f);
        float result = 0.f, sum = 0.f;

        for (int i = 0; i < N + 1; ++i)
        {
#if 1
            // In “Noise Hardware” (2-17) Perlin uses 0.6 but Gustavson uses
            // 0.5 instead, saying “else the noise is not continuous at
            // simplex boundaries”.
            // And indeed, the distance between any given simplex vertex and
            // the opposite hyperplane is 1/sqrt(2), so the contribution of
            // that vertex should never be > 0 for points further than
            // 1/sqrt(2). Hence 0.5 - d².
            float d = 0.5f - sqlength(world_pos - world_corner);
#else
            // DEBUG: this is the linear contribution of each vertex
            // in the skewed simplex. Unfortunately it creates artifacts.
            float d = ((i == 0) ? 1.f : pos[traversal_order[i - 1]])
                    - ((i == N) ? 0.f : pos[traversal_order[i]]);
#endif

            if (d > 0)
            {
                // Perlin uses 8d⁴ whereas Gustavson uses d⁴ and a final
                // multiplication factor at the end. Let’s go with
                // Gustavson, it’s a few multiplications less.
                d = d * d * d * d;

                //d = (3.f - 2.f * d) * d * d;
                //d = ((6 * d - 15) * d + 10) * d * d * d;

                result += d * dot(get_gradient(origin),
                                  world_pos - world_corner);
                sum += d;
            }

            if (i < N)
            {
                vec_t<float, N> v(0.f);
                v[traversal_order[i]] = 1.f;
                world_corner += unskew(v);
                origin[traversal_order[i]] += 1;
            }
        }

        // FIXME: Gustavson uses the value 70 for dimension 2, 32 for
        // dimension 3, and 27 for dimension 4, and uses non-unit gradients
        // of length sqrt(2), sqrt(2) and sqrt(3). Find out where this comes
        // from and maybe find a more generic formula.
        float const k = N == 2 ? (70.f * sqrt(2.f)) // approx. 99
                      : N == 3 ? (70.f * sqrt(2.f)) // approx. 45
                      : N == 4 ? (70.f * sqrt(3.f)) // approx. 47
                      :          50.f;
        //return k * result / sum;
        return k * result;
    }

    inline vec_t<float, N> get_gradient(vec_t<int, N> origin) const
    {
        /* Quick shuffle table:
         * strings /dev/urandom | grep . -nm256 | sort -k2 -t: | sed 's|:.*|,|'
         * Then just replace “256” with “0”. */
        static int const shuffle[256] =
        {
            111, 14, 180, 186, 221, 114, 219, 79, 66, 46, 152, 81, 246, 200,
            141, 172, 85, 244, 112, 92, 34, 106, 218, 205, 236, 7, 121, 115,
            109, 131, 10, 96, 188, 148, 17, 107, 94, 182, 235, 163, 143, 63,
            248, 202, 52, 154, 37, 241, 53, 129, 25, 159, 242, 38, 171, 213,
            6, 203, 255, 193, 42, 209, 28, 176, 210, 60, 54, 144, 3, 71, 89,
            116, 12, 237, 67, 216, 252, 178, 174, 164, 98, 234, 32, 26, 175,
            24, 130, 128, 113, 99, 212, 62, 11, 75, 185, 73, 93, 31, 30, 44,
            122, 173, 139, 91, 136, 162, 194, 41, 56, 101, 68, 69, 211, 151,
            97, 55, 83, 33, 50, 119, 156, 149, 208, 157, 253, 247, 161, 133,
            230, 166, 225, 204, 224, 13, 110, 123, 142, 64, 65, 155, 215, 9,
            197, 140, 58, 77, 214, 126, 195, 179, 220, 232, 125, 147, 8, 39,
            187, 27, 217, 100, 134, 199, 88, 206, 231, 250, 74, 2, 135, 120,
            21, 245, 118, 243, 82, 183, 238, 150, 158, 61, 4, 177, 146, 153,
            117, 249, 254, 233, 90, 222, 207, 48, 15, 18, 20, 16, 47, 0, 51,
            165, 138, 127, 169, 72, 1, 201, 145, 191, 192, 239, 49, 19, 160,
            226, 228, 84, 181, 251, 36, 87, 22, 43, 70, 45, 105, 5, 189, 95,
            40, 196, 59, 57, 190, 80, 104, 167, 78, 124, 103, 240, 184, 170,
            137, 29, 23, 223, 108, 102, 86, 198, 227, 35, 229, 76, 168, 132,
        };

        /* Generate 2^(N+2) random vectors, but at least 2^5 (32) and not
         * more than 2^20 (~ 1 million). */
        int const gradient_count = 1 << min(max(N + 2, 5), 20);

        static auto build_gradients = [&]()
        {
            array<vec_t<float, N>> ret;
            for (int k = 0; k < gradient_count; ++k)
            {
                vec_t<float, N> v;
                for (int i = 0; i < N; ++i)
                    v[i] = rand(-1.f, 1.f);
                ret << normalize(v);
            }
            return ret;
        };

        static array<vec_t<float, N>> const gradients = build_gradients();

        int idx = m_seed;
        for (int i = 0; i < N; ++i)
            idx ^= shuffle[(idx + origin[i]) & 255];

        idx &= (gradient_count - 1);
#if 0
        // DEBUG: only output a few gradients
        if (idx > 2)
            return vec_t<float, N>(0);
#endif
        return gradients[idx];
    }

    static inline vec_t<float, N> skew(vec_t<float, N> const &v)
    {
        /* Quoting Perlin in “Hardware Noise” (2-18):
         *   The “skew factor” f should be set to f = sqrt(N+1), so that
         *   the point (1,1,...1) is transformed to the point (f,f,...f). */
        float const sum = dot(v, vec_t<float, N>(1));
        float const f = sqrt(1.f + N);
        return v + vec_t<float, N>(sum * (f - 1) / N);
    }

    static inline vec_t<float, N> unskew(vec_t<float, N> const &v)
    {
        float const sum = dot(v, vec_t<float, N>(1));
        float const f = sqrt(1.f + N);
        return v + vec_t<float, N>(sum * (1 / f - 1) / N);
    }

    /* For a given world position, extract grid coordinates (origin) and
     * the corresponding delta position (pos). */
    inline void get_origin(vec_t<float, N> const & world_position,
                           vec_t<int, N> & origin, vec_t<float, N> & pos) const
    {
        // Finding floor point index
        for (int i = 0; i < N; ++i)
            origin[i] = ((int)world_position[i]) - (world_position[i] < 0);

        // Extracting decimal part from simplex sample
        pos = world_position - (vec_t<float, N>)origin;
    }

private:
    void debugprint()
    {
        // Print some debug information
        printf("Simplex Noise of Dimension %d\n", N);

        long long int n = 1; for (int i = 1; i <= N; ++i) n *= i;
        printf(" - each hypercube cell has %lld simplices "
               "with %d vertices and %d edges\n", n, N + 1, N * (N + 1) / 2);

        vec_t<float, N> diagonal(1.f);
        printf(" - regular hypercube:\n");
        printf("   · edge length 1\n");
        printf("   · diagonal length %f\n", length(diagonal));
        printf(" - unskewed parallelotope:\n");
        printf("   · edge length %f\n", length(unskew(diagonal)));
        printf("   · diagonal length %f\n", length(unskew(diagonal)));
        printf("   · simplex edge lengths between %f and %f\n",
               sqrt((float)N/(N+1)), sqrt((N+1)*(N+1)/4/(float)(N+1)));

        /* Generate simplex vertices */
        vec_t<float, N> vertices[N + 1];
        vertices[0] = vec_t<float, N>(0.f);
        for (int i = 0; i < N; ++i)
        {
            vertices[i + 1] = vertices[i];
            vertices[i + 1][i] += 1.f;
        }
        for (int i = 0; i < N + 1; ++i)
            vertices[i] = unskew(vertices[i]);

        for (int i = 0; i < N + 1; ++i)
        {
            printf(" - vertex %d\n", i);

#if 0
            /* Coordinates for debugging purposes */
            printf("   · [");
            for (int k = 0; k < N; ++k)
                printf(" %f", vertices[i][k]);
            printf(" ]\n");
#endif

            /* Analyze edge lengths from that vertex */
            float minlength = 1.0f;
            float maxlength = 0.0f;
            for (int j = 0; j < N + 1; ++j)
            {
                if (i == j)
                    continue;

                float l = length(vertices[i] - vertices[j]);
                minlength = min(minlength, l);
                maxlength = max(maxlength, l);
            }
            printf("   · edge lengths between %f and %f\n",
                   minlength, maxlength);

#if 0
            /* Experimental calculation of the distance to the opposite
             * hyperplane, by picking random points. Works reasonably
             * well up to dimension 6. After that, we’d need something
             * better such as gradient walk. */
            float mindist = 1.0f;
            for (int run = 0; run < 10000000; ++run)
            {
                vec_t<float, N> p(0.f);
                float sum = 0.f;
                for (int j = 0; j < N + 1; ++j)
                {
                    if (i == j)
                        continue;
                    float k = rand(1.f);
                    p += k * vertices[j];
                    sum += k;
                }
                mindist = min(mindist, distance(vertices[i], p / sum));
            }
            printf("   · approx. dist. to opposite hyperplane: %f\n", mindist);
#endif

#if 0
            /* Find a normal vector to the opposite hyperplane. First, pick
             * any point p(i0) on the hyperplane. We just need i0 != i. Then,
             * build a matrix where each row is p(i0)p(j) for all j != i0.
             * Multiplying this matrix by the normal vectors gives a vector
             * full of zeroes except at position i. So we build a vector
             * full of zeroes except at position i, and multiply it by the
             * matrix inverse. */
            int i0 = (i == 0) ? 1 : 0;
            mat_t<float, N, N> m;
            for (int j = 0; j < N; ++j)
            {
                auto v = vertices[j < i0 ? j : j + 1] - vertices[i0];
                for (int k = 0; k < N; ++k)
                    m[k][j] = v[k];
            }
            vec_t<float, N> p(0.f);
            p[i < i0 ? i : i - 1] = 1.f;
            auto normal = normalize(inverse(m) * p);

            /* Find distance from current vertex to the opposite hyperplane.
             * Just use the projection theorem in N dimensions. */
            auto w = vertices[i] - vertices[i0];
            float dist = abs(dot(normal, w));
            printf("   · distance to opposite hyperplane: %f\n", dist);
#endif
        }
        printf("\n");
    }

    /* A user-provided random seed. Defaults to zero. */
    int m_seed;
};

}

