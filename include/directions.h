#pragma once

#include <math.h>
#include <points.h>

enum Direction
{
   dNO    = 0,
   dUP    = 1,
   dDOWN  = 2,
   dLEFT  = 3,
   dRIGHT = 4,

   dFIRST = 1,
   dLAST  = 4
};

typedef Point2D<int> Point2i;

Point2i Shift( Point2i p, Direction dir, int step );

Direction Revers( Direction dir );

