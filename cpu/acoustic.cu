#include "acoustic.h"

/**
 * @brief calculate the next generation of P_t+1 from P_t and P_t-1
 *
 * The formula is described as:
 *
 * P_t+1 = (dt)^2 * ((c^2 / (dx)^2) * stencil(P_t) + A) + 2*P_t - P_t-1
 *
 * where
 *  dt = 0.002, defined in global_var.cpp
 *  c  = 3000,  defined in gloabl_var.cpp
 *  dx = 200,   defined in global_var.cpp
 *  A is read from the alreadly calculated array of riker wavelet formula
 *
 * @param Pt P in current point
 * @param PtM1 P_t-1 (Pt minus 1)
 * @param A The amplitude of the wave at t
 * @param PtP1 P_t+1 (Pt Plus 1)
 */
void acoustic(float *Pt, float *PtM1, float A, float *PtP1)
{
  stencil_cpu(Pt, g_tmp, g_nz, g_ny, g_nx);

  for (int i = 1; i < g_size; i++) {
    PtP1[i] = (g_dt*g_dt) *  ( (g_c*g_c / (g_dx*g_dx)) * g_tmp[i] + A ) +
              2 * Pt[i] - PtM1[i-1];
  }
}
