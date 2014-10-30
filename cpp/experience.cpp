#include <experience.h>

//
// GameStat
//

void GameStat::Add( double value )
{
   average = (average*count + value) / (count+1);
   ++count;
}

//GameStat& GameStat::operator = ( const GameStat& stat )
//{
//   average = stat.average;
//   count   = stat.count;
//
//   return *this;
//}

//
// TurnExp
//

void TurnExp::Init( const Point2i& pnt_from, const Point2i& pnt_to )
{
   from = pnt_from;
   to = pnt_to;
   stat.Init();
}

//TurnExp& TurnExp::operator = ( const TurnExp& exp )
//{
//   from = exp.from;
//   to = exp.to;
//   stat = exp.stat;
//
//   return *this;
//}

//
// FieldStateExp
//

void FieldStateExp::Init( const Field& fld )
{
   field.Init( fld );
   exp.Clear();
}

FieldStateExp::~FieldStateExp()
{
   for( int i = exp.Size()-1; i >= 0; --i )
      delete exp[i];
}

//FieldStateExp& FieldStateExp::operator = ( const FieldStateExp& fs_exp )
//{
//   field = fs_exp.field;
//   exp.Copy( fs_exp.exp );
//
//   return *this;
//}

int FieldStateExp::FindIdx( const Point2i& from, const Point2i& to ) const
{
   for( int i = exp.Size()-1; i >=0; --i )
      if( exp[i]->GetFrom() == from && exp[i]->GetTo() == to )
         return i;
   return -1;
}

const TurnExp* FieldStateExp::Find( const Point2i& from, const Point2i& to ) const
{
   int idx = FindIdx( from, to );
   return idx >= 0 ? exp[idx] : 0;
}

const TurnExp* FieldStateExp::GetTheBestTurn( int min_cnt ) const
{
   int cnt = exp.Size();
   if( cnt <= 0 )
      return 0;

   const TurnExp* the_best = 0;
   for( int i = 0; i < cnt; ++i )
     if( exp[i]->GetStat().GetCount() >= min_cnt )
       if( !the_best || exp[i]->GetStat().GetAverage() > the_best->GetStat().GetAverage() )
         the_best = exp[i];

   return the_best;
}

void FieldStateExp::Add( const Point2i& from, const Point2i& to, int turn_gap )
{
   int idx = FindIdx( from, to );
   TurnExp* turn_exp = idx >= 0 ? exp[idx] : 0;
   if( !turn_exp )
   {
      turn_exp = new TurnExp();
      turn_exp->Init( from, to );
      exp.Append( 1, &turn_exp );
   }
   turn_exp->Add( turn_gap );
}

//
// Experience
//

Experience::~Experience()
{
   for( int i = exp.Size()-1; i >= 0; --i )
      delete exp[i];
}

int Experience::FindIdx( const Field& field ) const
{
   for( int i = exp.Size()-1; i >= 0; --i )
      if( exp[i]->GetField() == field )
         return i;
   return -1;
}

const FieldStateExp* Experience::Find( const Field& field ) const
{
   int idx = FindIdx( field );
   return (idx >= 0 ? exp[idx] : 0);
}

void Experience::Add( const Field& field, const Point2i& from, const Point2i& to, int turn_gap )
{
   int idx = FindIdx( field );
   FieldStateExp* fld_exp = idx >= 0 ? exp[idx] : 0;
   if( !fld_exp )
   {
      int size = exp.Size();
      fld_exp = new FieldStateExp();
      fld_exp->Init( field );
      exp.Append( 1, &fld_exp );
   }
   fld_exp->Add( from, to, turn_gap );
}

int Experience::GetTurnNum() const
{
   int cnt = 0;
   for( int i = exp.Size()-1; i >=0; --i )
      cnt += exp[i]->GetTurnNum();
   return cnt;
}

