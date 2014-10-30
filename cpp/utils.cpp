#include <utils.h>

/* A crossing:
     o
     o
   oo oo
     o
     0    */
POINT Cross[CrossSize] = { {-2,0}, {-1,0}, {1,0}, {2,0},
                           {0,-2}, {0,-1}, {0,1}, {0,2} };

Player NextPlayer( Player player )
{
   return player==pWHITE ? pBLACK :
          player==pBLACK ? pWHITE : pNO;
}

void GetTargetArea( const Field& field, int dx, int dy, Player player, POINT& p1, POINT& p2 )
{
  if( player == pWHITE )
  {
    POINT sz = field.Size();
    p1.Init( sz.x-dx, sz.y-dy );
    p2.Init( sz.x-1,  sz.y-1  );
  }
  else
  {
    p1.Init( 0, 0 );
    p2.Init( dx-1, dy-1 );
  }
}

void GetInitialArea( const Field& field, int dx, int dy, Player player, POINT& p1, POINT& p2 )
{
  GetTargetArea( field, dx, dy, NextPlayer(player), p1, p2 );
}

void InitField( Field& field, int dx, int dy )
{
   int size_x = field.Size().x, size_y = field.Size().y;
   Point2i p, p1, p2;

   for( p.x = 0; p.x < size_x; ++p.x )
      for( p.y = 0; p.y < size_y; ++p.y )
         field( p ) = pNO;

   GetTargetArea( field, dx, dy, pBLACK, p1, p2 );
   for( p.x = p1.x; p.x <= p2.x; ++p.x )
      for( p.y = p1.y; p.y <= p2.y; ++p.y )
         field( p ) = pWHITE;

   GetTargetArea( field, dx, dy, pWHITE, p1, p2 );
   for( p.x = p1.x; p.x <= p2.x; ++p.x )
      for( p.y = p1.y; p.y <= p2.y; ++p.y )
         field( p ) = pBLACK;
}

void InvertPoint( const Field& fld, const Point2i& src, Point2i& dst )
{
   dst.x = fld.Size().x - src.x - 1;
   dst.y = fld.Size().y - src.y - 1;
}

static void FindPossibleTurnsLow( const Field& field, const Point2i& from,
                                 Field& turns, Point2i* points, int& count,
                                 Direction back )
{
   Point2i p1, p2;

   for( Direction d = dFIRST; d <= dLAST; d = Direction(d+1) )
      if( back == dNO || d != back ) // не возвращаемся назад
      {
         p1 = Shift( from, d, 1 );
         p2 = Shift( from, d, 2 );
         if( field.DoesContain(p2) &&
             field(p1) != pNO && field(p2) == pNO &&
             turns(p2) == pNO )
         {
            turns( p2 ) = pWHITE;
            if( points )
               points[count] = p2;
            ++count;
            FindPossibleTurnsLow( field, p2, turns, points, count, Revers(d) );
         }
      }
}

int FindPossibleTurns( const Field& field, const Point2i& from,
                       Field& turns, Point2i* points )
{
   static Point2i p;
   int count = 0;

   COMPILE_CHECK( pNO==0, pNO_must_be_equal_to_null );
   turns.InitNull();

   for( Direction d = dFIRST; d <= dLAST; d = Direction(d+1) )
   {
      p = Shift( from, d, 1 );
      if( field.DoesContain(p) && field(p) == pNO )
      {
         turns(p) = pWHITE;
         if( points )
            points[count] = p;
         ++count;
      }
   }

   FindPossibleTurnsLow( field, from, turns, points, count, dNO );

   return count;
}

void PrintField( const Field& field )
{
  POINT p, sz = field.Size();
  Player player;

  for( p.y = 0; p.y < sz.y; ++p.y )
  {
    for( p.x = 0; p.x < sz.x; ++p.x )
    {
      player = field( p );
      printf( "%c", player == pWHITE ? '0' : player == pBLACK ? '1' : '.' );
    }
    printf( "\n" );
  }
}

bool IsChipEnclosed( const Field& field, const POINT& pnt )
{
  static POINT p;
  static Player opponent;

  opponent = NextPlayer( field( pnt ) );
  for( int i = 0; i < CrossSize; ++i )
  {
    p = pnt;
    p += Cross[i];
    if( field.DoesContain(p) && field(p) != opponent )
      return false;
  }

  printf( "Enclosed: (%d,%d)\n", pnt.x, pnt.y );
  /*PrintField( field );
  printf( "\n" );*/
  return true;
}
