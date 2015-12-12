
class FeatStruct : public FeatureNode, public StringIndexedObject
  {
  private:

   void WriteBackPath ( File*, const FeatureNode& ) const ;
   void WriteDFS      ( File*, FeatureNode& ) const ;

  public:
    // Input and output methods...
    FeatStruct&       Read  ( File*, int& ) ;
    const FeatStruct& Write ( File* ) const ;
  } ;

//////////////////////////////////////////////////////////////////////

typedef Ref<FeatStruct> RefFeatStruct ;

//////////////////////////////////////////////////////////////////////

class FeatTable : public StringAssoc<FeatStruct>
  {
  public:
    // Input and output methods...
    FeatTable&       Read  ( File*, int& ) ;
    const FeatTable& Write ( File* ) const ;
  } ;


