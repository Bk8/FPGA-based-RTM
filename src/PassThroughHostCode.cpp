#include <iostream>
#include <iomanip>
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
bool passVerify(float *a, float *b, size_t size);
void allocArray(float **Pt_array, float **Pt_fpga_array, size_t n_array, size_t n_elem);
void deleteArray(float **Pt_array, float **Pt_fpga_array, size_t size);
void stream2FPGA(max_device_handle_t *device);
void runAndCheck(max_device_handle_t *device, float **Pt_array, float **Pt_fpga_array, float *A, size_t size, int n_iter);
void initFPGA(max_maxfile_t **maxfile, max_device_handle_t **device, const char *device_name);

int main(int argc, char* argv[])
{
  char *device_name = (argc==2 ? argv[1] : NULL);
  max_maxfile_t       *maxfile;
  max_device_handle_t *device;
  float               *Pt_array[g_niter + 2];
  float               *Pt_fpga_array[g_niter + 2];
  float                A[g_niter];

  cout << "Opening and configuring FPGA." << endl;
  initFPGA(&maxfile, &device, device_name);

  allocArray(Pt_array, Pt_fpga_array, g_niter + 2, g_size);
  get_amplitude(A, g_niter);

  cout << "Streaming data to/from FPGA." << endl;
  stream2FPGA(device);

  cout << "Begin to execute function: max_run" << endl;
  runAndCheck(device, Pt_array, Pt_fpga_array, A, g_size, g_niter);

  cout << "Shutting down" << endl;
  max_close_device(device);
  max_destroy(maxfile);

  deleteArray(Pt_array, Pt_fpga_array, g_niter + 2);
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

bool passVerify(float *a, float *b, size_t size)
{
  bool ret = true;
  const float epsilon = 1e-23;

  for(size_t i = 0; i < size; i++) {
    if (abs((a[i] - b[i]) / a[i]) > epsilon) {
      ret = false;
      cout << setw(10) << i << setw(20) << a[i] << setw(20) << b[i] << endl;
    }
  }

  return ret;
}

void allocArray(float **Pt_array, float **Pt_fpga_array,
               size_t n_array, size_t n_elem)
{
  for (size_t i = 0; i < n_array; i++) {
    Pt_array[i]      = new float[n_elem];
    Pt_fpga_array[i] = new float[n_elem];
  }

  for (size_t i = 0; i < n_elem; i++) {
    Pt_array[1][i] = Pt_fpga_array[1][i] = (float)rand() / 1000000;
    Pt_array[0][i] = Pt_fpga_array[0][i] = (float)rand() / 1000000;
  }
}

void deleteArray(float **Pt_array, float **Pt_fpga_array, size_t size)
{
  for (size_t i = 0; i < size; i++) {
    delete [] Pt_array[i];
    delete [] Pt_fpga_array[i];
  }
}

void stream2FPGA(max_device_handle_t *device)
{
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
}

void runAndCheck(max_device_handle_t *device, float **Pt_array, 
                 float **Pt_fpga_array, float *A, size_t size, int n_iter)
{
  size_t n_bytes = size * sizeof(float);
  for (int i = 1; i < n_iter + 1; i++) {
    cout << i - 1 << "st run" << endl;

    // run with FPGA
    max_run(device,
        max_input("Pt_stream", Pt_fpga_array[i], n_bytes),
        max_input("PtM1_stream", Pt_fpga_array[i-1], n_bytes),
        max_output("PtP1_stream", Pt_fpga_array[i+1], n_bytes),
        max_runfor("PassThroughKernel", size),
        max_end());

    // run with CPU
    acoustic(Pt_array[i], Pt_array[i-1], Pt_array[i+1]);

    Pt_array[i][at(0, g_ny/2, g_nx/2, g_ny, g_nx)]      += A[i-1];
    Pt_fpga_array[i][at(0, g_ny/2, g_nx/2, g_ny, g_nx)] += A[i-1];

    cout << (passVerify(Pt_array[i+1], Pt_fpga_array[i+1], size) ? 
             "OK. Test passed" : "!!!Test Failed!!!")
         << endl << endl;
  }
}

void initFPGA(max_maxfile_t **maxfile, max_device_handle_t **device, 
              const char *device_name)
{
  *maxfile = max_maxfile_init_PassThrough();
  *device = max_open_device(*maxfile, device_name);
  max_set_terminate_on_error(*device);
}
