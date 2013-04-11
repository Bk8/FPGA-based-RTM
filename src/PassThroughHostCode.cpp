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
  float *Pt_array[g_niter + 2];
  float *Pt_fpga_array[g_niter + 2];

  for (int i = 0; i < g_niter + 2; i++) {
    Pt_array[i] = new float[g_size];
    Pt_fpga_array[i] = new float[g_size];
  }
  //float *Pt        = new float[g_size];
  //float *PtM1      = new float[g_size];
  //float *PtP1      = new float[g_size];
  //float *Pt_fpga   = new float[g_size];
  //float *PtM1_fpga = new float[g_size];
  //float *PtP1_fpga = new float[g_size];
  float *A         = new float[g_niter];
  get_amplitude(A, g_niter);

  // init array
  for (int i = 0; i < g_size; i++) {
    //Pt_array[1][i] = Pt_fpga_array[1][i] = 0;
    //Pt_array[0][i] = Pt_fpga_array[0][i] = 0;
    Pt_array[1][i] = Pt_fpga_array[1][i] = (float)rand() / 1000000;
    Pt_array[0][i] = Pt_fpga_array[0][i] = (float)rand() / 1000000;
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
  size_t n_bytes = g_size * sizeof(float);
  float *hello = new float[g_size];
  for (int i = 2; i < g_niter + 2; i++) {
    cout << i - 1 << "st run" << endl;

    max_run(device,
        max_input("Pt_stream", Pt_fpga_array[i-1], n_bytes),
        max_input("PtM1_stream", Pt_fpga_array[i-2], n_bytes),
        max_output("PtP1_stream", Pt_fpga_array[i], n_bytes),
        max_runfor("PassThroughKernel", g_size),
        max_end());

    acoustic(Pt_array[i-1], Pt_array[i-2], Pt_array[i]);

    Pt_array[i][at(0, g_ny/2, g_nx/2, g_ny, g_nx)]      += A[i-2];
    Pt_fpga_array[i][at(0, g_ny/2, g_nx/2, g_ny, g_nx)] += A[i-2];

    printf("Checking data read from FPGA.\n");
    float epsilon = 1e-23;
    for(int j = 0; j < g_size; j++) {
      if ((abs(Pt_array[i][j] - Pt_fpga_array[i][j])) / abs(Pt_array[i][j]) > epsilon) {
        status = 1;
        printf("%10d%20f%20f\n", j, Pt_array[i][j], Pt_fpga_array[i][j]);
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
