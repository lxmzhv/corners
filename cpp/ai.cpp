#include <ai.h>

//
// class AI
//

void AI::Init( const Field& fld )
{
  size = fld.Size();
  field.Init( fld );
  field2.Init( fld );

  GetTargetArea( fld, dx, dy, pWHITE, target1[0], target1[1] );
  GetTargetArea( fld, dx, dy, pBLACK, target2[0], target2[1] );

  if( dx > 0 && dy > 0 )
    team = new Point2i[dx*dy];

  level_data = new LevelData[deep+1];
  for( int i = 0; i <= deep; ++i )
    level_data[i].Init( size.x, size.y );
}

int AI::Distance( Player active, const Point2i& pnt )
{
  return (options & aioINDIVIDUAL_PRIORITY) ?
         PointDistance( pnt, active ) : SumDistance( active );
}

int AI::EvaluateManualTurn( const Point2i& from, const Point2i& to )
{
  int diff = NO_DIFF;
  int cur_dist = Distance( player, from );

  if( !EvaluateTurn( from, to, player, deep, cur_dist, foALLOW_NEGATIVE_TURNS, &diff ) )
    return NO_DIFF;

  return diff;
}

bool AI::IsOpponentEnclosedByTurn( const Point2i& to, Player active )
{
  static POINT p, p1, p2;
  static Player opponent;
  static int i;

  opponent = NextPlayer( active );
  GetInitialArea( field, dx, dy, opponent, p1, p2 );

  for( i = 0; i < CrossSize; ++i )
  {
    p = to;
    p += Cross[i];
    if( IsPntInside(p, p1, p2) && field(p) == opponent && IsChipEnclosed(field, p) )
      return true;
  }

  return false;
}

bool AI::EvaluateTurn( const Point2i& from, const Point2i& to, Player active,
                       int deep_rest, int cur_dist, FindOption opt, int* p_diff )
{
  int  new_dist, d, d2;
  bool allowed;

  if( from == to )
    return false;

  field( to ) = field( from );
  field( from ) = pNO;

  new_dist = Distance( active, to );
  d = cur_dist - new_dist;

  allowed = (d >= 0 || opt & foALLOW_NEGATIVE_TURNS);

  if( IsOpponentEnclosedByTurn( to, active ) )
  {
    d += dx*dy*(size.x+size.y); // somewhat relatively big
    allowed = true;
  }
  else if( allowed && deep_rest > 0 )
  {
    if( new_dist == 0 ) // Player has finished!
    {
      /* Increase dist difference so that the shortest
       * finishing turn is the most important */
      d += deep_rest;
    }
    else if( options & aioFRIENDLY )
    {
      if( deep_rest > 1 && FindTurn( active, deep_rest-2, opt, &d2 ) )
        d += d2;
    }
    else
    {
      if( FindTurn( NextPlayer(active), deep_rest-1, opt, &d2 ) )
        d -= d2;
      else if( deep_rest > 1 && FindTurn( active, deep_rest-2, opt, &d2 ) )
        d += d2;
    }
  }

  field( from ) = field( to );
  field( to ) = pNO;

  InitArg( p_diff, d );

  return allowed;
}

bool AI::FindTurnFromPoint( const Point2i& from, int cur_dist, Player active, int deep_rest, FindOption opt,
                            int& diff, Point2i* p_to ) // output
{
  bool        found = false;
  LevelData  &ldata = level_data[deep_rest];
  Field      &turns = ldata.GetField();
  Point2i    *points = ldata.GetPoints();
  Array<int> &to_idx = ldata.to_idx;

  int num = FindPossibleTurns( field, from, ldata.GetField(), points );

  if( options & aioINDIVIDUAL_PRIORITY )
    cur_dist = PointDistance( from, active );

  Point2i *p = points;
  int d;
  to_idx.Clear();
  for( int i = 0; i < num; ++i, ++p )
  {
    if( !EvaluateTurn( from, *p, active, deep_rest, cur_dist, opt, &d ) )
      continue;

    if( !found || d > diff )
    {
      diff = d;
      found = true;
      to_idx.Clear();
      to_idx.Append( 1, &i );
    }
    else if( d == diff )
      to_idx.Append( 1, &i );
  }

  int cnt = to_idx.Size();
  if( p_to != NULL && cnt > 0 )
  {
    int idx = (cnt == 1 ? 0 : rand()%cnt);
    InitArg( p_to, points[to_idx[idx]] );
  }

  return found;
}

bool AI::IsRiched( const Point2i& p, Player active )
{
  POINT* trg = TargetArea( active );
  return IsPntInside( p, trg[0], trg[1] );
}

bool AI::FindTurnLow( Player active, int deep_rest, FindOption opt, int& diff, Point2i* from, Point2i* to )
{
  bool found = false;
  Point2i pnt, p;
  int d, s, sum, dist = 0;
  bool riched = (opt&foRICHED_ONLY) != 0;
  bool not_riched = (opt&foNOT_RICHED_ONLY) != 0;

  dist = (options & aioINDIVIDUAL_PRIORITY) ? 0 : SumDistance( active );

  sum = (active == pWHITE ? 0 : size.x + size.y);
  for( p.y = 0; p.y < size.y; p.y++ )
    for( p.x = 0; p.x < size.x; p.x++ )
      if( field(p) == active )
      {
        if( riched && !IsRiched(p,active) )
          continue;

        if( not_riched && IsRiched(p,active) )
          continue;

        if( !FindTurnFromPoint( p, dist, active, deep_rest, opt, d, &pnt ) )
          continue;

        s = p.x + p.y;
        if( !found || d > diff ||
            d == diff && ( (active == pWHITE ? s < sum : s > sum) ||
                            s == sum && CheckProb(0.5) ) )
        {
          diff = d;
          sum = s;
          InitArg( from, p );
          InitArg( to, pnt );
          found = true;
        }
      }

  return found;
}

bool AI::FindTurn( Player active, int deep_rest, FindOption opt, int* p_diff, Point2i* from, Point2i* to )
{
  int diff;

  if( options & aioINDIVIDUAL_PRIORITY )
  {
    // Weak algorithm, finds the best move for single chip only.
    
    Point2i from2, to2;
    int diff2;

    bool found1 = FindTurnLow( active, deep_rest, FindOption(opt|foNOT_RICHED_ONLY), diff, from, to );
    bool found2 = FindTurnLow( active, deep_rest, FindOption(opt|foRICHED_ONLY), diff2, &from2, &to2 );

    if( !found1 && !found2 )
      return false;

    if( found1 && !found2 ||
        found1 && found2 && diff >= diff2 )
    {
      InitArg( p_diff, diff );
      return true;
    }

    InitArg( p_diff, diff2 );
    InitArg( from, from2 );
    InitArg( to, to2 );
  }
  else
  {
    // Strong algorithm, finds the best move basing on improvement of all chip positions

    if( !FindTurnLow( active, deep_rest, opt, diff, from, to ) )
      return false;

    InitArg( p_diff, diff );
  }

  return true;
}

bool AI::FindBetterTurn( const Field& fld, Point2i& from, Point2i& to )
{
  if( (options&aioEXPERIENCE) == 0 )
    return false;

  const FieldStateExp* fld_exp = exp.Find( fld );
  if( !fld_exp )
    return false;

  const TurnExp* turn_exp = fld_exp->Find( from, to );
  if( !turn_exp )
    return false;

  const TurnExp* the_best_turn = fld_exp->GetTheBestTurn( 3 );
  if( !the_best_turn || turn_exp == the_best_turn )
    return false;

  if( turn_exp->GetStat().GetAverage() > the_best_turn->GetStat().GetAverage() - NULL_DOUBLE )
    return false;

  from = the_best_turn->GetFrom();
  to = the_best_turn->GetTo();
  return true;
}

bool AI::Turn( const Field& fld, Point2i& from, Point2i& to, int* diff )
{
  field.Init( fld );

  from.x = from.y = to.x = to.y = -1;

  bool found = FindTurn( player, deep, foALLOW_NEGATIVE_TURNS, diff, &from, &to );

  /* Perhaps we can allow negative turns initially
  bool found = FindTurn( player, deep, foVOID, diff, &from, &to );
  if( !found ) // no good turns - go somewhere
    found = FindTurn( player, 0, foALLOW_NEGATIVE_TURNS, diff, &from, &to );*/

  if( found )
    FindBetterTurn( fld, from, to );

  return found;
}

void AI::GameResult( const Array<GameTurn>& turns, int turn_gap )
{
  if( (options&aioEXPERIENCE) == 0 )
    return;

  Point2i p1, p2, p3, p4;

  InitField( field, dx, dy );
  InitField( field2, dx, dy );

  Player active = pWHITE;
  for( int i = 0; i < turns.Size(); ++i )
  {
    p1 = turns[i].from;
    p2 = turns[i].to;

    if( p1 != p2 )
    {
      InvertPoint( field, p1, p3 );
      InvertPoint( field, p2, p4 );

      if( active == player )
        exp.Add( field, p1, p2, turn_gap );
      else
        exp.Add( field2, p3, p4, -turn_gap );

      field( p2 ) = field( p1 );
      field( p1 ) = pNO;

      field2( p4 ) = field2( p3 );
      field2( p3 ) = pNO;
    }

    active = NextPlayer( active );
  }

  stat.Add( (double)turn_gap );
}

void AI::FindTarget( const Point2i& pnt, Player active, bool allow_busy, Point2i& trg_pnt )
{
  static Point2i p, corner;
  POINT* trg;

  trg = TargetArea( active );
  corner = (active == pWHITE ? trg[1] : trg[0]);

  trg_pnt = pnt;

  if( pnt == corner )
    return;

  int sum = (active == pWHITE ? 0 : size.x + size.y);
  int sqr_len = size.x + size.y;
  int s_lim = sum, s;

  if( IsPntInside( pnt, trg[0], trg[1] ) )
    s_lim = pnt.x + pnt.y;

  for( p.x = trg[0].x; p.x <= trg[1].x; ++p.x )
    for( p.y = trg[0].y; p.y <= trg[1].y; ++p.y )
      if( allow_busy ? field(p) != active : field(p) == pNO )
      {
        s = p.x + p.y;
        if( active == pWHITE ? (s > s_lim && (s > sum || s == sum && SquareLength_2d(pnt-p) < sqr_len)) :
            (s < s_lim && (s < sum || s == sum && SquareLength_2d(pnt-p) < sqr_len)) )
        {
          trg_pnt = p;
          sqr_len = SquareLength_2d( pnt - trg_pnt );
          sum = s;
        }
      }
}

int AI::PointDistance( const Point2i& pnt, Player active )
{
  static Point2i trg_pnt;

  FindTarget( pnt, active, false, trg_pnt );
  if( pnt == trg_pnt )
    FindTarget( pnt, active, true, trg_pnt );

  return SquareLength_2d( pnt - trg_pnt );
}

void AI::FillTeam( Player active )
{
  static POINT p;
  int i = 0;

  for( p.x = 0; p.x < size.x; ++p.x )
    for( p.y = 0; p.y < size.y; ++p.y )
      if( field(p) == active )
      {
        team[i] = p;
        ++i;
      }
}

int FindNearestPoint( const POINT& p, Point2i* elems, int num, int& dist )
{
  static SPoint2D<int> vec;
  int d, idx = -1;

  dist = 0;

  for( int i = 0; i < num; ++i )
  {
    // Do not use overloaded operators here to speed up calculations
    vec.x = elems[i].x - p.x;
    vec.y = elems[i].y - p.y;

    d = SquareLength_2d( vec );
    if( idx < 0 || d < dist )
    {
      idx = i;
      dist = d;
      if( d == 0 )
        break;
    }
  }

  return idx;
}

int AI::SumDistance( Player active )
{
  static POINT *trg;
  static POINT p;
  int num = dx*dy, s_max = dx+dy, diff;

  trg = TargetArea( active );
  FillTeam( active );

  int first = 0, nearest;
  int dist = 0, d;

  if( active == pWHITE )
  {
    for( int s = 0; s <= s_max; ++s )
    {
      p.x = trg[1].x;
      p.y = trg[1].y-s;

      diff = p.y - trg[0].y;
      if( diff < 0 )
      {
        p.y -= diff;
        p.x += diff;
      }

      for( ; p.y <= trg[1].y && p.x >= trg[0].x; --p.x, ++p.y )
      {
        nearest = first + FindNearestPoint( p, team + first, num-first, d );
        dist += d;
        SwapPoints( team[first], team[nearest] );
        ++first;
      }
    }
  }
  else
  {
    for( int s = 0; s <= s_max; ++s )
    {
      p.x = trg[0].x;
      p.y = trg[0].y+s;

      diff = p.y - trg[1].y;
      if( diff > 0 )
      {
        p.y -= diff;
        p.x += diff;
      }

      for( ; p.y >= trg[0].y && p.x <= trg[1].x; ++p.x, --p.y )
      {
        nearest = first + FindNearestPoint( p, team + first, num-first, d );
        dist += d;
        SwapPoints( team[first], team[nearest] );
        ++first;
      }
    }
  }

  return dist;
}

//
// class LevelData
//
void LevelData::Init( int sz_x, int sz_y )
{
  if( sz_x > 0 && sz_y > 0 )
  {
    field.Init( sz_x, sz_y );
    points = new Point2i[sz_x*sz_y];
  }
}
