/*  -*- c++ -*-
    keyrequester.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.


    Based on kpgpui.h
    Copyright (C) 2001,2002 the KPGP authors
    See file libkdenetwork/AUTHORS.kpgp for details

    This file is part of KPGP, the KDE PGP/GnuPG support library.

    KPGP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifndef __KLEO_UI_KEYREQUESTER_H__
#define __KLEO_UI_KEYREQUESTER_H__

#include <qwidget.h>
#include <kleo/cryptobackend.h>

#include <vector>
#include <kdepimmacros.h>

namespace Kleo {
  class KeyListView;
  class KeyListViewItem;
}

namespace GpgME {
  class Key;
  class KeyListResult;
}

class QStringList;
class QString;
class QPushButton;
class QLabel;

namespace Kleo {

  /// Base class for SigningKeyRequester and EncryptionKeyRequester
  class KDE_EXPORT KeyRequester : public QWidget {
    Q_OBJECT
  public:
    KeyRequester( unsigned int allowedKeys, bool multipleKeys=false,
		  QWidget * parent=0, const char * name=0 );
    // Constructor for Qt Designer
    KeyRequester( QWidget * parent=0, const char * name=0 );
    ~KeyRequester();

    const GpgME::Key & key() const;
    /** Preferred method to set a key for
	non-multi-KeyRequesters. Doesn't start a backend
	KeyListJob.
    */
    void setKey( const GpgME::Key & key );

    const std::vector<GpgME::Key> & keys() const;
    /** Preferred method to set a key for multi-KeyRequesters. Doesn't
	start a backend KeyListJob.
    */
    void setKeys( const std::vector<GpgME::Key> & keys );

    QString fingerprint() const;
    /** Set the key by fingerprint. Starts a background KeyListJob to
	retrive the complete GpgME::Key object
    */
    void setFingerprint( const QString & fingerprint );

    QStringList fingerprints() const;
    /** Set the keys by fingerprint. Starts a background KeyListJob to
	retrive the complete GpgME::Key objects
    */
    void setFingerprints( const QStringList & fingerprints );


    QPushButton * eraseButton();
    QPushButton * dialogButton();

    void setDialogCaption( const QString & caption );
    void setDialogMessage( const QString & message );

    bool isMultipleKeysEnabled() const;
    void setMultipleKeysEnabled( bool enable );

    unsigned int allowedKeys() const;
    void setAllowedKeys( unsigned int allowed );

    void setInitialQuery( const QString & s ) { mInitialQuery = s; }
    const QString & initialQuery() const { return mInitialQuery; }

  signals:
    void changed();

  private:
    void init();
    void startKeyListJob( const QStringList & fingerprints );
    void updateKeys();

  private slots:
    void slotNextKey( const GpgME::Key & key );
    void slotKeyListResult( const GpgME::KeyListResult & result );
    void slotDialogButtonClicked();
    void slotEraseButtonClicked();

  private:
    const CryptoBackend::Protocol * mOpenPGPBackend;
    const CryptoBackend::Protocol * mSMIMEBackend;
    QLabel * mLabel;
    QPushButton * mEraseButton;
    QPushButton * mDialogButton;
    QString mDialogCaption, mDialogMessage, mInitialQuery;
    bool mMulti;
    unsigned int mKeyUsage;
    int mJobs;
    std::vector<GpgME::Key> mKeys;
    std::vector<GpgME::Key> mTmpKeys;

  private:
    class Private;
    Private * d;
  protected:
    virtual void virtual_hook( int, void* );
  };


  class KDE_EXPORT EncryptionKeyRequester : public KeyRequester {
    Q_OBJECT
  public:
    enum { OpenPGP = 1, SMIME = 2, AllProtocols = OpenPGP|SMIME };

    /**
     * Preferred constructor
     */
    EncryptionKeyRequester( bool multipleKeys=false, unsigned int proto=AllProtocols,
			    QWidget * parent=0, const char * name=0,
			    bool onlyTrusted=true, bool onlyValid=true );
    /**
     * Constructor for Qt designer
     */
    EncryptionKeyRequester( QWidget * parent=0, const char * name=0 );
    ~EncryptionKeyRequester();

    void setAllowedKeys( unsigned int proto, bool onlyTrusted=true, bool onlyValid=true );

  private:
    class Private;
    Private * d;
  protected:
    virtual void virtual_hook( int, void* );
  };


  class KDE_EXPORT SigningKeyRequester : public KeyRequester {
    Q_OBJECT
  public:
    enum { OpenPGP = 1, SMIME = 2, AllProtocols = OpenPGP|SMIME };

    /**
     * Preferred constructor
     * @param multipleKeys whether multiple keys can be selected
     *
     * @param proto the allowed protocols, OpenPGP and/or SMIME
     * @param onlyTrusted only show trusted keys
     * @param onlyValid only show valid keys
     */
    SigningKeyRequester( bool multipleKeys=false, unsigned int proto=AllProtocols,
			 QWidget * parent=0, const char * name=0,
			 bool onlyTrusted=true, bool onlyValid=true );
    /**
     * Constructor for Qt designer
     */
    SigningKeyRequester( QWidget * parent=0, const char * name=0 );
    ~SigningKeyRequester();

    /*
     * Those parameters affect the parameters given to the key selection dialog.
     * @param proto the allowed protocols, OpenPGP and/or SMIME
     * @param onlyTrusted only show trusted keys
     * @param onlyValid only show valid keys
     */
    void setAllowedKeys( unsigned int proto, bool onlyTrusted=true, bool onlyValid=true );

  private:
    class Private;
    Private * d;
  protected:
    virtual void virtual_hook( int, void* );
  };

}

#endif // __KLEO_UI_KEYREQUESTER_H__
