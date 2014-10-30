#pragma once

#include <funcs.h>
#include <pointers.h>
#include <directions.h>

typedef unsigned char Player;

// Do not change pWHITE and pBLACK values, since they are used in
// messages as "Player #1" or "Player #2"
const Player pNO	  = 0;
const Player pWHITE = 1;
const Player pBLACK = 2;
const Player pBOTH  = 3;

class Field
{
   public:
            Field( int sz_x = 0, int sz_y = 0 );
       void Init( int sz_x, int sz_y );
       void Init( const POINT& sz ) { Init(sz.x,sz.y); }
       void Init( const Field& fld );
       void InitNull();

    Player& operator() ( int x, int y );
     Player operator() ( int x, int y ) const;
    Player& operator() ( const POINT& p );
     Player operator() ( const POINT& p ) const;
       bool operator == ( const Field& fld ) const;

      POINT Size() const { return sz; }
       bool DoesContain( const POINT& p ) const { return p.x>=0 && p.y>=0 && p.x<sz.x && p.y<sz.y; }

   private:
       P<Player> field;
           POINT sz;

            Field( const Field& ) {}
     Field& operator = ( const Field& ) { return *this; }
};
