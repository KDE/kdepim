/*  -*- c++ -*-
    keyselectiondialog.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

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

#include "kleo_export.h"
#include "kleo/cryptobackend.h"

#include <gpgme++/key.h>

#include <kdialog.h>

#include <QPixmap>

#include <vector>

class QVBoxLayout;
class QCheckBox;
class QPixmap;
class QTimer;
class QRegExp;
class QPoint;

namespace Kleo {
  class KeyListView;
  class KeyListViewItem;
}

namespace GpgME {
  class KeyListResult;
}

namespace Kleo {

  class KLEO_EXPORT KeySelectionDialog : public KDialog {
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

    KeySelectionDialog( const QString & title,
                        const QString & text,
                        const std::vector<GpgME::Key> & selectedKeys=std::vector<GpgME::Key>(),
                        unsigned int keyUsage=AllKeys,
                        bool extendedSelection=false,
                        bool rememberChoice=false,
                        QWidget * parent=0,
                        bool modal=true );
    KeySelectionDialog( const QString & title,
                        const QString & text,
                        const QString & initialPattern,
                        const std::vector<GpgME::Key> & selectedKeys,
                        unsigned int keyUsage=AllKeys,
                        bool extendedSelection=false,
                        bool rememberChoice=false,
                        QWidget * parent=0,
                        bool modal=true );
    KeySelectionDialog( const QString & title,
                        const QString & text,
                        const QString & initialPattern,
                        unsigned int keyUsage=AllKeys,
                        bool extendedSelection=false,
                        bool rememberChoice=false,
                        QWidget * parent=0,
                        bool modal=true );
    ~KeySelectionDialog();

    /** Returns the key ID of the selected key in single selection mode.
        Otherwise it returns a null key. */
    const GpgME::Key & selectedKey() const;

    QString fingerprint() const;

    /** Returns a list of selected key IDs. */
    const std::vector<GpgME::Key> & selectedKeys() const { return mSelectedKeys; }

    /// Return all the selected fingerprints
    QStringList fingerprints() const;

    /// Return the selected openpgp fingerprints
    QStringList pgpKeyFingerprints() const;
    /// Return the selected smime fingerprints
    QStringList smimeFingerprints() const;

    bool rememberSelection() const;

    // Could be used by derived classes to insert their own widget
    QVBoxLayout* topLayout() const { return mTopLayout; }

  private Q_SLOTS:
    void slotRereadKeys();
    void slotStartCertificateManager( const QString &query = QString() );
    void slotStartSearchForExternalCertificates() {
      slotStartCertificateManager( mInitialQuery );
    }
    void slotKeyListResult( const GpgME::KeyListResult & );
    void slotSelectionChanged();
    void slotCheckSelection() { slotCheckSelection( 0 ); }
    void slotCheckSelection( Kleo::KeyListViewItem * );
    void slotRMB( Kleo::KeyListViewItem *, const QPoint & );
    void slotRecheckKey();
    void slotTryOk();
    void slotOk();
    void slotCancel();
    void slotSearch( const QString & text );
    void slotSearch();
    void slotFilter();

  private:
    void filterByKeyID( const QString & keyID );
    void filterByKeyIDOrUID( const QString & keyID );
    void filterByUID( const QString & uid );
    void showAllItems();
    bool anyChildMatches( const Kleo::KeyListViewItem * item, QRegExp & rx ) const;

    void connectSignals();
    void disconnectSignals();

    void startKeyListJobForBackend( const Kleo::CryptoBackend::Protocol *, const std::vector<GpgME::Key> &, bool );
    void startValidatingKeyListing();

    void init( bool, bool, const QString &, const QString & );

  private:
    QVBoxLayout* mTopLayout;
    Kleo::KeyListView * mKeyListView;
    const Kleo::CryptoBackend::Protocol * mOpenPGPBackend;
    const Kleo::CryptoBackend::Protocol * mSMIMEBackend;
    QCheckBox * mRememberCB;
    std::vector<GpgME::Key> mSelectedKeys, mKeysToCheck;
    unsigned int mKeyUsage;
    QTimer * mCheckSelectionTimer;
    QTimer * mStartSearchTimer;
    // cross-eventloop temporaries:
    QString mSearchText;
    const QString mInitialQuery;
    Kleo::KeyListViewItem * mCurrentContextMenuItem;
    int mTruncated, mListJobCount, mSavedOffsetY;
  };

}

#endif // __KLEO_UI_KEYSELECTIONDIALOG_H__
