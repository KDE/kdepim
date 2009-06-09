/*  -*- c++ -*-
    kpgpui.h

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

#ifndef KPGPUI_H
#define KPGPUI_H

#include "libkpgp_export.h"
#include "kpgp.h"

#include <QtCore/QString>     // is a member in KeyRequester
#include <QtCore/QVector>     // used in KeyApprovalDialog
#include <QtGui/QWidget>      // base class of Config
#include <QtGui/QCheckBox>    // used in inlined methods
#include <QtGui/QPixmap>
#include <QtGui/QLabel>
#include <Qt3Support/Q3PtrVector>

#include <kdebug.h>       // used in inlined methods
#include <kdialog.h>      // base class of all dialogs here
#include <kpassworddialog.h>

class QString;
class QRegExp;
class QByteArray;
class QCheckBox;            // needed by Config, KeySelectionDialog
class Q3MultiLineEdit;      // needed by CipherTextDialog
class QComboBox;            // needed by Config
class QPixmap;              // needed by KeySelectionDialog
class QPushButton;          // needed by KeyRequester
class QTimer;               // needed by KeySelectionDialog
class QGroupBox;
class Q3ListViewItem;

class K3ListView;           // needed by KeySelectionDialog
class KPasswordEdit;        // needed by PassphraseDialog

namespace Kpgp {

class Module;
class KeyList;              // needed by KeySelectionDialog
class Key;                  // needed by KeySelectionDialog
class KeyIDList;            // needed by KeySelectionDialog

/** the passphrase dialog */
class KPGP_EXPORT PassphraseDialog : public KPasswordDialog
{
  Q_OBJECT

  public:
    explicit PassphraseDialog( QWidget *parent=0,
                               const QString &caption=QString(),
                               const QString &keyID=QString());
    virtual ~PassphraseDialog();

    QString passphrase();
};


// -------------------------------------------------------------------------
/** A widget for configuring the pgp interface. Can be included into
    a tabdialog. This widget by itself does not provide an apply/cancel
    button mechanism. */
class KPGP_EXPORT Config : public QWidget
{
  Q_OBJECT

  public:
    explicit Config( QWidget *parent = 0, bool encrypt = true );
    virtual ~Config();

    virtual void setValues();
    virtual void applySettings();
    QGroupBox* optionsGroupBox() { return mpOptionsGroupBox; }
  Q_SIGNALS:
    void changed();

  protected:
    Module *pgp;
    QCheckBox *storePass;
    QCheckBox *encToSelf;
    QCheckBox *showCipherText;
    QCheckBox *showKeyApprovalDlg;
    QComboBox *toolCombo;
    QGroupBox* mpOptionsGroupBox;
};


// -------------------------------------------------------------------------
#define KeySelectionDialogSuper KDialog
class KPGP_EXPORT KeySelectionDialog: public KeySelectionDialogSuper
{
  Q_OBJECT

  enum TrustCheckMode { NoExpensiveTrustCheck,
                        AllowExpensiveTrustCheck,
                        ForceTrustCheck
                      };

  public:
    /** allowedKeys: see kpgp.h
     */
    KeySelectionDialog( const KeyList& keyList,
                        const QString& title,
                        const QString& text = QString(),
                        const KeyIDList& keyIds = KeyIDList(),
                        const bool rememberChoice = false,
                        const unsigned int allowedKeys = AllKeys,
                        const bool extendedSelection = false,
                        QWidget *parent=0 );
    virtual ~KeySelectionDialog();

    /** Returns the key ID of the selected key in single selection mode.
        Otherwise it returns a null string. */
    virtual KeyID key() const;

    /** Returns a list of selected key IDs. */
    virtual KeyIDList keys() const
      { return mKeyIds; }

    virtual bool rememberSelection() const
      { if( mRememberCB )
          return mRememberCB->isChecked();
        else
          return false;
      }

  protected Q_SLOTS:
    virtual void slotRereadKeys();
    virtual void slotSelectionChanged( Q3ListViewItem* );
    virtual void slotSelectionChanged();
    virtual void slotCheckSelection( Q3ListViewItem* = 0 );
    virtual void slotRMB( Q3ListViewItem*, const QPoint&, int );
    virtual void slotRecheckKey();
    virtual void slotOk();
    virtual void slotCancel();
    virtual void slotSearch( const QString & text );
    virtual void slotFilter();

  private:
    void filterByKeyID( const QString & keyID );
    void filterByKeyIDOrUID( const QString & keyID );
    void filterByUID( const QString & uid );
    void showAllItems();
    bool anyChildMatches( const Q3ListViewItem * item, QRegExp & rx ) const;

    void initKeylist( const KeyList& keyList, const KeyIDList& keyIds );

    QString keyInfo( const Kpgp::Key* ) const;

    QString beautifyFingerprint( const QByteArray& ) const;

    // Returns the key ID of the key the given QListViewItem belongs to
    KeyID getKeyId( const Q3ListViewItem* ) const;

    // Returns: -1 = unusable, 0 = unknown, 1 = valid, but untrusted, 2 = trusted
    int keyValidity( const Kpgp::Key* ) const;

    // Updates the given QListViewItem with the data of the given key
    void updateKeyInfo( const Kpgp::Key*, Q3ListViewItem* ) const;

    /** Checks if choosing the given key is allowed
        Returns:
        -1 = key must not be chosen,
         0 = not enough information to decide whether the give key is allowed
             or not,
         1 = key can be chosen
    */
    int keyAdmissibility( Q3ListViewItem*,
                          TrustCheckMode = NoExpensiveTrustCheck ) const;

    // Perform expensive trust checks for the given keys
    bool checkKeys( const QList<Q3ListViewItem*>& ) const;

  private:
    K3ListView *mListView;
    QCheckBox *mRememberCB;
    QPixmap *mKeyGoodPix, *mKeyBadPix, *mKeyUnknownPix, *mKeyValidPix;
    KeyIDList mKeyIds;
    unsigned int mAllowedKeys;
    QTimer* mCheckSelectionTimer;
    QTimer* mStartSearchTimer;
    QString mSearchText;
    Q3ListViewItem* mCurrentContextMenuItem;

  static const int sCheckSelectionDelay;
};

class KPGP_EXPORT KeyRequester: public QWidget
{
  Q_OBJECT

public:
  explicit KeyRequester( QWidget * parent=0, bool multipleKeys=false,
                         unsigned int allowedKeys=AllKeys, const char * name=0 );
  virtual ~KeyRequester();

  KeyIDList keyIDs() const;
  void setKeyIDs( const KeyIDList & keyIDs );

  QPushButton * eraseButton() const { return mEraseButton; }
  QPushButton * dialogButton() const { return mDialogButton; }

  void setDialogCaption( const QString & caption );
  void setDialogMessage( const QString & message );

  bool isMultipleKeysEnabled() const;
  void setMultipleKeysEnabled( bool enable );

  int allowedKeys() const;
  void setAllowedKeys( int allowed );

protected:
  /** Reimplement this to return a list of selected keys. */
  virtual KeyIDList keyRequestHook( Module * pgp ) const = 0;

protected:
  QLabel * mLabel;
  QPushButton * mEraseButton;
  QPushButton * mDialogButton;
  QString mDialogCaption, mDialogMessage;
  bool mMulti;
  int mAllowedKeys;
  KeyIDList mKeys;

protected Q_SLOTS:
  void slotDialogButtonClicked();
  void slotEraseButtonClicked();

Q_SIGNALS:
  void changed();

private:
  class Private;
  Private * d;
protected:
  virtual void virtual_hook( int, void* );
};


class KPGP_EXPORT PublicKeyRequester : public KeyRequester {
  Q_OBJECT
public:
  explicit PublicKeyRequester( QWidget * parent=0, bool multipleKeys=false,
                               unsigned int allowedKeys=PublicKeys,
                               const char * name=0 );
  virtual ~PublicKeyRequester();

protected:
  KeyIDList keyRequestHook( Module * pgp ) const;

private:
  typedef KeyRequester base;
  class Private;
  Private * d;
protected:
  virtual void virtual_hook( int, void* );
};


class KPGP_EXPORT SecretKeyRequester : public KeyRequester {
  Q_OBJECT
public:
  explicit SecretKeyRequester( QWidget * parent=0, bool multipleKeys=false,
                               unsigned int allowedKeys=SecretKeys,
                               const char * name=0 );
  virtual ~SecretKeyRequester();

protected:
  KeyIDList keyRequestHook( Module * pgp ) const;

private:
  typedef KeyRequester base;
  class Private;
  Private * d;
protected:
  virtual void virtual_hook( int, void* );
};


// -------------------------------------------------------------------------
class KPGP_EXPORT KeyApprovalDialog: public KDialog
{
  Q_OBJECT

  public:
    KeyApprovalDialog( const QStringList&,
                       const QVector<KeyIDList>&,
                       const int allowedKeys,
                       QWidget *parent = 0 );
    virtual ~KeyApprovalDialog() {}

    QVector<KeyIDList> keys() const { return mKeys; }

    bool preferencesChanged() const { return mPrefsChanged; }

  protected Q_SLOTS:
    void slotPrefsChanged( int ) { mPrefsChanged = true; }
    void slotChangeEncryptionKey( int );
    virtual void slotOk();
    virtual void slotCancel();

  private:
    QVector<KeyIDList> mKeys;
    int mAllowedKeys;
    int mEncryptToSelf;
    bool mPrefsChanged;
    Q3PtrVector<QLabel> mAddressLabels;
    Q3PtrVector<QLabel> mKeyIdsLabels;
    //QPtrVector<QListBox> mKeyIdListBoxes;
    Q3PtrVector<QComboBox> mEncrPrefCombos;
};


// -------------------------------------------------------------------------
class KPGP_EXPORT CipherTextDialog: public KDialog
{
  Q_OBJECT

  public:
    explicit CipherTextDialog( const QByteArray &text,
                               const QByteArray &charset=0,
                               QWidget *parent=0 );
    virtual ~CipherTextDialog() {}

  private:
    void setMinimumSize();
    Q3MultiLineEdit *mEditBox;
};

} // namespace Kpgp

#endif
