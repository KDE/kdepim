/*
    kpgpbase6.cpp

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

#include <string.h> /* strncmp */

#include <qdatetime.h>

#include <klocale.h>
#include <kdebug.h>

#define PGP6 "pgp"

namespace Kpgp {

Base6::Base6()
  : Base2()
{
}


Base6::~Base6()
{
}


int
Base6::decrypt( Block& block, const char *passphrase )
{
  int index, index2;
  int exitStatus = 0;

  clear();
  input = block.text();
  exitStatus = run( PGP6 " +batchmode +language=C -f", passphrase);
  if( !output.isEmpty() )
    block.setProcessedText( output );
  block.setError( error );

  if(exitStatus == -1) {
    errMsg = i18n("error running PGP");
    status = RUN_ERR;
    block.setStatus( status );
    return status;
  }

  // encrypted message
  if( error.find("File is encrypted.") != -1)
  {
    //kdDebug(5100) << "kpgpbase: message is encrypted" << endl;
    status |= ENCRYPTED;
    if((index = error.find("Key for user ID")) != -1)
    {
      // Find out the key for which the phrase is needed
      index  = error.find(':', index) + 2;
      index2 = error.find('\n', index);
      block.setRequiredUserId( error.mid(index, index2 - index) );
      //kdDebug(5100) << "Base: key needed is \"" << block.requiredUserId() << "\"!\n";

      // Test output length to find out, if the passphrase is
      // bad. If someone knows a better way, please fix this.
      /// ### This could be done by forcing PGP6 to be more verbose
      ///     by adding an additional '+verbose=2' to the command line
      if (!passphrase || !output.length())
      {
	errMsg = i18n("Bad passphrase; could not decrypt.");
	//kdDebug(5100) << "Base: passphrase is bad" << endl;
        status |= BADPHRASE;
        status |= ERROR;
      }
    }
    else if( error.find("You do not have the secret key needed to decrypt this file.") != -1)
    {
      errMsg = i18n("You do not have the secret key for this message.");
      //kdDebug(5100) << "Base: no secret key for this message" << endl;
      status |= NO_SEC_KEY;
      status |= ERROR;
    }
  }

  // signed message

  // Examples (made with PGP 6.5.8)
  /* Example no. 1 (signed with unknown key):
   * File is signed.  signature not checked.
   * Signature made 2001/11/25 11:55 GMT
   * key does not meet validity threshold.
   *
   * WARNING:  Because this public key is not certified with a trusted
   * signature, it is not known with high confidence that this public key
   * actually belongs to: "(KeyID: 0x475027BD)".
   */
  /* Example no. 2 (signed with untrusted key):
   * File is signed.  Good signature from user "Joe User <joe@foo.bar>".
   * Signature made 2001/12/05 13:09 GMT
   *
   * WARNING:  Because this public key is not certified with a trusted
   * signature, it is not known with high confidence that this public key
   * actually belongs to: "Joe User <joe@foo.bar>".
   */
  /* Example no. 3 (signed with trusted key):
   * File is signed.  Good signature from user "Joe User <joe@foo.bar>".
   * Signature made 2001/12/05 13:09 GMT
   */
  if(((index = error.find("File is signed.")) != -1)
    || (error.find("Good signature") != -1 ))
  {
    //kdDebug(5100) << "Base: message is signed" << endl;
    status |= SIGNED;
    // determine the signature date
    if( ( index2 = error.find( "Signature made", index ) ) != -1 )
    {
      index2 += 15;
      int eol = error.find( '\n', index2 );
      block.setSignatureDate( error.mid( index2, eol-index2 ) );
      kdDebug(5100) << "Message was signed on '" << block.signatureDate() << "'\n";
    }
    else
      block.setSignatureDate( QCString() );
    // determine signature status and signature key
    if( error.find("signature not checked") != -1)
    {
      index = error.find("KeyID:",index);
      block.setSignatureKeyId( error.mid(index+9,8) );
      block.setSignatureUserId( QString::null );
      status |= UNKNOWN_SIG;
      status |= GOODSIG;
    }
    else if((index = error.find("Good signature")) != -1 )
    {
      status |= GOODSIG;
      // get signer
      index = error.find('"',index)+1;
      index2 = error.find('"', index);
      block.setSignatureUserId( error.mid(index, index2-index) );

      // get key ID of signer
      index = error.find("KeyID:",index2);
      if (index == -1)
        block.setSignatureKeyId( QCString() );
      else
        block.setSignatureKeyId( error.mid(index+9,8) );
    }
    else if( error.find("Can't find the right public key") != -1 )
    {
      // #### fix this hack
      // #### This doesn't happen with PGP 6.5.8 because it seems to
      // #### automatically create an empty pubring if it doesn't exist.
      status |= UNKNOWN_SIG;
      status |= GOODSIG; // this is a hack...
      block.setSignatureUserId( i18n("??? (file ~/.pgp/pubring.pkr not found)") );
      block.setSignatureKeyId( "???" );
    }
    else
    {
      status |= ERROR;
      block.setSignatureUserId( QString::null );
      block.setSignatureKeyId( QCString() );
    }
  }
  //kdDebug(5100) << "status = " << status << endl;
  block.setStatus( status );
  return status;
}


Key*
Base6::readPublicKey( const KeyID& keyID,
                      const bool readTrust /* = false */,
                      Key* key /* = 0 */ )
{
  int exitStatus = 0;

  status = 0;
  exitStatus = run( PGP6 " +batchmode -compatible +verbose=0 +language=C -kvvc "
                    "0x" + keyID, 0, true );

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
    exitStatus = run( PGP6 " +batchmode -compatible +verbose=0 +language=C -kc "
                      "0x" + keyID, 0, true );

    if(exitStatus != 0) {
      status = ERROR;
      return 0;
    }

    parseTrustDataForKey( key, output );
  }

  return key;
}


KeyList
Base6::publicKeys( const QStringList & patterns )
{
  return doGetPublicKeys( PGP6 " +batchmode -compatible +verbose=0 "
                          "+language=C -kvvc", patterns );
}


/*
QStrList
Base6::pubKeys()
{
  int index, index2;
  int exitStatus = 0;
  int compatibleMode = 1;

  status = 0;
  exitStatus = run("pgp +batchmode +language=C -kv -f");

  if(exitStatus != 0) {
    status = ERROR;
    return 0;
  }

  //truncate trailing "\n"
  if (error.length() > 1) error.truncate(error.length()-1);

  QStrList publicKeys;
  index = error.find("bits/keyID",1); // skip first to "\n"
  if (index ==-1)
  {
    index = error.find("Type bits",1); // skip first to "\n"
    if (index == -1)
      return 0;
    else
      compatibleMode = 0;
  }

  while( (index = error.find("\n",index)) != -1)
  {
    //parse line
    QCString line;
    if( (index2 = error.find("\n",index+1)) != -1)
      // skip last line
    {
      int index3;
      if (compatibleMode)
      {
        int index_pub = error.find("pub ",index);
        int index_sec = error.find("sec ",index);
        if (index_pub < 0)
          index3 = index_sec;
        else if (index_sec < 0)
          index3 = index_pub;
        else
          index3 = (index_pub < index_sec ? index_pub : index_sec);
      }
      else
      {
        int index_rsa = error.find("RSA ",index);
        int index_dss = error.find("DSS ",index);
        if (index_rsa < 0)
          index3 = index_dss;
        else if (index_dss < 0)
          index3 = index_rsa;
        else
          index3 = (index_rsa < index_dss ? index_rsa : index_dss);
      }

      if( (index3 >index2) || (index3 == -1) )
      {
	// second address for the same key
	line = error.mid(index+1,index2-index-1);
	line = line.stripWhiteSpace();
      } else {
	// line with new key
	int index4 = error.find(QRegExp("/\\d{2}/\\d{2} "), index);
	line = error.mid(index4+7,index2-index4-7);
      }
      //kdDebug(5100) << "Base: found key for " << (const char *)line << endl;

      // don't add PGP's comments to the key list
      if (strncmp(line.data(),"*** KEY EXPIRED ***",19) &&
          line.find(QRegExp("^expires \\d{4}/\\d{2}/\\d{2}")) < 0 &&
          strncmp(line.data(),"*** DEFAULT SIGNING KEY ***",27)) {
        publicKeys.append(line);
      }
    }
    else
      break;
    index = index2;
  }

  // Also look for pgp key groups
  exitStatus = run("pgp +batchmode +language=C -gv -f");

  if(exitStatus != 0) {
    status = ERROR;
    return 0;
  }

  index = 0;
  while ( (index = error.find("\n >", index)) != -1 ) {
    QCString line;
    index += 4;
    index2 = error.find(" \"", index);
    line = error.mid(index, index2-index+1).stripWhiteSpace();

    //kdDebug(5100) << "Base6: found key group for " << line << endl;
    publicKeys.append(line);
  }

  return publicKeys;
}
*/


KeyList
Base6::secretKeys( const QStringList & patterns )
{
  return publicKeys( patterns );
}


int
Base6::isVersion6()
{
  int exitStatus = 0;

  exitStatus = run( PGP6, 0, true );

  if(exitStatus == -1) {
    errMsg = i18n("error running PGP");
    status = RUN_ERR;
    return 0;
  }

  if( error.find("Version 6") != -1)
  {
    //kdDebug(5100) << "kpgpbase: pgp version 6.x detected" << endl;
    return 1;
  }

  //kdDebug(5100) << "kpgpbase: not pgp version 6.x" << endl;
  return 0;
}


Key*
Base6::parseKeyData( const QCString& output, int& offset, Key* key /* = 0 */ )
// This function parses the data for a single key which is output by PGP 6
// with the following command line arguments:
//   +batchmode -compatible +verbose=0 +language=C -kvvc
// It expects the key data to start at offset and returns the start of
// the next key's data in offset.
{
  if( ( strncmp( output.data() + offset, "DSS", 3 ) != 0 ) &&
      ( strncmp( output.data() + offset, "RSA", 3 ) != 0 ) )
  {
    kdDebug(5100) << "Unknown key type or corrupt key data.\n";
    return 0;
  }

  Subkey *subkey = 0;
  bool firstLine = true;
  bool canSign = false;
  bool canEncr = false;
  bool fpr = false;

  while( true )
  {
    int eol;

    // search the end of the current line
    if( ( eol = output.find( '\n', offset ) ) == -1 )
      break;

    //kdDebug(5100) << "Parsing: " << output.mid(offset, eol-offset) << endl;

    if( firstLine && ( !strncmp( output.data() + offset, "DSS", 3 ) ||
                       !strncmp( output.data() + offset, "RSA", 3 ) ) )
    { // line contains primary key data
      // Example 1:
      // RSA  1024      0xE2D074D3 2001/09/09 Test Key <testkey@xyz>
      // Example 2 (disabled key):
      // RSA@ 1024      0x8CCB2C1B 2001/11/04 Disabled Test Key <disabled@xyz>
      // Example 3 (expired key):
      // RSA  1024      0x7B94827D 2001/09/09 *** KEY EXPIRED ***
      // Example 4 (revoked key):
      // RSA  1024      0x956721F9 2001/09/09 *** KEY REVOKED ***
      // Example 5 (default signing key):
      // RSA  1024      0x12345678 2001/09/09 *** DEFAULT SIGNING KEY ***
      // Example 6 (expiring key):
      // RSA  2048      0xC11DB2E5 2000/02/24 expires 2001/12/31
      // Example 7 (complex example):
      // DSS  1024      0x80E104A7 2000/06/05 expires 2002/05/31
      // DSS  1024      0x80E104A7 2001/06/27 *** KEY REVOKED ***expires 2002/06/27
      //  DH  1024      0x80E104A7 2000/06/05 *** KEY REVOKED ****** KEY EXPIRED ***
      //kdDebug(5100)<<"Primary key data:\n";
      bool sign = false;
      bool encr = false;

      // set default key capabilities
      if( !strncmp( output.data() + offset, "DSS", 3 ) )
        sign = true;
      if( !strncmp( output.data() + offset, "RSA", 3 ) )
      {
        sign = true;
        encr = true;
      }

      int pos, pos2;

      if( key == 0 )
        key = new Key();
      else
        key->clear();

      subkey = new Subkey( "", false );
      key->addSubkey( subkey );
      // expiration date defaults to never
      subkey->setExpirationDate( -1 );

      // Key Flags
      switch( output[offset+3] )
      {
      case ' ': // nothing special
        break;
      case '@': // disabled key
        subkey->setDisabled( true );
        key->setDisabled( true );
        break;
      default:
        kdDebug(5100) << "Unknown key flag.\n";
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
      // although epoch is of course UTC. But as PGP 6 anyway doesn't print
      // the time this doesn't matter too much.
      subkey->setCreationDate( epoch.secsTo( dt ) );

      // User ID or key properties
      pos = pos2 + 1;
      while( output[pos] == ' ' )
        pos++;
      while( pos < eol )
      { // loop over User ID resp. key properties
        if( !strncmp( output.data() + pos, "*** KEY REVOKED ***", 19 ) )
        {
          sign = false;
          encr = false;
          subkey->setRevoked( true );
          key->setRevoked( true );
          pos += 19;
          //kdDebug(5100) << "Key was revoked.\n";
        }
        else if( !strncmp( output.data() + pos, "*** KEY EXPIRED ***", 19 ) )
        {
          sign = false;
          encr = false;
          subkey->setExpired( true );
          key->setExpired( true );
          pos += 19;
          //kdDebug(5100) << "Key has expired.\n";
        }
        else if( !strncmp( output.data() + pos, "expires ", 8 ) )
        {
          pos += 8;
          int year = output.mid( pos, 4 ).toInt();
          int month = output.mid( pos+5, 2 ).toInt();
          int day = output.mid( pos+8, 2 ).toInt();
          QDateTime dt( QDate( year, month, day ), QTime( 00, 00 ) );
          // Here the same comments as for the creation date are valid.
          subkey->setExpirationDate( epoch.secsTo( dt ) );
          pos += 10;
          //kdDebug(5100) << "Key expires...\n";
        }
        else if( !strncmp( output.data() + pos, "*** DEFAULT SIGNING KEY ***", 27 ) )
        {
          pos += 27;
          //kdDebug(5100) << "Key is default signing key.\n";
        }
        else
        {
          QCString uid = output.mid( pos, eol-pos );
          key->addUserID( uid );
          pos = eol;
          //kdDebug(5100) << "User ID:"<<uid<<endl;
        }
      }
      // set key capabilities of the primary subkey
      subkey->setCanEncrypt( encr );
      subkey->setCanSign( sign );
      subkey->setCanCertify( sign );
      // remember the global key capabilities
      canSign = sign;
      canEncr = encr;
    }
    else if( !strncmp( output.data() + offset, "DSS", 3 ) ||
             !strncmp( output.data() + offset, " DH", 3 ) ||
             !strncmp( output.data() + offset, "RSA", 3 ) )
    { // line contains secondary key data (or data for the next key)
      if( fpr )
        break; // here begins the next key's data
      //kdDebug(5100)<<"Secondary key data:\n";

      if( key == 0 )
        break;

      bool sign = false;
      bool encr = false;

      // set default key capabilities
      if( !strncmp( output.data() + offset, "DSS", 3 ) )
        sign = true;
      if( !strncmp( output.data() + offset, " DH", 3 ) )
        encr = true;
      if( !strncmp( output.data() + offset, "RSA", 3 ) )
      {
        sign = true;
        encr = true;
      }

      int pos, pos2;

      // Key Length of secondary key (ignored)
      pos = offset + 4;
      while( output[pos] == ' ' )
        pos++;
      pos2 = output.find( ' ', pos );

      // Key ID (ignored as it is anyway equal to the primary key id)
      pos = pos2 + 1;
      while( output[pos] == ' ' )
        pos++;
      pos2 = output.find( ' ', pos );

      // Creation Date of secondary key (ignored)
      pos = pos2 + 1;
      while( output[pos] == ' ' )
        pos++;
      pos2 = output.find( ' ', pos );

      // User ID or key properties
      pos = pos2 + 1;
      while( output[pos] == ' ' )
        pos++;
      while( pos < eol )
      { // loop over User ID resp. key properties
        if( !strncmp( output.data() + pos, "*** KEY REVOKED ***", 19 ) )
        {
          sign = false;
          encr = false;
          pos += 19;
          //kdDebug(5100) << "Key was revoked.\n";
        }
        else if( !strncmp( output.data() + pos, "*** KEY EXPIRED ***", 19 ) )
        {
          sign = false;
          encr = false;
          pos += 19;
          //kdDebug(5100) << "Key has expired.\n";
        }
        else if( !strncmp( output.data() + pos, "expires ", 8 ) )
        {
          pos += 18; // skip the expiration date
          //kdDebug(5100) << "Key expires...\n";
        }
        else if( !strncmp( output.data() + pos, "*** DEFAULT SIGNING KEY ***", 27 ) )
        {
          pos += 27;
          //kdDebug(5100) << "Key is default signing key.\n";
        }
        else
        {
          QCString uid = output.mid( pos, eol-pos );
          key->addUserID( uid );
          pos = eol;
          //kdDebug(5100) << "User ID:"<<uid<<endl;
        }
      }
      // store the global key capabilities
      canSign |= sign;
      canEncr |= encr;
    }
    else if( !strncmp( output.data() + offset, "Unknown type", 12 ) )
    { // line contains key data of unknown type (ignored)
      kdDebug(5100)<<"Unknown key type.\n";
    }
    else if( output[offset] == ' ' )
    { // line contains additional key data
      if( key == 0 )
        break;
      //kdDebug(5100)<<"Additional key data:\n";

      int pos = offset + 1;
      while( output[pos] == ' ' )
        pos++;

      if( !strncmp( output.data() + pos, "Key fingerprint = ", 18 ) )
      { // line contains a fingerprint
        // Example:
        //           Key fingerprint =  D0 6C BB 3A F5 16 82 C4  F3 A0 8A B3 7B 16 99 70

        fpr = true; // we found a fingerprint

        pos += 18;
        QCString fingerprint = output.mid( pos, eol-pos );
        // remove white space from the fingerprint
	for ( int idx = 0 ; (idx = fingerprint.find(' ', idx)) >= 0 ; )
	  fingerprint.replace( idx, 1, "" );

        //kdDebug(5100)<<"Fingerprint: "<<fingerprint<<endl;
        subkey->setFingerprint( fingerprint );
      }
      else
      { // line contains an additional user id
        // Example:
        //                               Test key (2nd user ID) <abc@xyz>

        //kdDebug(5100)<<"User ID: "<<output.mid( pos, eol-pos )<<endl;
        key->addUserID( output.mid( pos, eol-pos ) );
      }
    }
    else if( !strncmp( output.data() + offset, "sig", 3 ) )
    { // line contains signature data (ignored)
      //kdDebug(5100)<<"Signature.\n";
    }
    else // end of key data
      break;

    firstLine = false;
    offset = eol + 1;
  }

  if( key != 0 )
  {
    // set the global key capabilities
    key->setCanEncrypt( canEncr );
    key->setCanSign( canSign );
    key->setCanCertify( canSign );
    //kdDebug(5100)<<"Key capabilities: "<<(canEncr?"E":"")<<(canSign?"SC":"")<<endl;
  }

  return key;
}


Key*
Base6::parseSingleKey( const QCString& output, Key* key /* = 0 */ )
{
  int offset;

  // search start of header line
  if( !strncmp( output.data(), "Type bits", 9 ) )
    offset = 9;
  else
  {
    offset = output.find( "\nType bits" );
    if( offset == -1 )
      return 0;
    else
      offset += 10;
  }

  // key data begins in the next line
  offset = output.find( '\n', offset ) + 1;
  if( offset == 0 )
    return 0;

  key = parseKeyData( output, offset, key );

  //kdDebug(5100) << "finished parsing keys" << endl;

  return key;
}


KeyList
Base6::parseKeyList( const QCString& output, bool secretKeys )
{
  kdDebug(5100) << "Kpgp::Base6::parseKeyList()" << endl;
  KeyList keys;
  Key *key = 0;
  int offset;

  // search start of header line
  if( !strncmp( output.data(), "Type bits", 9 ) )
    offset = 0;
  else
  {
    offset = output.find( "\nType bits" ) + 1;
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
      key->setSecret( secretKeys );
      keys.append( key );
    }
  }
  while( key != 0 );

  //kdDebug(5100) << "finished parsing keys" << endl;

  return keys;
}


void
Base6::parseTrustDataForKey( Key* key, const QCString& str )
{
  if( ( key == 0 ) || str.isEmpty() )
    return;

  QCString keyID = "0x" + key->primaryKeyID();
  UserIDList userIDs = key->userIDs();

  // search the start of the trust data
  int offset = str.find( "\n\n  KeyID" );
  if( offset == -1 )
    return;

  offset = str.find( '\n', offset ) + 1;
  if( offset == 0 )
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
