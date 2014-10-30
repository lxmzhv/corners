#include <directions.h>

// Вспомогательные функции
Point2i Shift( Point2i p, Direction dir, int step )
{
   Point2i res = p;
   switch( dir )
   {
      case dUP:     res.y -= step;   break;
      case dDOWN:   res.y += step;   break;
      case dLEFT:   res.x -= step;   break;
      case dRIGHT:  res.x += step;   break;
   }
   return res;
}

Direction Revers( Direction dir )
{
   switch( dir )
   {
      case dUP:    return dDOWN;
      case dDOWN:  return dUP;
      case dLEFT:  return dRIGHT;
      case dRIGHT: return dLEFT;
   }

   return dNO;
}

