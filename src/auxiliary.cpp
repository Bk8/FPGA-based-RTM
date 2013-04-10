#include "auxiliary.h"

size_t at(size_t height, size_t row,
        size_t col, size_t nRow, size_t nCol)
{
    return (height * nRow * nCol) + (row * nCol) + col;
}
