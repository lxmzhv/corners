#include <game.h>

//
// class Game
//
Game::Game( int size_x, int size_y, int sz_x, int sz_y, AIDifficulty difficulty, AIOption opt, int ai_deep ):
   field( size_x, size_y ), dx( sz_x ), dy( sz_y )
{
   AIOption ai_opt = AIOption(aioEXPERIENCE | opt);
   if( difficulty == aidNORMAL )
     ai_opt = AIOption(ai_opt | aioINDIVIDUAL_PRIORITY);

   player1 = new AI( sz_x, sz_y, pWHITE, ai_deep, ai_opt );
   player2 = new AI( sz_x, sz_y, pBLACK, ai_deep, ai_opt );

   Init();
   wins1 = wins2 = draws = 0;
}

bool Game::Init()
{
   int size_x = field.Size().x, size_y = field.Size().y;
   if( !(size_x > 1 && size_y > 1 && dx >= 1 && dy >= 1 && (dx <= size_x/2 || dy <= size_y/2)) )
      return false;

   InitField( field, dx, dy );

   winner = finished = pNO;
   winTurn = -1;
   winGap = NO_GAP;
   active = pWHITE;
   turn = 1;
   turns.Clear();

   player1->Init( field );
   player2->Init( field );

   return true;
}

Player Game::GetState( Point2i p ) const
{
  if( field.DoesContain( p ) )
     return field( p );
  return pNO;
}

// Проверка корректности хода
bool Game::CheckTurn( const Point2i& from, const Point2i& to )
{
   if( !field.DoesContain(from) || !field.DoesContain(to) )
      return false;

   if( field(from) != active || field(to) != pNO || from == to )
      return false;

   POINT sz = field.Size();
   Field fturns( sz.x, sz.y );

   FindPossibleTurns( field, from, fturns );

   return fturns(to) != pNO;
}

void Game::SwitchPlayer()
{
   active = NextPlayer( active );
   if( active == pWHITE )
      ++turn;
}

bool Game::SkipTurn()
{
   bool active_finished = IsFinished(active);

   if( !active_finished )
     return false;

   if( active_finished && IsFinished(NextPlayer(active)) )
     return true;

   GameTurn turn;
   turn.from.Init( -1, -1 );
   turn.to.Init( -1, -1 );
   turn.diff = NO_DIFF;
   turns.Append( 1, &turn );

   SwitchPlayer();

   return true;
}

bool Game::IsOver() const
{
  if( winner == pNO )
    return false;

  if( finished != pNO )
    return IsFinished(pWHITE) && IsFinished(pBLACK);

  return true; /* a player has been surrounded */
}

void Game::OnGameOver( int win_gap )
{
  winGap = win_gap;

  if( winGap == 0 )
    ++draws;
  else if( winner == pWHITE )
    ++wins1;
  else if( winner == pBLACK )
    ++wins2;

  int gap = winner == pWHITE ? winGap : -winGap;
  player1->GameResult( turns, gap );
  player2->GameResult( turns, -gap );
  stat.Add( gap );
}

bool Game::ManualTurn( const Point2i& from, const Point2i& to, bool evaluate )
{
  if( !CheckTurn( from, to ) )
    return false;

  int diff = NO_DIFF;

  if( evaluate )
    diff = ActivePlayer()->EvaluateManualTurn( from, to );
  return ApplyTurn( from, to, diff );
}

bool Game::ApplyTurn( const Point2i& from, const Point2i& to, int diff )
{
  field(to) = field(from);
  field(from) = pNO;

  if( winner == pNO && IsEnclosed( NextPlayer(active) ) )
  {
    winTurn = turn;
    winner = active;
    POINT sz = field.Size();
    OnGameOver( sz.x + sz.y );
  }

  if( winner == pNO && finished == pNO && IsFinished( active ) )
  {
    finished = active;
    winTurn = turn;
    if( active == pBLACK )
      winner = pBLACK;
  }

  if( winner == pNO && finished == pWHITE && active == pBLACK )
    winner = IsFinished( pBLACK ) ? pBOTH : finished;

  if( winner != pNO && winGap == NO_GAP )
    if( winner == pBOTH || winner != active && IsFinished( active ) )
      OnGameOver( turn - winTurn ); // The second player has finished too

  GameTurn gturn;
  gturn.from = from;
  gturn.to = to;
  gturn.diff = diff;
  turns.Append( 1, &gturn );

  SwitchPlayer();

  return true;
}

void Game::Rollback( int num )
{
   Point2i from, to;
   int size;

   for( int i = 0; i < num; ++i )
   {
      size = turns.Size();
      if( size > 0 )
      {
         to = turns[size-1].to;
         from = turns[size-1].from;
         turns.Resize( size-1 );

         if( field.DoesContain(from) && field.DoesContain(to) )
         {
            field(from) = field(to);
            field(to) = pNO;
         }
         active = NextPlayer(active);
      }
   }

   turn = 1 + turns.Size()/2;

   bool finished1 = IsFinished(active);
   bool finished2 = IsFinished(NextPlayer(active));

   if( !finished1 && !finished2 )
   {
      finished = pNO;
      winTurn = -1;
   }

   if( !finished1 || !finished2 )
      winGap = NO_GAP;

   if( winTurn == -1 || winTurn > turn ||
       winTurn == turn && active == pBLACK )
      winner = pNO;
}

bool Game::GetLastTurn( GameTurn* p_turn, int num ) const
{
  int idx = turns.Size() - 1 - num;
  if( idx < 0 )
  {
    if( p_turn != NULL )
    {
      p_turn->from.Init( -1, -1 );
      p_turn->to.Init( -1, -1 );
      p_turn->diff = NO_DIFF;
    }
    return false;
  }

  if( p_turn != NULL )
    *p_turn = turns[idx];

  return true;
}

bool Game::RandomTurn( Point2i& from, Point2i& to, int* diff )
{
  Field fturns;
  P<Point2i> points, turns;
  Point2i p;
  int chip_idx, chip_cnt, cell_cnt, cnt, num, i;
  POINT sz = field.Size();

  cell_cnt = sz.x*sz.y;
  chip_cnt = dx*dy;
  chip_idx = 0;
  points = new Point2i[cell_cnt];
  turns = new Point2i[2*chip_cnt*cell_cnt];
  cnt = 0;
  for( p.y = 0; p.y < sz.y; p.y++ )
    for( p.x = 0; p.x < sz.x; p.x++ )
      if( field(p) == active )
      {
        fturns.Init(sz);
        num = FindPossibleTurns( field, p, fturns, points );
        for( i = 0; i < num; ++i, ++cnt )
        {
          turns[2*cnt] = p;
          turns[2*cnt+1] = points[i];
        }
      }

  if( cnt == 0 )
    return false;

  int idx = (cnt == 1 ? 0 : rand()%cnt);
  from = turns[2*idx];
  to   = turns[2*idx+1];

  *diff = ActivePlayer()->EvaluateManualTurn( from, to );
  return true;
}

bool Game::AITurn()
{
  Point2i from, to;
  bool move = false;
  bool active_finished = IsFinished(active);
  int  diff = NO_DIFF;

  if( winner != pNO && finished == pNO )
    return false; // A player was enclosed, game is over

  if( active_finished && IsFinished(NextPlayer(active)) )
    return false;

  if( !active_finished )
    if( IsInitialPosition( active ) )
      move = RandomTurn( from, to, &diff );
    else
      move = ActivePlayer()->Turn( field, from, to, &diff );

  if( !move || from == to )
  {
    SkipTurn();
    return false;
  }

  return ApplyTurn( from, to, diff );
}

bool Game::IsTargetAreaFilledWith( Player area_player, Player player ) const
{
   Point2i p, p1, p2;

   GetTargetArea( field, dx, dy, area_player, p1, p2 );

   for( p.x = p1.x; p.x <= p2.x; ++p.x )
      for( p.y = p1.y; p.y <= p2.y; ++p.y )
         if( field(p) != player )
            return false;

   return true;
}

bool Game::IsFinished( Player player ) const
{
   return IsTargetAreaFilledWith( player, player );
}

bool Game::IsEnclosed( Player player ) const
{
  POINT p, p1, p2;

  GetInitialArea( field, dx, dy, player, p1, p2 );
  for( p.x = p1.x; p.x <= p2.x; ++p.x )
    for( p.y = p1.y; p.y <= p2.y; ++p.y )
      if( field(p) == player && IsChipEnclosed( field, p ) )
        return true;

  return false;
}

bool Game::IsInitialPosition( Player player ) const
{
   return IsTargetAreaFilledWith( NextPlayer(player), player );
}
