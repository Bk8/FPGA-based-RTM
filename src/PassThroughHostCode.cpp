#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "foo.h"
#include <MaxCompilerRT.h>

#define DATA_SIZE 1024

int main(int argc, char* argv[])
{
  char *device_name = (argc==2 ? argv[1] : NULL);
  max_maxfile_t* maxfile;
  max_device_handle_t* device;
  uint32_t *data_in, *data_out;
  int status = 0;

  printf("Opening and configuring FPGA.\n");

  maxfile = max_maxfile_init_PassThrough();
  device = max_open_device(maxfile, device_name);
  max_set_terminate_on_error(device);

  printf("Generating data to send to FPGA.\n");
  data_in = (uint32_t *)malloc(DATA_SIZE * sizeof(uint32_t));
  data_out =(uint32_t *)malloc(DATA_SIZE * sizeof(uint32_t));
  /*data_in = new uint32_t[DATA_SIZE];*/
  /*data_out = new uint32_t[DATA_SIZE];*/

  if(!data_in || !data_out) {
    fprintf(stderr, "Failed to allocate memory for data I/O.\n");
    return 1;
  }
  for(int i = 0; i < DATA_SIZE; i++) {
    data_in[i] = i + 1;
    data_out[i] = 0;
  }

  printf("Streaming data to/from FPGA.\n");
  max_run(device,
      max_input("x", data_in, DATA_SIZE * sizeof(uint32_t)),
      max_output("y", data_out, DATA_SIZE * sizeof(uint32_t)),
      max_runfor("PassThroughKernel", DATA_SIZE),
      max_end());

  printf("Checking data read from FPGA.\n");
  for(int i = 0; i < DATA_SIZE; i++)
    if(data_out[i] != i+1) {
      fprintf(stderr, "Output data @ %d = %d (expected %d)\n",
        i, data_out[i], i+1);
      status = 1;
    }

  if (status)
    printf("Test failed.\n");
  else
    printf("Test passed OK!\n");

  printf("Shutting down\n");
  max_close_device(device);
  max_destroy(maxfile);

  hi();
  hi();
  return status;

}
