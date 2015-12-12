
//////////////////////////////////////////////////////////////////////
//
//  StringIndexedObject
//
//////////////////////////////////////////////////////////////////////

class StringIndexedObject
  {
  private:
    static int iTest ;
    String* psString ;
  public:
    StringIndexedObject ( )             { psString=NULL; }
    ~StringIndexedObject ( )            { delete [] psString; psString=NULL; }
    void SetString ( const String* ps ) { delete [] psString; psString=new String[strlen(ps)+1]; strcpy(psString,ps); }
    const String* GetString ( ) const   { return(psString); }
  } ;


//////////////////////////////////////////////////////////////////////
//
//  StringAssoc
//
//////////////////////////////////////////////////////////////////////

template <class T>
class StringAssoc
  {
  private:

    T   tEmpty ;
    T** pptEntries ;
    T*  ptObjects ;
    int iLast ;
    int iMax ;

  public:

    // Constructors and destructor methods...
    StringAssoc  ( ) ;
    ~StringAssoc ( ) ;

    // Specification methods...
    void Init      ( const int& ) ;    // CODE REVIEW: Should not have a limit.
    T&   AddObject ( const String* ) ;

    // Extraction methods...
    const T& GetObject ( const String* ) const ;
    T* Iterate   ( const T* ) const ;
  } ;


//////////////////////////////////////////////////////////////////////

template <class T>
StringAssoc<T>::StringAssoc ( )
  {
  iMax       = 0 ;
  ptObjects  = NULL ;
  pptEntries = NULL ;
  iLast      = -1 ;
  }


template <class T>
StringAssoc<T>::~StringAssoc ( )
  {
  delete [] ptObjects ;
  delete [] pptEntries ;
  }


template <class T>
void StringAssoc<T>::Init ( const int& iSize )
  {
  iMax       = iSize ;
  ptObjects  = new T  [iMax] ;
  pptEntries = new T* [iMax] ;  // CODE REVIEW: Won't need with hash...
  iLast      = -1 ;
  }


template <class T>
T& StringAssoc<T>::AddObject ( const String* ps )
  {
  int iSearchFrom = 0 ;
  int iSearchTo   = iLast ;
  int iSearchTarg = -1 ;
  int iDiff       = 1 ;
  int i ;

  // Binary search for target string...
  while ( iSearchFrom <= iSearchTo )
    {
    iSearchTarg = ( (iSearchFrom + iSearchTo) / 2 ) ;
    iDiff = strcmp ( ps, pptEntries[iSearchTarg]->GetString() ) ;

    if ( iDiff <  0 ) iSearchTo = iSearchTarg - 1 ;
    if ( iDiff == 0 ) break ;
    if ( iDiff >  0 ) iSearchFrom = iSearchTarg + 1 ;
    }

  // If list over-filled, print error...
  if ( iLast >= iMax )
    {
    fprintf ( stderr, "HASH ERROR: Too many entries. (Raise max above %d.)\n", iMax ) ;
    return tEmpty ;
    }

  // If target not found...
  if ( iDiff != 0 )
    {
    // Adjust for overshot...
    if ( iDiff > 0 ) iSearchTarg++ ;

    // Shift array...
    for ( i = iLast; i >= iSearchTarg ; i-- )
      pptEntries[i+1] = pptEntries[i] ;

    // Add new key and entry...
    pptEntries[iSearchTarg] = &ptObjects[++iLast] ;
    pptEntries[iSearchTarg]->SetString ( ps ) ;
    }

  return ( iLast < iMax ) ? *pptEntries[iSearchTarg] : tEmpty ;
  }


template <class T>
const T& StringAssoc<T>::GetObject ( const String* ps ) const
  {
  int iSearchFrom = 0 ;
  int iSearchTo   = iLast ;
  int iSearchTarg = -1 ;
  int iDiff       = 1 ;

  // Binary search for target string...
  while ( iSearchFrom <= iSearchTo )
    {
    iSearchTarg = ( (iSearchFrom + iSearchTo) / 2 ) ;
    iDiff = strcmp ( ps, pptEntries[iSearchTarg]->GetString() ) ;

    if ( iDiff <  0 ) iSearchTo = iSearchTarg - 1 ;
    if ( iDiff == 0 ) break ;
    if ( iDiff >  0 ) iSearchFrom = iSearchTarg + 1 ;
    }

  return ( iDiff == 0 ) ? *pptEntries[iSearchTarg] : tEmpty ;
  }

template <class T>
T* StringAssoc<T>::Iterate ( const T* pt ) const
  {
  if ( NULL == pt ) return &ptObjects[0] ;
  return ( pt+1 > &ptObjects[iLast] ) ? NULL : const_cast<T*>(pt+1) ;
  }
