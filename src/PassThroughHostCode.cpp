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
void runAndCheck(max_device_handle_t *device, float **Pt_array, float **Pt_fpga_array, float *source, float **receivers, float **receivers_fpga, size_t size, int n_iter);
void get_amplitude(float *A, int n);
bool passVerify(float *a, float *b, size_t size);
void allocArray(float **Pt_array, float **Pt_fpga_array, float **receivers, float **receivers_fpga, size_t nt, size_t n_elem);
void deleteArray(float **Pt_array, float **Pt_fpga_array, float **receivers, float **receivers_fpga, size_t size);
void stream2FPGA(max_device_handle_t *device);
void forwardMigrationAndCheck(max_device_handle_t *device, float **Pt_array, float **Pt_fpga_array, float *source, float **receivers, float **receivers_fpga, size_t size, int n_iter);
void reverseMigrationAndCheck(max_device_handle_t *device, float **Pt_array, float **Pt_fpga_array, float **receivers, float **receivers_fpga, float *image, float *image_fpga, size_t size, int n_iter);
void initFPGA(max_maxfile_t **maxfile, max_device_handle_t **device, const char *device_name);

int main(int argc, char* argv[])
{
  char *device_name = (argc==2 ? argv[1] : NULL);
  max_maxfile_t       *maxfile;
  max_device_handle_t *device;
  float               *Pt_array[g_nt + 2];
  float               *Pt_fpga_array[g_nt + 2];
  float               source[g_nt];
  float               *receivers[g_nx *g_ny];
  float               *receivers_fpga[g_ny * g_ny];
  float               image[g_size] = {0};
  float               image_fpga[g_size] = {0};

  cout << "Opening and configuring FPGA." << endl;
  initFPGA(&maxfile, &device, device_name);

  allocArray(Pt_array, Pt_fpga_array, receivers, receivers_fpga, g_nt, g_size);
  get_amplitude(source, g_nt);

  cout << "Streaming data to/from FPGA." << endl;
  stream2FPGA(device);

  cout << "Begin to execute function: max_run" << endl;
  forwardMigrationAndCheck(device, Pt_array, Pt_fpga_array, source, receivers, receivers_fpga, g_size, g_nt);

  cout << "Begin to Reverse Migration" << endl;
  reverseMigrationAndCheck(device, Pt_array, Pt_fpga_array, 
                           receivers, receivers_fpga,
                           image, image_fpga,
                           g_size, g_nt);

  cout << "Shutting down" << endl;
  max_close_device(device);
  max_destroy(maxfile);

  deleteArray(Pt_array, Pt_fpga_array, receivers, receivers_fpga, g_nt + 2);
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
                float **receivers, float **receivers_fpga, 
                size_t nt, size_t n_elem)
{
  size_t n_array = nt + 2;

  for (size_t i = 0; i < g_nx * g_ny; i++) {
    receivers[i] = new float[nt];
    receivers_fpga[i] = new float[nt];
  }

  for (size_t i = 0; i < n_array; i++) {
    Pt_array[i]      = new float[n_elem];
    Pt_fpga_array[i] = new float[n_elem];
  }

  for (size_t i = 0; i < n_elem; i++) {
    Pt_array[1][i] = Pt_fpga_array[1][i] = (float)rand() / 1000000;
    Pt_array[0][i] = Pt_fpga_array[0][i] = (float)rand() / 1000000;
  }
}

void deleteArray(float **Pt_array, float **Pt_fpga_array, 
                 float **receivers, float **receivers_fpga,
                 size_t size)
{
  for (size_t i = 0; i < g_nx * g_ny; i++) {
    delete [] receivers[i];
    delete [] receivers_fpga[i];
  }

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

void forwardMigrationAndCheck(max_device_handle_t *device, 
                 float **Pt_array, float **Pt_fpga_array, 
                 float *source, 
                 float **receivers, float **receivers_fpga,
                 size_t size, int n_iter)
{
  size_t n_bytes = size * sizeof(float);
  for (int i = 1; i < n_iter + 1; i++) {
    cout << i - 1 << "st run: ";

    // run with FPGA
    max_run(device,
        max_input("Pt_stream", Pt_fpga_array[i], n_bytes),
        max_input("PtM1_stream", Pt_fpga_array[i-1], n_bytes),
        max_output("PtP1_stream", Pt_fpga_array[i+1], n_bytes),
        max_runfor("PassThroughKernel", size),
        max_end());

    // run with CPU
    acoustic(Pt_array[i], Pt_array[i-1], Pt_array[i+1]);

    // add source injection
    Pt_array[i+1][at(0, g_ny/2, g_nx/2, g_ny, g_nx)]      += source[i-1];
    Pt_fpga_array[i+1][at(0, g_ny/2, g_nx/2, g_ny, g_nx)] += source[i-1];

    // record receivers 
    for (size_t irecv = 0; irecv < g_nx * g_ny; irecv++) {
      receivers[irecv][i-1] = Pt_array[i+1][irecv];
      receivers_fpga[irecv][i-1] = Pt_fpga_array[i+1][irecv];
    }

    cout << (passVerify(Pt_array[i+1], Pt_fpga_array[i+1], size) ? 
             "OK. Test passed" : "!!!Test Failed!!!")
         << endl << endl;
  }
}

void reverseMigrationAndCheck(max_device_handle_t *device, 
                 float **Pt_array, float **Pt_fpga_array, 
                 float **receivers, float **receivers_fpga,
                 float *image, float *image_fpga,
                 size_t size, int n_iter)
{
  float Pt[g_size]        = {0};
  float PtM1[g_size]      = {0};
  float PtP1[g_size]      = {0};
  float Pt_fpga[g_size]   = {0};
  float PtM1_fpga[g_size] = {0};
  float PtP1_fpga[g_size] = {0};

  size_t n_bytes = size * sizeof(float);
  for (int i = n_iter; i >= 1; i--) {
    cout << i - 1 << "st run: ";

    // run with FPGA
    max_run(device,
        max_input("Pt_stream", Pt_fpga, n_bytes),
        max_input("PtM1_stream", PtP1_fpga, n_bytes),
        max_output("PtP1_stream", PtM1_fpga, n_bytes),
        max_runfor("PassThroughKernel", size),
        max_end());

    // run with CPU
    acoustic(Pt, PtP1, PtM1);

    // add receivers injection
    // where you collect the receivers's wave then
    // where you add the injection
    for (size_t irecv = 0; irecv < g_nx * g_ny; irecv++) {
      PtM1[irecv]      += receivers[irecv][i];
      PtM1_fpga[irecv] += receivers_fpga[irecv][i];
    }

    // generate the image
    for (int j = 0; j < g_size; j++) {
      image[j] += Pt_array[i-1][j] * PtM1[j];
      image_fpga[j] += Pt_fpga_array[i-1][j] * PtM1_fpga[j];
    }

    cout << (passVerify(Pt_array[i-1], Pt_fpga_array[i-1], size) ? 
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
