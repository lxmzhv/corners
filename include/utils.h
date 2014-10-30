#pragma once
#include <field.h>
#include <stdio.h>

Player NextPlayer( Player player );
void GetTargetArea( const Field& field, int dx, int dy, Player player, POINT& p1, POINT& p2 );
void GetInitialArea( const Field& field, int dx, int dy, Player player, POINT& p1, POINT& p2 );
void InitField( Field& field, int dx, int dy );
void InvertPoint( const Field& fld, const Point2i& src, Point2i& dst );
int FindPossibleTurns( const Field& field, const Point2i& from, Field& turns, Point2i* points = NULL );
bool IsChipEnclosed( const Field& field, const POINT& p );

inline bool IsPntInside( const POINT& p, const POINT& p_min, const POINT& p_max )
{
   return p_min.x <= p.x && p.x <= p_max.x &&
          p_min.y <= p.y && p.y <= p_max.y;
}

inline void SwapPoints( Point2i& p1, Point2i& p2 )
{
   static Point2i p;

   if( &p1 == &p2 )
      return;

  p = p1;
  p1 = p2;
  p2 = p;
}

const int CrossSize = 8;
extern POINT Cross[CrossSize];
