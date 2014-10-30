// Строка

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <numbers.h>
#include <memory.h>

String::String( int len ): size(0)
{
   Init( len );
}

String::String( const char* str ): size(0)
{
   *this = str;
}

String::String( const String& str ): size(0)
{
   *this = str;
}

//String& String::operator = ( const char* str )
//{
//	if( value != str )
//      if( !str )
//      {
//         Init( 0 );
//         *value = 0;
//      }
//      else
//      {
//         int l = strlen(str);
//         Init( l );
//         strcpy( value, l+1, str );
//      }
//	return *this;
//}
//
//String& String::operator = ( char ch )
//{
//   Init( 1 );
//	value[0]=ch;
//	value[1]='\0';
//	return *this;
//}

void String::Init( int sz )
{
   if( !value || size < sz+1 )
      value = new char[ size = sz+1 ];
   *value = 0;
}

void String::Resize( int sz )
{
   if( sz+1 <= size )   return;
   size = sz+1;
   VP<char> s( new char[size] );
   strncpy( s, value, size );
   value = s;
}

String& String::operator += ( const char* str )
{
   if( !str )   return *this;
   int l1 = Length(), l2 = strlen(str), l = l1+l2;
   Resize( l );
   strncat( value, str, l+1 );
   return *this;
}

String& String::operator += ( char ch )
{
   int l = Length();
   Resize( l+1 );
   value[l] = ch;
   value[l+1] = 0;
   return *this;
}

String& String::operator += ( long val )
{
   const int buf_sz = sizeof(val)*3;
   static char buf[buf_sz];
   *buf = 0;

   sprintf( buf, "%ld", val );
   return *this += buf;
}

String& String::operator += ( int val )
{
   const int buf_sz = sizeof(val)*3;
   static char buf[buf_sz];
   *buf = 0;

   sprintf( buf, "%d", val );
   return *this += buf;
}

String& String::operator += ( ulong val )
{
   const int buf_sz = sizeof(val)*3;
   static char buf[buf_sz];
   *buf = 0;

   sprintf( buf, "%lu", val );
   return *this += buf;
}

String& String::operator += ( double val )
{
   const int buf_sz = sizeof(val)*10;
   static char buf[buf_sz];
   *buf = 0;

   sprintf( buf, "%g", val );
   return *this += buf;
}

//String String::operator + ( const char* str ) const
//{
//   String res( Length() + (str ? strlen(str) : 0) );
//   res = value;   
//   return res += str;
//}
//
//String String::operator + ( char ch ) const
//{
//	 String res( Length()+1 );
//    res = value;
//	 return res += ch;
//}
//
String operator + ( char ch, const String &str )
{
   String res( str.Length()+1 );
   res = ch;   
   res += str;
   return res;
}
