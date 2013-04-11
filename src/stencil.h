#ifndef STENCIL_H

#define STENCIL_H

#include "auxiliary.h"

const float w3 = 1.0/90.0;
const float w2 = -3.0/20;
const float w1 = 1.5;
const float w0 = -49.0/18;

template <typename T>
T stencil(
    const T *in,
    size_t iz,
    size_t iy,
    size_t ix,
    size_t ny,
    size_t nx)
{
  return
      (in[at(iz, iy, ix - 3, ny, nx)] + in[at(iz, iy, ix + 3, ny, nx)]) * w3 +
      (in[at(iz, iy, ix - 2, ny, nx)] + in[at(iz, iy, ix + 2, ny, nx)]) * w2 +
      (in[at(iz, iy, ix - 1, ny, nx)] + in[at(iz, iy, ix + 1, ny, nx)]) * w1 +

      (in[at(iz, iy - 3, ix, ny, nx)] + in[at(iz, iy + 3, ix, ny, nx)]) * w3 +
      (in[at(iz, iy - 2, ix, ny, nx)] + in[at(iz, iy + 2, ix, ny, nx)]) * w2 +
      (in[at(iz, iy - 1, ix, ny, nx)] + in[at(iz, iy + 1, ix, ny, nx)]) * w1 +

      (in[at(iz - 3, iy, ix, ny, nx)] + in[at(iz + 3, iy, ix, ny, nx)]) * w3 +
      (in[at(iz - 2, iy, ix, ny, nx)] + in[at(iz + 2, iy, ix, ny, nx)]) * w2 +
      (in[at(iz - 1, iy, ix, ny, nx)] + in[at(iz + 1, iy, ix, ny, nx)]) * w1 +

      (in[at(iz, iy, ix, ny, nx)] * w0) * 3;

}
//template <typename T>
//T stencil(
  //const T *a,
  //size_t   height,
  //size_t   row,
  //size_t   col,
  //size_t   nRow,
  //size_t   nCol)
//{
    //return
        //a[at( height     , row     , col - 3 ,nRow , nCol)] * c1 +
        //a[at( height     , row     , col - 2 ,nRow , nCol)] * c2 +
        //a[at( height     , row     , col - 1 ,nRow , nCol)] * c3 +
        //a[at( height     , row     , col     ,nRow , nCol)] * c4 +
        //a[at( height     , row     , col + 1 ,nRow , nCol)] * c5 +
        //a[at( height     , row     , col + 2 ,nRow , nCol)] * c6 +
        //a[at( height     , row     , col + 3 ,nRow , nCol)] * c7 +

        //a[at( height     , row - 3 , col     ,nRow , nCol)] * c1 +
        //a[at( height     , row - 2 , col     ,nRow , nCol)] * c2 +
        //a[at( height     , row - 1 , col     ,nRow , nCol)] * c3 +
        //a[at( height     , row     , col     ,nRow , nCol)] * c4 +
        //a[at( height     , row + 1 , col     ,nRow , nCol)] * c5 +
        //a[at( height     , row + 2 , col     ,nRow , nCol)] * c6 +
        //a[at( height     , row + 3 , col     ,nRow , nCol)] * c7 +

        //a[at( height - 3 , row     , col     ,nRow , nCol)] * c1 +
        //a[at( height - 2 , row     , col     ,nRow , nCol)] * c2 +
        //a[at( height - 1 , row     , col     ,nRow , nCol)] * c3 +
        //a[at( height     , row     , col     ,nRow , nCol)] * c4 +
        //a[at( height + 1 , row     , col     ,nRow , nCol)] * c5 +
        //a[at( height + 2 , row     , col     ,nRow , nCol)] * c6 +
        //a[at( height + 3 , row     , col     ,nRow , nCol)] * c7 ;
//}

template <typename T>
void stencil_cpu(
  const T *in,
  T       *out,
  size_t   nHeight,
  size_t   nRow,
  size_t   nCol)
{
    for (size_t height = 0; height < nHeight; height++) {
        for (size_t row = 0; row < nRow; row++) {
            for (size_t col = 0; col < nCol; col++) {
                if ((height >= 3 && height < nHeight - 3) &&
                        (row >= 3 && row < nRow - 3) &&
                        (col >= 3 && col < nCol - 3))
                {
                    out[at(height,row,col, nRow, nCol)] =
                        stencil(in, height, row, col, nRow, nCol);
                } else {
                    out[at(height,row,col, nRow, nCol)] =
                      in[at(height, row, col, nRow, nCol)];
                }
            }
        }
    }
}

#endif /* end of include guard: STENCIL_H */
