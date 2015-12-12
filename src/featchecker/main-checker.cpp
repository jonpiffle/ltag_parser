
#include "nl-chkr-headers.h"

int main ( int iArgs, String* psArgs[] )
  {
  FeatTable       ftLex ;
  FeatTable       ftGram ;
  File*           pfLex ;
  File*           pfGram ;
  int             c ;
  FeatStruct*     pfs ;
  Forest*         pf ;

  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  ftLex.Init  ( 1000 ) ;
  ftGram.Init ( 2000 ) ;

  // If wrong number of arguments, correct input...
  if ( iArgs < 3 )
    printf ( "\nusage: %s <lexical features file> <grammar features file>\n\n", psArgs[0] ) ;
  else
    {
    // Load lexical features...
    pfLex = fopen ( psArgs[1], "r" ) ;
    if ( !pfLex ) fprintf ( stderr, "\nERROR: File %s not found.\n\n", psArgs[1] ) ;
    else ftLex.Read ( pfLex, c=' ' ) ;
    fprintf ( stderr, "\nLexical features loaded.\n\n" ) ;

    // Load grammar features...
    pfGram = fopen ( psArgs[2], "r" ) ;
    if ( !pfGram ) fprintf ( stderr, "\nERROR: File %s not found.\n\n", psArgs[2] ) ;
    else ftGram.Read ( pfGram, c=' ' ) ;
    fprintf ( stderr, "\nGrammar features loaded.\n\n" ) ;

    // Read each derivation in standard input...
    for ( c=getc(stdin); c!=EOF; c=getc(stdin))
      {
      pf = new Forest ;
      pf->Init ( 15000 ) ;
      pf->Read ( stdin, c, ftLex, ftGram ) ;
      pf->Write ( stdout ) ;
      delete pf ;
      }
    }
    return 0;
  }



