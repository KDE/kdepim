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

#include <kdialogbase.h>  // base class of all dialogs here
#include <tqwidget.h>      // base class of Config
#include <tqcheckbox.h>    // used in inlined methods
#include <kdebug.h>       // used in inlined methods
#include <tqcstring.h>     // used in return-by-value
#include <tqstring.h>      // is a member in KeyRequester
#include <tqvaluevector.h> // used in KeyApprovalDialog

#include "kpgp.h"

#include <kdepimmacros.h>

class TQString;
class TQRegExp;
class TQCString;
class TQCheckBox;            // needed by Config, KeySelectionDialog
class TQMultiLineEdit;       // needed by CipherTextDialog
class TQComboBox;            // needed by Config
class TQPixmap;              // needed by KeySelectionDialog
class TQPushButton;          // needed by KeyRequester
class TQTimer;               // needed by KeySelectionDialog

class KListView;            // needed by KeySelectionDialog
class KPasswordEdit;        // needed by PassphraseDialog

namespace Kpgp {

class Module;
class KeyList;              // needed by KeySelectionDialog
class Key;                  // needed by KeySelectionDialog
class KeyIDList;            // needed by KeySelectionDialog

/** the passphrase dialog */
class KDE_EXPORT PassphraseDialog : public KDialogBase
{
  Q_OBJECT

  public:
    PassphraseDialog( TQWidget *parent=0, const TQString &caption=TQString::null,
                      bool modal=true, const TQString &keyID=TQString::null);
    virtual ~PassphraseDialog();

    const char * passphrase();

  private:
    KPasswordEdit *lineedit;
};


// -------------------------------------------------------------------------
/** a widget for configuring the pgp interface. Can be included into
    a tabdialog. This widget by itself does not provide an apply/cancel
    button mechanism. */
class KDE_EXPORT Config : public QWidget
{
  Q_OBJECT

  public:
    Config(TQWidget *parent = 0, const char *name = 0, bool encrypt =true);
    virtual ~Config();

    virtual void setValues();
    virtual void applySettings();
    TQGroupBox* optionsGroupBox() { return mpOptionsGroupBox; };
  signals:
    void changed();

  protected:
    Module *pgp;
    TQCheckBox *storePass;
    TQCheckBox *encToSelf;
    TQCheckBox *showCipherText;
    TQCheckBox *showKeyApprovalDlg;
    TQComboBox *toolCombo;
    TQGroupBox* mpOptionsGroupBox;
};


// -------------------------------------------------------------------------
#define KeySelectionDialogSuper KDialogBase
class KDE_EXPORT KeySelectionDialog: public KeySelectionDialogSuper
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
                        const TQString& title,
                        const TQString& text = TQString::null,
                        const KeyIDList& keyIds = KeyIDList(),
                        const bool rememberChoice = false,
                        const unsigned int allowedKeys = AllKeys,
                        const bool extendedSelection = false,
                        TQWidget *parent=0, const char *name=0,
                        bool modal=true );
    virtual ~KeySelectionDialog();

    /** Returns the key ID of the selected key in single selection mode.
        Otherwise it returns a null string. */
    virtual KeyID key() const;
 
    /** Returns a list of selected key IDs. */
    virtual KeyIDList keys() const
      { return mKeyIds; };

    virtual bool rememberSelection() const
      { if( mRememberCB )
          return mRememberCB->isChecked();
        else
          return false;
      };

  protected slots:
    virtual void slotRereadKeys();
    virtual void slotSelectionChanged( TQListViewItem* );
    virtual void slotSelectionChanged();
    virtual void slotCheckSelection( TQListViewItem* = 0 );
    virtual void slotRMB( TQListViewItem*, const TQPoint&, int );
    virtual void slotRecheckKey();
    virtual void slotOk();
    virtual void slotCancel();
    virtual void slotSearch( const TQString & text );
    virtual void slotFilter();

  private:
    void filterByKeyID( const TQString & keyID );
    void filterByKeyIDOrUID( const TQString & keyID );
    void filterByUID( const TQString & uid );
    void showAllItems();
    bool anyChildMatches( const TQListViewItem * item, TQRegExp & rx ) const;

    void initKeylist( const KeyList& keyList, const KeyIDList& keyIds );

    TQString keyInfo( const Kpgp::Key* ) const;

    TQString beautifyFingerprint( const TQCString& ) const;

    // Returns the key ID of the key the given TQListViewItem belongs to
    KeyID getKeyId( const TQListViewItem* ) const;

    // Returns: -1 = unusable, 0 = unknown, 1 = valid, but untrusted, 2 = trusted
    int keyValidity( const Kpgp::Key* ) const;

    // Updates the given TQListViewItem with the data of the given key
    void updateKeyInfo( const Kpgp::Key*, TQListViewItem* ) const;

    /** Checks if choosing the given key is allowed
        Returns:
        -1 = key must not be chosen,
         0 = not enough information to decide whether the give key is allowed
             or not,
         1 = key can be chosen
    */
    int keyAdmissibility( TQListViewItem*,
                          TrustCheckMode = NoExpensiveTrustCheck ) const;

    // Perform expensive trust checks for the given keys
    bool checkKeys( const TQValueList<TQListViewItem*>& ) const;

  private:
    KListView *mListView;
    TQCheckBox *mRememberCB;
    TQPixmap *mKeyGoodPix, *mKeyBadPix, *mKeyUnknownPix, *mKeyValidPix;
    KeyIDList mKeyIds;
    unsigned int mAllowedKeys;
    TQTimer* mCheckSelectionTimer;
    TQTimer* mStartSearchTimer;
    TQString mSearchText;
    TQListViewItem* mCurrentContextMenuItem;

  static const int sCheckSelectionDelay;
};

class KDE_EXPORT KeyRequester: public QWidget
{
  Q_OBJECT

public:
  KeyRequester( TQWidget * parent=0, bool multipleKeys=false,
		unsigned int allowedKeys=AllKeys, const char * name=0 );
  virtual ~KeyRequester();

  KeyIDList keyIDs() const;
  void setKeyIDs( const KeyIDList & keyIDs );

  TQPushButton * eraseButton() const { return mEraseButton; }
  TQPushButton * dialogButton() const { return mDialogButton; }

  void setDialogCaption( const TQString & caption );
  void setDialogMessage( const TQString & message );

  bool isMultipleKeysEnabled() const;
  void setMultipleKeysEnabled( bool enable );

  int allowedKeys() const;
  void setAllowedKeys( int allowed );

protected:
  /** Reimplement this to return a list of selected keys. */
  virtual KeyIDList keyRequestHook( Module * pgp ) const = 0;

protected:
  TQLabel * mLabel;
  TQPushButton * mEraseButton;
  TQPushButton * mDialogButton;
  TQString mDialogCaption, mDialogMessage;
  bool mMulti;
  int mAllowedKeys;
  KeyIDList mKeys;

protected slots:
  void slotDialogButtonClicked();
  void slotEraseButtonClicked();

signals:
  void changed();

private:
  class Private;
  Private * d;
protected:
  virtual void virtual_hook( int, void* );
};


class KDE_EXPORT PublicKeyRequester : public KeyRequester {
  Q_OBJECT
public:
  PublicKeyRequester( TQWidget * parent=0, bool multipleKeys=false,
		      unsigned int allowedKeys=PublicKeys, const char * name=0 );
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


class KDE_EXPORT SecretKeyRequester : public KeyRequester {
  Q_OBJECT
public:
  SecretKeyRequester( TQWidget * parent=0, bool multipleKeys=false,
		      unsigned int allowedKeys=SecretKeys, const char * name=0 );
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
class KDE_EXPORT KeyApprovalDialog: public KDialogBase
{
  Q_OBJECT

  public:
    KeyApprovalDialog( const TQStringList&,
                       const TQValueVector<KeyIDList>&,
                       const int allowedKeys,
                       TQWidget *parent = 0, const char *name = 0,
                       bool modal = true );
    virtual ~KeyApprovalDialog() {};

    TQValueVector<KeyIDList> keys() const { return mKeys; };

    bool preferencesChanged() const { return mPrefsChanged; }

  protected slots:
    void slotPrefsChanged( int ) { mPrefsChanged = true; };
    void slotChangeEncryptionKey( int );
    virtual void slotOk();
    virtual void slotCancel();

  private:
    TQValueVector<KeyIDList> mKeys;
    int mAllowedKeys;
    int mEncryptToSelf;
    bool mPrefsChanged;
    TQPtrVector<TQLabel> mAddressLabels;
    TQPtrVector<TQLabel> mKeyIdsLabels;
    //TQPtrVector<TQListBox> mKeyIdListBoxes;
    TQPtrVector<TQComboBox> mEncrPrefCombos;
};


// -------------------------------------------------------------------------
class KDE_EXPORT CipherTextDialog: public KDialogBase
{
  Q_OBJECT

  public:
    CipherTextDialog( const TQCString & text, const TQCString & charset=0,
                      TQWidget *parent=0, const char *name=0, bool modal=true );
    virtual ~CipherTextDialog() {};

  private:
    void setMinimumSize();
    TQMultiLineEdit *mEditBox;
};

} // namespace Kpgp

#endif
