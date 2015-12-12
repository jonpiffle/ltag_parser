
//////////////////////////////////////////////////////////////////////

class FeatureNode ;

//////////////////////////////////////////////////////////////////////

class Label : public StringIndexedObject
  { } ;

class StringLabel : public StringAssoc<Label>
  {
  public:
    StringLabel ( int i ) { Init(i); }
  } ;

typedef Ref<Label> RefLabel ;

//////////////////////////////////////////////////////////////////////

class FeatureArc
  {
  friend class FeatureNode ;
  friend class List<FeatureArc> ;
  friend class ListedObject<FeatureArc> ;

  private:

    const Label* plLabel ;
    FeatureNode* pfnDest ;

  public:

    // Constructor and destructor methods...
    FeatureArc  ( ) ;
    FeatureArc  ( const Label&, FeatureNode* ) ;
    ~FeatureArc ( ) ;

    // Boolean methods...
    bool operator== ( const FeatureArc& ) const ;

    // Extraction methods...
    const Label& GetLabel ( ) const ;
    FeatureNode& GetDest  ( ) const ;
  } ;

//////////////////////////////////////////////////////////////////////

class FeatureNode
  {
  private:

    List<RefLabel>   lrlLabels ;
    List<FeatureArc> lfaArcs ;
    FeatureNode*     pfnMark ;

    // Constants...
    static FeatureNode* pfnFree ;
    static StringLabel  salLabels ;

    // Helper methods...
    void MarkDFS       ( ) ;
    void UnmarkDFS     ( ) ;
    void CopyDFS       ( FeatureNode* ) ;
    bool UnifyDFS      ( FeatureNode* ) ;
    void CopyUnionDFS  ( FeatureNode* ) ;
    bool SubsumeDFS    ( FeatureNode* ) ;
    void PrintDFS      ( File* ) ;
    void EraseDFS      ( ) ;

    FeatureNode ( int ) ;

  public:

    // Constants...
    static const FeatureNode EET ;

    // Constructor and destructor methods...
    FeatureNode  ( ) ;
    FeatureNode  ( const FeatureNode& ) ;
    ~FeatureNode ( ) ;

    // Boolean methods...
    bool operator== ( const FeatureNode& ) const ;
    bool operator!= ( const FeatureNode& ) const ;
    bool operator<= ( const FeatureNode& ) const ;

    // Specification methods...
    void         Eet        ( ) ;
    FeatureNode& operator=  ( const FeatureNode& ) ;
    FeatureNode& Set        ( const FeatureNode& ) ;
    FeatureNode& Set        ( const FeatureNode&, const FeatureNode& ) ;
    bool         AddLabel   ( const String* ) ;
    FeatureNode& AddFeature ( const String* ) ;
    bool         AddFeature ( const String*, const FeatureNode& ) ;

    // Extraction methods...
    const List<RefLabel>&   GetLabels   ( ) const ;
    const List<FeatureArc>& GetFeatures ( ) const ;
    FeatureNode*            GetFeature  ( const String* ) const ;

    void Print ( File* ) const ;
  } ;

