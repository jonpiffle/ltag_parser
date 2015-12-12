
////////////////////////////////////////////////////////////////////////////////

class StringObject
  {
  private:
    String* psString ;

  public:
    StringObject ( )                                  { psString=NULL; }
    StringObject ( const StringObject& s )            { if ( s.psString ) SetString(s.psString); }
    ~StringObject ( )                                 { delete [] psString; psString=NULL; }

    StringObject& operator= ( const StringObject& s ) { if ( s.psString ) SetString(s.psString); }
    void SetString ( const String* ps )               { delete [] psString; psString=new String[strlen(ps)+1]; strcpy(psString,ps); }

    bool operator== ( const StringObject& s ) const   { return !(!psString || !s.psString) && !strcmp(psString,s.psString) ; }
    const String* GetString ( ) const                 { return(psString); }
  } ;


////////////////////////////////////////////////////////////////////////////////

class ChartItem ;
class NamedChartItem ;

class ChartOp
  {
  friend class ChartItem ;
  friend class Forest ;

  private:
    ChartItem* pciLeft ;
    ChartItem* pciRight ;

  public:
    ChartOp ( ) : pciLeft(NULL), pciRight(NULL) { }
  } ;


//typedef Ref<ChartOp> RefChartOp ;


////////////////////////////////////////////////////////////////////////////////

class ChartItem : public List<ChartOp> //, public StringIndexedObject // CODE REVIEW: string?!
  {
  public:

    enum Type { TOP, BOTTOM } ;

  private:

    StringObject    sTree ;
    StringObject    sAnchor ;
    StringObject    sNode ;
    StringObject    sPosTag ;
    StringObject    sNodeType ;
    StringObject    sRoot ;
    StringObject    sFoot ;
    StringObject    sSite ;
    Type            tType ;
    FeatureNode*    pfnFeats ;
    List<ChartItem> lciChecked ;
    bool            bReached ;

  public:

    // Constructor / destructor methods...
    ChartItem  ( ) : pfnFeats(NULL), bReached(false) { }
    ChartItem  ( const StringObject& sT, const StringObject& sA, const StringObject& sN, const Type& t ) :
      sTree(sT), sAnchor(sA), sNode(sN), tType(t), pfnFeats(NULL), bReached(false) { }
    ~ChartItem ( ) { delete pfnFeats; }

    // Specification methods...
    StringObject&    SetTree      ( ) { return sTree; }
    StringObject&    SetAnchor    ( ) { return sAnchor; }
    StringObject&    SetNode      ( ) { return sNode; }
    StringObject&    SetPosTag    ( ) { return sPosTag; }
    StringObject&    SetNodeType  ( ) { return sNodeType; }
    StringObject&    SetRoot      ( ) { return sRoot; }
    StringObject&    SetFoot      ( ) { return sFoot; }
    StringObject&    SetSite      ( ) { return sSite; }
    Type&            SetType      ( ) { return tType; }
    FeatureNode&     SetFeatures  ( ) { if(!pfnFeats)pfnFeats=new FeatureNode; return *pfnFeats; }
    void             AddChecked   ( ChartItem*, ChartItem* ) ;
    ChartItem&       AddChecked   ( ) { return lciChecked.Add(); }

    // Extraction methods...
    bool                   operator==   ( const ChartItem& ) const ;
    const StringObject&    GetTree      ( ) const { return sTree; }
    const StringObject&    GetAnchor    ( ) const { return sAnchor; }
    const StringObject&    GetNode      ( ) const { return sNode; }
    const StringObject&    GetPosTag    ( ) const { return sPosTag; }
    const StringObject&    GetNodeType  ( ) const { return sNodeType; }
    const StringObject&    GetRoot      ( ) const { return sRoot; }
    const StringObject&    GetFoot      ( ) const { return sFoot; }
    const StringObject&    GetSite      ( ) const { return sSite; }
    const Type&            GetType      ( ) const { return tType; }
    const FeatureNode&     GetFeatures  ( ) const { return (!pfnFeats)?FeatureNode::EET:*pfnFeats; }
    const List<ChartItem>& GetChecked   ( ) ;

    // Input and output methods...
    //ChartItem&       Read  ( File*, int&, const FeatTable&, const FeatTable& ) ;
    void Write ( File* ) const ;
  } ;


class NamedChartItem : public ChartItem, public StringIndexedObject
  { } ;

class Forest : public StringAssoc<NamedChartItem>
  {
  private:

    StringObject sSentence ;

  public:

    // Input and output methods...
    Forest&       Read  ( File*, int&, const FeatTable&, const FeatTable& ) ;
    void Write ( File* ) const ;
  } ;



