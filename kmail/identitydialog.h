/*  -*- mode: C++; c-file-style: "gnu" -*-
    identitydialog.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
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

#ifndef __KMAIL_IDENTITYDIALOG_H__
#define __KMAIL_IDENTITYDIALOG_H__

#include <kdialogbase.h>


class TQLineEdit;
class TQCheckBox;
class TQComboBox;
class TQString;
class TQStringList;
class SimpleStringListEditor;
class TemplatesConfiguration;
class KPushButton;
namespace Kleo {
  class EncryptionKeyRequester;
  class SigningKeyRequester;
}
namespace KPIM {
  class Identity;
}
namespace KMail {
  class SignatureConfigurator;
  class XFaceConfigurator;
  class DictionaryComboBox;
  class FolderRequester;
}

namespace KMail {

  class IdentityDialog : public KDialogBase {
    Q_OBJECT
  public:
    IdentityDialog( TQWidget * parent=0, const char * name = 0 );
    virtual ~IdentityDialog();

    void setIdentity( /*_not_ const*/ KPIM::Identity & ident );

    void updateIdentity( KPIM::Identity & ident );

  public slots:
    void slotUpdateTransportCombo( const TQStringList & sl );

  protected slots:
    void slotAboutToShow( TQWidget * w );
    /*! \reimp */
    void slotOk();
    // copy default templates to identity templates
    void slotCopyGlobal();

  private:
    bool checkFolderExists( const TQString & folder, const TQString & msg );
    bool validateAddresses( const TQString & addresses );

  protected:
    // "general" tab:
    TQLineEdit                    *mNameEdit;
    TQLineEdit                    *mOrganizationEdit;
    TQLineEdit                    *mEmailEdit;
    SimpleStringListEditor       *mAliasEdit;
    // "cryptography" tab:
    TQWidget                      *mCryptographyTab;
    Kleo::SigningKeyRequester    *mPGPSigningKeyRequester;
    Kleo::EncryptionKeyRequester *mPGPEncryptionKeyRequester;
    Kleo::SigningKeyRequester    *mSMIMESigningKeyRequester;
    Kleo::EncryptionKeyRequester *mSMIMEEncryptionKeyRequester;
    TQComboBox                    *mPreferredCryptoMessageFormat;
    // "advanced" tab:
    TQLineEdit                    *mReplyToEdit;
    TQLineEdit                    *mBccEdit;
    KMail::DictionaryComboBox    *mDictionaryCombo;
    FolderRequester              *mFccCombo;
    FolderRequester              *mDraftsCombo;
    FolderRequester              *mTemplatesCombo;
    TQCheckBox                    *mTransportCheck;
    TQComboBox                    *mTransportCombo; // should be a KMTransportCombo...
    // "templates" tab:
    TemplatesConfiguration       *mWidget;
    TQCheckBox                    *mCustom;
    KPushButton                  *mCopyGlobal;
    // "signature" tab:
    KMail::SignatureConfigurator *mSignatureConfigurator;
    // "X-Face" tab:
    KMail::XFaceConfigurator *mXFaceConfigurator;
  };

} // namespace KMail

#endif // __KMAIL_IDENTITYDIALOG_H__
