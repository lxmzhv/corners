// Действия над числами
#include <numbers.h>
#include <string.h>

DLL_OBJ double log2( double a )
{
   static const double LOG2 = log(2.0);
   return log(a)/LOG2;
}

DLL_OBJ int log2( int a )
{
   int d = 0;
   for( ; a > 1; ++d )
      a >>= 1;
   return d;
}

