#include <kmime_mdn.h>
using namespace KMime::MDN;

#include <qcstring.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <iostream>
using std::cout;
using std::cerr;
#include <cstdlib>
using std::exit;
using std::endl;

#define _GNU_SOURCE 1
#include <getopt.h>

void usage( const char * msg=0 ) {
  if ( msg )
    cerr << msg << endl;
  cerr << "usage: test_mdn <options>\n"
    "where options include the following:" << endl
       << "FIXME" << endl;
  exit( 1 );
}

int main( int argc, char * argv[] ) {

  QString finalRecipient;
  QString originalRecipient;
  QCString originalMessageId;
  ActionMode actionMode = ManualAction;
  SendingMode sendingMode = SentManually;
  DispositionType dispositionType = Displayed;
  QValueList<DispositionModifier> dispositionModifiers;
  QString special;

  while ( true ) {
    int option_index = 0;
    static const struct option long_options[] = {
      { "action-mode", 1, 0, 'a' },
      { "disposition-type", 1, 0, 'd' },
      { "final-recipient", 1, 0, 'f' },
      { "original-message-id", 1, 0, 'i' },
      { "disposition-modifiers", 1, 0, 'm' },
      { "original-recipient", 1, 0, 'o' },
      { "sending-mode", 1, 0, 's' },
      { 0, 0, 0, 0 }
    };

    int c = getopt_long( argc, argv, "a:d:f:i:m:o:s:",
			 long_options, &option_index );
    if ( c == -1 ) break;

#define EQUALS(x) !qstricmp( optarg, x )

    switch ( c ) {

    case 'a': // --action-mode
      if ( EQUALS( "manual-action" ) )
	actionMode = ManualAction;
      else if ( EQUALS( "automatic-action" ) )
	actionMode = AutomaticAction;
      else
	usage( "unknown action mode!" );
      break;

    case 'd': // --disposition-type
      if ( EQUALS( "displayed" ) )
	dispositionType = Displayed;
      else if ( EQUALS( "deleted" ) )
	dispositionType = Deleted;
      else if ( EQUALS( "dispatched" ) )
	dispositionType = Dispatched;
      else if ( EQUALS( "processed" ) )
	dispositionType = Processed;
      else if ( EQUALS( "denied" ) )
	dispositionType = Denied;
      else if ( EQUALS( "failed" ) )
	dispositionType = Failed;
      else
	usage( "unknown disposition type!" );
      break;

    case 'f': // --final-recipient
      if ( optarg && *optarg )
	finalRecipient = QString::fromUtf8( optarg );
      else
	usage( "--final-recipient is missing a value" );
      break;

    case 'i': // --original-message-id
      if ( optarg && *optarg )
	originalMessageId = optarg;
      else
	usage( "--original-message-id is missing a value" );
      break;

    case 'm': // --disposition-modifier
      if ( EQUALS( "error" ) )
	dispositionModifiers << Error;
      else if ( EQUALS( "warning" ) )
	dispositionModifiers << Warning;
      else if ( EQUALS( "superseded" ) )
	dispositionModifiers << Superseded;
      else if ( EQUALS( "expired" ) )
	dispositionModifiers << Expired;
      else if ( EQUALS( "mailbox-terminated" ) )
	dispositionModifiers << MailboxTerminated;
      else
	usage( "unknwon disposition modifier!" );
      break;

    case 'o': // --original-recipient
      if ( optarg && *optarg )
	originalRecipient = QString::fromUtf8( optarg );
      else
	usage( "--original-recipient is missing a value" );
      break;

    case 's': // --sending-mode
      if ( EQUALS( "MDN-sent-manually" ) )
	sendingMode = SentManually;
      else if ( EQUALS( "MDN-sent-automatically" ) )
	sendingMode = SentAutomatically;
      else
	usage( "unknown sending mode" );
      break;

    default:
      usage( "unknown option encountered!" );
    }
  }

  if ( optind < argc )
    special = QString::fromUtf8( argv[optind++] );
  if ( optind < argc )
    usage( "too many arguments!" );

  QCString result = dispositionNotificationBodyContent( finalRecipient,
						       originalRecipient.latin1(),
						       originalMessageId,
						       dispositionType,
						       actionMode,
						       sendingMode,
						       dispositionModifiers,
						       special );
  cout << "Result:\n" << result.data();

  return 0;
}
