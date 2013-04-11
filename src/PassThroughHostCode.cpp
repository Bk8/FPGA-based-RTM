#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <MaxCompilerRT.h>

#include "global_var.h"
#include "acoustic.h"
#include "stencil.h"
#include "sysTime.h"

using namespace std;
void get_amplitude(float *A, int n);

int main(int argc, char* argv[])
{
  char *device_name = (argc==2 ? argv[1] : NULL);
  max_maxfile_t* maxfile;
  max_device_handle_t* device;
  int status = 0;

  cout << "Opening and configuring FPGA." << endl;

  maxfile = max_maxfile_init_PassThrough();
  device = max_open_device(maxfile, device_name);
  max_set_terminate_on_error(device);

  cout << "Generating data to send to FPGA." << endl;
  float *Pt        = new float[g_size];
  float *PtM1      = new float[g_size];
  float *PtP1      = new float[g_size];
  float *Pt_fpga   = new float[g_size];
  float *PtM1_fpga = new float[g_size];
  float *PtP1_fpga = new float[g_size];
  float *A         = new float[g_niter];

  //float A = 100;
  get_amplitude(A, g_niter);

  // init array
  for (int i = 0; i < g_size; i++) {
    //Pt[i] = Pt_fpga[i] = (float)rand() / 100000;
    //PtM1[i] = PtM1_fpga[i] = (float)rand() / 100000;
    Pt[i] = Pt_fpga[i] = 0;
    PtM1[i] = PtM1_fpga[i] = 0;
  }

  cout << "Streaming data to/from FPGA." << endl;
  max_set_scalar_input_f(device, "PassThroughKernel.w3", w3, FPGA_A);
  max_set_scalar_input_f(device, "PassThroughKernel.w2", w2, FPGA_A);
  max_set_scalar_input_f(device, "PassThroughKernel.w1", w1, FPGA_A);
  max_set_scalar_input_f(device, "PassThroughKernel.w0", w0, FPGA_A);
  max_set_scalar_input_f(device, "PassThroughKernel.dt", g_dt, FPGA_A);
  max_set_scalar_input_f(device, "PassThroughKernel.c",  g_c, FPGA_A);
  max_set_scalar_input_f(device, "PassThroughKernel.dx", g_dx, FPGA_A);
  max_set_scalar_input_f(device, "PassThroughKernel.xMax", g_nx, FPGA_A);
  max_set_scalar_input_f(device, "PassThroughKernel.yMax", g_ny, FPGA_A);
  max_set_scalar_input_f(device, "PassThroughKernel.zMax", g_nz, FPGA_A);

  max_set_runtime_param(device, "PassThroughKernel.nx_offset", g_nx);
  max_set_runtime_param(device, "PassThroughKernel.nxy_offset", g_nx * g_ny);
  max_upload_runtime_params(device, FPGA_A);

  cout << "Begin to execute function: max_run" << endl;
  size_t n_bytes = g_size * sizeof *Pt;
  for (int i = 0; i < g_niter; i++) {
    cout << i + 1 << "st run" << endl;

    max_run(device,
        max_input("Pt_stream", Pt_fpga, n_bytes),
        max_input("PtM1_stream", PtM1_fpga, n_bytes),
        max_output("PtP1_stream", PtP1_fpga, n_bytes),
        max_runfor("PassThroughKernel", g_size),
        max_end());

    acoustic(Pt, PtM1, PtP1);

    PtP1[at(0, g_ny/2, g_nx/2, g_ny, g_nx)]      += A[i];
    PtP1_fpga[at(0, g_ny/2, g_nx/2, g_ny, g_nx)] += A[i];

    // swap the pointers of host
    float *tmp = PtM1;
    PtM1       = Pt;
    Pt         = PtP1;
    PtP1       = tmp;

    // swap the pointers of fpga
    tmp       = PtM1_fpga;
    PtM1_fpga = Pt_fpga;
    Pt_fpga   = PtP1_fpga;
    PtP1_fpga = tmp;

    printf("Checking data read from FPGA.\n");
    float epsilon = 1e-23;
    for(int j = 0; j < g_size; j++) {
      if ((abs(PtP1[j] - PtP1_fpga[j])) / abs(PtP1[j]) > epsilon) {
        status = 1;
        printf("%10d%20f%20f\n", j, PtP1[j], PtP1_fpga[j]);
        //break;
      }
    }

    if (status)
      cout <<"Test failed." << endl;
    else
      cout << "Test passed OK!" << endl;

    cout << "end " << i + 1 << "st run" << endl;
  }


  cout << "Shutting down" << endl;
  max_close_device(device);
  max_destroy(maxfile);

  return status;

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
