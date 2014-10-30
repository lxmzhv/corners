#pragma once

#include <field.h>
#include <utils.h>
#include <experience.h>

enum FindOption
{
   foVOID                 = 0x00,
   foALLOW_NEGATIVE_TURNS = 0x01,
   foRICHED_ONLY          = 0x02,
   foNOT_RICHED_ONLY      = 0x04,
};

enum AIOption
{
   aioINDIVIDUAL_PRIORITY = 0x01, // the best turn is computed basing on the best move of single chip
   aioFRIENDLY            = 0x02, // do not evaluate opponent turns
   aioEXPERIENCE          = 0x04, // cumulate and use game experience
   aioDEFAULT             = 0x00
};

enum AIDifficulty
{
  aidNORMAL = 0,
  aidHARD   = 1
};

const int NO_DIFF = -1000;

class GameTurn
{
  public:
      GameTurn(): from(-1,-1), to(-1,-1), diff(NO_DIFF) {}

  Point2i from;
  Point2i to;
  int     diff;
};

class LevelData
{
   public:
   Array<int> to_idx;

             LevelData() {}
        void Init( int sz_x, int sz_y );
      Field& GetField() { return field; }
    Point2i* GetPoints() { return points; }

   private:
          Field field;
    VP<Point2i> points;
};

class AI
{
   public:
                AI( int sz_x, int sz_y, Player p, int deep_, AIOption opt = aioDEFAULT):
                   dx(sz_x), dy(sz_y), player(p), deep(Max(deep_,0)), options(opt) {}
           void Init( const Field& fld );
           bool Turn( const Field& fld, Point2i& from, Point2i& to, int* diff = NULL );
           void GameResult( const Array<GameTurn>& turns, int turn_gap );
            int EvaluateManualTurn( const Point2i& from, const Point2i& to );

  const GameStat& GetStat() const { return stat; }
const Experience& GetExp() const { return exp; }

   private:
         Player player;
          POINT size;
            int dx, dy;     // starting chip position (distance from corner)
            int deep;       // how many turns to evaluate (0 means evaluate only one turn)
       AIOption options;
          POINT target1[2]; // target area for the pWHITE player
          POINT target2[2]; // target area for the pBLACK player
          Field field, field2; // two internal copies of the game field
  VP<LevelData> level_data; // deep+1 buffers (to speed up turn evaluation)
    VP<Point2i> team;       // a buffer for chips
       GameStat stat;
     Experience exp;

           bool FindPath( Point2i from, Point2i to, Direction whence );
           bool TryTurn( Point2i pold, Point2i pnew );
           bool FindTurn( Player active, int deep, FindOption opt, int* diff = 0, Point2i* from = 0, Point2i* to = 0 );
           bool FindTurnLow( Player active, int deep, FindOption opt, int& diff, Point2i* from, Point2i* to );
           bool FindTurnFromPoint( const Point2i& from, int cur_dist, Player active, int deep, FindOption opt, int& diff, Point2i* to );
           bool EvaluateTurn( const Point2i& from, const Point2i& to, Player active, int deep, int cur_dist, FindOption opt, int* p_diff );
           bool FindBetterTurn( const Field& fld, Point2i& from, Point2i& to );

          bool IsRiched( const Point2i& p, Player active );
          bool IsOpponentEnclosedByTurn( const Point2i& to, Player active );

           int PointDistance( const Point2i& pnt, Player active );
           int SumDistance( Player active );
           int Distance( Player active, const Point2i& pnt );

          void FindTarget( const Point2i& pnt, Player active, bool allow_busy, Point2i& target );
          void FillTeam( Player active );

        inline POINT* TargetArea( Player p ) { return p == pWHITE ? target1 : target2; }
};
