/*
    kpgpbaseG.cpp

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

#include <klocale.h>
#include <kprocess.h>
#include <kdebug.h>

#include <qtextcodec.h>

#include <string.h> /* strncmp */

namespace Kpgp {

BaseG::BaseG()
  : Base()
{
  // determine the version of gpg (the method is equivalent to gpgme's method)
  runGpg( "--version", 0 );
  int eol = output.find( '\n' );
  if( eol > 0 ) {
    int pos = output.findRev( ' ', eol - 1 );
    if( pos != -1 ) {
      mVersion = output.mid( pos + 1, eol - pos - 1 );
      kdDebug(5100) << "found GnuPG " << mVersion << endl;
    }
  }
}


BaseG::~BaseG()
{
}


int
BaseG::encrypt( Block& block, const KeyIDList& recipients )
{
  return encsign( block, recipients, 0 );
}


int
BaseG::clearsign( Block& block, const char *passphrase )
{
  return encsign( block, KeyIDList(), passphrase );
}


int
BaseG::encsign( Block& block, const KeyIDList& recipients,
                const char *passphrase )
{
  QCString cmd;
  int exitStatus = 0;

  if(!recipients.isEmpty() && passphrase != 0)
    cmd = "--batch --armor --sign --encrypt --textmode";
  else if(!recipients.isEmpty())
    cmd = "--batch --armor --encrypt --textmode";
  else if(passphrase != 0)
    cmd = "--batch --escape-from --clearsign";
  else
  {
    kdDebug(5100) << "kpgpbase: Neither recipients nor passphrase specified." << endl;
    return OK;
  }

  if(passphrase != 0)
    cmd += addUserId();

  if(!recipients.isEmpty())
  {
    cmd += " --set-filename stdin";

    QCString pgpUser = Module::getKpgp()->user();
    if(Module::getKpgp()->encryptToSelf() && !pgpUser.isEmpty()) {
      cmd += " -r 0x";
      cmd += pgpUser;
    }

    for( KeyIDList::ConstIterator it = recipients.begin();
         it != recipients.end(); ++it ) {
      cmd += " -r 0x";
      cmd += (*it);
    }
  }

  clear();
  input = block.text();
  exitStatus = runGpg(cmd.data(), passphrase);
  if( !output.isEmpty() )
    block.setProcessedText( output );
  block.setError( error );

  if( exitStatus != 0 )
  {
    // this error message is later hopefully overwritten
    errMsg = i18n( "Unknown error." );
    status = ERROR;
  }

#if 0
  // #### FIXME: As we check the keys ourselves the following problems
  //             shouldn't occur. Therefore I don't handle them for now.
  //             IK 01/2002
  if(!recipients.isEmpty())
  {
    int index = 0;
    bool bad = FALSE;
    unsigned int num = 0;
    QCString badkeys = "";
    // Examples:
    // gpg: 0x12345678: skipped: public key not found
    // gpg: 0x12345678: skipped: public key is disabled
    // gpg: 0x12345678: skipped: unusable public key
    // (expired or revoked key)
    // gpg: 23456789: no info to calculate a trust probability
    // (untrusted key, 23456789 is the key Id of the encryption sub key)
    while((index = error.find("skipped: ",index)) != -1)
    {
      bad = TRUE;
      index = error.find('\'',index);
      int index2 = error.find('\'',index+1);
      badkeys += error.mid(index, index2-index+1) + ", ";
      num++;
    }
    if(bad)
    {
      badkeys.stripWhiteSpace();
      if(num == recipients.count())
        errMsg = i18n("Could not find public keys matching the userid(s)\n"
                      "%1;\n"
                      "the message is not encrypted.")
                     .arg( badkeys.data() );
      else
        errMsg = i18n("Could not find public keys matching the userid(s)\n"
                      "%1;\n"
                      "these persons will not be able to read the message.")
                     .arg( badkeys.data() );
      status |= MISSINGKEY;
      status |= ERROR;
    }
  }
#endif
  if( passphrase != 0 )
  {
    // Example 1 (bad passphrase, clearsign only):
    // gpg: skipped `0x12345678': bad passphrase
    // gpg: [stdin]: clearsign failed: bad passphrase
    // Example 2 (bad passphrase, sign & encrypt):
    // gpg: skipped `0x12345678': bad passphrase
    // gpg: [stdin]: sign+encrypt failed: bad passphrase
    // Example 3 (unusable secret key, clearsign only):
    // gpg: skipped `0x12345678': unusable secret key
    // gpg: [stdin]: clearsign failed: unusable secret key
    // Example 4 (unusable secret key, sign & encrypt):
    // gpg: skipped `0xAC0EB35D': unusable secret key
    // gpg: [stdin]: sign+encrypt failed: unusable secret key
    if( error.find("bad passphrase") != -1 )
    {
      errMsg = i18n("Signing failed because the passphrase is wrong.");
      status |= BADPHRASE;
      status |= ERR_SIGNING;
      status |= ERROR;
    }
    else if( error.find("unusable secret key") != -1 )
    {
      errMsg = i18n("Signing failed because your secret key is unusable.");
      status |= ERR_SIGNING;
      status |= ERROR;
    }
    else if( !( status & ERROR ) )
    {
      //kdDebug(5100) << "Base: Good Passphrase!" << endl;
      status |= SIGNED;
    }
  }

  //kdDebug(5100) << "status = " << status << endl;
  block.setStatus( status );
  return status;
}


int
BaseG::decrypt( Block& block, const char *passphrase )
{
  int index, index2;
  int exitStatus = 0;

  clear();
  input = block.text();
  exitStatus = runGpg("--batch --decrypt", passphrase);
  if( !output.isEmpty() && ( error.find( "gpg: quoted printable" ) == -1 ) )
    block.setProcessedText( output );
  block.setError( error );

  if(exitStatus == -1) {
    errMsg = i18n("Error running gpg");
    status = RUN_ERR;
    block.setStatus( status );
    return status;
  }

  // Example 1 (good passphrase, decryption successful):
  // gpg: encrypted with 2048-bit ELG-E key, ID 12345678, created 2000-11-11
  //       "Foo Bar <foo@bar.xyz>"
  //
  // Example 2 (bad passphrase):
  // gpg: encrypted with 1024-bit RSA key, ID 12345678, created 1991-01-01
  //       "Foo Bar <foo@bar.xyz>"
  // gpg: public key decryption failed: bad passphrase
  // gpg: decryption failed: secret key not available
  //
  // Example 3 (no secret key available):
  // gpg: encrypted with RSA key, ID 12345678
  // gpg: decryption failed: secret key not available
  //
  // Example 4 (good passphrase for second key, decryption successful):
  // gpg: encrypted with 2048-bit ELG-E key, ID 12345678, created 2000-01-01
  //       "Foo Bar (work) <foo@bar.xyz>"
  // gpg: public key decryption failed: bad passphrase
  // gpg: encrypted with 2048-bit ELG-E key, ID 23456789, created 2000-02-02
  //       "Foo Bar (home) <foo@bar.xyz>"
  if( error.find( "gpg: encrypted with" ) != -1 )
  {
    //kdDebug(5100) << "kpgpbase: message is encrypted" << endl;
    status |= ENCRYPTED;
    if( error.find( "\ngpg: decryption failed" ) != -1 )
    {
      if( ( index = error.find( "bad passphrase" ) ) != -1 )
      {
        if( passphrase != 0 )
        {
          errMsg = i18n( "Bad passphrase; could not decrypt." );
          kdDebug(5100) << "Base: passphrase is bad" << endl;
          status |= BADPHRASE;
          status |= ERROR;
        }
        else
        {
          // Search backwards the user ID of the needed key
          index2 = error.findRev('"', index) - 1;
          index = error.findRev("      \"", index2) + 7;
          // The conversion from UTF8 is necessary because gpg stores and
          // prints user IDs in UTF8
          block.setRequiredUserId( QString::fromUtf8( error.mid( index, index2 - index + 1 ) ) );
          kdDebug(5100) << "Base: key needed is \"" << block.requiredUserId() << "\"!" << endl;
        }
      }
      else if( error.find( "secret key not available" ) != -1 )
      {
        // no secret key fitting this message
        status |= NO_SEC_KEY;
        status |= ERROR;
        errMsg = i18n("You do not have the secret key needed to decrypt this message.");
        kdDebug(5100) << "Base: no secret key for this message" << endl;
      }
    }
    // check for persons
#if 0
    // ##### FIXME: This information is anyway currently not used
    //       I'll change it to always determine the recipients.
    index = error.find("can only be read by:");
    if(index != -1)
    {
      index = error.find('\n',index);
      int end = error.find("\n\n",index);

      mRecipients.clear();
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

  // Example 1 (unknown signature key):
  // gpg: Signature made Wed 02 Jan 2002 11:26:33 AM CET using DSA key ID 2E250C64
  // gpg: Can't check signature: public key not found
  if((index = error.find("Signature made")) != -1)
  {
    //kdDebug(5100) << "Base: message is signed" << endl;
    status |= SIGNED;
    // get signature date and signature key ID
    // Example: Signature made Sun 06 May 2001 03:49:27 PM CEST using DSA key ID 12345678
    index2 = error.find("using", index+15);
    block.setSignatureDate( error.mid(index+15, index2-(index+15)-1) );
    kdDebug(5100) << "Message was signed on '" << block.signatureDate() << "'\n";
    index2 = error.find("key ID ", index2) + 7;
    block.setSignatureKeyId( error.mid(index2,8) );
    kdDebug(5100) << "Message was signed with key '" << block.signatureKeyId() << "'\n";
    // move index to start of next line
    index = error.find('\n', index2)+1;

    if ((error.find("Key matching expected", index) != -1)
        || (error.find("Can't check signature", index) != -1))
    {
      status |= UNKNOWN_SIG;
      status |= GOODSIG;
      block.setSignatureUserId( QString::null );
    }
    else if( error.find("Good signature", index) != -1 )
    {
      status |= GOODSIG;
      // get the primary user ID of the signer
      index = error.find('"',index);
      index2 = error.find('\n',index+1);
      index2 = error.findRev('"', index2-1);
      block.setSignatureUserId( error.mid( index+1, index2-index-1 ) );
    }
    else if( error.find("BAD signature", index) != -1 )
    {
      //kdDebug(5100) << "BAD signature" << endl;
      status |= ERROR;
      // get the primary user ID of the signer
      index = error.find('"',index);
      index2 = error.find('\n',index+1);
      index2 = error.findRev('"', index2-1);
      block.setSignatureUserId( error.mid( index+1, index2-index-1 ) );
    }
    else if( error.find("Can't find the right public key", index) != -1 )
    {
      // #### fix this hack
      // I think this can't happen anymore because if the pubring is missing
      // the current GnuPG creates a new empty one.
      status |= UNKNOWN_SIG;
      status |= GOODSIG; // this is a hack...
      block.setSignatureUserId( i18n("??? (file ~/.gnupg/pubring.gpg not found)") );
    }
    else
    {
      status |= ERROR;
      block.setSignatureUserId( QString::null );
    }
  }
  //kdDebug(5100) << "status = " << status << endl;
  block.setStatus( status );
  return status;
}


Key*
BaseG::readPublicKey( const KeyID& keyID,
                      const bool readTrust /* = false */,
                      Key* key /* = 0 */ )
{
  int exitStatus = 0;

  status = 0;
  if( readTrust )
    exitStatus = runGpg( "--batch --list-public-keys --with-fingerprint --with-colons --fixed-list-mode 0x" + keyID, 0, true );
  else
    exitStatus = runGpg( "--batch --list-public-keys --with-fingerprint --with-colons --fixed-list-mode --no-expensive-trust-checks 0x" + keyID, 0, true );

  if(exitStatus != 0) {
    status = ERROR;
    return 0;
  }

  int offset;
  // search start of key data
  if( !strncmp( output.data(), "pub:", 4 ) )
    offset = 0;
  else {
    offset = output.find( "\npub:" );
    if( offset == -1 )
      return 0;
    else
      offset++;
  }

  key = parseKeyData( output, offset, key );

  return key;
}


KeyList
BaseG::publicKeys( const QStringList & patterns )
{
  int exitStatus = 0;

  // the option --with-colons should be used for interprocess communication
  // with gpg (according to Werner Koch)
  QCString cmd = "--batch --list-public-keys --with-fingerprint --with-colons "
                 "--fixed-list-mode --no-expensive-trust-checks";
  for ( QStringList::ConstIterator it = patterns.begin();
        it != patterns.end(); ++it ) {
    cmd += " ";
    cmd += KProcess::quote( *it ).local8Bit();
  }
  status = 0;
  exitStatus = runGpg( cmd, 0, true );

  if(exitStatus != 0) {
    status = ERROR;
    return KeyList();
  }

  // now we need to parse the output for public keys
  KeyList publicKeys = parseKeyList(output, false);

  // sort the list of public keys
  publicKeys.sort();

  return publicKeys;
}


KeyList
BaseG::secretKeys( const QStringList & patterns )
{
  int exitStatus = 0;

  // the option --with-colons should be used for interprocess communication
  // with gpg (according to Werner Koch)
  QCString cmd = "--batch --list-secret-keys --with-fingerprint --with-colons "
                 "--fixed-list-mode";
  for ( QStringList::ConstIterator it = patterns.begin();
        it != patterns.end(); ++it ) {
    cmd += " ";
    cmd += KProcess::quote( *it ).local8Bit();
  }
  status = 0;
  exitStatus = runGpg( cmd, 0, true );

  if(exitStatus != 0) {
    status = ERROR;
    return KeyList();
  }

  // now we need to parse the output for secret keys
  KeyList secretKeys = parseKeyList(output, true);

  // sort the list of secret keys
  secretKeys.sort();

  return secretKeys;
}


int
BaseG::signKey(const KeyID& keyID, const char *passphrase)
{
  QCString cmd;
  int exitStatus = 0;

  cmd = "--batch";
  cmd += addUserId();
  cmd += " --sign-key 0x";
  cmd += keyID;

  status = 0;
  exitStatus = runGpg(cmd.data(), passphrase);

  if (exitStatus != 0)
    status = ERROR;

  return status;
}


QCString
BaseG::getAsciiPublicKey(const KeyID& keyID)
{
  int exitStatus = 0;

  if (keyID.isEmpty())
    return QCString();

  status = 0;
  exitStatus = runGpg("--batch --armor --export 0x" + keyID, 0, true);

  if(exitStatus != 0) {
    status = ERROR;
    return QCString();
  }

  return output;
}


Key*
BaseG::parseKeyData( const QCString& output, int& offset, Key* key /* = 0 */ )
// This function parses the data for a single key which is output by GnuPG
// with the following command line arguments:
//   --batch --list-public-keys --with-fingerprint --with-colons
//   --fixed-list-mode [--no-expensive-trust-checks]
// It expects the key data to start at offset and returns the start of
// the next key's data in offset.
// Subkeys are currently ignored.
{
  int index = offset;

  if(    ( strncmp( output.data() + offset, "pub:", 4 ) != 0 )
      && ( strncmp( output.data() + offset, "sec:", 4 ) != 0 ) ) {
    return 0;
  }

  if( key == 0 )
    key = new Key();
  else
    key->clear();

  QCString keyID;
  bool firstKey = true;

  while( true )
  {
    int eol;
    // search the end of the current line
    if( ( eol = output.find( '\n', index ) ) == -1 )
      break;

    bool bIsPublicKey = false;
    if( ( bIsPublicKey = !strncmp( output.data() + index, "pub:", 4 ) )
        || !strncmp( output.data() + index, "sec:", 4 ) )
    { // line contains primary key data
      // Example: pub:f:1024:17:63CB691DFAEBD5FC:860451781::379:-:::scESC:

      // abort parsing if we found the start of the next key
      if( !firstKey )
        break;
      firstKey = false;

      key->setSecret( !bIsPublicKey );

      Subkey *subkey = new Subkey( QCString(), !bIsPublicKey );

      int pos = index + 4; // begin of 2nd field
      int pos2 = output.find( ':', pos );
      for( int field = 2; field <= 12; field++ )
      {
        switch( field )
        {
        case 2: // the calculated trust
          if( pos2 > pos )
          {
            switch( output[pos] )
            {
            case 'o': // unknown (this key is new to the system)
              break;
            case 'i': // the key is invalid, e.g. missing self-signature
              subkey->setInvalid( true );
              key->setInvalid( true );
              break;
            case 'd': // the key has been disabled
              subkey->setDisabled( true );
              key->setDisabled( true );
              break;
            case 'r': // the key has been revoked
              subkey->setRevoked( true );
              key->setRevoked( true );
              break;
            case 'e': // the key has expired
              subkey->setExpired( true );
              key->setExpired( true );
              break;
            case '-': // undefined (no path leads to the key)
            case 'q': // undefined (no trusted path leads to the key)
            case 'n': // don't trust this key at all
            case 'm': // the key is marginally trusted
            case 'f': // the key is fully trusted
            case 'u': // the key is ultimately trusted (secret key available)
              // These values are ignored since we determine the key trust
              // from the trust values of the user ids.
              break;
            default:
              kdDebug(5100) << "Unknown trust value\n";
            }
          }
          break;
        case 3: // length of key in bits
          if( pos2 > pos )
            subkey->setKeyLength( output.mid( pos, pos2-pos ).toUInt() );
          break;
        case 4: //  the key algorithm
          if( pos2 > pos )
            subkey->setKeyAlgorithm( output.mid( pos, pos2-pos ).toUInt() );
          break;
        case 5: // the long key id
          keyID = output.mid( pos, pos2-pos );
          subkey->setKeyID( keyID );
          break;
        case 6: // the creation date (in seconds since 1970-01-01 00:00:00)
          if( pos2 > pos )
            subkey->setCreationDate( output.mid( pos, pos2-pos ).toLong() );
          break;
        case 7: // the expiration date (in seconds since 1970-01-01 00:00:00)
          if( pos2 > pos )
            subkey->setExpirationDate( output.mid( pos, pos2-pos ).toLong() );
          else
            subkey->setExpirationDate( -1 ); // key expires never
          break;
        case 8: // local ID (ignored)
        case 9: // Ownertrust (ignored for now)
        case 10: // User-ID (always empty in --fixed-list-mode)
        case 11: // signature class (always empty except for key signatures)
          break;
        case 12: // key capabilities
          for( int i=pos; i<pos2; i++ )
            switch( output[i] )
            {
            case 'e':
              subkey->setCanEncrypt( true );
              break;
            case 's':
              subkey->setCanSign( true );
              break;
            case 'c':
              subkey->setCanCertify( true );
              break;
            case 'E':
              key->setCanEncrypt( true );
              break;
            case 'S':
              key->setCanSign( true );
              break;
            case 'C':
              key->setCanCertify( true );
              break;
            default:
              kdDebug(5100) << "Unknown key capability\n";
            }
          break;
        }
        pos = pos2 + 1;
        pos2 = output.find( ':', pos );
      }
      key->addSubkey( subkey );
    }
    else if( !strncmp( output.data() + index, "uid:", 4 ) )
    { // line contains a user id
      // Example: uid:f::::::::Philip R. Zimmermann <prz@pgp.com>:

      UserID *userID = new UserID( "" );

      int pos = index + 4; // begin of 2nd field
      int pos2 = output.find( ':', pos );
      for( int field=2; field <= 10; field++ )
      {
        switch( field )
        {
        case 2: // the calculated trust
          if( pos2 > pos )
          {
            switch( output[pos] )
            {
            case 'i': // the user id is invalid, e.g. missing self-signature
              userID->setInvalid( true );
              break;
            case 'r': // the user id has been revoked
              userID->setRevoked( true );
              break;
            case '-': // undefined (no path leads to the key)
            case 'q': // undefined (no trusted path leads to the key)
              userID->setValidity( KPGP_VALIDITY_UNDEFINED );
              break;
            case 'n': // don't trust this key at all
              userID->setValidity( KPGP_VALIDITY_NEVER );
              break;
            case 'm': // the key is marginally trusted
              userID->setValidity( KPGP_VALIDITY_MARGINAL );
              break;
            case 'f': // the key is fully trusted
              userID->setValidity( KPGP_VALIDITY_FULL );
              break;
            case 'u': // the key is ultimately trusted (secret key available)
              userID->setValidity( KPGP_VALIDITY_ULTIMATE );
              break;
            default:
              kdDebug(5100) << "Unknown trust value\n";
            }
          }
          break;
        case 3: // these fields are empty
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
          break;
        case 10: // User-ID
          QCString uid = output.mid( pos, pos2-pos );
          // replace "\xXX" with the corresponding character;
          // other escaped characters, i.e. \n, \r etc., are ignored
          // because they shouldn't appear in user IDs
          for ( int idx = 0 ; (idx = uid.find( "\\x", idx )) >= 0 ; ++idx ) {
            char str[2] = "x";
            str[0] = (char) QString( uid.mid( idx + 2, 2 ) ).toShort( 0, 16 );
            uid.replace( idx, 4, str );
          }
          QString uidString = QString::fromUtf8( uid.data() );
          // check whether uid was utf-8 encoded
          bool isUtf8 = true;
          for ( unsigned int i = 0; i + 1 < uidString.length(); ++i ) {
            if ( uidString[i].unicode() == 0xdbff &&
                 uidString[i+1].row() == 0xde ) {
              // we found a non-Unicode character (see QString::fromUtf8())
              isUtf8 = false;
              break;
            }
          }
          if( !isUtf8 ) {
            // The user id isn't utf-8 encoded. It was most likely
            // created with PGP which either used latin1 or koi8-r.
            kdDebug(5100) << "User Id '" << uid
                          << "' doesn't seem to be utf-8 encoded." << endl;

            // We determine the ratio between non-ASCII and ASCII chars.
            // A koi8-r user id should have lots of non-ASCII chars.
            int nonAsciiCount = 0, asciiCount = 0;

            // We only look at the first part of the user id (i. e. everything
            // before the email address resp. before a comment)
            for( signed char* ch = (signed char*)uid.data();
                 *ch && ( *ch != '(' ) && ( *ch != '<' );
                 ++ch ) {
              if( ( ( *ch >= 'A' ) && ( *ch <= 'Z' ) )
                  || ( ( *ch >= 'a' ) && ( *ch <= 'z' ) ) )
                ++asciiCount;
              else if( *ch < 0 )
                ++nonAsciiCount;
            }
            kdDebug(5100) << "ascii-nonAscii ratio : " << asciiCount
                          << ":" << nonAsciiCount << endl;
            if( nonAsciiCount > asciiCount ) {
              // assume koi8-r encoding
              kdDebug(5100) << "Assume koi8-r encoding." << endl;
              QTextCodec *codec = QTextCodec::codecForName("KOI8-R");
              uidString = codec->toUnicode( uid.data() );
              // check the case of the first two characters to find out
              // whether the user id is probably CP1251 encoded (for some
              // reason in CP1251 the lower case characters have smaller
              // codes than the upper case characters, so if the first char
              // of the koi8-r decoded user id is lower case and the second
              // char is upper case then it's likely that the user id is
              // CP1251 encoded)
              if( ( uidString.length() >= 2 )
                  && ( uidString[0].lower() == uidString[0] )
                  && ( uidString[1].upper() == uidString[1] ) ) {
                // koi8-r decoded user id has inverted case, so assume
                // CP1251 encoding
                kdDebug(5100) << "No, it doesn't seem to be koi8-r. "
                                 "Use CP 1251 instead." << endl;
                QTextCodec *codec = QTextCodec::codecForName("CP1251");
                uidString = codec->toUnicode( uid.data() );
              }
            }
            else {
              // assume latin1 encoding
              kdDebug(5100) << "Assume latin1 encoding." << endl;
              uidString = QString::fromLatin1( uid.data() );
            }
          }
          userID->setText( uidString );
          break;
        }
        pos = pos2 + 1;
        pos2 = output.find( ':', pos );
      }

      // user IDs are printed in UTF-8 by gpg (if one uses --with-colons)
      key->addUserID( userID );
    }
    else if( !strncmp( output.data() + index, "fpr:", 4 ) )
    { // line contains a fingerprint
      // Example: fpr:::::::::17AFBAAF21064E513F037E6E63CB691DFAEBD5FC:

      if (key == 0) // invalid key data
	break;

      // search the fingerprint (it's in the 10th field)
      int pos = index + 4;
      for( int i = 0; i < 8; i++ )
        pos = output.find( ':', pos ) + 1;
      int pos2 = output.find( ':', pos );

      key->setFingerprint( keyID, output.mid( pos, pos2-pos ) );
    }
    index = eol + 1;
  }

  //kdDebug(5100) << "finished parsing key data\n";

  offset = index;

  return key;
}


KeyList
BaseG::parseKeyList( const QCString& output, bool secretKeys )
{
  KeyList keys;
  Key *key = 0;
  int offset;

  // search start of key data
  if(    !strncmp( output.data(), "pub:", 4 )
      || !strncmp( output.data(), "sec:", 4 ) )
    offset = 0;
  else {
    if( secretKeys )
      offset = output.find( "\nsec:" );
    else
      offset = output.find( "\npub:" );
    if( offset == -1 )
      return keys;
    else
      offset++;
  }

  do {
    key = parseKeyData( output, offset );
    if( key != 0 )
      keys.append( key );
  }
  while( key != 0 );

  //kdDebug(5100) << "finished parsing keys" << endl;

  return keys;
}


} // namespace Kpgp
