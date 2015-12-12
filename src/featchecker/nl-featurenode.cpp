
#include "nl-headers-trunc.h"

//////////////////////////////////////////////////////////////////////
//
//  FeatureArc
//
//////////////////////////////////////////////////////////////////////

FeatureArc::FeatureArc ( )
  {
  plLabel = NULL ;
  pfnDest = NULL ;
  }

FeatureArc::FeatureArc ( const Label& l, FeatureNode* pfn )
  {
  plLabel = &l ;
  pfnDest = pfn ;
  }

FeatureArc::~FeatureArc ( )
  {
  }

bool FeatureArc::operator== ( const FeatureArc& fa ) const
  {
  return ( plLabel == fa.plLabel && pfnDest == fa.pfnDest ) ;
  }

const Label& FeatureArc::GetLabel ( ) const
  {
  assert ( plLabel ) ;
  return *plLabel ;
  }

FeatureNode& FeatureArc::GetDest ( ) const
  {
  assert ( pfnDest ) ;
  return *pfnDest ;
  }

//////////////////////////////////////////////////////////////////////
//
//  FeatureNode
//
//////////////////////////////////////////////////////////////////////

FeatureNode* FeatureNode::pfnFree = (FeatureNode*)-1 ;

StringLabel FeatureNode::salLabels ( 5000 ) ;

const FeatureNode FeatureNode::EET ( 0 ) ; 

//////////////////////////////////////////////////////////////////////

#ifdef CHECKLEAKS ////////////
static int iNodes = 0 ;     //
#endif ///////////////////////

FeatureNode::FeatureNode ( int )
  {
  lrlLabels = List<RefLabel> ( ) ;
  lfaArcs   = List<FeatureArc> ( ) ;
  pfnMark   = NULL ;

  lrlLabels.Add().SetReferent ( Label() ) ;

  #ifdef CHECKLEAKS //////////
  iNodes++ ;                //
  #endif /////////////////////
  }

//////////////////////////////////////////////////////////////////////

void FeatureNode::MarkDFS ( )
  {
  Listed(FeatureArc)* pfa ;

  pfnMark = this ;

  // For each arc, if destination is unmarked, mark it...
  foreach ( pfa, lfaArcs )
    if ( NULL == pfa->pfnDest->pfnMark )
      pfa->pfnDest->MarkDFS ( ) ;
  }

//////////////////////////////////////////////////////////////////////

void FeatureNode::UnmarkDFS ( )
  {
  Listed(FeatureArc)* pfa ;

  pfnMark = NULL ;

  // For each arc, if destination is marked, unmark it...
  foreach ( pfa, lfaArcs )
    if ( NULL != pfa->pfnDest->pfnMark )
      pfa->pfnDest->UnmarkDFS ( ) ;
  }

//////////////////////////////////////////////////////////////////////

void FeatureNode::CopyDFS ( FeatureNode* pfnOrig )
  {
  FeatureNode*        pfnNew ;
  Listed(FeatureArc)* pfa ;

  // Mark original...
  pfnOrig->pfnMark = this ;

  // Copy each label...
  lrlLabels = pfnOrig->lrlLabels ;

  // Copy each arc...
  foreach ( pfa, pfnOrig->lfaArcs )
    {
    // If destination is already copied, use existing copy...
    if ( NULL != pfa->pfnDest->pfnMark )
      lfaArcs.Add() = FeatureArc ( *pfa->plLabel, pfa->pfnDest->pfnMark ) ;

    // Otherwise use new copy...
    else
      {
      pfnNew = new FeatureNode ;
      lfaArcs.Add() = FeatureArc ( *pfa->plLabel, pfnNew ) ;
      pfnNew->CopyDFS ( pfa->pfnDest ) ;
      }
    }
  }

//////////////////////////////////////////////////////////////////////

bool FeatureNode::UnifyDFS ( FeatureNode* pfnOrig2 )
  {
  FeatureNode*        pfnOrig1 = this ;
  FeatureNode*        pfn1 ;
  FeatureNode*        pfn2 ;
  Listed(RefLabel)*   prl1 ;
  Listed(RefLabel)*   prl2 ;
  Listed(FeatureArc)* pfa1 ;
  Listed(FeatureArc)* pfa2 ;
  bool                bOut = false ;
  bool                bEvery, bAny ;

  // If mark loops already unified, return...
  pfn1 = pfnOrig1 ;
  do { if ( pfn1 == pfnOrig2 ) return true ;
     } while ( pfnOrig1 != (pfn1 = pfn1->pfnMark) ) ;

  // Twist together mark loops...
  pfn1 = pfnOrig1->pfnMark ;
  pfnOrig1->pfnMark = pfnOrig2->pfnMark ;
  pfnOrig2->pfnMark = pfn1 ;

  // Find a labelled node in loop...
  for ( pfn1 = this; pfn1->pfnMark != this && pfn1->lrlLabels == List<RefLabel>(); pfn1 = pfn1->pfnMark ) ;
  // For each label, if it appears in every other node, succeed...
  foreach ( prl1, pfn1->lrlLabels )
    {
    bEvery = true ;
    for ( pfn2 = pfn1->pfnMark; pfn2 != pfn1; pfn2 = pfn2->pfnMark )
      {
      bAny = false ;
      foreach ( prl2, pfn2->lrlLabels )
        if ( *prl1 == *prl2 ) bAny = true ;
      if ( (bAny == false && pfn2->lrlLabels != List<RefLabel>()) ||
           pfn2->lfaArcs != List<FeatureArc>() )
        bEvery = false ;
      }
    if ( bEvery == true ) bOut = true ;
    }
  if ( pfn1->lrlLabels == List<RefLabel>() ) bOut = true ;

  // For each pair of nodes in mark loops, unify each pair of like-labelled arcs...
  for ( pfn1 = pfnOrig2->pfnMark; pfn1 != pfnOrig1->pfnMark; pfn1 = pfn1->pfnMark )
    for ( pfn2 = pfnOrig1->pfnMark; pfn2 != pfnOrig2->pfnMark; pfn2 = pfn2->pfnMark )
      foreach ( pfa1, pfn1->lfaArcs )
        foreach ( pfa2, pfn2->lfaArcs )
          if ( pfa1->plLabel == pfa2->plLabel &&
               !pfa1->pfnDest->UnifyDFS(pfa2->pfnDest) )
            bOut = false ;

  return bOut ;
  }

//////////////////////////////////////////////////////////////////////

void FeatureNode::CopyUnionDFS ( FeatureNode* pfnLoop )
  {
  FeatureNode*        pfn1 ;
  FeatureNode*        pfn2 ;
  FeatureNode*        pfnL ;
  FeatureNode*        pfnD ;
  FeatureNode*        pfnNew ;
  Listed(RefLabel)*   prl1 ;
  Listed(RefLabel)*   prl2 ;
  Listed(FeatureArc)* pfa ;
  Listed(FeatureArc)* pfaC ;
  bool                bEvery, bAny ;

  // Find a labelled node in loop...
  for ( pfn1 = pfnLoop;
        pfn1->pfnMark != pfnLoop && pfn1->lrlLabels == List<RefLabel>();
        pfn1 = pfn1->pfnMark ) ;
  // For each label, if it appears in every other node, copy it...
  foreach ( prl1, pfn1->lrlLabels )
    {
    bEvery = true ;
    for ( pfn2 = pfn1->pfnMark; pfn2 != pfnLoop; pfn2 = pfn2->pfnMark )
      {
      bAny = false ;
      foreach ( prl2, pfn2->lrlLabels )
        if ( *prl1 == *prl2 ) bAny = true ;
      if ( bAny == false && pfn2->lrlLabels != List<RefLabel>() ) bEvery = false ;
      }
    if ( bEvery == true ) lrlLabels.Add() = *prl1 ;
    }

  // Transform loop into chain ending with copy...
  pfnL = pfnLoop->pfnMark ;
  pfnLoop->pfnMark = this ;

  // Copy each arc in each node in loop...
  for ( ; pfnL != NULL; pfnL = pfnL->pfnMark )
    {
    foreach ( pfa, pfnL->lfaArcs )
      {
      // Check if arc is already in copy...
      foreach ( pfaC, lfaArcs )
        if ( pfaC->plLabel == pfa->plLabel ) break ;

      // If arc is not already in copy, add it...
      if ( NULL == pfaC )
        {
        // Determine if destination is already a chain...
        for ( pfnD = pfa->pfnDest;
              pfnD->pfnMark != NULL && pfnD->pfnMark != pfa->pfnDest;
              pfnD = pfnD->pfnMark ) ;

        // If destination is already a chain, use copy at end of chain...
        if ( pfnD->pfnMark == NULL )
          lfaArcs.Add() = FeatureArc ( *pfa->plLabel, pfnD ) ;

        // Otherwise use new copy...
        else 
          {
          pfnNew = new FeatureNode ;
          lfaArcs.Add() = FeatureArc ( *pfa->plLabel, pfnNew ) ;
          pfnNew->CopyUnionDFS ( pfa->pfnDest ) ;
          }
        }
      }
    }
  }

//////////////////////////////////////////////////////////////////////

bool FeatureNode::SubsumeDFS ( FeatureNode* pfnLarger )
  {
  Listed(RefLabel)*   prl1 ;
  Listed(RefLabel)*   prl2 ;
  Listed(FeatureArc)* pfa1 ;
  Listed(FeatureArc)* pfa2 ;
  bool                bOut = true ;
  bool                bAny ;

  // If marked, return true if points to larger, false otherwise...
  if ( pfnMark != NULL ) return ( pfnMark == pfnLarger ) ;

  // Mark...
  pfnMark = pfnLarger ;

  // If smaller is wildcard, true...
  if ( lrlLabels == List<RefLabel>() && lfaArcs == List<FeatureArc>() )
    bOut = true ;

  // Otherwise, if larger is wildcard, fail...
  else if ( pfnLarger->lrlLabels == List<RefLabel>() && pfnLarger->lfaArcs == List<FeatureArc>() )
    bOut = false ;

  // Otherwise, if both have labels...
  else if ( lrlLabels != List<RefLabel>() && pfnLarger->lrlLabels != List<RefLabel>() )
    {
    // For every label in larger, make sure counterpart exists in smaller...
    foreach ( prl2, pfnLarger->lrlLabels )
      {
      bAny = false ;
      foreach ( prl1, lrlLabels )
        if ( *prl1 == *prl2 )
          bAny = true ;
      if ( !bAny ) bOut = false ;
      }
    }

  // Otherwise, if both have arcs...
  else if ( lfaArcs != List<FeatureArc>() && pfnLarger->lfaArcs != List<FeatureArc>() )
    {
    // For every arc in smaller, make sure counterpart exists in larger, and subsumes...
    foreach ( pfa1, lfaArcs )
      {
      bAny = false ;
      foreach ( pfa2, pfnLarger->lfaArcs )
        if ( pfa1->plLabel == pfa2->plLabel )
          bAny = pfa1->pfnDest->SubsumeDFS ( pfa2->pfnDest ) ;
      if ( !bAny ) bOut = false ;
      }
    }

  // Otherwise, both have different types of information, so fail...
  else bOut = false ;

  return bOut ;
  }

//////////////////////////////////////////////////////////////////////

void FeatureNode::PrintDFS ( File* pfOut )
  {
  Listed(RefLabel)*   prl ;
  Listed(FeatureArc)* pfa ;

  // Mark as non-null...
  pfnMark = this ;

  // Print header...
  fprintf ( pfOut, "%x: ", this ) ;

  // Print each label...
  foreach ( prl, lrlLabels )
    fprintf ( pfOut, "/%s", (RefLabel()==*prl) ? "EET" : prl->GetReferent().GetString() ) ;

  // Print each outgoing arc...
  foreach ( pfa, lfaArcs )
    fprintf ( pfOut, "%s=%x ", pfa->plLabel->GetString(), pfa->pfnDest ) ;

  // Print trailer...
  fprintf ( pfOut, "\n" ) ;

  // Recurse on each unvisited arc...
  foreach ( pfa, lfaArcs )
    if ( NULL == pfa->pfnDest->pfnMark )
      pfa->pfnDest->PrintDFS ( pfOut ) ;
  }

//////////////////////////////////////////////////////////////////////

void FeatureNode::EraseDFS ( )
  {
  Listed(FeatureArc)* pfa ;

  // Mark as non-null...
  pfnMark = this ;

  // Recurse on each unvisited arc...
  foreach ( pfa, lfaArcs )
    if ( NULL == pfa->pfnDest->pfnMark )
      pfa->pfnDest->EraseDFS ( ) ;

  // Add to free list...
  pfnMark = pfnFree ;
  pfnFree = this ;
  }

//////////////////////////////////////////////////////////////////////
//
//  Constructor and destructor methods...
//
//////////////////////////////////////////////////////////////////////

FeatureNode::FeatureNode ( )
  {
  lrlLabels = List<RefLabel> ( ) ;
  lfaArcs   = List<FeatureArc> ( ) ;
  pfnMark   = NULL ;

  #ifdef CHECKLEAKS //////////
  iNodes++ ;                //
  #endif /////////////////////
  }

FeatureNode::FeatureNode ( const FeatureNode& fn )
  {
  lrlLabels = List<RefLabel> ( ) ;
  lfaArcs   = List<FeatureArc> ( ) ;
  pfnMark   = NULL ;

  CopyDFS ( const_cast<FeatureNode*>(&fn) ) ;
  const_cast<FeatureNode*>(&fn)->UnmarkDFS ( ) ;

  #ifdef CHECKLEAKS //////////
  iNodes++ ;                //
  #endif /////////////////////
  }

FeatureNode::~FeatureNode ( )
  {
  // Run eraser one time only, leaving free list anchored at root...
  if ( (FeatureNode*)-1 == pfnFree ) EraseDFS ( ) ;

  // Deallocate free list, anchored at root...
  if ( (FeatureNode*)-1 != pfnMark ) delete pfnMark ;

  // Reset free list...
  pfnFree = (FeatureNode*)-1 ;

  #ifdef CHECKLEAKS //////////////////////////////
  iNodes-- ;                                    //
  if ( (FeatureNode*)-1 == pfnMark )            //
    fprintf ( stderr, "%d nodes.\n", iNodes ) ; //
  #endif /////////////////////////////////////////
  }

//////////////////////////////////////////////////////////////////////
//
//  Specification methods...
//
//////////////////////////////////////////////////////////////////////

FeatureNode& FeatureNode::operator= ( const FeatureNode& fn )
  {
  // Run eraser one time only, leaving free list anchored at root...
  if ( (FeatureNode*)-1 == pfnFree ) EraseDFS ( ) ;

  // Deallocate free list, anchored at root...
  if ( (FeatureNode*)-1 != pfnMark ) delete pfnMark ;

  // Reset free list...
  pfnFree = (FeatureNode*)-1 ;

  lrlLabels = List<RefLabel> ( ) ;
  lfaArcs   = List<FeatureArc> ( ) ;
  pfnMark   = NULL ;

  CopyDFS ( const_cast<FeatureNode*>(&fn) ) ;
  const_cast<FeatureNode*>(&fn)->UnmarkDFS ( ) ;

  return *this ;
  }

FeatureNode& FeatureNode::Set ( const FeatureNode& fn )
  {
  // Run eraser one time only, leaving free list anchored at root...
  if ( (FeatureNode*)-1 == pfnFree ) EraseDFS ( ) ;

  // Deallocate free list, anchored at root...
  if ( (FeatureNode*)-1 != pfnMark ) delete pfnMark ;

  // Reset free list...
  pfnFree = (FeatureNode*)-1 ;

  lrlLabels = List<RefLabel> ( ) ;
  lfaArcs   = List<FeatureArc> ( ) ;
  pfnMark   = NULL ;

  CopyDFS ( const_cast<FeatureNode*>(&fn) ) ;
  const_cast<FeatureNode*>(&fn)->UnmarkDFS ( ) ;

  return *this ;
  }

FeatureNode& FeatureNode::Set ( const FeatureNode& fn1, const FeatureNode& fn2 )
  {
  // Run eraser one time only, leaving free list anchored at root...
  if ( (FeatureNode*)-1 == pfnFree ) EraseDFS ( ) ;

  // Deallocate free list, anchored at root...
  if ( (FeatureNode*)-1 != pfnMark ) delete pfnMark ;

  // Reset free list...
  pfnFree = (FeatureNode*)-1 ;

  lrlLabels = List<RefLabel> ( ) ;
  lfaArcs   = List<FeatureArc> ( ) ;
  pfnMark   = NULL ;

  const_cast<FeatureNode*>(&fn1)->MarkDFS ( ) ;  
  const_cast<FeatureNode*>(&fn2)->MarkDFS ( ) ;  

  if ( true == const_cast<FeatureNode*>(&fn1)->UnifyDFS(const_cast<FeatureNode*>(&fn2)) ) 
    CopyUnionDFS ( const_cast<FeatureNode*>(&fn1) ) ;
  else *this = EET ;

  const_cast<FeatureNode*>(&fn1)->UnmarkDFS ( ) ;  
  const_cast<FeatureNode*>(&fn2)->UnmarkDFS ( ) ;  

  return *this ;
  }

bool FeatureNode::AddLabel ( const String* ps )
  {
  Listed(RefLabel)* prl ;
  RefLabel          rlNew ;

  // Get unique label code...
  rlNew.SetReferent ( salLabels.AddObject(ps) ) ;

  // Fail if node already has same label...
  foreach ( prl, lrlLabels )
    if ( rlNew == *prl )
      return false ;

  lrlLabels.Add() = rlNew ;
  return true ;
  }

FeatureNode& FeatureNode::AddFeature ( const String* ps )
  {
  Listed(FeatureArc)* pfa ;
  Label*              plNew ;
  FeatureNode*        pfn ;

  // Get unique label code...
  plNew = &salLabels.AddObject ( ps ) ;

  // If node already has same outgoing arc, return it's destination...
  foreach ( pfa, lfaArcs )
    if ( plNew == pfa->plLabel )
      return *pfa->pfnDest ;

  lfaArcs.Add() = FeatureArc ( *plNew, pfn = new FeatureNode ) ;
  return *pfn ;  
  }

bool FeatureNode::AddFeature ( const String* ps, const FeatureNode& fn )
  {
  Listed(FeatureArc)* pfa ;
  Label*              plNew ;

  // Get unique label code...
  plNew = &salLabels.AddObject ( ps ) ;

  // Fail if node already has same outgoing arc...
  foreach ( pfa, lfaArcs )
    if ( plNew == pfa->plLabel )
      return true ;

  lfaArcs.Add() = FeatureArc ( *plNew, const_cast<FeatureNode*>(&fn) ) ;
  return true ;  
  }

//////////////////////////////////////////////////////////////////////
//
//  Extraction methods...
//
//////////////////////////////////////////////////////////////////////

const List<RefLabel>& FeatureNode::GetLabels ( ) const
  {
  return lrlLabels ;
  }

const List<FeatureArc>& FeatureNode::GetFeatures ( ) const
  {
  return lfaArcs ;
  }

FeatureNode* FeatureNode::GetFeature ( const String* ps ) const
  {
  Listed(FeatureArc)* pfa ;
  Label*              plNew ;

  // Get unique label code...
  plNew = &salLabels.AddObject ( ps ) ;

  // Return node with same outgoing arc...
  foreach ( pfa, lfaArcs )
    if ( plNew == pfa->plLabel )
      return pfa->pfnDest ;

  return NULL ;
  }

bool FeatureNode::operator== ( const FeatureNode& fn ) const
  {
  return ( *const_cast<FeatureNode*>(this) <= *const_cast<FeatureNode*>(&fn) &&
           *const_cast<FeatureNode*>(&fn) <= *const_cast<FeatureNode*>(this) ) ;
  }

bool FeatureNode::operator!= ( const FeatureNode& fn ) const
  {
  return ( !operator==(fn) ) ;
  }

bool FeatureNode::operator<= ( const FeatureNode& fn ) const
  {
  bool bOut ;

  bOut = const_cast<FeatureNode*>(this)->SubsumeDFS ( const_cast<FeatureNode*>(&fn) ) ;
  const_cast<FeatureNode*>(this)->UnmarkDFS ( ) ;

  return bOut ;
  }

void FeatureNode::Print ( File* pfOut ) const
  {
  const_cast<FeatureNode*>(this)->PrintDFS ( pfOut ) ;
  const_cast<FeatureNode*>(this)->UnmarkDFS ( ) ;
  }

