#include <mimelib/boyermor.h>
#include <mimelib/string.h>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

static const char * _haystack =
  "haystackNeedleHaystackneedlehaYstackneeDlehaystack";

int main( int argc, char * argv[] ) {

  if ( argc == 3 ) { // manual test
    DwString needle( argv[1] );
    DwString haystack( argv[2] );

    DwBoyerMoore csbm( needle ); // case-sensitive
    DwBoyerMoore cisbm( needle ); // case-insensitive

    cout << "Case-sensitive search found ";
    for ( size_t idx = 0 ; ( idx = csbm.FindIn( haystack, idx ) ) != DwString::npos ; ++idx )
      cout << (int)idx << " ";
    cout << endl;
    cout << "Case-insensitive search found ";
    for ( size_t idx = 0 ; ( idx = cisbm.FindIn( haystack, idx, false ) ) != DwString::npos ; ++idx )
      cout << (int)idx << " ";
    cout << endl;
    exit( 0 );
  } else if ( argc == 1 ) { // automated test
    DwString haystack( _haystack );

    DwBoyerMoore needle_cs( "needle" );
    DwBoyerMoore needle_cis( "needle" );
    DwBoyerMoore Needle_cs( "Needle" );
    DwBoyerMoore Needle_cis( "Needle" );
    DwBoyerMoore neeDle_cs( "neeDle" );
    DwBoyerMoore neeDle_cis( "neeDle" );

    assert( needle_cs.FindIn( haystack, 0 ) == 22 );
    assert( needle_cs.FindIn( haystack, 23 ) == DwString::npos );

    assert( needle_cis.FindIn( haystack, 0, false ) == 8 );
    assert( needle_cis.FindIn( haystack, 9, false  ) == 22 );
    assert( needle_cis.FindIn( haystack, 23, false  ) == 36 );
    assert( needle_cis.FindIn( haystack, 37, false  ) == DwString::npos );

    assert( Needle_cs.FindIn( haystack, 0 ) == 8 );
    assert( Needle_cs.FindIn( haystack, 9 ) == DwString::npos );

    assert( Needle_cis.FindIn( haystack, 0, false  ) == 8 );
    assert( Needle_cis.FindIn( haystack, 9, false  ) == 22 );
    assert( Needle_cis.FindIn( haystack, 23, false  ) == 36 );
    assert( Needle_cis.FindIn( haystack, 37, false  ) == DwString::npos );

    assert( neeDle_cs.FindIn( haystack, 0 ) == 36 );
    assert( neeDle_cs.FindIn( haystack, 37 ) == DwString::npos );

    assert( neeDle_cis.FindIn( haystack, 0, false  ) == 8 );
    assert( neeDle_cis.FindIn( haystack, 9, false  ) == 22 );
    assert( neeDle_cis.FindIn( haystack, 23, false  ) == 36 );
    assert( neeDle_cis.FindIn( haystack, 37, false  ) == DwString::npos );

  } else {
    cerr << "usage: test_boyermor [ <needle> <haystack> ]" << endl;
    exit( 1 );
  }

  return 0;
};
