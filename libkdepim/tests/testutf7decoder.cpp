#include "qutf7codec.h"
#include "qutf7codec.cpp"
#include <tqtextstream.h>
#include <string.h>
#include <assert.h>

int main( int argc, char * argv[] ) {
  if ( argc == 1 ) {
    (void)new QUtf7Codec;

    TQTextCodec * codec = TQTextCodec::codecForName("utf-7");
    assert(codec);

    TQTextIStream my_cin(stdin);
    my_cin.setCodec(codec);

    TQTextOStream my_cout(stdout);
    
    TQString buffer = my_cin.read();

    my_cout << buffer;
  } else {
    qWarning("usage: testutf7decoder string_to_decode\n");
  }
  TQTextCodec::deleteAllCodecs();
}
