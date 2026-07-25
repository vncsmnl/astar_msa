/*!
 * \file TrioAlignSIMD.cpp
 * \author Vinícius Manoel
 * \copyright MIT License
 *
 * \brief Highway SIMD implementation of the TrioAlign k-axis vectorization.
 *
 * Uses Google Highway for performance-portable SIMD (AVX2/AVX-512/SSE4/NEON).
 * Strategy: vectorize the innermost k-loop, processing multiple k values
 * simultaneously within a single SIMD register.
 */
#include "TrioAlignSIMD.h"

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "TrioAlignSIMD.cpp"
#include <hwy/foreach_target.h>
#include <hwy/highway.h>

#include <algorithm>
#include <climits>

HWY_BEFORE_NAMESPACE();
namespace msa_simd {
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

/*!
 * Helper: compute the linear index into the 3D matrix.
 * Layout: row-major, i slowest, k fastest.
 * index = i * (s2_l+1)*(s3_l+1) + j * (s3_l+1) + k
 */
static HWY_INLINE size_t idx3D(int i, int j, int k, int s2_l, int s3_l) {
  return static_cast<size_t>(i) * (s2_l + 1) * (s3_l + 1) +
         static_cast<size_t>(j) * (s3_l + 1) + static_cast<size_t>(k);
}

/*!
 * SIMD kernel: processes the k-axis for a fixed (i, j).
 *
 * Hybrid approach:
 * - Pass 1 (SIMD): compute min of 6 independent candidates (cases 0,1,3,4,5,6)
 *   and store into the output row.
 * - Pass 2 (scalar): sweep backwards merging case 2 (at(i,j,k+1) + 2*GapCost)
 *   with the SIMD result to produce the final cell values.
 */
void TrioCostSIMDRowImpl(int *HWY_RESTRICT matrix, int /*s1_l*/, int s2_l,
                         int s3_l, int i, int j, char s1_char, char s2_char,
                         const char *HWY_RESTRICT s3,
                         const int *HWY_RESTRICT cost_lut, int gap_cost) {
  const hn::ScalableTag<int32_t> d;
  const size_t N = hn::Lanes(d);

  const int two_gap = 2 * gap_cost;
  const auto v_two_gap = hn::Set(d, two_gap);

  // Pre-compute scalar costs that are constant across k
  // cost(s1[i], s2[j]) — same for all k
  const int cost_s1_s2 = cost_lut[(int)s1_char * 128 + (int)s2_char];
  const auto v_cost_s1_s2 = hn::Set(d, cost_s1_s2);

  // Base pointer for current (i, j, 0)
  int *base_ij = matrix + idx3D(i, j, 0, s2_l, s3_l);
  // Neighbor row pointers (already computed layers)
  const int *base_i1_j = matrix + idx3D(i + 1, j, 0, s2_l, s3_l); // (i+1, j, *)
  const int *base_i_j1 = matrix + idx3D(i, j + 1, 0, s2_l, s3_l); // (i, j+1, *)
  const int *base_i1_j1 =
      matrix + idx3D(i + 1, j + 1, 0, s2_l, s3_l); // (i+1, j+1, *)

  // --- Pass 1: SIMD computation of 6 independent candidates ---
  // Process k in chunks of N lanes, from k=0 to k=s3_l-1
  // Note: we process forward (k=0..s3_l-1) since the 6 independent
  // candidates only depend on (i+1,*) and (*,j+1,*) layers.
  // Case 2 (k+1 same-row dependency) is handled in Pass 2.

  int k = 0;
  for (; k + static_cast<int>(N) <= s3_l; k += static_cast<int>(N)) {
    // --- Case 0: at(i+1, j, k) + 2*GapCost ---
    auto v_c0 = hn::Add(hn::LoadU(d, base_i1_j + k), v_two_gap);

    // --- Case 1: at(i, j+1, k) + 2*GapCost ---
    auto v_c1 = hn::Add(hn::LoadU(d, base_i_j1 + k), v_two_gap);

    // --- Case 3: at(i+1, j+1, k) + cost(s1[i], s2[j]) + 2*GapCost ---
    auto v_c3 =
        hn::Add(hn::Add(hn::LoadU(d, base_i1_j1 + k), v_cost_s1_s2), v_two_gap);

    // --- Case 4: at(i+1, j, k+1) + cost(s1[i], s3[k]) + 2*GapCost ---
    auto v_c4_base = hn::LoadU(d, base_i1_j + k + 1);
    // Build cost vector: cost(s1[i], s3[k+lane]) for each lane
    HWY_ALIGN int32_t cost_s1_s3_arr[HWY_MAX_LANES_D(hn::ScalableTag<int32_t>)];
    for (size_t lane = 0; lane < N; ++lane) {
      cost_s1_s3_arr[lane] =
          cost_lut[(int)s1_char * 128 + (int)(unsigned char)s3[k + lane]];
    }
    auto v_cost_s1_s3 = hn::Load(d, cost_s1_s3_arr);
    auto v_c4 = hn::Add(hn::Add(v_c4_base, v_cost_s1_s3), v_two_gap);

    // --- Case 5: at(i, j+1, k+1) + cost(s2[j], s3[k]) + 2*GapCost ---
    auto v_c5_base = hn::LoadU(d, base_i_j1 + k + 1);
    HWY_ALIGN int32_t cost_s2_s3_arr[HWY_MAX_LANES_D(hn::ScalableTag<int32_t>)];
    for (size_t lane = 0; lane < N; ++lane) {
      cost_s2_s3_arr[lane] =
          cost_lut[(int)s2_char * 128 + (int)(unsigned char)s3[k + lane]];
    }
    auto v_cost_s2_s3 = hn::Load(d, cost_s2_s3_arr);
    auto v_c5 = hn::Add(hn::Add(v_c5_base, v_cost_s2_s3), v_two_gap);

    // --- Case 6: at(i+1, j+1, k+1) + cost(s1,s2) + cost(s1,s3) + cost(s2,s3)
    // ---
    auto v_c6_base = hn::LoadU(d, base_i1_j1 + k + 1);
    auto v_c6 = hn::Add(hn::Add(hn::Add(v_c6_base, v_cost_s1_s2), v_cost_s1_s3),
                        v_cost_s2_s3);

    // --- Compute min of 6 candidates ---
    auto v_min = hn::Min(v_c0, v_c1);
    v_min = hn::Min(v_min, v_c3);
    v_min = hn::Min(v_min, v_c4);
    v_min = hn::Min(v_min, v_c5);
    v_min = hn::Min(v_min, v_c6);

    // Store intermediate min (without case 2) into the output row
    hn::StoreU(v_min, d, base_ij + k);
  }

  // Scalar tail: handle remaining k values
  for (; k < s3_l; ++k) {
    int min6 = INT_MAX;

    // Case 0: at(i+1, j, k) + 2*GapCost
    min6 = std::min(min6, base_i1_j[k] + two_gap);

    // Case 1: at(i, j+1, k) + 2*GapCost
    min6 = std::min(min6, base_i_j1[k] + two_gap);

    // Case 3: at(i+1, j+1, k) + cost(s1,s2) + 2*GapCost
    min6 = std::min(min6, base_i1_j1[k] + cost_s1_s2 + two_gap);

    // Case 4: at(i+1, j, k+1) + cost(s1,s3[k]) + 2*GapCost
    int c_s1_s3 = cost_lut[(int)s1_char * 128 + (int)(unsigned char)s3[k]];
    min6 = std::min(min6, base_i1_j[k + 1] + c_s1_s3 + two_gap);

    // Case 5: at(i, j+1, k+1) + cost(s2,s3[k]) + 2*GapCost
    int c_s2_s3 = cost_lut[(int)s2_char * 128 + (int)(unsigned char)s3[k]];
    min6 = std::min(min6, base_i_j1[k + 1] + c_s2_s3 + two_gap);

    // Case 6: at(i+1, j+1, k+1) + cost(s1,s2) + cost(s1,s3) + cost(s2,s3)
    min6 = std::min(min6, base_i1_j1[k + 1] + cost_s1_s2 + c_s1_s3 + c_s2_s3);

    base_ij[k] = min6;
  }

  // --- Pass 2: Scalar backward sweep to merge case 2 ---
  // Case 2: at(i, j, k+1) + 2*GapCost
  // This has a serial dependency along k (each k depends on k+1).
  // at(i, j, s3_l) is already initialized (boundary), so we sweep from s3_l-1
  // down to 0.
  for (k = s3_l - 1; k >= 0; --k) {
    int case2 = base_ij[k + 1] + two_gap;
    base_ij[k] = std::min(base_ij[k], case2);
  }
}

} // namespace HWY_NAMESPACE
} // namespace msa_simd
HWY_AFTER_NAMESPACE();

// ===== Dynamic dispatch boilerplate =====
#if HWY_ONCE

namespace msa_simd {

HWY_EXPORT(TrioCostSIMDRowImpl);

// Dispatches to the best available SIMD target
void CallTrioCostSIMDRow(int *matrix, int s1_l, int s2_l, int s3_l, int i,
                         int j, char s1_char, char s2_char, const char *s3,
                         const int *cost_lut, int gap_cost) {
  HWY_DYNAMIC_DISPATCH(TrioCostSIMDRowImpl)
  (matrix, s1_l, s2_l, s3_l, i, j, s1_char, s2_char, s3, cost_lut, gap_cost);
}

} // namespace msa_simd

// Public C-linkage function
void trioCostSIMD_Row(int *matrix, int s1_l, int s2_l, int s3_l, int i, int j,
                      char s1_char, char s2_char, const char *s3,
                      const int *cost_lut, int gap_cost) {
  msa_simd::CallTrioCostSIMDRow(matrix, s1_l, s2_l, s3_l, i, j, s1_char,
                                s2_char, s3, cost_lut, gap_cost);
}

#endif // HWY_ONCE
