/*
    kpgpbase5.cpp

    Copyright (C) 2001,2002 the KPGP authors
    See file AUTHORS.kpgp for details

    This file is part of KPGP, the KDE PGP/GnuPG support library.

    KPGP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kpgpbase.h"
#include "kpgp.h"

#include <string.h> /* strncmp */

#include <qregexp.h>
#include <qdatetime.h>

#include <klocale.h>
#include <kprocess.h>
#include <kdebug.h>


namespace Kpgp {

Base5::Base5()
  : Base()
{
}


Base5::~Base5()
{
}


int
Base5::encrypt( Block& block, const KeyIDList& recipients )
{
  return encsign( block, recipients, 0 );
}


int
Base5::clearsign( Block& block, const char *passphrase )
{
  return encsign( block, KeyIDList(), passphrase );
}


int
Base5::encsign( Block& block, const KeyIDList& recipients,
                const char *passphrase )
{
  QCString cmd;
  int exitStatus = 0;
  int index;
  // used to work around a bug in pgp5. pgp5 treats files
  // with non ascii chars (umlauts, etc...) as binary files, but
  // we want a clear signature
  bool signonly = false;

  if(!recipients.isEmpty() && passphrase != 0)
    cmd = "pgpe +batchmode -afts ";
  else if(!recipients.isEmpty())
    cmd = "pgpe +batchmode -aft ";
  else if(passphrase != 0)
  {
    cmd = "pgps +batchmode -abft ";
    signonly = true;
  }
  else
  {
    errMsg = i18n("Neither recipients nor passphrase specified.");
    return OK;
  }

  if(passphrase != 0)
    cmd += addUserId();

  if(!recipients.isEmpty())
  {
    if(Module::getKpgp()->encryptToSelf())
    {
      cmd += " -r 0x";
      cmd += Module::getKpgp()->user();
    }

    for( KeyIDList::ConstIterator it = recipients.begin();
         it != recipients.end(); ++it ) {
      cmd += " -r 0x";
      cmd += (*it);
    }
  }

  clear();
  input = block.text();

  if (signonly)
  {
    input.append("\n");
    input.replace(QRegExp("[ \t]+\n"), "\n");   //strip trailing whitespace
  }
  //We have to do this otherwise it's all in vain

  exitStatus = run(cmd.data(), passphrase);
  block.setError( error );

  if(exitStatus != 0)
    status = ERROR;

  // now parse the returned info
  if(error.find("Cannot unlock private key") != -1)
  {
    errMsg = i18n("The passphrase you entered is invalid.");
    status |= ERROR;
    status |= BADPHRASE;
  }
//if(!ignoreUntrusted)
//{
    QCString aStr;
    index = -1;
    while((index = error.find("WARNING: The above key",index+1)) != -1)
    {
      int index2 = error.find("But you previously",index);
      int index3 = error.find("WARNING: The above key",index+1);
      if(index2 == -1 || (index2 > index3 && index3 != -1))
      {
	// the key wasn't valid, no encryption to this person
	// extract the person
	index2 = error.find('\n',index);
	index3 = error.find('\n',index2+1);
	aStr += error.mid(index2+1, index3-index2-1);
	aStr += ", ";
      }
    }
    if(!aStr.isEmpty())
    {
      aStr.truncate(aStr.length()-2);
      if(error.find("No valid keys found") != -1)
	errMsg = i18n("The key(s) you want to encrypt your message "
		      "to are not trusted. No encryption done.");
      else
	errMsg = i18n("The following key(s) are not trusted:\n%1\n"
                      "Their owner(s) will not be able to decrypt the message.")
		     .arg(aStr);
      status |= ERROR;
      status |= BADKEYS;
    }
//}
  if((index = error.find("No encryption keys found for")) != -1)
  {
    index = error.find(':',index);
    int index2 = error.find('\n',index);

    errMsg = i18n("Missing encryption key(s) for:\n%1")
      .arg(error.mid(index,index2-index));
//    errMsg = QString("Missing encryption key(s) for: %1")
//      .arg(error.mid(index,index2-index));
    status |= ERROR;
    status |= MISSINGKEY;
  }

  if(signonly) {
    // dash-escape the input
    if (input[0] == '-')
      input = "- " + input;
    for ( int idx = 0 ; (idx = input.find("\n-", idx)) >= 0 ; idx += 4 )
      input.replace(idx, 2, "\n- -");

    output = "-----BEGIN PGP SIGNED MESSAGE-----\n\n" + input + "\n" + output;
  }

  block.setProcessedText( output );
  block.setStatus( status );
  return status;
}


int
Base5::decrypt( Block& block, const char *passphrase )
{
  int exitStatus = 0;

  clear();
  input = block.text();
  exitStatus = run("pgpv -f +batchmode=1", passphrase);
  if( !output.isEmpty() )
    block.setProcessedText( output );
  block.setError( error );

  if(exitStatus == -1) {
    errMsg = i18n("Error running PGP");
    status = RUN_ERR;
    block.setStatus( status );
    return status;
  }

  // lets parse the returned information.
  int index;

  index = error.find("Cannot decrypt message");
  if(index != -1)
  {
    //kdDebug(5100) << "message is encrypted" << endl;
    status |= ENCRYPTED;

    // ok. we have an encrypted message. Is the passphrase bad,
    // or do we not have the secret key?
    if(error.find("Need a pass phrase") != -1)
    {
      if(passphrase != 0)
      {
	errMsg = i18n("Bad passphrase; could not decrypt.");
	kdDebug(5100) << "Base: passphrase is bad" << endl;
	status |= BADPHRASE;
	status |= ERROR;
      }
    }
    else
    {
      // we don't have the secret key
      status |= NO_SEC_KEY;
      status |= ERROR;
      errMsg = i18n("You do not have the secret key needed to decrypt this message.");
      kdDebug(5100) << "Base: no secret key for this message" << endl;
    }
    // check for persons
#if 0
    // ##### FIXME: This information is anyway currently not used
    //       I'll change it to always determine the recipients.
    index = error.find("can only be decrypted by:");
    if(index != -1)
    {
      index = error.find('\n',index);
      int end = error.find("\n\n",index);

      mRecipients.clear();
      int index2;
      while( (index2 = error.find('\n',index+1)) <= end )
      {
	QCString item = error.mid(index+1,index2-index-1);
	item.stripWhiteSpace();
	mRecipients.append(item);
	index = index2;
      }
    }
#endif
  }
  index = error.find("Good signature");
  if(index != -1)
  {
    //kdDebug(5100) << "good signature" << endl;
    status |= SIGNED;
    status |= GOODSIG;

    // get key ID of signer
    index = error.find("Key ID ", index) + 7;
    block.setSignatureKeyId( error.mid(index, 8) );

    // get signer
    index = error.find('"',index) + 1;
    int index2 = error.find('"', index);
    block.setSignatureUserId( error.mid(index, index2-index) );

    /// ### FIXME get signature date
    block.setSignatureDate( "" );
  }
  index = error.find("BAD signature");
  if(index != -1)
  {
    //kdDebug(5100) << "BAD signature" << endl;
    status |= SIGNED;
    status |= ERROR;

    // get key ID of signer
    index = error.find("Key ID ", index) + 7;
    block.setSignatureKeyId( error.mid(index, 8) );

    // get signer
    index = error.find('"',index) + 1;
    int index2 = error.find('"', index);
    block.setSignatureUserId( error.mid(index, index2-index) );

    /// ### FIXME get signature date
    block.setSignatureDate( "" );
  }
  index = error.find("Signature by unknown key");
  if(index != -1)
  {
    index = error.find("keyid: 0x",index) + 9;
    block.setSignatureKeyId( error.mid(index, 8) );
    block.setSignatureUserId( QString::null );
    // FIXME: not a very good solution...
    status |= SIGNED;
    status |= GOODSIG;

    /// ### FIXME get signature date
    block.setSignatureDate( "" );
  }

  //kdDebug(5100) << "status = " << status << endl;
  block.setStatus( status );
  return status;
}


Key*
Base5::readPublicKey( const KeyID& keyId, const bool readTrust, Key* key )
{
  int exitStatus = 0;

  status = 0;
  exitStatus = run( "pgpk -ll 0x" + keyId, 0, true );

  if(exitStatus != 0) {
    status = ERROR;
    return 0;
  }

  key = parseSingleKey( output, key );

  if( key == 0 )
  {
    return 0;
  }

  if( readTrust )
  {
    exitStatus = run( "pgpk -c 0x" + keyId, 0, true );

    if(exitStatus != 0) {
      status = ERROR;
      return 0;
    }

    parseTrustDataForKey( key, output );
  }

  return key;
}


KeyList
Base5::publicKeys( const QStringList & patterns )
{
  int exitStatus = 0;

  QCString cmd = "pgpk -ll";
  for ( QStringList::ConstIterator it = patterns.begin();
        it != patterns.end(); ++it ) {
    cmd += " ";
    cmd += KProcess::quote( *it ).local8Bit();
  }
  status = 0;
  exitStatus = run( cmd, 0, true );

  if(exitStatus != 0) {
    status = ERROR;
    return KeyList();
  }

  // now we need to parse the output for public keys
  KeyList keys = parseKeyList( output, false );

  // sort the list of public keys
  keys.sort();

  return keys;
}


KeyList
Base5::secretKeys( const QStringList & patterns )
{
  int exitStatus = 0;

  status = 0;
  QCString cmd = "pgpk -ll";
  for ( QStringList::ConstIterator it = patterns.begin();
        it != patterns.end(); ++it ) {
    cmd += " ";
    cmd += KProcess::quote( *it ).local8Bit();
  }
  status = 0;
  exitStatus = run( cmd, 0, true );

  if(exitStatus != 0) {
    status = ERROR;
    return KeyList();
  }

  // now we need to parse the output for secret keys
  KeyList keys = parseKeyList( output, true );

  // sort the list of public keys
  keys.sort();

  return keys;
}


QCString Base5::getAsciiPublicKey(const KeyID& keyID)
{
  int exitStatus = 0;

  if (keyID.isEmpty())
    return QCString();

  status = 0;
  exitStatus = run( "pgpk -xa 0x" + keyID, 0, true );

  if(exitStatus != 0) {
    status = ERROR;
    return QCString();
  }

  return output;
}


int
Base5::signKey(const KeyID& keyID, const char *passphrase)
{
  QCString cmd;
  int exitStatus = 0;

  if(passphrase == 0) return false;

  cmd = "pgpk -s -f +batchmode=1 0x";
  cmd += keyID;
  cmd += addUserId();

  status = 0;
  exitStatus = run(cmd.data(), passphrase);

  if (exitStatus != 0)
    status = ERROR;

  return status;
}

//-- private functions --------------------------------------------------------

Key*
Base5::parseKeyData( const QCString& output, int& offset, Key* key /* = 0 */ )
// This function parses the data for a single key which is output by PGP 5
// with the following command line:
//   pgpk -ll
// It expects the key data to start at offset and returns the start of
// the next key's data in offset.
{
  if( ( strncmp( output.data() + offset, "pub", 3 ) != 0 ) &&
      ( strncmp( output.data() + offset, "sec", 3 ) != 0 ) )
  {
    kdDebug(5100) << "Unknown key type or corrupt key data.\n";
    return 0;
  }

  if( key == 0 )
    key = new Key();
  else
    key->clear();

  Subkey *subkey = 0;
  bool primaryKey = true;

  while( true )
  {
    int eol;

    // search the end of the current line
    eol = output.find( '\n', offset );
    if( ( eol == -1 ) || ( eol == offset ) )
      break;

    //kdDebug(5100) << "Parsing: " << output.mid(offset, eol-offset) << endl;

    if( !strncmp( output.data() + offset, "pub", 3 ) ||
        !strncmp( output.data() + offset, "sec", 3 ) ||
        !strncmp( output.data() + offset, "sub", 3 ) )
    { // line contains key data
      //kdDebug(5100)<<"Key data:\n";
      int pos, pos2;

      subkey = new Subkey( "", false );
      key->addSubkey( subkey );

      // Key Flags
      /* From the PGP 5 manual page for pgpk:
         Following this column is a single  character  which
         describes other attributes of the object:

         @  The object is disabled
         +  The object is axiomatically trusted  (i.e.,  it's
            your key)
      */
      switch( output[offset+3] )
      {
      case ' ': // nothing special
        break;
      case '@': // disabled key
        subkey->setDisabled( true );
        key->setDisabled( true );
        break;
      default: // all other flags are ignored
        //kdDebug(5100) << "Unknown key flag.\n";
        ;
      }

      // Key Length
      pos = offset + 4;
      while( output[pos] == ' ' )
        pos++;
      pos2 = output.find( ' ', pos );
      subkey->setKeyLength( output.mid( pos, pos2-pos ).toUInt() );
      //kdDebug(5100) << "Key Length: "<<subkey->keyLength()<<endl;

      // Key ID
      pos = pos2 + 1;
      while( output[pos] == ' ' )
        pos++;
      pos += 2; // skip the '0x'
      pos2 = output.find( ' ', pos );
      subkey->setKeyID( output.mid( pos, pos2-pos ) );
      //kdDebug(5100) << "Key ID: "<<subkey->keyID()<<endl;

      // Creation Date
      pos = pos2 + 1;
      while( output[pos] == ' ' )
        pos++;
      pos2 = output.find( ' ', pos );
      int year = output.mid( pos, 4 ).toInt();
      int month = output.mid( pos+5, 2 ).toInt();
      int day = output.mid( pos+8, 2 ).toInt();
      QDateTime dt( QDate( year, month, day ), QTime( 00, 00 ) );
      QDateTime epoch( QDate( 1970, 01, 01 ), QTime( 00, 00 ) );
      // The calculated creation date isn't exactly correct because QDateTime
      // doesn't know anything about timezones and always assumes local time
      // although epoch is of course UTC. But as PGP 5 anyway doesn't print
      // the time this doesn't matter too much.
      subkey->setCreationDate( epoch.secsTo( dt ) );

      // Expiration Date
      // if the primary key has been revoked the expiration date is not printed
      if( primaryKey || !key->revoked() )
      {
        pos = pos2 + 1;
        while( output[pos] == ' ' )
          pos++;
        pos2 = output.find( ' ', pos );
        if( output[pos] == '-' )
        { // key doesn't expire
          subkey->setExpirationDate( -1 );
        }
        else if( !strncmp( output.data() + pos, "*REVOKED*", 9 ) )
        { // key has been revoked
          subkey->setRevoked( true );
          key->setRevoked( true );
        }
        else
        {
          int year = output.mid( pos, 4 ).toInt();
          int month = output.mid( pos+5, 2 ).toInt();
          int day = output.mid( pos+8, 2 ).toInt();
          QDateTime dt( QDate( year, month, day ), QTime( 00, 00 ) );
          subkey->setCreationDate( epoch.secsTo( dt ) );
          // has the key already expired?
          if( QDateTime::currentDateTime() >= dt )
          {
            subkey->setExpired( true );
            key->setExpired( true );
          }
        }
      }
      else if( key->revoked() )
        subkey->setRevoked( true );

      // Key algorithm (RSA, DSS, Diffie-Hellman)
      bool sign = false;
      bool encr = false;
      pos = pos2 + 1;
      while( output[pos] == ' ' )
        pos++;
      pos2 = output.find( ' ', pos );
      if( !strncmp( output.data() + pos, "RSA", 3 ) )
      {
        sign = true;
        encr = true;
      }
      else if( !strncmp( output.data() + pos, "DSS", 3 ) )
        sign = true;
      else if( !strncmp( output.data() + pos, "Diffie-Hellman", 14 ) )
        encr = true;
      else
        kdDebug(5100)<<"Unknown key algorithm\n";

      // set key capabilities of the subkey
      subkey->setCanEncrypt( encr );
      subkey->setCanSign( sign );
      subkey->setCanCertify( sign );

      if( primaryKey )
      {
        // Global key capabilities
        bool canSign = false;
        bool canEncr = false;
        pos = pos2 + 1;
        while( output[pos] == ' ' )
          pos++;
        pos2 = eol;
        if( !strncmp( output.data() + pos, "Sign & Encrypt", 14 ) )
        {
          canSign = true;
          canEncr = true;
        }
        else if( !strncmp( output.data() + pos, "Sign only", 9 ) )
          canSign = true;
        else if( !strncmp( output.data() + pos, "Encrypt only", 12 ) )
          canEncr = true;
        else
          kdDebug(5100)<<"Unknown key capability\n";

        // set the global key capabilities
        if( !key->expired() && !key->revoked() )
        {
          key->setCanEncrypt( canEncr );
          key->setCanSign( canSign );
          key->setCanCertify( canSign );
        }
        //kdDebug(5100)<<"Key capabilities: "<<(key->canEncrypt()?"E":"")<<(key->canSign()?"SC":"")<<endl;
        primaryKey = false;
      }
    }
    else if( !strncmp( output.data() + offset, "f16", 3 ) ||
             !strncmp( output.data() + offset, "f20", 3 ) )
    { // line contains a fingerprint
      /* Examples:
         f16    Fingerprint16 = DE 2A 77 08 78 64 7C 42  72 75 B1 A7 3E 42 3F 79
         f20    Fingerprint20 = 226F 4B63 6DA2 7389 91D1  2A49 D58A 3EC1 5214 181E

       */
      int pos = output.find( '=', offset+3 ) + 2;
      QCString fingerprint = output.mid( pos, eol-pos );
      // remove white space from the fingerprint
      for ( int idx = 0 ; (idx = fingerprint.find(' ', idx)) >= 0 ; )
	fingerprint.replace( idx, 1, "" );
      subkey->setFingerprint( fingerprint );
      //kdDebug(5100)<<"Fingerprint: "<<fingerprint<<endl;
    }
    else if( !strncmp( output.data() + offset, "uid", 3 ) )
    { // line contains a uid
      int pos = offset+5;
      QCString uid = output.mid( pos, eol-pos );
      key->addUserID( uid );
      // displaying of uids which contain non-ASCII characters is broken in
      // PGP 5.0i; it shows these characters as \ooo and truncates the uid
      // because it doesn't take the 3 extra characters per non-ASCII char
      // into account. Example (with an UTF-8 encoded &ouml;):
      // uid  Ingo Kl\303\266cker <ingo.kloecker@epo
      // because of this and because we anyway don't know which charset was
      // used to encode the uid we don't try to decode it
    }
    else if ( !strncmp( output.data() + offset, "sig", 3 ) ||
              !strncmp( output.data() + offset, "SIG", 3 ) ||
              !strncmp( output.data() + offset, "ret", 3 ) )
    { // line contains a signature
      // SIG = sig with own key; ret = sig with revoked key
      // we ignore it for now
    }

    offset = eol + 1;
  }

  return key;
}


Key*
Base5::parseSingleKey( const QCString& output, Key* key /* = 0 */ )
{
  int offset;

  // search start of header line
  if( !strncmp( output.data(), "Type Bits", 9 ) )
    offset = 0;
  else
  {
    offset = output.find( "\nType Bits" ) + 1;
    if( offset == 0 )
      return 0;
  }

  // key data begins in the next line
  offset = output.find( '\n', offset ) + 1;
  if( offset == -1 )
    return 0;

  key = parseKeyData( output, offset, key );

  //kdDebug(5100) << "finished parsing keys" << endl;

  return key;
}


KeyList
Base5::parseKeyList( const QCString& output, bool onlySecretKeys )
{
  KeyList keys;
  Key *key = 0;
  int offset;

  // search start of header line
  if( !strncmp( output.data(), "Type Bits", 9 ) )
    offset = 0;
  else
  {
    offset = output.find( "\nType Bits" ) + 1;
    if( offset == 0 )
      return keys;
  }

  // key data begins in the next line
  offset = output.find( '\n', offset ) + 1;
  if( offset == -1 )
    return keys;

  do
  {
    key = parseKeyData( output, offset );
    if( key != 0 )
    {
      // if only secret keys should be read test if the key is secret
      if( !onlySecretKeys || !key->secret() )
        keys.append( key );
      // skip the blank line which separates the keys
      offset++;
    }
  }
  while( key != 0 );

  //kdDebug(5100) << "finished parsing keys" << endl;

  return keys;
}


void
Base5::parseTrustDataForKey( Key* key, const QCString& str )
{
  if( ( key == 0 ) || str.isEmpty() )
    return;

  QCString keyID = "0x" + key->primaryKeyID();
  UserIDList userIDs = key->userIDs();

  // search the start of the trust data
  int offset = str.find( "\n\n  KeyID" ) + 9;
  if( offset == -1 + 9 )
    return;

  offset = str.find( '\n', offset ) + 1;
  if( offset == -1 + 1 )
    return;

  bool ultimateTrust = false;
  if( !strncmp( str.data() + offset+13, "ultimate", 8 ) )
    ultimateTrust = true;

  while( true )
  { // loop over all trust information about this key

    int eol;

    // search the end of the current line
    if( ( eol = str.find( '\n', offset ) ) == -1 )
      break;

    if( str[offset+23] != ' ' )
    { // line contains a validity value for a user ID

      // determine the validity
      Validity validity = KPGP_VALIDITY_UNKNOWN;
      if( !strncmp( str.data() + offset+23, "complete", 8 ) )
        if( ultimateTrust )
          validity = KPGP_VALIDITY_ULTIMATE;
        else
          validity = KPGP_VALIDITY_FULL;
      else if( !strncmp( str.data() + offset+23, "marginal", 8 ) )
        validity = KPGP_VALIDITY_MARGINAL;
      else if( !strncmp( str.data() + offset+23, "invalid", 7 ) )
        validity = KPGP_VALIDITY_UNDEFINED;

      // determine the user ID
      int pos = offset + 33;
      QString uid = str.mid( pos, eol-pos );

      // set the validity of the corresponding user ID
      for( UserIDListIterator it( userIDs ); it.current(); ++it )
        if( (*it)->text() == uid )
        {
          kdDebug(5100)<<"Setting the validity of "<<uid<<" to "<<validity<<endl;
          (*it)->setValidity( validity );
          break;
        }
    }

    offset = eol + 1;
  }
}


} // namespace Kpgp
