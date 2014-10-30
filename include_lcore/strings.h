// Строка

#pragma once
#include <funcs.h>
#include <string.h>
#include <pointers.h>

class DLL_OBJ String
{
   public:
              String( int len = 0 );
              String( const char* str );
              String( const String& str );

          int Length() const { return (int)strlen(value); }
         void Clear() { *value = 0; }
         void Resize( int sz );

      String& operator += ( const char* );
      String& operator += ( char );
      String& operator += ( long );
      String& operator += ( int );
      String& operator += ( ulong );
      String& operator += ( double );

      template< class T >
      String& operator << ( const T& val ) { return *this += val; }

      template< class T >
      String& operator = ( const T& val ) { Clear(); return *this += val; }

              operator char* ()  { return value; }
              operator const char* ()  const { return value; }

      //String& operator = ( const char* str );
      //String& operator = ( char ch );
      //String& operator << ( const char* str ) { return *this += str; }
      //String& operator << ( char ch )  { return *this += ch; }
      //String& operator << ( long val )  { return *this += val; }
       //String operator + ( const char* str ) const;
       //String operator + ( char ch ) const ;

   private:
     VP<char> value;
          int size;

         void Init( int sz );
};

String DLL_OBJ operator + (char ch, const String &String2);
