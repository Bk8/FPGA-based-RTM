#include "global_var.h"

int   g_c  = 3000;   /* velocity */
int   g_dx = 200;    /* delta x  */
float g_dt = 0.002f; /* delta t  */
float g_tmp[g_size]; /* array storing stencil of Pt */
const char *g_rw_fname = "../riker_wavelet/rw_2000_f.dat";
