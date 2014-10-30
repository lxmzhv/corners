// Работа с указателями

#pragma once

#include <funcs.h>

template< class T >
class P
{
   public:
                  P( T* init=0 ): ptr(init){}
                  ~P() { Clear(); }
     virtual void Clear() { if( ptr ) delete ptr; ptr = 0; }
                  P<T>& operator = ( P<T>& init )
                  {
                     if( &init != this )
                     {
                        Clear();
                        ptr = init.GetPtr();
                     }
                     return *this;
                  }
                  P<T>& operator = ( T* init )
                  { 
                     if( init!=ptr )
                     {
                        Clear();
                        ptr = init;
                     }
                     return *this;
                  }
               T* operator -> () const { return ptr; }
               T& operator *() const { return *ptr; }
                  operator T*() const { return ptr; }
               T* GetPtr() { T* p = ptr; ptr = 0; return p; }

   protected:
               T* ptr;

   private:
                  P( const P<T>& ) {}

};

template< class T >
class VP: public P<T>
{
   public:
                   VP( T* init=0 ): P<T>(init) {}
          virtual ~VP() { Clear(); }
      virtual void Clear() { if( this->ptr )   delete[] this->ptr; this->ptr = 0; }
            VP<T>& operator = ( T* init ) { this->P<T>::operator=(init); return *this; }
            VP<T>& operator = ( VP<T>& init ) { this->P<T>::operator=(init); return *this; }

   private:
                   VP( const VP<T>& ) {}
            VP<T>& operator = ( const P<T>& ) { return *this; }
};

template<class T>
void InitArg( T* ptr, const T& value )
{
   if( ptr != NULL )
      *ptr = value;
}
