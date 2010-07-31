/*  -*- c++ -*-
    signatureconfigurator.cpp

    KMail, the KDE mail client.
    Copyright (c) 2002 the KMail authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef __KMAIL_SIGNATURECONFIGURATOR_H__
#define __KMAIL_SIGNATURECONFIGURATOR_H__

#include <tqwidget.h>

#include <libkpimidentities/identity.h> // for Signature::Type
using KPIM::Signature;

class QComboBox;
class QCheckBox;
class KURLRequester;
class KLineEdit;
class QString;
class QPushButton;
class QTextEdit;

namespace KMail {

  class SignatureConfigurator : public TQWidget {
    Q_OBJECT
  public:
    SignatureConfigurator( TQWidget * parent=0, const char * name=0 );
    virtual ~SignatureConfigurator();

    bool isSignatureEnabled() const;
    void setSignatureEnabled( bool enable );

    Signature::Type signatureType() const;
    void setSignatureType( Signature::Type type );

    TQString inlineText() const;
    void setInlineText( const TQString & text );

    TQString fileURL() const;
    void setFileURL( const TQString & url );

    TQString commandURL() const;
    void setCommandURL( const TQString & url );

    /**
       Conveniece method.
       @return a Signature object representing the state of the widgets.
     **/
    Signature signature() const;
    /**
       Convenience method. Sets the widgets according to @p sig
    **/
    void setSignature( const Signature & sig );

  protected slots:
    void slotEnableEditButton( const TQString & );
    void slotEdit();

  protected:
    TQCheckBox     * mEnableCheck;
    TQComboBox     * mSourceCombo;
    KURLRequester * mFileRequester;
    TQPushButton   * mEditButton;
    KLineEdit     * mCommandEdit;
    TQTextEdit     * mTextEdit;
  };

} // namespace KMail

#endif // __KMAIL_SIGNATURECONFIGURATOR_H__


