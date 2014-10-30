#pragma once

#include <gtk/gtk.h>
#include <libintl.h> /* it's needed for translation of strings via gettext */

#include <game.h>

/* Macros for use with gettext */
#define _( x ) gettext( x )

enum GameColors
{
  COLORS_DARK  = 0,
  COLORS_LIGHT = 1
};

enum ChipShape
{
  SHAPE_SQUARE = 0,
  SHAPE_CIRCLE = 1,

  SHAPE_DEFAULT = SHAPE_CIRCLE,
};

enum PlayerType
{
  pHUMAN = 0x0,
  pAI    = 0x1,
};

enum GameModes
{
  gmHUMAN_AI    = pHUMAN | (pAI    << 1),
  gmAI_HUMAN    = pAI    | (pHUMAN << 1),
  gmHUMAN_HUMAN = pHUMAN | (pHUMAN << 1),
  gmAI_AI       = pAI    | (pAI    << 1)
};

class GameOptions
{
  public:
    POINT        field_size;
    POINT        chip_area_size;
    AIDifficulty difficulty;
    AIOption     ai_opt;
    int          ai_deep;
    bool         friendly_ai;

    GameOptions()
    {
      field_size.x = field_size.y = 8;
      chip_area_size.x = 4;
      chip_area_size.y = 3;
      difficulty = aidNORMAL;
      ai_opt = aioDEFAULT;
      ai_deep = 2;
    }
};

void NewGame();
void About( GtkAction *action, GtkWidget* window );
void Rules( GtkAction *action, GtkWidget* window );
void Preferences( GtkAction *action, GtkWidget* window );

extern P<Game> game;
extern GameOptions Options;
