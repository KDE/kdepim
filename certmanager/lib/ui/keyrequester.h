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

#include <vector>

namespace Kleo {
  class KeyListView;
  class KeyListViewItem;
  class CryptoBackend;
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

  class KeyRequester : public QWidget {
    Q_OBJECT
  public:
    KeyRequester( const CryptoBackend * backend,
		  unsigned int allowedKeys, bool multipleKeys=false,
		  QWidget * parent=0, const char * name=0 );
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


    QPushButton * eraseButton() const;
    QPushButton * dialogButton() const;

    void setDialogCaption( const QString & caption );
    void setDialogMessage( const QString & message );

    bool isMultipleKeysEnabled() const;
    void setMultipleKeysEnabled( bool enable );

    unsigned int allowedKeys() const;
    void setAllowedKeys( unsigned int allowed );

  private:
    void startKeyListJob( const QStringList & fingerprints );
    void updateKeys();

  private slots:
    void slotNextKey( const GpgME::Key & key );
    void slotKeyListResult( const GpgME::KeyListResult & result );
    void slotDialogButtonClicked();
    void slotEraseButtonClicked();

  private:
    const CryptoBackend * mBackend;
    QLabel * mLabel;
    QPushButton * mEraseButton;
    QPushButton * mDialogButton;
    QString mDialogCaption, mDialogMessage;
    bool mMulti;
    unsigned int mKeyUsage;
    std::vector<GpgME::Key> mKeys;
    std::vector<GpgME::Key> mTmpKeys;

  private:
    class Private;
    Private * d;
  protected:
    virtual void virtual_hook( int, void* );
  };


  class EncryptionKeyRequester : public KeyRequester {
    Q_OBJECT
  public:
    EncryptionKeyRequester( const CryptoBackend * backend,
			    bool multipleKeys=false,
			    QWidget * parent=0, const char * name=0,
			    bool onlyTrusted=true, bool onlyValid=true );
    ~EncryptionKeyRequester();

  private:
    class Private;
    Private * d;
  protected:
    virtual void virtual_hook( int, void* );
  };


  class SigningKeyRequester : public KeyRequester {
    Q_OBJECT
  public:
    SigningKeyRequester( const CryptoBackend * backend,
			bool multipleKeys=false,
			QWidget * parent=0, const char * name=0,
			bool onlyTrusted=true, bool onlyValid=true );
    ~SigningKeyRequester();

  private:
    class Private;
    Private * d;
  protected:
    virtual void virtual_hook( int, void* );
  };

}

#endif // __KLEO_UI_KEYREQUESTER_H__
