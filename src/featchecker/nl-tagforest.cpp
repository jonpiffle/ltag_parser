
#include "nl-chkr-headers.h"


bool ChartItem::operator== ( const ChartItem& ci ) const
  {
  return ( sTree == ci.sTree &&
           sAnchor == ci.sAnchor &&
           sNode == ci.sNode &&
           tType == ci.tType &&
           ((!pfnFeats && !pfnFeats) || (pfnFeats && pfnFeats && *pfnFeats == *ci.pfnFeats)) &&
           lciChecked == ci.lciChecked ) ;     
  }


////////////////////////////////////////////////////////////////////////////////
//
//  Forest Building Methods...
//
//  Features are based on the graph model of feature logic in Stuart M. 
//  Shieber's 1992 book, "Constraint-based grammar formalisms : parsing 
//  and type inference for natural and computer languages," MIT Press, c1992.
//
//  Algorithm takes a TAG shared forest, as described in Vijayshankar & Wier, 
//  "The use of shared forests in tree adjoining grammar parsing," EACL 1993, 
//  (an and/or graph of disjunctive chart items and conjunctive parse operations),
//  traverses it depth-first, and builds another forest with separate nodes 
//  for items that differ by their features.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//  GetChecked
//
//  Given a node in the non-feature-checked forest, this method 
//  returns the corresponding nodes in the feature-checked forest.
//
////////////////////////////////////////////////////////////////////////////////

const List<ChartItem>& ChartItem::GetChecked ( )
  {
  Listed(ChartOp)*       pco ;
  Listed(ChartItem)*     pciL ;
  Listed(ChartItem)*     pciR ;
  const List<ChartItem>* plciL ;
  const List<ChartItem>* plciR ;

  // If no items are listed...
  if ( lciChecked == List<ChartItem>() )
    {
    // For each chart operation/production yielding this item...
    foreach ( pco, *this )
      // If there is a left side in the production...
      if ( pco->pciLeft )
        {
        // For each feature-expanded chart item on left side of production...
        plciL = &pco->pciLeft->GetChecked() ;
        foreach ( pciL, *plciL )
          {
          // If there is a right side in the production...
          if ( pco->pciRight )
            {
            // For each feature-expanded chart item on right side of production...
            plciR = &pco->pciRight->GetChecked() ;
            foreach ( pciR, *plciR )
              // Combine left and right features and add if successful & unique...
              AddChecked ( (TOP==pco->pciRight->tType)?pciL:pciR, (TOP==pco->pciRight->tType)?pciR:pciL ) ;
            }
          else
            // Combine left and right features and add if successful & unique...
            AddChecked ( pciL, NULL ) ;
          }
        }
    }

  return lciChecked ;
  }


////////////////////////////////////////////////////////////////////////////////
//
//  AddChecked
//
//  Given a pair of chart-item (`or') nodes in the feature-checked forest, 
//  inserts a parse production node into the feature-checked forest which 
//  `ands' the two together, if their features allow them to be combined, 
//  and if the combined node is not already in the feature-checked forest.
//
////////////////////////////////////////////////////////////////////////////////

void ChartItem::AddChecked ( ChartItem* pciL, ChartItem* pciR )
  {
  ChartItem*         pci ;
  Listed(ChartItem)* pciDup ;
  FeatureNode*       pfn ;
  FeatureNode*       pfnChild ;
  ChartOp*           pco ;

  // If start...
  if ( NULL == sNode.GetString() && pciL && pciL->pfnFeats && !pciR )
    {
    pfn = new FeatureNode ( *pciL->pfnFeats ) ;
    }

  // If projection...
  else if ( pciL && TOP == pciL->tType && BOTTOM == tType )
    {
    if ( pciR && pciR->pfnFeats )
      (pfn = new FeatureNode)->Set ( *pciL->pfnFeats, *pciR->pfnFeats ) ;
    else
      pfn = new FeatureNode ( *pciL->pfnFeats ) ;
    }

  // If substitution...
  else if ( pciL && TOP == pciL->tType && TOP == tType && pciL->pfnFeats )
    {
    pfn = new FeatureNode ;
    pfn->AddFeature ( sNode.GetString(), * new FeatureNode(*pciL->pfnFeats->GetFeature("Dn")) ) ;
    }

  // If adjunction...
  else if ( pciL && BOTTOM == pciL->tType && pciR )
    {
    pfnChild = new FeatureNode ;
    pfnChild->AddFeature ( sNode.GetString(), * new FeatureNode(*pciR->pfnFeats->GetFeature("Dn")) ) ;
    (pfn = new FeatureNode)->Set ( *pciL->pfnFeats, *pfnChild ) ;
    }

  // If null adjunction...
  else if ( pciL && BOTTOM == pciL->tType && !pciR )
    {
    pfnChild = new FeatureNode ;
    pfnChild->AddFeature(sNode.GetString()).AddFeature( "t", pfnChild->AddFeature(sNode.GetString()).AddFeature("b") ) ;
    (pfn = new FeatureNode)->Set ( *pciL->pfnFeats, *pfnChild ) ;
    }

  // If unification fails, don't add...
  if ( FeatureNode::EET == *pfn )
    {
    delete pfn ;
    return ;
    }

  // If already exists, don't add...
  foreach ( pciDup, lciChecked )
    if ( *pciDup->pfnFeats == *pfn )
      {
      pco = &pciDup->Add() ;
      pco->pciLeft  = pciL ;
      pco->pciRight = pciR ;
      delete pfn ;
      return ;
      }

  // Add feature-checked chart item...
  pci = &lciChecked.Add() ;
  pci->sTree    = sTree ;
  pci->sAnchor  = sAnchor ;
  pci->sNode    = sNode ;
  pci->sPosTag  = sPosTag ;
  pci->sNodeType = sNodeType ;
  pci->tType    = tType ;
  pci->sRoot    = sRoot ;
  pci->sFoot    = sFoot ;
  pci->sSite    = sSite ;
  pci->pfnFeats = pfn ;
  pco = &pci->Add() ;
  pco->pciLeft  = pciL ;
  pco->pciRight = pciR ;

  return ;
  }


////////////////////////////////////////////////////////////////////////////////
//
//  Input / Output Methods...
//
////////////////////////////////////////////////////////////////////////////////

void ChartItem::Write ( File* pf ) const
  {
  Listed(ChartOp)* pco ;

  FeatStruct* pfs ;

  // Express label, colon, whitespace...
  fprintf ( pf, "%x: ", this ) ;

  // Express tree, open bracket, anchor, close bracket...
  if ( GetTree().GetString() && GetAnchor().GetString() )
    fprintf ( pf, "%s[%s]", GetTree().GetString(), 
                            GetAnchor().GetString() ) ;

  // Express open angle, node, slash, type, close angle...
  fprintf ( pf, "<%s/%s>", (GetNode().GetString()) ? GetNode().GetString() : "", 
                           (ChartItem::BOTTOM == GetType()) ? "bot" : "top" ) ;

  // Express open angle, pos tag, close angle, whitespace...
  fprintf ( pf, "<%s>", (GetPosTag().GetString()) ? GetPosTag().GetString() : "" ) ;

  // Express open angle, node type, close angle, whitespace...
  fprintf ( pf, "<%s>", (GetNodeType().GetString()) ? GetNodeType().GetString() : "" ) ;

  // Express open angle, root, slash, foot, close angle...
  fprintf ( pf, "<%s/%s>", (GetRoot().GetString()) ? GetRoot().GetString() : "", 
                             (GetFoot().GetString()) ? GetFoot().GetString() : "" ) ;

  // Express open angle, site, close angle...
  fprintf ( pf, "<%s>", (GetSite().GetString()) ? GetSite().GetString() : "" ) ;

  // Express productions...
  if ( NULL != Iterate(NULL) ) fprintf ( pf, " " ) ;
  foreach ( pco, *this )
    {
    // Express open bracket, left side, comma, right side, close bracket, whitespace...
    pco->pciLeft ? fprintf(pf,"[%x,",pco->pciLeft) : fprintf(pf,"[(nil),") ;
    pco->pciRight ? fprintf(pf,"%x]",pco->pciRight) : fprintf(pf,"(nil)]") ;
    }

  // If no productions (i.e. if anchor)...
  if ( NULL == this->Iterate(NULL) )
    {
    // Express features...
    pfs = new FeatStruct ;
    pfs->Set ( GetFeatures() , FeatureNode() ) ;
    fprintf ( pf, "[" ) ;
    pfs->Write ( stdout ) ;
    fprintf ( pf, "]" ) ;
    delete pfs ;
    }

  fprintf ( pf, "\n" ) ;

  // Explore children...
  foreach ( pco, *this )
    {
    if ( pco->pciLeft && FeatureNode::EET != pco->pciLeft->GetFeatures() && !pco->pciLeft->bReached  )
      {
      pco->pciLeft->bReached = true ;
      pco->pciLeft->Write ( pf ) ;
      }
    if ( pco->pciRight && FeatureNode::EET != pco->pciRight->GetFeatures() && !pco->pciRight->bReached )
      {
      pco->pciRight->bReached = true ;
      pco->pciRight->Write ( pf ) ;
      }
    }
  }


////////////////////////////////////////////////////////////////////////////////

Forest& Forest::Read ( File* pf, int& c, const FeatTable& ftLex, const FeatTable& ftGram )
  {
  int             i ;
  static String   psBuff [ 1000 ] ;
  NamedChartItem* pnci ;
  ChartItem*      pci ;
  ChartOp*        pco ;
  FeatureNode*    pfnNew ;
  FeatureNode*    pfnOld ;

  // Consume header line...
  CONSUME_ALL ( c, pf, WHITESPACE(c) ) ;
  CONSUME_ONE ( c, pf, c=='b', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c=='e', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c=='g', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c=='i', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c=='n', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c==' ', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c=='s', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c=='e', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c=='n', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c=='t', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c=='=', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c=='\"', "ERROR.\n" ) ;
  CONSUME_STR ( c, pf, c!='\"', psBuff, i ) ;
  CONSUME_ONE ( c, pf, c=='\"', "ERROR.\n" ) ;
  CONSUME_ONE ( c, pf, c=='\n', "ERROR.\n" ) ;
  sSentence.SetString ( psBuff ) ;

  // For each node in forest...
  while ( c!=EOF )
    {
    // Consume label, colon, whitespace...
    CONSUME_STR ( c, pf, c!=':' && c!='\n', psBuff, i ) ;
    if ( 0 == strcmp(psBuff,"end") ) {
       // CONSUME_ALL(c,pf,WHITESPACE(c)); 
       return *this;
    }
    pnci = & AddObject ( psBuff ) ;
    CONSUME_ONE ( c, pf, c==':', "ERROR: No :.\n" ) ;
    CONSUME_ALL ( c, pf, SPACE(c) ) ;

    // If label is start...
    if ( 0 == strcmp(psBuff,"start") ) 
      {
      // Consume sublabel and whitespace...
      CONSUME_STR ( c, pf, ALPHANUM(c), psBuff, i ) ;
      CONSUME_ALL ( c, pf, SPACE(c) ) ;
      pci = ( pnci->Add().pciLeft = &AddObject(psBuff) ) ;

      // Consume open bracket, feature, close bracket, and whitespace...
      CONSUME_ONE ( c, pf, c=='[', "ERROR: No [.\n" ) ;
      CONSUME_STR ( c, pf, c!=' ' && c!=']', psBuff, i ) ;
      CONSUME_ONE ( c, pf, c==']', "ERROR: No ].\n" ) ;
      CONSUME_ALL ( c, pf, SPACE(c) ) ;
      pci->SetFeatures() = ftLex.GetObject ( psBuff ) ;
      }
    // If label is not start...
    else
      {
      // Consume tree...
      CONSUME_STR ( c, pf, c!='[', psBuff, i ) ;
      pnci->SetTree().SetString ( psBuff ) ;
      CONSUME_ALL ( c, pf, SPACE(c) ) ;

      // Consume open bracket, anchor, close bracket, whitespace...
      CONSUME_ONE ( c, pf, c=='[', "ERROR: No [.\n" ) ;
      CONSUME_STR ( c, pf, c!=']', psBuff, i ) ;
      pnci->SetAnchor().SetString ( psBuff ) ;
      CONSUME_ONE ( c, pf, c==']', "ERROR: No ].\n" ) ;
      CONSUME_ALL ( c, pf, SPACE(c) ) ;

      // Consume open angle, node, slash, type, close angle, whitespace...
      CONSUME_ONE ( c, pf, c=='<', "ERROR: No <.\n" ) ;
      CONSUME_STR ( c, pf, c!='/' && c!='>', psBuff, i ) ;
      pnci->SetNode().SetString ( psBuff ) ;
      CONSUME_ONE ( c, pf, c=='/', "ERROR: No /.\n" ) ;
      CONSUME_STR ( c, pf, c!='>', psBuff, i ) ;
      pnci->SetType() = ( 't'==psBuff[0] ) ? ChartItem::TOP : ChartItem::BOTTOM ;
      CONSUME_ONE ( c, pf, c=='>', "ERROR: No >.\n" ) ;
      CONSUME_ALL ( c, pf, SPACE(c) ) ;

      // Consume open angle, pos tag, close angle, whitespace...
      CONSUME_ONE ( c, pf, c=='<', "ERROR: No <.\n" ) ;
      CONSUME_STR ( c, pf, c!='>', psBuff, i ) ;
      pnci->SetPosTag().SetString ( psBuff ) ;
      CONSUME_ONE ( c, pf, c=='>', "ERROR: No >.\n" ) ;
      CONSUME_ALL ( c, pf, SPACE(c) ) ;

      // Consume open angle, node type, close angle, whitespace...
      CONSUME_ONE ( c, pf, c=='<', "ERROR: No <.\n" ) ;
      CONSUME_STR ( c, pf, c!='>', psBuff, i ) ;
      pnci->SetNodeType().SetString ( psBuff ) ;
      CONSUME_ONE ( c, pf, c=='>', "ERROR: No >.\n" ) ;
      CONSUME_ALL ( c, pf, SPACE(c) ) ;

      // Consume open angle, root, slash, foot, close angle, whitespace...
      CONSUME_ONE ( c, pf, c=='<', "ERROR: No <.\n" ) ;
      CONSUME_STR ( c, pf, c!='/' && c!='>', psBuff, i ) ;
      pnci->SetRoot().SetString ( psBuff ) ;
      CONSUME_ONE ( c, pf, c=='/', "ERROR: No /.\n" ) ;
      CONSUME_STR ( c, pf, c!='>', psBuff, i ) ;
      pnci->SetFoot().SetString ( psBuff ) ;
      CONSUME_ONE ( c, pf, c=='>', "ERROR: No >.\n" ) ;
      CONSUME_ALL ( c, pf, SPACE(c) ) ;

      // Consume open angle, site, close angle, whitespace...
      if ( c=='<' )
        {
        CONSUME_ONE ( c, pf, c=='<', "ERROR: No <.\n" ) ;
        CONSUME_STR ( c, pf, c!='>', psBuff, i ) ;
        if ( psBuff[0] != '\0' )
          pnci->SetSite().SetString ( psBuff ) ;
        CONSUME_ONE ( c, pf, c=='>', "ERROR: No >.\n" ) ;
        CONSUME_ALL ( c, pf, SPACE(c) ) ;
        }

      // Consume open bracket...
      CONSUME_ONE ( c, pf, c=='[', "ERROR: No [.\n" ) ;
      while ( c!=']' )
        {
        // Consume open angle, lexical features, close angle...
        CONSUME_ONE ( c, pf, c=='<', "ERROR: No <.\n" ) ;
        pfnOld = new FeatureNode ;
        while ( c!='>' )
          {
          // Consume lexical features...
          CONSUME_STR ( c, pf, c!=' ' && c!='>', psBuff, i ) ;
          pfnNew = new FeatureNode ;
          pfnNew->Set ( *pfnOld, ftLex.GetObject(psBuff) ) ;
          delete pfnOld ;
          pfnOld = pfnNew ;
          CONSUME_ALL ( c, pf, SPACE(c) ) ;
          }
        pfnNew = new FeatureNode ;
        pfnNew->Set ( *pfnOld, ftGram.GetObject(pnci->GetTree().GetString()) ) ;
        delete pfnOld ;
        pfnOld = pfnNew ;
        if ( '\0' != pnci->GetRoot().GetString()[0] )
          pfnOld->AddFeature("Dn").AddFeature ( "t", pfnOld->AddFeature(pnci->GetRoot().GetString()).AddFeature("t") ) ;
        if ( '\0' != pnci->GetFoot().GetString()[0] )
          pfnOld->AddFeature("Dn").AddFeature ( "b", pfnOld->AddFeature(pnci->GetFoot().GetString()).AddFeature("b") ) ;
        pci = &pnci->AddChecked() ;
        pci->SetTree()     = pnci->GetTree() ;
        pci->SetAnchor()   = pnci->GetAnchor() ;
        pci->SetNode()     = pnci->GetNode() ;
        pci->SetPosTag()   = pnci->GetPosTag() ;
        pci->SetNodeType() = pnci->GetNodeType() ;
        pci->SetType()     = pnci->GetType() ;
        pci->SetRoot()     = pnci->GetRoot() ;
        pci->SetFoot()     = pnci->GetFoot() ;
        pci->SetSite()     = pnci->GetSite() ;
        pci->SetFeatures() = *pfnOld ;
        delete pfnOld ;
        pfnOld = NULL ;
        CONSUME_ONE ( c, pf, c=='>', "ERROR: No >.\n" ) ;
        }
      // Consume close bracket, whitespace...
      CONSUME_ONE ( c, pf, c==']', "ERROR: No ].\n" ) ;
      CONSUME_ALL ( c, pf, SPACE(c) ) ;

      // Consume productions...
      while ( c=='[' )
        {
        pco = & pnci->Add() ;
        // CODE REVIEW: Note Anoop's left and right is opposite of mine.
        // Consume open bracket, left-arg, comma, right-arg, close bracket, whitespace...
        CONSUME_ONE ( c, pf, c=='[', "ERROR: No [.\n" ) ;
        CONSUME_STR ( c, pf, c!=',', psBuff, i ) ;
        pco->pciLeft = ( 0!=strcmp("(nil)",psBuff) ) ? &AddObject(psBuff) : NULL ;
        CONSUME_ONE ( c, pf, c==',', "ERROR: No ,.\n" ) ;
        CONSUME_STR ( c, pf, c!=']', psBuff, i ) ;
        pco->pciRight = ( 0!=strcmp("(nil)",psBuff) ) ? &AddObject(psBuff) : NULL ;
        CONSUME_ONE ( c, pf, c==']', "ERROR: No ].\n" ) ;
        CONSUME_ALL ( c, pf, SPACE(c) ) ;
        }
      }
    CONSUME_ALL ( c, pf, WHITESPACE(c) ) ;
    }
  return *this ;
  }


////////////////////////////////////////////////////////////////////////////////

void Forest::Write ( File* pf ) const
  {
  Listed(NamedChartItem)* pnci ;
  Listed(ChartItem)*      pci ;
  Listed(ChartOp)*        pco ;
  FeatureNode             fn ;

  fprintf ( pf, "begin sent=\"%s\"\n", sSentence.GetString() ) ;

  // For each start node in forest...
  foreach ( pco, GetObject("start") )
    foreach ( pci, pco->pciLeft->GetChecked() )
      if ( fn.Set(pco->pciLeft->GetFeatures(), pci->GetFeatures()) != FeatureNode::EET )
        {
//fprintf(pf,"--------------------\n");
//pco->pciLeft->GetFeatures().Print(pf);
//fprintf(pf,"--------------------\n");
//pci->GetFeatures().Print(pf);
//fprintf(pf,"--------------------\n");

        // Write immediate child...
        fprintf ( pf, "start: %x []\n", pci ) ;
        pci->Write ( pf ) ;
        }

  fprintf ( pf, "end\n" ) ;
  }



