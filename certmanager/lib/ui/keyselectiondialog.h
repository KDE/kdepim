/*  -*- c++ -*-
    keyselectiondialog.h

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

#ifndef __KLEO_UI_KEYSELECTIONDIALOG_H__
#define __KLEO_UI_KEYSELECTIONDIALOG_H__

#include <kdialogbase.h>

#include <vector>

class QCheckBox;
class QPixmap;
class QTimer;
class QListViewItem;
class QRegExp;
class QPoint;

namespace Kleo {
  class KeyListView;
  class KeyListViewItem;
  class CryptoBackend;
}

namespace GpgME {
  class Key;
  class KeyListResult;
}

namespace Kleo {

  class KeySelectionDialog : public KDialogBase {
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
      AllKeys = PublicKeys | SecretKeys
    };

    KeySelectionDialog( const QString & title,
                        const QString & text=QString::null,
                        const Kleo::CryptoBackend * backend=0,
			const std::vector<GpgME::Key> & selectedKeys=std::vector<GpgME::Key>(),
                        unsigned int keyUsage=AllKeys,
                        bool extendedSelection=false,
			bool rememberChoice=false,
                        QWidget * parent=0, const char * name=0,
                        bool modal=true );
    ~KeySelectionDialog();

    /** Returns the key ID of the selected key in single selection mode.
        Otherwise it returns a null key. */
    const GpgME::Key & selectedKey() const;
 
    /** Returns a list of selected key IDs. */
    const std::vector<GpgME::Key> & selectedKeys() const { return mSelectedKeys; }

    bool rememberSelection() const;

  private slots:
    void slotRereadKeys();
    void slotKeyListResult( const GpgME::KeyListResult & );
    void slotSelectionChanged();
    void slotCheckSelection() { slotCheckSelection( 0 ); }
    void slotCheckSelection( Kleo::KeyListViewItem * );
    void slotRMB( Kleo::KeyListViewItem *, const QPoint &, int );
    void slotRecheckKey();
    void slotOk();
    void slotCancel();
    void slotSearch( const QString & text );
    void slotFilter();

  private:
    void filterByKeyID( const QString & keyID );
    void filterByKeyIDOrUID( const QString & keyID );
    void filterByUID( const QString & uid );
    void showAllItems();
    bool anyChildMatches( const Kleo::KeyListViewItem * item, QRegExp & rx ) const;

    void connectSignals();
    void disconnectSignals();

    void startKeyListJobForBackend( const Kleo::CryptoBackend * );

#if 0
    QString keyInfo( const Kpgp::Key* ) const;
    QString beautifyFingerprint( const QCString& ) const;
#endif

  private:
    Kleo::KeyListView * mKeyListView;
    const Kleo::CryptoBackend * mBackend;
    QCheckBox * mRememberCB;
    std::vector<GpgME::Key> mSelectedKeys;
    unsigned int mKeyUsage;
    QTimer * mCheckSelectionTimer;
    QTimer * mStartSearchTimer;
    // cross-eventloop temporaries:
    QString mSearchText;
    QListViewItem * mCurrentContextMenuItem;
    int mTruncated, mListJobCount, mSavedOffsetY;
  };

}

#endif // __KLEO_UI_KEYSELECTIONDIALOG_H__
