#include <field.h>
#include <string.h>

//
// class Field
//

Field::Field( int sz_x, int sz_y )
{
   sz.x = sz.y = 0;
   Init( sz_x, sz_y );
}

void Field::Init( int sz_x, int sz_y )
{
   if( sz_x != sz.x || sz_y != sz.y )
   {
     sz.y = sz.x = 0;

     if( sz_x > 0 && sz_y > 0 )
     {
        field = new Player[sz_x*sz_y];
        if( field )
        {
           sz.x = sz_x;
           sz.y = sz_y;
        }
     }
   }
   InitNull();
}

void Field::Init( const Field& fld )
{
   Init( fld.sz.x, fld.sz.y );
   if( sz.x != 0 && sz.y != 0 )
      memcpy( field, fld.field, sz.x*sz.y*sizeof(Player) );
}

bool Field::operator == ( const Field& fld ) const
{
   if( sz.x != fld.sz.x || sz.y != fld.sz.y )
      return false;
   if( sz.x == 0 || sz.y == 0 )
      return true;
   return memcmp( field, fld.field, sz.x*sz.y*sizeof(Player) ) == 0;
}

void Field::InitNull()
{
   if( sz.x > 0 && sz.y > 0 )
      memset( field, 0, sz.x*sz.y*sizeof(Player) );
}

Player& Field::operator() ( int x, int y )
{ 
  return field[y*sz.x+x];
}

Player Field::operator() ( int x, int y ) const
{ 
  return field[y*sz.x+x];
}

Player& Field::operator() ( const POINT& p )
{ 
  return field[p.y*sz.x+p.x];
}

Player Field::operator() ( const POINT& p ) const
{ 
  return field[p.y*sz.x+p.x];
}

