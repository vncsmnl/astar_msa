/*!
 * \class TrioAlign
 * \author Vinícius Manoel
 * \copyright MIT License
 */
#include "TrioAlign.h"

#include <algorithm>
#include <climits>
#include <iostream>
#include <string>

#include "Cost.h"

/*!
 * Creates a trio align of the sequences \a s1, \a s2 and \a s3.
 * It is important to remember which sequences we are using, so save this
 * information as the trio \a t
 */
TrioAlign::TrioAlign(Trio t, const std::string &s1, const std::string &s2, const std::string &s3)
    : m_trio(t)
{
    Align(s1, s2, s3);
}

// Calculates the 1D index for 3D coordinates (i, j, k).
// Uses row-major order: i varies slowest, k varies fastest
inline size_t TrioAlign::index3D(int i, int j, int k) const
{
    return static_cast<size_t>(i) * (s2_l + 1) * (s3_l + 1) +
           static_cast<size_t>(j) * (s3_l + 1) +
           static_cast<size_t>(k);
}

// Mutable access to the matrix
inline int &TrioAlign::at(int i, int j, int k)
{
    return m_matrix[index3D(i, j, k)];
}

// Immutable access to the matrix
inline int TrioAlign::at(int i, int j, int k) const
{
    return m_matrix[index3D(i, j, k)];
}

//! Initialize all internal matrixes with sizes \a size1, \a size2 and \a size3
void TrioAlign::initMatrix(int size1, int size2, int size3)
{
    s1_l = size1;
    s2_l = size2;
    s3_l = size3;

    // Allocate the vector with the necessary size for the 3D matrix
    // (size1+1) * (size2+1) * (size3+1) elements
    size_t total_size = static_cast<size_t>(s1_l + 1) *
                        static_cast<size_t>(s2_l + 1) *
                        static_cast<size_t>(s3_l + 1);
    m_matrix.resize(total_size);
}

/*!
 * Helper function to calculate and save on the matrix the cost of
 * the cell with coords \a i, \a j and \a k
 */
void TrioAlign::trioCost(int i, int j, int k, const std::string &s1, const std::string &s2, const std::string &s3)
{
    int min_value = INT_MAX;

    // 7 possible cases for aligning 3 sequences:
    // 1. Advance in s1 only (gap in s2 and s3)
    int c0 = at(i + 1, j, k) + 2 * Cost::GapCost;
    min_value = std::min(min_value, c0);

    // 2. Advance in s2 only (gap in s1 and s3)
    int c1 = at(i, j + 1, k) + 2 * Cost::GapCost;
    min_value = std::min(min_value, c1);

    // 3. Advance in s3 only (gap in s1 and s2)
    int c2 = at(i, j, k + 1) + 2 * Cost::GapCost;
    min_value = std::min(min_value, c2);

    // 4. Advance in s1 and s2 (gap in s3)
    int c3 = at(i + 1, j + 1, k) + Cost::cost(s1.at(i), s2.at(j)) + 2 * Cost::GapCost;
    min_value = std::min(min_value, c3);

    // 5. Advance in s1 and s3 (gap in s2)
    int c4 = at(i + 1, j, k + 1) + Cost::cost(s1.at(i), s3.at(k)) + 2 * Cost::GapCost;
    min_value = std::min(min_value, c4);

    // 6. Advance in s2 and s3 (gap in s1)
    int c5 = at(i, j + 1, k + 1) + Cost::cost(s2.at(j), s3.at(k)) + 2 * Cost::GapCost;
    min_value = std::min(min_value, c5);

    // 7. Advance in all 3 sequences
    int c6 = at(i + 1, j + 1, k + 1) + Cost::cost(s1.at(i), s2.at(j)) + Cost::cost(s1.at(i), s3.at(k)) + Cost::cost(s2.at(j), s3.at(k));
    min_value = std::min(min_value, c6);

    at(i, j, k) = min_value;
}

//! Do a trio alignment (3D dynamic programming)
void TrioAlign::Align(const std::string &s1, const std::string &s2, const std::string &s3)
{
    int i, j, k;
    initMatrix(s1.length(), s2.length(), s3.length());

    // Initialize the final cell (all sequences consumed)
    at(s1_l, s2_l, s3_l) = 0;

    // Initialize the edges of the 3D matrix
    // Edge where only s3 varies (s1 and s2 at the end)
    for (k = s3_l - 1; k >= 0; k--)
    {
        at(s1_l, s2_l, k) = at(s1_l, s2_l, k + 1) + 2 * Cost::GapCost;
    }

    // Edge where only s2 varies (s1 and s3 at the end)
    for (j = s2_l - 1; j >= 0; j--)
    {
        at(s1_l, j, s3_l) = at(s1_l, j + 1, s3_l) + 2 * Cost::GapCost;
    }

    // Edge where only s1 varies (s2 and s3 at the end)
    for (i = s1_l - 1; i >= 0; i--)
    {
        at(i, s2_l, s3_l) = at(i + 1, s2_l, s3_l) + 2 * Cost::GapCost;
    }

    // Faces of the 3D matrix
    // Face s1-s2 (k = s3_l)
    for (i = s1_l - 1; i >= 0; i--)
    {
        for (j = s2_l - 1; j >= 0; j--)
        {
            int cost_match = Cost::cost(s1.at(i), s2.at(j)) + 2 * Cost::GapCost;
            int cost_gap1 = 2 * Cost::GapCost;
            int cost_gap2 = 2 * Cost::GapCost;

            int min_cost = std::min({at(i + 1, j, s3_l) + cost_gap1,
                                     at(i, j + 1, s3_l) + cost_gap2,
                                     at(i + 1, j + 1, s3_l) + cost_match});
            at(i, j, s3_l) = min_cost;
        }
    }

    // Face s1-s3 (j = s2_l)
    for (i = s1_l - 1; i >= 0; i--)
    {
        for (k = s3_l - 1; k >= 0; k--)
        {
            int cost_match = Cost::cost(s1.at(i), s3.at(k)) + 2 * Cost::GapCost;
            int cost_gap1 = 2 * Cost::GapCost;
            int cost_gap2 = 2 * Cost::GapCost;

            int min_cost = std::min({at(i + 1, s2_l, k) + cost_gap1,
                                     at(i, s2_l, k + 1) + cost_gap2,
                                     at(i + 1, s2_l, k + 1) + cost_match});
            at(i, s2_l, k) = min_cost;
        }
    }

    // Face s2-s3 (i = s1_l)
    for (j = s2_l - 1; j >= 0; j--)
    {
        for (k = s3_l - 1; k >= 0; k--)
        {
            int cost_match = Cost::cost(s2.at(j), s3.at(k)) + 2 * Cost::GapCost;
            int cost_gap1 = 2 * Cost::GapCost;
            int cost_gap2 = 2 * Cost::GapCost;

            int min_cost = std::min({at(s1_l, j + 1, k) + cost_gap1,
                                     at(s1_l, j, k + 1) + cost_gap2,
                                     at(s1_l, j + 1, k + 1) + cost_match});
            at(s1_l, j, k) = min_cost;
        }
    }

    // Fill the interior of the 3D matrix
    for (i = s1_l - 1; i >= 0; i--)
    {
        for (j = s2_l - 1; j >= 0; j--)
        {
            for (k = s3_l - 1; k >= 0; k--)
            {
                trioCost(i, j, k, s1, s2, s3);
            }
        }
    }

#ifdef PAIRALIGN_SCORE
    std::cout << at(0, 0, 0) << std::endl;
#endif
}

//! Return the value of the 3D-matrix with coords i, j and k
int TrioAlign::getScore(const int i, const int j, const int k) const
{
    return at(i, j, k);
}
