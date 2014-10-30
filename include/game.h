#pragma once

#include <ai.h>

const int NO_GAP = -1;

class Game
{
   public:
                // size_ - это размер поля; sz_ - это начальная расстановка
                // шашек (всего sz_x*sz_y шашек каждого типа)
                Game( int size_x, int size_y, int sz_x, int sz_y, AIDifficulty difficulty, AIOption opt, int ai_deep );
           bool Init();
            int CountX() const { return dx; }
            int CountY() const { return dy; }
            int Count() const { return dx*dy; }
         Player GetActive() const { return active; }
         Player GetState( Point2i p ) const;
   const Field& GetField() const { return field; }
            int GetCurTurn() const { return turn; }
            int GetFinishedTurns() const { return active == pWHITE ? turn-1 : turn; }
           bool GetLastTurn( GameTurn* p_turn, int num = 0 ) const;
         Player GetWinner() const { return winner; }
            int GetWinTurn() const { return winTurn; }
            int GetWinGap() const { return winGap; }
            int GetWins1() const { return wins1; }
            int GetWins2() const { return wins2; }
            int GetDraws() const { return draws; }
const GameStat& GetGameStat() const { return stat; }
      const AI& GetPlayer1() const { return *player1; }
      const AI& GetPlayer2() const { return *player2; }
           bool IsOver() const;
           bool IsFinished( Player player ) const;
           bool IsEnclosed( Player player ) const;
           bool IsInitialPosition( Player player ) const;

           bool AITurn();
           bool ManualTurn( const Point2i& from, const Point2i& to, bool evaluate );
           void Rollback( int num );
           bool SkipTurn();

   protected:
         Field field;
           int dx, dy; // начальное положения шашек (от угла)
        Player active; // кто ходит
           int turn;
        Player finished;
        Player winner;
           int winTurn;
           int winGap;
Array<GameTurn> turns; // все ходы
         P<AI> player1, player2;

      GameStat stat;
           int wins1, wins2, draws; // статистика по играм (сколько побед, поражений и ничьих)

           bool CheckTurn( const Point2i& from, const Point2i& to );
           bool ApplyTurn( const Point2i& from, const Point2i& to, int diff );
           bool RandomTurn( Point2i& from, Point2i& to, int* diff );
           void SwitchPlayer();
           void OnGameOver( int win_gap );
            AI* ActivePlayer() { return (active == pWHITE) ? player1 : player2; }
           bool IsTargetAreaFilledWith( Player area_player, Player player ) const;
};

