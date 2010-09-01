/*  -*- c++ -*-
    keyselectiondialog.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarävdalens Datakonsult AB

    Based on kpgpui.h
    Copyright (C) 2001,2002 the KPGP authors
    See file libkdenetwork/AUTHORS.kpgp for details

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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
*/

#ifndef __KLEO_UI_KEYSELECTIONDIALOG_H__
#define __KLEO_UI_KEYSELECTIONDIALOG_H__

#include <kdialogbase.h>

#include <kleo/cryptobackend.h>
#include <gpgmepp/key.h>
#include <kdepimmacros.h>
#include <vector>

class TQVBoxLayout;
class TQCheckBox;
class TQPixmap;
class TQTimer;
class TQListViewItem;
class TQRegExp;
class TQPoint;

namespace Kleo {
  class KeyListView;
  class KeyListViewItem;
}

namespace GpgME {
  class KeyListResult;
}

namespace Kleo {

  class KDE_EXPORT KeySelectionDialog : public KDialogBase {
    Q_OBJECT
  public:

    enum KeyUsage {
      PublicKeys = 1,
      SecretKeys = 2,
      EncryptionKeys = 4,
      SigningKeys = 8,
      ValidKeys = 16,
      TrustedKeys = 32,
      CertificationKeys = 64,
      AuthenticationKeys = 128,
      OpenPGPKeys = 256,
      SMIMEKeys = 512,
      AllKeys = PublicKeys | SecretKeys | OpenPGPKeys | SMIMEKeys,
      ValidEncryptionKeys = AllKeys | EncryptionKeys | ValidKeys,
      ValidTrustedEncryptionKeys = AllKeys | EncryptionKeys | ValidKeys | TrustedKeys
    };

    KeySelectionDialog( const TQString & title,
                        const TQString & text,
			const std::vector<GpgME::Key> & selectedKeys=std::vector<GpgME::Key>(),
                        unsigned int keyUsage=AllKeys,
                        bool extendedSelection=false,
			bool rememberChoice=false,
                        TQWidget * parent=0, const char * name=0,
                        bool modal=true );
    KeySelectionDialog( const TQString & title,
                        const TQString & text,
                        const TQString & initialPattern,
			const std::vector<GpgME::Key> & selectedKeys,
                        unsigned int keyUsage=AllKeys,
                        bool extendedSelection=false,
			bool rememberChoice=false,
                        TQWidget * parent=0, const char * name=0,
                        bool modal=true );
    KeySelectionDialog( const TQString & title,
                        const TQString & text,
			const TQString & initialPattern,
                        unsigned int keyUsage=AllKeys,
                        bool extendedSelection=false,
			bool rememberChoice=false,
                        TQWidget * parent=0, const char * name=0,
                        bool modal=true );
    ~KeySelectionDialog();

    /** Returns the key ID of the selected key in single selection mode.
        Otherwise it returns a null key. */
    const GpgME::Key & selectedKey() const;

    TQString fingerprint() const;

    /** Returns a list of selected key IDs. */
    const std::vector<GpgME::Key> & selectedKeys() const { return mSelectedKeys; }

    /// Return all the selected fingerprints
    TQStringList fingerprints() const;

    /// Return the selected openpgp fingerprints
    TQStringList pgpKeyFingerprints() const;
    /// Return the selected smime fingerprints
    TQStringList smimeFingerprints() const;

    bool rememberSelection() const;
  protected slots:
    // reimplemented to avoid popping up the help, since we
    // override the button
    void slotHelp();

    // Could be used by derived classes to insert their own widget
    TQVBoxLayout* topLayout() const { return mTopLayout; }

  private slots:
    void slotRereadKeys();
    void slotStartCertificateManager( const TQString &query = TQString() );
    void slotStartSearchForExternalCertificates() {
      slotStartCertificateManager( mInitialQuery );
    }
    void slotKeyListResult( const GpgME::KeyListResult & );
    void slotSelectionChanged();
    void slotCheckSelection() { slotCheckSelection( 0 ); }
    void slotCheckSelection( Kleo::KeyListViewItem * );
    void slotRMB( Kleo::KeyListViewItem *, const TQPoint & );
    void slotRecheckKey();
    void slotTryOk();
    void slotOk();
    void slotCancel();
    void slotSearch( const TQString & text );
    void slotSearch();
    void slotFilter();

  private:
    void filterByKeyID( const TQString & keyID );
    void filterByKeyIDOrUID( const TQString & keyID );
    void filterByUID( const TQString & uid );
    void showAllItems();
    bool anyChildMatches( const Kleo::KeyListViewItem * item, TQRegExp & rx ) const;

    void connectSignals();
    void disconnectSignals();

    void startKeyListJobForBackend( const Kleo::CryptoBackend::Protocol *, const std::vector<GpgME::Key> &, bool );
    void startValidatingKeyListing();

    void init( bool, bool, const TQString &, const TQString & );

  private:
    TQVBoxLayout* mTopLayout;
    Kleo::KeyListView * mKeyListView;
    const Kleo::CryptoBackend::Protocol * mOpenPGPBackend;
    const Kleo::CryptoBackend::Protocol * mSMIMEBackend;
    TQCheckBox * mRememberCB;
    std::vector<GpgME::Key> mSelectedKeys, mKeysToCheck;
    unsigned int mKeyUsage;
    TQTimer * mCheckSelectionTimer;
    TQTimer * mStartSearchTimer;
    // cross-eventloop temporaries:
    TQString mSearchText;
    const TQString mInitialQuery;
    Kleo::KeyListViewItem * mCurrentContextMenuItem;
    int mTruncated, mListJobCount, mSavedOffsetY;
  };

}

#endif // __KLEO_UI_KEYSELECTIONDIALOG_H__
