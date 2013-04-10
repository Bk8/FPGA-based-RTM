/**
 * @file main.cpp
 * @brief It calculate the acoustic wave equation by CPU. More information
 * could be found in the README.mkd file.
 * @author Conghui He
 * @version 1.0
 * @date Wed 27 Mar 2013 08:59:27 PM CST
 */

#include "global_var.h"
#include "acoustic.h"
#include "sysTime.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;
void get_amplitude(float *A, int n);

int main(void)
{
  float *Pt = new float[g_size];
  float *PtM1 = new float[g_size];
  float *PtP1 = new float[g_size];
  float *A = new float[g_niter];
  SysTime time;

  get_amplitude(A, g_niter);

  time.start();
  for (int i = 0; i < g_niter; i++) {
    acoustic(Pt, PtM1, A[i], PtP1);
  }
  time.stop();

  cout << "Size:  (" << g_nx << " x " << g_ny << " x " 
       << g_nz << ");\t # of Iteration: " << g_niter << ": \n"
       << "Time elapsed: " << time.elapsedS() << " sec" << endl;

  delete [] Pt;
  delete [] PtM1;
  delete [] PtP1;
  delete [] A;

  return 0;
}


/**
 * @brief Read the riker wavelet amplitude into array A
 *
 * @param A output array
 * @param n n elements
 */
void get_amplitude(float *A, int n)
{
  ifstream ifs(g_rw_fname, ios::binary);

  if (!ifs) {
    cerr << "can not open file " << g_rw_fname << " to read\n";
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < n; i++) {
    ifs.read(reinterpret_cast<char *>(&A[i]), sizeof *A);
  }

  ifs.close();
}
