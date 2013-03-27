/**
 * @file main.cpp
 * @brief The file compute a list of *Riker Wavelet* amplitude. For more 
 * information, see the README.mkd file.
 * @author Conghui He
 * @version 1.0
 * @date Wed 27 Mar 2013 07:29:18 PM CST
 */

#define _USE_MATH_DEFINES
#include <iostream>
#include <cassert>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <iomanip>

using namespace std;
void usage(const char *progname);
inline float riker_wavelet(float t);

int main(int argc, char *argv[])
{
  const float delta_t = 0.002;

  /* check the user's command line input */
  if (argc != 3) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  int max_iteration = atoi(argv[1]);
  assert(max_iteration > 0);
  const char *fname = argv[2];

  /* open the file in binary mode */
  ofstream ofs(fname, ios::binary);
  if (!ofs) {
    cerr << "can not open " << fname << "\n";
    exit(EXIT_FAILURE);
  }

  /* perform a iteration to generate the amplitude */
  float t = 0;
  for (int i = 0; i < max_iteration; i++, t += delta_t) {
    float ans = riker_wavelet(t);

    /* write to the file */
    ofs.write(reinterpret_cast<char *>(&ans), sizeof ans);
  }

  /* close the file */
  ofs.close();

  return 0;
}

void usage(const char *progname)
{
  cerr << "usage: " << progname << "   {50, 100, 150, ...}   " 
       << "output_filename" << "\n";
}

/* perform the core computation */
inline float riker_wavelet(float t)
{
  const float f = 15.0;
  float m = (M_PI * f * (t - 1/f)) * (M_PI * f * (t - 1/f));

  return (1 - 2 * m) * exp(-m);
}

