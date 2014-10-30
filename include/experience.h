#pragma once

#include <field.h>
#include <array.h>

//using namespace std;

class GameStat
{
   public:
           GameStat() { Init(); }
      void Init() { count = 0; average = 0.0; }
      void Add( double value );
    double GetAverage() const { return average; }
       int GetCount() const { return count; }

   private:
      double average;
         int count;

                GameStat( const GameStat& ) {}
      GameStat& operator = ( const GameStat& ) { return *this; }
};

class TurnExp
{
   public:
                  TurnExp() {}
             void Init( const Point2i& from, const Point2i& to );
             void Add( double value ) { stat.Add( value ); }
   const Point2i& GetFrom() const { return from; }
   const Point2i& GetTo() const { return to; }
  const GameStat& GetStat() const { return stat; }

   private:
       Point2i from, to;
      GameStat stat;

                  TurnExp( const TurnExp& exp ) {}
         TurnExp& operator = ( const TurnExp& exp ) { return *this; }
};

typedef TurnExp* TurnExpPtr;

class FieldStateExp
{
   public:
                   FieldStateExp() {}
                   ~FieldStateExp();
              void Init( const Field& fld );
              void Add( const Point2i& from, const Point2i& to, int turn_gap );
      const Field& GetField() const { return field; }
               int GetTurnNum() const { return exp.Size(); }
 const TurnExpPtr* GetTurnExp() const { return &exp[0]; }
    const TurnExp* GetTheBestTurn( int min_cnt = 1 ) const;
    const TurnExp* Find( const Point2i& from, const Point2i& to ) const;

   private:
             Field field;
   Array<TurnExp*> exp;

              int FindIdx( const Point2i& from, const Point2i& to ) const;

                  FieldStateExp( const FieldStateExp& ) {}
   FieldStateExp& operator = ( const FieldStateExp& ) { return *this; }
};

class Experience
{
   public:
           Experience() {}
           ~Experience();
      void Add( const Field& field, const Point2i& from, const Point2i& to, int turn_gap );
       int GetFieldNum() const { return exp.Size(); }
       int GetTurnNum() const;

const FieldStateExp* Find( const Field& field ) const;

   private:
      Array<FieldStateExp*> exp;

       int FindIdx( const Field& field ) const;

               Experience( const Experience& ) {}
   Experience& operator = ( const Experience& ) { return *this; }
};
