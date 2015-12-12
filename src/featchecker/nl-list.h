
#ifdef HACKLIST ////////////////////////////////////////////
#define Listed(x) x                                       //
#define Static(x) static_cast<ListedObject<T>*>(x)        //
#else //////////////////////////////////////////////////////
#define Listed(x) ListedObject<x>
#define Static(x) x
#endif /////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//  Container macros
//
////////////////////////////////////////////////////////////////////////////////

// Standard loop...
#define foreach(p,c) for ( p=(c).Iterate(NULL); p!=NULL; p=(c).Iterate(p) )

// True unless proven false...
#define setifall(y,p,c,x) for ( p=(c).Iterate(NULL), y=true; p!=NULL && y; y &= (x), p=(c).Iterate(p) )

// False unless proven true...
#define setifexists(y,p,c,x) for ( p=(c).Iterate(NULL), y=false; p!=NULL && !y; y |= (x), p=(c).Iterate(p) )

////////////////////////////////////////////////////////////////////////////////

template <class T>
class Ref
  {
  private:
    const T* ptObj ;
  public:
    Ref ( )                                    { ptObj=NULL; }
    bool operator== ( const Ref<T>& rt ) const { return(ptObj==rt.ptObj); }
    void     SetReferent ( const T& t )        { ptObj=&t; }
    const T& GetReferent ( ) const             { assert(ptObj); return(*ptObj); }
  } ;


////////////////////////////////////////////////////////////////////////////////

template <class T>
class ListedObject ;

template <class T>
class List
  {
  private:

    #ifdef HACKLIST ////////////////////////////////////////////
    void* plotLast ;                                          //
    #else //////////////////////////////////////////////////////
    ListedObject<T>* plotLast ;
    #endif /////////////////////////////////////////////////////

  public:

    // Constructor and destructor methods...
    List  ( ) ;
    List  ( const T& ) ;
    List  ( const List<T>& ) ;
    List  ( const List<T>&, const List<T>& ) ;
    ~List ( ) ;

    // Overloaded operators...
    List<T>& operator=  ( const List<T>& ) ;
    bool     operator== ( const List<T>& ) const ;
    bool     operator!= ( const List<T>& ) const ;

    // Specification methods...
    T&       Add  ( ) ;
    T&       Push ( ) ;
    void     Pop  ( ) ;

    // Extraction methods...
    Listed(T)* GetFirst ( ) const ;
    Listed(T)* GetLast  ( ) const ;
    Listed(T)* Iterate  ( const Listed(T)* ) const ;
  } ;

////////////////////////////////////////////////////////////////////////////////

template <class T>
class ListedObject : public T
  {
  friend class List<T> ;

  private:

    ListedObject<T>* plotNext ;

  public:

    ListedObject<T>& operator= ( const ListedObject<T>& lot ) { T::operator=(lot); return(*this); }
  } ;


////////////////////////////////////////////////////////////////////////////////


template <class T>
List<T>::List ( )
  {
  plotLast = NULL ;
  }

template <class T>
List<T>::List ( const T& t )
  {
  plotLast = NULL ;

  Add() = t ;
  }

template <class T>
List<T>::List ( const List<T>& lt )
  {
  ListedObject<T>* pt ;

  plotLast = NULL ;

  foreach ( pt, lt )
    Add() = *pt ;
  }

template <class T>
List<T>::List ( const List<T>& lt1, const List<T>& lt2 )
  {
  ListedObject<T>* pt ;

  plotLast = NULL ;

  foreach ( pt, lt1 )
    Add() = *pt ;
  foreach ( pt, lt2 )
    Add() = *pt ;
  }

template <class T>
List<T>::~List ( )
  {
  ListedObject<T>* plot ;
  ListedObject<T>* plot2 ;
  if ( NULL != (plot = Static(plotLast)) )
    do { plot2 = plot->plotNext ;
         delete plot ;
       } while ( (Static(plotLast)) != (plot = plot2) ) ;
  plotLast = NULL ;
  }

template <class T>
List<T>& List<T>::operator= ( const List<T>& lt )
  {
  Listed(T)* pt ;

  this->~List ( ) ;
  plotLast = NULL ;

  foreach ( pt, lt )
    Add() = *pt ;

  return *this ;
  }

template <class T>
bool List<T>::operator== ( const List<T>& lt ) const
  {
  Listed(T)* pt1 ;
  Listed(T)* pt2 ;

  for ( pt1 = Iterate(NULL), pt2 = lt.Iterate(NULL);
        pt1 != NULL && pt2 != NULL ;
        pt1 = Iterate(pt1), pt2 = lt.Iterate(pt2) )
    if ( !(*pt1 == *pt2) ) return false ;

  return ( pt1 == NULL && pt2 == NULL ) ;
  }

template <class T>
bool List<T>::operator!= ( const List<T>& lt ) const
  {
  return !(*this == lt) ;
  }

template <class T>
T& List<T>::Add ( )
  {
  ListedObject<T>* plot = new ListedObject<T> ;

  if ( NULL != plotLast )
    {
    plot->plotNext = Static(plotLast)->plotNext ;
    Static(plotLast)->plotNext = plot ;
    Static(plotLast) = plot ;
    }
  else
    {
    plot->plotNext = plot ;
    Static(plotLast) = plot ;
    }

  return *plot ;
  }

template <class T>
T& List<T>::Push ( )
  {
  ListedObject<T>* plot = new ListedObject<T> ;

  if ( NULL != plotLast )
    {
    plot->plotNext = Static(plotLast)->plotNext->plotNext ;
    Static(plotLast)->plotNext->plotNext = plot ;
    }
  else
    {
    plot->plotNext = plot ;
    Static(plotLast) = plot ;
    }

  return *plot ;
  }

template <class T>
void List<T>::Pop ( )
  {
  ListedObject<T>* plot = Static(plotLast)->plotNext ;

  if ( plot->plotNext == plot )
    plotLast = NULL ;
  else
    Static(plotLast)->plotNext = plot->plotNext ;

  delete plot ;
  }

template <class T>
Listed(T)* List<T>::GetFirst ( ) const
  {
  return ( NULL != plotLast ) ? Static(plotLast)->plotNext : NULL ;
  }

template <class T>
Listed(T)* List<T>::GetLast ( ) const
  {
  return ( NULL != plotLast ) ? Static(plotLast) : NULL ;
  }

template <class T>
Listed(T)* List<T>::Iterate ( const Listed(T)* plot ) const
  {
  #ifdef HACKLIST ////////////////////////////////////////////
  ListedObject<T>* x ;                                      //
  #endif /////////////////////////////////////////////////////

  return ( NULL == plot && NULL != plotLast ) ? Static(plotLast)->plotNext :
         ( Static(plot) != Static(plotLast) ) ? Static(plot)->plotNext : NULL ;
  }






