/*!
 * \file SIMDAligner.h
 * \author Vinícius Manoel
 * \copyright MIT License
 *
 * \brief SIMD-accelerated trio alignment kernel using Google Highway.
 *
 * Vectorizes the innermost k-loop of the 3D DP by processing multiple
 * k values simultaneously (8 lanes for AVX2/int32, 16 for AVX-512).
 *
 * Strategy: k-axis vectorization (Strategy B).
 * - Cases 0,1,3,4,5,6 (6 of 7 candidates) are fully independent across k
 *   and computed via SIMD Load/GatherIndex + Add + Min.
 * - Case 2 (at(i,j,k+1) + 2*GapCost) has a serial k-dependency and is
 *   merged in a scalar backward sweep after the SIMD pass.
 */
#ifndef _SIMDALIGNER_H
#define _SIMDALIGNER_H

/*!
 * SIMD-accelerated computation of the interior cells for a fixed (i, j).
 * Fills matrix[i][j][k] for k in [0, s3_l-1] using vectorized min-of-7.
 *
 * The hybrid approach:
 * 1. SIMD pass: compute min of 6 independent candidates (cases 0,1,3,4,5,6)
 * 2. Scalar sweep: merge case 2 (k+1 dependency) backwards
 *
 * \param matrix     Pointer to the linearized 3D matrix (row-major: i * (s2_l+1)*(s3_l+1) + j * (s3_l+1) + k)
 * \param s1_l       Length of sequence 1
 * \param s2_l       Length of sequence 2
 * \param s3_l       Length of sequence 3
 * \param i          Fixed i-coordinate (current s1 position)
 * \param j          Fixed j-coordinate (current s2 position)
 * \param s1_char    Character s1[i]
 * \param s2_char    Character s2[j]
 * \param s3         Pointer to sequence 3 C-string
 * \param cost_lut   Flat cost lookup table [128*128] (r*128+l indexing)
 * \param gap_cost   Gap cost value (Cost::GapCost)
 */
void trioCostSIMD_Row(int *matrix,
                      int s1_l, int s2_l, int s3_l,
                      int i, int j,
                      char s1_char, char s2_char,
                      const char *s3,
                      const int *cost_lut,
                      int gap_cost);

#endif // _SIMDALIGNER_H