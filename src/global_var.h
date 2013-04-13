#ifndef GLOBAL_VAR_H

#define GLOBAL_VAR_H

extern int   g_c;
extern int   g_dx;
extern float g_dt;
extern float g_tmp[];
extern const char *g_rw_fname;

const int dimension = 96;
const int g_nx = dimension;
const int g_ny = dimension;
const int g_nz = dimension;
const int g_size = g_nx * g_ny * g_nz;
const int g_nt = 10; /* number of iteration (timestamps) of process */
const int g_ns = 1; /* # of shots */


#endif /* end of include guard: GLOBAL_VAR_H */
