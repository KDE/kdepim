/*
    kpgpbase.h

    Copyright (C) 2001,2002 the KPGP authors
    See file AUTHORS.kpgp for details

    This file is part of KPGP, the KDE PGP/GnuPG support library.

    KPGP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef KPGPBASE_H
#define KPGPBASE_H

#include "kpgpkey.h"
#include "kpgpblock.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Kpgp {

class Base
{
public:

  /** virtual class used internally by kpgp */
  Base();
  virtual ~Base();


  /** Encrypts the message with the given keys. */
  virtual int encrypt( Block& , const KeyIDList& ) { return OK; }

  /** Clearsigns the message with the currently set key. */
  virtual int clearsign( Block& , const char *) { return OK; }

  /** Encrypts and signs the message with the given keys. */
  virtual int encsign( Block& , const KeyIDList& ,
                      const char * = 0) { return OK; }

  /** Decrypts the message. */
  virtual int decrypt( Block& , const char * = 0) { return OK; }

  /** Verifies the message. */
  virtual int verify( Block& block ) { return decrypt( block, 0 ); }


  /** Reads the key data for the given key and returns it. If
      <em>readTrust</em> is true then the trust of this key will be determined.
      If <em>key</em> is not null then the key data will be stored in given
      key.
   */
  virtual Key* readPublicKey( const KeyID&, const bool = false,
                              Key* = 0 )
    { return 0; }

  /** Returns the list of public keys in the users public keyring. */
  virtual KeyList publicKeys( const QStringList & = QStringList() )
 { return KeyList(); }

  /** Returns the list of secret keys in the users secret keyring. */
  virtual KeyList secretKeys( const QStringList & = QStringList() )
 { return KeyList(); }

  /** Returns the ascii armored data of the public key with the
      given key id. */
  virtual QByteArray getAsciiPublicKey(const KeyID& ) { return QByteArray(); }

  /** Signs the given key with the currently set user key. This is currently
      not implemented. */
  virtual int signKey(const KeyID& , const char *) { return OK; }


  /** Returns an error message if an error occurred during the last
      operation. */
  virtual QString lastErrorMessage() const;


protected:
  virtual int run( const char *cmd, const char *passphrase = 0,
                   bool onlyReadFromPGP = false );
  virtual int runGpg( const char *cmd, const char *passphrase = 0,
                      bool onlyReadFromGnuPG = false );
  virtual void clear();

  QByteArray addUserId();

  QByteArray input;
  QByteArray output;
  QByteArray error;
  QString errMsg;

  QByteArray mVersion;

  int status;

};

// ---------------------------------------------------------------------------

class Base2 : public Base
{

public:
  Base2();
  virtual ~Base2();

  virtual int encrypt( Block& block, const KeyIDList& recipients );
  virtual int clearsign( Block& block, const char *passphrase );
  virtual int encsign( Block& block, const KeyIDList& recipients,
                       const char *passphrase = 0 );
  virtual int decrypt( Block& block, const char *passphrase = 0 );
  virtual int verify( Block& block ) { return decrypt( block, 0 ); }

  virtual Key* readPublicKey( const KeyID& keyID,
                              const bool readTrust = false,
                              Key* key = 0 );
  virtual KeyList publicKeys( const QStringList & patterns = QStringList() );
  virtual KeyList secretKeys( const QStringList & patterns = QStringList() );
  virtual QByteArray getAsciiPublicKey( const KeyID& keyID );
  virtual int signKey( const KeyID& keyID, const char *passphrase );

protected:
  KeyList doGetPublicKeys( const QByteArray & cmd,
                           const QStringList & patterns );
  virtual KeyList parseKeyList( const QByteArray&, bool );

private:
  Key* parsePublicKeyData( const QByteArray& output, Key* key = 0 );
  void parseTrustDataForKey( Key* key, const QByteArray& str );
};

class BaseG : public Base
{

public:
  BaseG();
  virtual ~BaseG();

  virtual int encrypt( Block& block, const KeyIDList& recipients );
  virtual int clearsign( Block& block, const char *passphrase );
  virtual int encsign( Block& block, const KeyIDList& recipients,
                       const char *passphrase = 0 );
  virtual int decrypt( Block& block, const char *passphrase = 0 );
  virtual int verify( Block& block ) { return decrypt( block, 0 ); }

  virtual Key* readPublicKey( const KeyID& keyID,
                              const bool readTrust = false,
                              Key* key = 0 );
  virtual KeyList publicKeys( const QStringList & patterns = QStringList() );
  virtual KeyList secretKeys( const QStringList & patterns = QStringList() );
  virtual QByteArray getAsciiPublicKey( const KeyID& keyID );
  virtual int signKey( const KeyID& keyID, const char *passphrase );

private:
  Key* parseKeyData( const QByteArray& output, int& offset, Key* key = 0 );
  KeyList parseKeyList( const QByteArray&, bool );
};


class Base5 : public Base
{

public:
  Base5();
  virtual ~Base5();

  virtual int encrypt( Block& block, const KeyIDList& recipients );
  virtual int clearsign( Block& block, const char *passphrase );
  virtual int encsign( Block& block, const KeyIDList& recipients,
                       const char *passphrase = 0 );
  virtual int decrypt( Block& block, const char *passphrase = 0 );
  virtual int verify( Block& block ) { return decrypt( block, 0 ); }

  virtual Key* readPublicKey( const KeyID& keyID,
                              const bool readTrust = false,
                              Key* key = 0 );
  virtual KeyList publicKeys( const QStringList & patterns = QStringList() );
  virtual KeyList secretKeys( const QStringList & patterns = QStringList() );
  virtual QByteArray getAsciiPublicKey( const KeyID& keyID );
  virtual int signKey( const KeyID& keyID, const char *passphrase );

private:
  Key* parseKeyData( const QByteArray& output, int& offset, Key* key = 0 );
  Key* parseSingleKey( const QByteArray& output, Key* key = 0 );
  KeyList parseKeyList( const QByteArray& output, bool );
  void parseTrustDataForKey( Key* key, const QByteArray& str );
};


class Base6 : public Base2
{

public:
  Base6();
  virtual ~Base6();

  virtual int decrypt( Block& block, const char *passphrase = 0 );
  virtual int verify( Block& block ) { return decrypt( block, 0 ); }

  virtual Key* readPublicKey( const KeyID& keyID,
                              const bool readTrust = false,
                              Key* key = 0 );
  virtual KeyList publicKeys( const QStringList & patterns = QStringList() );
  virtual KeyList secretKeys( const QStringList & patterns = QStringList() );

  virtual int isVersion6();

protected:
  virtual KeyList parseKeyList( const QByteArray &, bool );

private:
  Key* parseKeyData( const QByteArray& output, int& offset, Key* key = 0 );
  Key* parseSingleKey( const QByteArray& output, Key* key = 0 );
  void parseTrustDataForKey( Key* key, const QByteArray& str );
};

// ---------------------------------------------------------------------------
// inlined functions

inline QString
Base::lastErrorMessage() const
{
  return errMsg;
}


} // namespace Kpgp

#endif
