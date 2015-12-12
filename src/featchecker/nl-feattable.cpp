
#include "nl-headers-trunc.h"

//////////////////////////////////////////////////////////////////////
//
//  FeatStruct
//
//////////////////////////////////////////////////////////////////////

FeatStruct& FeatStruct::Read ( File* pf, int& c )
  {
  int           i ;
  static String psLeft  [ 128 ] ;
  static String psRight [ 128 ] ;
  FeatureNode   fnNew ;
  FeatureNode*  pfnOldLeft ;
  FeatureNode*  pfnOldRight ;
  FeatureNode*  pfnNewLeft ;
  FeatureNode*  pfnNewRight ;

  // Consume white space...
  while ( c==' ' ) c=getc(pf);

  // Consume feature equations...
  while ( c!=',' && c!=')' && c!=']' && c!='/' && c!='\"' && c!='\n' && c!='!' && c!=EOF )
    {
    fnNew = FeatureNode ( ) ;
    pfnOldLeft  = this ;
    pfnOldRight = this ;
    pfnNewLeft  = &fnNew ;
    pfnNewRight = &fnNew ;

    // Consume node label, and set as new left arc...
    for ( i=0; c!='.' && c!=':' && c!='<' && c!=' ' && c!='\n' && c!='/' && c!=',' && c!=')' && c!=']' && c!='\"' && c!='!' && c!=EOF; c=getc(pf) ) psLeft[i++]=c;
    psLeft[i] = '\0' ;

    // If t/b specified...
    if ( c=='.' )
      {
      // Consume dot...
      if ( c=='.' ) c=getc(pf) ;

      // Add previous left arc to left paths...
      pfnOldLeft = &pfnOldLeft->AddFeature ( psLeft ) ;
      pfnNewLeft = &pfnNewLeft->AddFeature ( psLeft ) ;

      // Consume "t" or "b", and set as new left arc...
      for ( i=0; c!=':' && c!='\n' && c!='/' && c!=',' && c!=')' && c!=']' && c!='\"' && c!='!' && c!=EOF; c=getc(pf) ) psLeft[i++]=c;
      psLeft[i] = '\0' ;
      }
    // If no t/b specified...
    else
      {
      // Add previous left arc to left paths...
      pfnOldLeft = &pfnOldLeft->AddFeature ( psLeft ) ;
      pfnNewLeft = &pfnNewLeft->AddFeature ( psLeft ) ;

      // Set "t" as left arc...
      psLeft[0] = 't' ;
      psLeft[1] = '\0' ;
      }

    // Consume white space...
    while ( c==' ' ) c=getc(pf);

    // Consume colon...
    if ( c==':' ) c=getc(pf) ;
    else fprintf ( stderr, "ERROR: No colon to begin path.\n" ) ;

    // Consume left angle...
    if ( c=='<' ) c=getc(pf) ;
    else fprintf ( stderr, "ERROR: No left angle to begin path.\n" ) ;

    // Consume white space...
    while ( c==' ' ) c=getc(pf);

    // Consume path...
    while ( c!='>' && c!=EOF )
      {
      // Add previous left arc to left paths...
      pfnOldLeft = &pfnOldLeft->AddFeature ( psLeft ) ;
      pfnNewLeft = &pfnNewLeft->AddFeature ( psLeft ) ;

      // Consume arc label, and set as new left arc...
      for ( i=0; c!=' ' && c!='>' && c!='\n' && c!='/' && c!=',' && c!=')' && c!=']' && c!='\"' && c!='!' && c!=EOF; c=getc(pf) ) psLeft[i++]=c;
      psLeft[i] = '\0' ;

      // Consume white space...
      while ( c==' ' || c=='\t' || c=='\n' ) c=getc(pf);
      }

    // Consume right angle...
    if ( c=='>' ) c=getc(pf) ;
    else fprintf ( stderr, "ERROR: No right angle to end path.\n" ) ;

    // Consume white space...
    while ( c==' ' ) c=getc(pf);

    // Consume equals sign...
    if ( c=='=' ) c=getc(pf) ;
    else fprintf ( stderr, "ERROR: No equals in feature equation.\n" ) ;

    // Consume white space...
    while ( c==' ' ) c=getc(pf);

    // Consume node label / atom label, and set as new right arc...
    for ( i=0; c!='.' && c!=':' && c!='<' && c!=' ' && c!='/' && c!=',' && c!=')' && c!=']' && c!='\n' && c!='\"' && c!='!' && c!=EOF; c=getc(pf) ) psRight[i++]=c;
    psRight[i] = '\0' ;

    // If label was part of atom specification...
    if ( c!='.' && c!=':' && c!='<' )
      {
      // Add previous left arc to left paths...
      pfnOldLeft = &pfnOldLeft->AddFeature ( psLeft ) ;
      // Add label at end of left path...
      pfnOldLeft->AddLabel ( psRight ) ;

      // Consume remainder of atom description...
      while ( c=='/' )
        {
        // Consume slash...
        if ( c=='/' ) c=getc(pf) ;

        // Consume atom label...
        for ( i=0; c!='!' && c!='/' && c!=' ' && c!='\t' && c!=',' && c!=')' && c!=']' && c!='\n' && c!='\"' && c!='!' && c!=EOF; c=getc(pf) ) psRight[i++]=c;
        psRight[i] = '\0' ;

        // Add additional label at end of path...
        pfnOldLeft->AddLabel ( psRight ) ;
        }
      }
    // If label was part of arc coreference...
    else
      {
      // If "t" or "b" specified...
      if ( c=='.' )
        {
        // Consume dot...
        if ( c=='.' ) c=getc(pf) ;

        // Add previous left arc to left paths...
        pfnOldRight = &pfnOldRight->AddFeature ( psRight ) ;
        pfnNewRight = &pfnNewRight->AddFeature ( psRight ) ;

        // Consume "t" or "b", and set as new left arc...
        for ( i=0; c!=':' && c!='\n' && c!='/' && c!=',' && c!=')' && c!=']' && c!='\"' && c!='!' && c!=EOF; c=getc(pf) ) psRight[i++]=c;
        psRight[i] = '\0' ;
        }
      // If no t/b specified...
      else
        {
        // Add previous left arc to left paths...
        pfnOldRight = &pfnOldRight->AddFeature ( psRight ) ;
        pfnNewRight = &pfnNewRight->AddFeature ( psRight ) ;

        // Set "t" as left arc...
        psRight[0] = 't' ;
        psRight[1] = '\0' ;
        }

      // Consume white space...
      while ( c==' ' ) c=getc(pf);

      // Consume colon...
      if ( c==':' ) c=getc(pf) ;
      else fprintf ( stderr, "ERROR: No colon to begin path.\n" ) ;

      // Consume white space...
      while ( c==' ' ) c=getc(pf);

      // Consume left angle...
      if ( c=='<' ) c=getc(pf) ;
      else fprintf ( stderr, "ERROR: No left angle to begin path.\n" ) ;

      // Consume white space...
      while ( c==' ' ) c=getc(pf);

      // Consume path...
      while ( c!='>' && c!=EOF )
        {
        // Add previous left arc to left paths...
        pfnOldRight = &pfnOldRight->AddFeature ( psRight ) ;
        pfnNewRight = &pfnNewRight->AddFeature ( psRight ) ;

        // Consume arc label, and set as new left arc...
        for ( i=0; c!=' ' && c!='>' && c!='\n' && c!='/' && c!=',' && c!=')' && c!=']' && c!='\"' && c!='!' && c!=EOF; c=getc(pf) ) psRight[i++]=c;
        psRight[i] = '\0' ;

        // Consume white space...
        while ( c==' ' || c=='\t' || c=='\n' ) c=getc(pf);
        }

      // Consume right angle...
      if ( c=='>' ) c=getc(pf) ;
      else fprintf ( stderr, "ERROR: No right angle to end path.\n" ) ;

      // If left node doesn't exist...
      if ( NULL == pfnOldLeft->GetFeature(psLeft) )
        // Set left path to reference right path...
        pfnOldLeft->AddFeature ( psLeft, pfnOldRight->AddFeature(psRight) ) ;
      // If right node doesn't exist...
      else if ( NULL == pfnOldRight->GetFeature(psRight) )
        // Set right path to reference left path...
        pfnOldRight->AddFeature ( psRight, pfnOldLeft->AddFeature(psLeft) ) ;
      // If both nodes exist...
      else
        {
        // Corefer paths in new structure...
        pfnNewLeft->AddFeature ( psLeft, pfnNewRight->AddFeature(psRight) ) ;
        // Unify new with copy of current structure, and set to replace current structure...
        Set ( FeatureNode(*this), fnNew ) ;
        }
      } // End else condition.

    // Consume white space...
    while ( c==' ' ) c=getc(pf);

    } // End while loop.

  // Return for path coreference specifications...
  return *this ;
  }


void FeatStruct::WriteBackPath ( File* pf, const FeatureNode& fn ) const
  {
  const FeatureNode*  pfnTemp ;
  Listed(FeatureArc)* pfa ;

  // If node has back path (i.e. not root)...
  if ( NULL != (pfnTemp = fn.GetFeature("@BACK@")) )
    {
    // Recurse back toward root...
    WriteBackPath ( pf, *pfnTemp ) ;

    // Find arc from backpath that leads to current node...
    foreach ( pfa, pfnTemp->GetFeatures() )
      if ( &fn == &pfa->GetDest() ) break ;

    assert ( pfa ) ;

    // Print delimiters depending on how far path continues...
    if ( NULL != pfnTemp->GetFeature("@BACK@") &&
         NULL != pfnTemp->GetFeature("@BACK@")->GetFeature("@BACK@") &&
         NULL != pfnTemp->GetFeature("@BACK@")->GetFeature("@BACK@")->GetFeature("@BACK@") )
      fprintf ( pf, "\201" ) ;

    // Print label of returning arc...
    fprintf ( pf, pfa->GetLabel().GetString() ) ;

    // Print delimiters depending on how far path continues...
    if ( NULL == pfnTemp->GetFeature("@BACK@") )
      fprintf ( pf, "." ) ;
    else if ( NULL == pfnTemp->GetFeature("@BACK@")->GetFeature("@BACK@") )
      fprintf ( pf, ":<" ) ;
    }
  }

void FeatStruct::WriteDFS ( File* pf, FeatureNode& fn ) const
  {
  Listed(FeatureArc)* pfa ;
  Listed(RefLabel)*   prl ;

  // If any labels on feature node...
  if ( List<RefLabel>() != fn.GetLabels() )
    {
    WriteBackPath ( pf, fn ) ;
    fprintf ( pf, ">=" ) ;

    // For each label in feature node...
    foreach ( prl, fn.GetLabels() )
      {
      if ( EET != fn )
        fprintf ( pf, prl->GetReferent().GetString() ) ;
      else fprintf ( pf, "EET" ) ;
      if ( NULL != fn.GetLabels().Iterate(prl) ) fprintf ( pf, "/" ) ;
      }
    fprintf ( pf, ";" ) ;
    }

  // For each non-backpath arc in feature node...
  foreach ( pfa, fn.GetFeatures() )
    if ( '@' != pfa->GetLabel().GetString()[0] )
      {
      // If not yet visited...
      if ( NULL == pfa->GetDest().GetFeature("@BACK@") )
        {
        // Specify back path...
        pfa->GetDest().AddFeature ( "@BACK@", fn ) ;
        WriteDFS ( pf, pfa->GetDest() ) ;
        }
      // If already visited...
      else
        {
        WriteBackPath ( pf, fn ) ;
        // Print delimiters depending on how far path continues...
        if ( NULL != fn.GetFeature("@BACK@") &&
             NULL != fn.GetFeature("@BACK@")->GetFeature("@BACK@") &&
             NULL != fn.GetFeature("@BACK@")->GetFeature("@BACK@")->GetFeature("@BACK@") )
          fprintf ( pf, "\201" ) ;
        fprintf ( pf, pfa->GetLabel().GetString() ) ;
        if ( NULL == fn.GetFeature("@BACK@") )
          fprintf ( pf, "." ) ;
        else if ( NULL == fn.GetFeature("@BACK@")->GetFeature("@BACK@") )
          fprintf ( pf, ":<" ) ;
        fprintf ( pf, ">=" ) ;
        WriteBackPath ( pf, pfa->GetDest() ) ;
        fprintf ( pf, ">" ) ;
        fprintf ( pf, ";" ) ;
        }
      }
  }

const FeatStruct& FeatStruct::Write ( File* pf ) const
  {
  FeatureNode fnCopy = *this ;

  WriteDFS ( pf, fnCopy ) ;

  return *this ;
  }


//////////////////////////////////////////////////////////////////////
//
//  FeatTable
//
//////////////////////////////////////////////////////////////////////

FeatTable& FeatTable::Read ( File* pf, int& c )
  {
  int         i = 0 ;
  String      psBuff [ 256 ] ;
  FeatStruct* pfs ;
  FeatStruct* pfsNew ;

  while ( c!=EOF )
    {
    // Consume whitespace and comments...
    while ( c==' ' || c=='\t' || c=='\n' || c==';' ) 
       {
       // Consume white space...
       if ( c==' ' || c=='\t' || c=='\n' )
         c=getc(pf) ;
       // Consume comment...
       else if ( c==';' )
         {
         // Consume entire line...
         for ( ; c!='\n' && c!=EOF ; c=getc(pf) ) ;

         // Consume carriage return...
         if ( c=='\n' ) c=getc(pf) ;
         }
       }

    // Quit if no more features...
    if ( c==EOF ) return *this ;

    // Consume feature name...
    for ( i = 0; c!=' ' && c!='\t' && c!='\n' && c!='<' && c!=EOF ; c=getc(pf) )
      psBuff[i++]=c;
    psBuff[i] = '\0'; i = 0;

    pfs = &AddObject ( psBuff ) ;

    // Consume white space...
    for ( ; c==' ' || c=='\t' || c=='\n' ; c=getc(pf) ) ;

    // Consume list of feature structures (equations or subfeatures)...
    do {
       // Consume comma (optional)...
       if ( c==',' ) c=getc(pf);

       // Consume white space...
       for ( ; c==' ' || c=='\t' || c=='\n' ; c=getc(pf) ) ;

       // Consume subfeature name...
       if ( c=='@' || c=='#' )
         {
         // Consume string...
         for ( i = 0; c!=' ' && c!='\t' && c!='\n' && c!=',' && c!='!' && c!=EOF ; c=getc(pf) )
           psBuff[i++]=c;
         psBuff[i] = '\0'; i = 0;
/*
         // Unify subfeature...
         pfsNew = new FeatStruct ;
         pfsNew->Set ( *pfs, GetObject(psBuff) ) ;
         *pfs = *pfsNew ;
*/
         }
       // Consume feature equation...
       else pfs->Read ( pf, c ) ;

       // Consume white space...
       for ( ; c==' ' || c=='\t' || c=='\n' ; c=getc(pf) ) ;

       } while ( c!='!' ) ;

    // Consume exclamation point...
    if ( c=='!' ) c=getc(pf) ;
    else fprintf ( stderr, "ERROR: No exclamation point between entries.\n" ) ;

    // Consume white space...
    for ( ; c==' ' || c=='\t' || c=='\n' ; c=getc(pf) ) ;
    }

  return *this ;
  }

const FeatTable& FeatTable::Write ( File* pf ) const
  {
  const FeatStruct* pfs ;

  foreach ( pfs, *this )
    {
    fprintf ( pf, "%s ", pfs->GetString() ) ;
    pfs->Write ( pf ) ;
    fprintf ( pf, "\n" ) ;
    }

  return *this ;
  }
