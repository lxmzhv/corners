#pragma once

#include <funcs.h>

template< class T >
struct SimplePoint
{
   typedef T ElemType;
   typedef SimplePoint<T> SPoint;
   T x, y, z;

          void Init( T a = 0, T b = 0, T c = 0 ) { x = a; y = b; z = c; }

       template<class V>
       SPoint& operator = ( const V& v ) { x=v.x, y=v.y, z=v.z; return *this; }

        SPoint operator + ( const SPoint& p ) const { return Create( x+p.x, y+p.y, z+p.z ); }
        SPoint operator - ( const SPoint& b ) const { return Create( x-b.x, y-b.y, z-b.z ); }
        SPoint operator - () const { return Create( -x, -y, -z ); }
       SPoint& operator += ( const SPoint& p ) { x+=p.x, y+=p.y, z+=p.z; return *this; }
       SPoint& operator -= ( const SPoint& p ) { x-=p.x, y-=p.y, z-=p.z; return *this; }

          bool operator == ( const SPoint& p ) const { return x == p.x && y == p.y && z == p.z; }
          bool operator != ( const SPoint& p ) const { return x != p.x || y != p.y || z != p.z; }

        SPoint operator / ( const long val ) const { return Create( x/val, y/val, z/val ); }
        SPoint operator * ( const long val ) const { return Create( x*val, y*val, z*val ); }
             T operator * ( const SPoint& p ) const { return x*p.x + y*p.y + z*p.z; } // Скалярное произведение

            T& operator[] ( int i ) { return i==0 ? x : i==1 ? y : z; }
      const T& operator[] ( int i ) const { return i==0 ? x : i==1 ? y : z; }

   private:
      inline SPoint Create( T x, T y, T z ) const
      {
         SPoint pnt;
         pnt.x = x;
         pnt.y = y;
         pnt.z = z;
         return pnt;
      }
};

template< class T >
class Point : public SimplePoint<T>
{
  public:
    template<class P>
    Point( const P& p ) { Init( p.x, p.y, p.z ); }
    Point( T a = 0, T b = 0, T c = 0 ) { Init( a, b, c ); }
};

typedef Point<double> DPoint;
typedef Point<float> FPoint;

// Плоскость
template<class T>
class Plane
{
  public:
    T a, b, c, d; // a*x + b*y + c*z + d = 0

    Plane(): a(0), b(0), c(0) {}
    // Инициализация по коэффициентам уравнения
    Plane( T& a1, T& b1, T& c1, T& d1 ): 
      a(a1), b(b1), c(c1), d(d1) {}

    // Инициализация по трём точкам
    Plane( Point<T>& p1, Point<T>& p2, Point<T>& p3 ) { Init( p1, p2, p3 ); }
    void Init( Point<T>& p1, Point<T>& p2, Point<T>& p3 )
    {
      Point<T> v1 = p1 - p2, v2 = p1 - p3;
      a = v1.y*v2.z - v1.z*v2.y;   
      b = v1.z*v2.x - v1.x*v2.z;   
      c = v1.x*v2.y - v1.y*v2.x;
      d = -(a*p1.x + b*p1.y + c*p1.z);
    }
    char Meen( Point<T>& p )
    { 
      T meen = a*p.x+b*p.y+c*p.z+d;
      return meen == 0 ? 0 : meen > 0 ? 1 : -1;
    }
};

class DLL_OBJ DPlane: public Plane<double>
{
   public:
              // Инициализация по коэффициентам уравнения
              DPlane( double a1=0, double b1=0, double c1=0, double d1=0 ): Plane<double>(a1,b1,c1,d1) {}
              DPlane( DPoint& p1, DPoint& p2, DPoint& p3 );

              // Пересечение с прямой, результат в res, если пересечения нет, вернёт false
         bool Cross( DPoint& p1, DPoint& p2, DPoint& res );
              // Когда точно известно, что пересечение есть
       DPoint Cross( DPoint& p1, DPoint& p2 );
};

template< class T >
struct SPoint2D
{
   typedef T ElemType;
   typedef SPoint2D<T> MyType;
           T x, y;

         void Init( T a = 0, T b = 0 ) { x = a; y = b; }

      template<class V>
      MyType& operator = ( const V& v ) { x=v.x; y=v.y; return *this; }

       MyType operator - ( const MyType& b ) const { return Create( x - b.x, y - b.y ); }
      MyType& operator -= ( const MyType& p ) { x-=p.x, y-=p.y; return *this; }
         bool operator == ( const MyType& p ) const { return x == p.x && y == p.y; }
         bool operator != ( const MyType& p ) const { return x != p.x || y != p.y; }
       MyType operator + ( const MyType& p ) const { return Create( x+p.x, y+p.y ); }
      MyType& operator += ( const MyType& p ) { x+=p.x, y+=p.y; return *this; }
       MyType operator / ( long val ) const { return Create( x/val, y/val ); }
       MyType operator / ( double val ) const { return Create( T(x/val), T(y/val) ); }
      MyType& operator /= ( long val ) const { return x/=val; y/=val; return *this; }
      MyType& operator *= ( long val ) { x*=val; y*=val; return *this; }
       MyType operator * ( long val ) const { return Create( x*val, y*val ); }
       MyType operator *= ( double val ) { x*=val; y*=val; return *this; }
       MyType operator * ( double val ) const { return Create( x*val, y*val ); }
           T& operator[] ( int i ) { return i==0 ? x : y; }

   private:
      inline MyType Create( T a, T b ) const
      {
         MyType pnt;
         pnt.x = a;
         pnt.y = b;
         return pnt;
      }
};

typedef SPoint2D<int> POINT;

template< class T >
class Point2D : public SPoint2D<T>
{
  public:
      template <class P>
      Point2D( const P& p ) { Init( p.x, p.y ); }
      Point2D( T a = 0, T b = 0 ) { Init( a, b ); }
};

class DPoint2D: public Point2D<double>
{
   public:
        template<class T2>
        DPoint2D( const T2& p ): Point2D<double>(p) {}
        DPoint2D( double a = 0, double b = 0 ): Point2D<double>(a,b) {}

   bool operator == ( const DPoint2D& p ) const { return IsNullNum(x - p.x) && IsNullNum(y - p.y); }
   bool operator != ( const DPoint2D& p ) const { return !IsNullNum(x - p.x) || !IsNullNum(y - p.y); }
};

// Прямая
template< class T >
class Line
{
   public:
      T a, b, c; // a*x + b*y + c = 0;

         Line( T _a=0, T _b=0, T _c=0 ): a(_a), b(_b), c(_c) {}
       T Value( Point2D<T>& p ) const { return a*p.x + b*p.y + c; }
       T GetX( T y ) const { return -(c + b*y)/a; }
       T GetY( T x ) const { return -(c + a*x)/b; }
};

template< class T >
inline double Length_2d( const T& p ) { return sqrt( double(sqr(p.x)+sqr(p.y)) ); }

template< class T >
inline T SquareLength_2d( const SPoint2D<T>& p ) { return abs(p.x) + abs(p.y); }

template< class T >
inline float Length( const T& p ) { return sqrt( float(sqr(p.x)+sqr(p.y)+sqr(p.z)) ); }

template< class T, class V1, class V2 >
void scvec( const T a, const V1& v, V2* res )
{
   res->x = v.x*a;
   res->y = v.y*a;
   res->z = v.z*a;
}

template< class T >
void ValidatePoint( T& pnt, const T& min, const T& max )
{
   Validate( pnt.x, min.x, max.x );
   Validate( pnt.y, min.y, max.y );
   Validate( pnt.z, min.z, max.z );
}
