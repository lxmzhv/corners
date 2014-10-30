#pragma once

#include <gtk/gtk.h>
#include <game.h>

enum
{
  doINVERT_X        = 1 << 0,
  doINVERT_Y        = 1 << 1,
  doSHOW_EXPERIENCE = 1 << 2,
  doEVALUATE_TURNS  = 1 << 3,
  doDEFAULT  = 0
};
typedef int DisplOption;

enum DrawChipOptions
{
  dcoNONE          = 0,
  dcoSELECTED      = 1 << 0,
  dcoPOSSIBLE_TURN = 1 << 1
};

class DisplCoor
{
  public:
        DisplCoor( POINT sz, DisplOption opt ): size(sz), invX(opt&doINVERT_X), invY(opt&doINVERT_Y) {}

    int X( double x ) const { return int(invX ? size.x-x : x); }
    int Y( double y ) const { return int(invY ? size.y-y : y); }
  POINT Size() const { return size; }

  private:
    POINT size;
    bool invX, invY;
};

// Don't change the order of fields, or update all structure initializations!
struct ColorTheme
{
  GdkColor field;
  GdkColor player1;
  GdkColor player2;
  GdkColor grid;
  GdkColor target_area;
  GdkColor selected;
  GdkColor arrow;
  GdkColor exp_arrow;
  GdkColor possible_turn;
};

void GetCellCoor( const Point2i& p, Point2i& cell_pnt );
void DrawField( GtkWidget* widget, GdkGC* gc, DisplOption opt, POINT selected,
                const Point2i* moving, const Field& possible_turns );

/* The order of themes is synchronous with the array ColorThemes */
enum ColorThemeIndex
{
  COLOR_THEME_CLASSIC = 0,
  COLOR_THEME_DARK    = 1,
  COLOR_THEME_LIGHT   = 2,

  COLOR_THEME_FIRST   = 0,
  COLOR_THEME_NUMBER  = 3,
  COLOR_THEME_DEFAULT = COLOR_THEME_CLASSIC
};

extern ColorThemeIndex ThemeIdx;
extern ChipShape Shape;
