/* test program for KMime::Codec's:
   compile with:
   g++ -I$QTDIR/include -I$KDEDIR/include -L$QTDIR/lib -L$KDEDIR/lib \
   -lqt-mt -lkdecore -lkdenetwork -O2 -pthread -DQT_THREAD_SUPPORT \
   -o test_kmime_codec{,.cpp}
*/

// return codes:
#define USAGE_DISPLAYED 1
#define UNKNOWN_CODEC 2
#define INFILE_READ_ERR 3
#define OUTFILE_WRITE_ERR 4

#include <../kmime_codecs.h>

#include <kdebug.h>

#include <cstdlib>
#include <iostream>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>
#include <cassert>

#include <qfile.h>
#include <qcstring.h> // QByteArray

using namespace KMime;
using namespace std;

static struct option long_options[] = {
  { "encode", 1, 0, 0 },
  { "decode", 1, 0, 0 },
  { "output-buffer-size", 1, 0, 0 },
  { "input-buffer-size", 1, 0, 0 },
  { "outfile", 1, 0, 0 },
  { "with-crlf", 0, 0, 0 },
  { "iterations", 1, 0, 0 },
  { "without-finish", 0, 0, 0 },
  { "verbose", 0, 0, 0 },
  { "usage-pattern", 1, 0, 0 },
  { 0, 0, 0, 0 }
};

void usage( const char * msg=0 ) {
  if ( msg && *msg )
    cerr << msg << endl;
  cerr << "usage: test_kmime_codec (--encode|--decode) "
    "<encoding-name> [options] infile\n"
    "where options include:\n\n"
    " --outfile <outfile>          write output into file <outfile>\n"
    " --output-buffer-size <size>  en/decode into chunks of <size> bytes\n"
    "                              default: 4096\n"
    " --input-buffer-size <size>   en/decode from chunks of <size> bytes\n"
    "                              default: slurp in whole file\n"
    " --with-crlf                  use CRLF instead of LF in output\n"
    " --iterations <number>        do more than one iteration\n"
    "                              default: 1\n"
    " --usage-pattern { kio | chunkwise | convenience-qba }\n"
    "                              use a certain usage pattern to be tested\n"
    "                              (default: chunkwise)\n"
    " --without-finish             don't call the finish() method\n"
    " --verbose                    output detailed progress information\n"
       << endl;
  exit(USAGE_DISPLAYED);
}

void missingParameterTo( const char * option ) {
  cerr << "Missing or malformed parameter to " << option << endl;
  usage();
}

static enum { Kio = 0, ChunkWise = 1, ConvenienceQBA = 3 }
pattern = ChunkWise;
static int outbufsize = 4096;
static int inbufsize = -1; // whole file
static bool writing = false;
static bool withCRLF = false;
static bool withFinish = true;
static bool verbose = false;

void encode_decode_kio( bool, const Codec *, const QByteArray &, QFile & );
void encode_decode_chunkwise( bool, const Codec *,
			      const QByteArray &, QFile & );
void encode_decode_convenience_qba( bool, const Codec *, const QByteArray &,
				    QFile & );

int main( int argc, char * argv[] ) {

  int iterations = 1;
  bool encode = false;
  bool decode = false;
  QCString outfilename, infilename;
  QCString encodingName;

  // options parsing:
  while( 1 ) {
    int option_index = 0;
    if ( getopt_long( argc, argv, "", long_options, &option_index ) )
      break;
    switch ( option_index ) {
    case 0: // encode
      if ( !optarg || !*optarg ) missingParameterTo( "--encode." );
      encode = true;
      encodingName = QCString(optarg);
      break;
    case 1: // decode
      if ( !optarg || !*optarg ) missingParameterTo( "--decode" );
      decode = true;
      encodingName = QCString(optarg);
      break;
    case 2: // output-buffer-size
      if ( !optarg || (outbufsize = atoi( optarg )) < 1 )
	missingParameterTo( "--output-buffer-size" );
      break;
    case 3: // input-buffer-size
      if ( !optarg || (inbufsize = atoi( optarg )) < 1 )
	missingParameterTo( "--input-buffer-size" );
      break;
    case 4: // outfile
      if ( !optarg || !*optarg ) missingParameterTo( "--outfile" );
      outfilename = QCString(optarg);
      writing = true;
      break;
    case 5: // with-crlf
      withCRLF = true;
      break;
    case 6: // iterations
      if ( !optarg || (iterations = atoi( optarg )) < 1 )
	missingParameterTo( "--iterations" );
      break;
    case 7: // without-finish
      withFinish = false;
      break;
    case 8: // verbose
      verbose = true;
      break;
    case 9: // usage-pattern
      if ( !qstricmp( "kio", optarg ) )
	pattern = Kio;
      else if ( !qstricmp( "chunkwise", optarg ) )
	pattern = ChunkWise;
      else if ( !qstricmp( "convenience-qba", optarg ) )
	pattern = ConvenienceQBA;
      else {
	cerr << "Unknown usage pattern \"" << optarg << "\"" << endl;
	usage();
      }
      break;
    default: usage( "Unknown option" );
    }
  }

  if ( !decode && !encode )
    usage( "You must specify exactly one of --encode, --decode." );
  if ( decode && encode )
    usage( "You must specify exactly one of --encode, --decode.");

  if ( verbose ) {
    if ( encode )
      kdDebug() << "encoding as " << encodingName << endl;
    else if ( decode )
      kdDebug() << "decoding " << encodingName << endl;
  }

  if ( optind != argc - 1 ) usage();

  QFile infile( argv[ optind ] );
  if (!infile.exists()) {
    kdDebug() << "infile \"" << infile.name() << "\" does not exist!" << endl;
    return INFILE_READ_ERR;
  }
  if (!infile.open( IO_ReadOnly )) {
    kdDebug() << "cannot open " << infile.name() << " for reading!"
	      << endl;
    return INFILE_READ_ERR;
  }

  QFile outfile( outfilename );
  if ( !outfilename.isEmpty() ) {
    if (!outfile.open( IO_WriteOnly|IO_Truncate )) {
      kdDebug() << "cannot open " << outfile.name() << " for writing!"
		<< endl;
      return OUTFILE_WRITE_ERR;
    }
  }

  if ( verbose ) {
    kdDebug() << "using output buffer size of " << outbufsize << endl;
    kdDebug() << "using  input buffer size of " << inbufsize << endl;
  }
  if ( !withFinish )
    kdWarning() << "omitting finish calls. Results may be truncated!" << endl;

  if ( inbufsize <= 0 )
    inbufsize = infile.size();

  // get a codec. Don't delete it later!!
  kdDebug( verbose ) << "obtaining codec for \""
		     << encodingName << "\"" << endl;
  Codec * codec = Codec::codecForName( encodingName );
  if ( !codec ) {
    kdDebug() << "unknown codec \"" << encodingName << "\"" << endl;
    return UNKNOWN_CODEC;
  }

  QByteArray infile_buffer = infile.readAll();

  for ( int i = 0 ; i < iterations ; ++i ) {
    kdDebug( verbose ) << "starting iteration " << i+1
		       << " of " << iterations << endl;
    switch ( pattern ) {
    case ChunkWise:
      encode_decode_chunkwise( encode, codec, infile_buffer, outfile );
      break;
    case Kio:
      encode_decode_kio( encode, codec, infile_buffer, outfile );
      break;
    case ConvenienceQBA:
      encode_decode_convenience_qba( encode, codec, infile_buffer, outfile );
      break;
    default:
      usage();
    }
  }

  return 0;
}

void encode_decode_convenience_qba( bool encode, const Codec * codec,
				    const QByteArray & infile_buffer,
				    QFile & outfile )
{
  QByteArray out;
  if ( encode )
    out = codec->encode( infile_buffer, withCRLF );
  else
    out = codec->decode( infile_buffer, withCRLF );
  if ( writing ) {
    Q_LONG written = outfile.writeBlock( out );
    assert( written == (Q_LONG)out.size() );
  }
}

void encode_kio_internal( Encoder * enc, QByteArray::ConstIterator & iit,
			  QByteArray::ConstIterator & iend,
			  QByteArray & out )
{
  out.resize( outbufsize );
  QByteArray::Iterator oit = out.begin();
  QByteArray::ConstIterator oend = out.end();

  while ( !enc->encode( iit, iend, oit, oend ) )
    if ( oit == oend ) return;

  while ( !enc->finish( oit, oend ) )
    if ( oit == oend ) return;

  out.truncate( oit - out.begin() );
}

void decode_kio_internal( Decoder * dec, QByteArray::ConstIterator & iit,
			  QByteArray::ConstIterator & iend,
			  QByteArray & out ) {
  out.resize( outbufsize );
  QByteArray::Iterator oit = out.begin();
  QByteArray::ConstIterator oend = out.end();

  while ( !dec->decode( iit, iend, oit, oend ) )
    if ( oit == oend ) return;

  while ( !dec->finish( oit, oend ) )
    if ( oit == oend ) return;

  out.truncate( oit - out.begin() );
}

void encode_decode_kio( bool encode, const Codec * codec,
			const QByteArray & infile_buffer, QFile & outfile )
{

  Encoder * enc = 0;
  Decoder * dec = 0;

  // Get an encoder. This one you have to delete!
  if ( encode ) {
    enc = codec->makeEncoder( withCRLF );
    assert( enc );
  } else {
    dec = codec->makeDecoder( withCRLF );
    assert( dec );
  }

  QByteArray::ConstIterator iit = infile_buffer.begin();
  QByteArray::ConstIterator iend = infile_buffer.end();

  QByteArray out;
  do {
    out = QByteArray();
    if ( encode )
      encode_kio_internal( enc, iit, iend, out );
    else
      decode_kio_internal( dec, iit, iend, out );
    if ( writing && out.size() ) {
      Q_LONG written = outfile.writeBlock( out );
      assert( written == (Q_LONG)out.size() );
    }
  } while ( out.size() );

  if ( encode )
    delete enc;
  else
    delete dec;
}

void encode_decode_chunkwise( bool encode, const Codec * codec,
			      const QByteArray & infile_buffer, QFile & outfile )
{
  Encoder * enc = 0;
  Decoder * dec = 0;


  QByteArray indata( inbufsize );
  QByteArray outdata( outbufsize );

  // we're going to need this below:
#define write_full_outdata_then_reset  do { \
     kdDebug( verbose ) << "  flushing output buffer." << endl; \
     if ( writing ) { \
       Q_LONG outlen = outfile.writeBlock( outdata.data(), \
					   outdata.size() ); \
       if ( outlen != (int)outdata.size() ) \
         exit(OUTFILE_WRITE_ERR); \
     } \
     oit = outdata.begin(); \
   } while ( false )

#define report_status(x,y) do { \
     kdDebug( verbose ) << "  " #x "() returned " #y " after processing " \
                        << iit - indata.begin() << " bytes of input.\n" \
			<< "   output iterator now at position " \
			<< oit - outdata.begin() << " of " \
			<< outdata.size() << endl; \
  } while ( false )

#define report_finish_status(y) do { \
     kdDebug( verbose ) << "  finish() returned " #y "\n" \
			<< "   output iterator now at position " \
			<< oit - outdata.begin() << " of " \
			<< outdata.size() << endl; \
  } while ( false )


  // Initialize the output iterators:
  QByteArray::Iterator oit = outdata.begin();
  QByteArray::Iterator oend = outdata.end();

  // Get an encoder. This one you have to delete!
  if ( encode ) {
    enc = codec->makeEncoder( withCRLF );
    assert( enc );
  } else {
    dec = codec->makeDecoder( withCRLF );
    assert( dec );
  }

  //
  // Loop over input chunks:
  //
  uint offset = 0;
  while ( offset < infile_buffer.size() ) {
    uint reallyRead = QMIN( indata.size(), infile_buffer.size() - offset );
    indata.duplicate( infile_buffer.begin() + offset, reallyRead );
    offset += reallyRead;

    kdDebug( verbose ) << " read " << reallyRead << " bytes (max: "
		       << indata.size() << ") from input." << endl;
    
    // setup input iterators:
    QByteArray::ConstIterator iit = indata.begin();
    QByteArray::ConstIterator iend = indata.begin() + reallyRead;
    
    if ( encode ) {
      //
      // Loop over encode() calls:
      //
      while ( !enc->encode( iit, iend, oit, oend ) ) {
	report_status( encode, false );
	if ( oit == oend )
	  // output buffer full:
	  write_full_outdata_then_reset;
      }
      report_status( encode, true );
    } else {
      //
      // Loop over decode() calls:
      //
      while ( !dec->decode( iit, iend, oit, oend ) ) {
	report_status( decode, false );
	if ( oit == oend )
	  // output buffer full:
	  write_full_outdata_then_reset;
      }
      report_status( decode, true );
    }
  } // end loop over input chunks

  //
  // Now finish the encoding/decoding:
  // (same loops as above, just s/encode|decode/finish())
  //
  if ( withFinish )
    if ( encode ) {
      while ( !enc->finish( oit, oend ) ) {
	report_finish_status( false );
	if ( oit == oend )
	  write_full_outdata_then_reset;
      }
      report_finish_status( true );
    } else {
      while ( !dec->finish( oit, oend ) ) {
	report_finish_status( false );
	if ( oit == oend )
	  write_full_outdata_then_reset;
      }
      report_finish_status( true );
    }
  
  //
  // Write out last (partial) output chunk:
  //
  if ( writing ) {
    Q_LONG outlen = outfile.writeBlock( outdata.data(),
					oit - outdata.begin() );
    if ( outlen != oit - outdata.begin() )
      exit(OUTFILE_WRITE_ERR);
  }
  
  //
  // Delete en/decoder:
  //
  if ( encode )
    delete enc;
  else
    delete dec;
}

