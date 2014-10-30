
#include <pointers.h>

template<class T>
class Array
{
  public:

             Array( int n_reserve = 0 ): size(0), bufSize(0) { Reserve( n_reserve ); }
        void Reserve( int num );
        void Resize( int new_size );
          T* Append( int num, const T* data = NULL );
        void Ins( int idx, int num, const T* data );
         int Size() const { return size; }
        void Clear() { Resize(0); }
        void Copy( const Array<T>& a ) { Clear(); Append( a.Size(), a ); }

    operator T* () { return arr; }
    operator const T* () const { return arr; }

  private:
    int   size;     // size of array
    int   bufSize;  // size of buffer (arr)
    VP<T> arr;
};

template<class T>
void Array<T>::Reserve( int num )
{
  if( num <= bufSize )
    return;

  bufSize = num;
  VP<T> data( new T[bufSize] );
  for( int i = 0; i < size; ++i )
    data[i] = arr[i];

  arr = data;
}

template<class T>
void Array<T>::Resize( int new_size )
{
  if( new_size > bufSize )
    Reserve( new_size * 4/3 );

  size = new_size;
}

template<class T>
T* Array<T>::Append( int num, const T* data )
{
  int old_size = size;
  Resize( old_size + num );

  if( data )
    for( int i = 0; i < num; ++i )
      arr[old_size+i] = data[i];
  return arr + old_size;
}

template<class T>
void Array<T>::Ins( int idx, int num, const T* data )
{
  int new_size = size + num;
  int old_size = size;
  int new_idx = idx + num;

  Resize( old_size + num );

  for( int i = old_size-1; i >= new_idx; --i )
    arr[i] = arr[i-num];

  for( int i = 0; i < num; ++i )
    arr[idx+i] = data[i];
}
