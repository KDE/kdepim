// test program for KMime::CharFreq.
// compile with g++ -I$QTDIR/include -L$QTDIR/lib -lqt(-mt) \
//                  -o test_charfreq test_charfreq.cpp

#include "../kmime_charfreq.cpp"

#include <iostream>

#include <qfile.h>

using namespace std;
using namespace KMime;

static const char * typeToString( int type ) {
  switch ( type ) {
  case CharFreq::EightBitData:
    return "eight bit data (binary)";
  case CharFreq::EightBitText:
    return "eight bit text";
  case CharFreq::SevenBitData:
    return "seven bit data";
  case CharFreq::SevenBitText:
    return "seven bit text";
  default:
    return "unknown type";
  }
}

int main( int argc, char **argv ) {
  for ( int i = 1 /*not program*/ ; i < argc ; i++ ) {
    QFile in( argv[i] );
    if ( !in.open( IO_ReadOnly ) ) {
      cerr << argv[i] << ": does not exist!" << endl;
      continue;
    }
    QByteArray ba = in.readAll();
    CharFreq cf( ba );
    cout << argv[i] << ": " << typeToString(cf.type()) << endl;
  }
  return 0;
}
