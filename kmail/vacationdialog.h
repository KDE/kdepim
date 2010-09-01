/*  -*- c++ -*-
    vacationdialog.h

    KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef __KMAIL_VACATIONDIALOG_H__
#define __KMAIL_VACATIONDIALOG_H__

#include "kdialogbase.h"

class TQString;
class TQCheckBox;
class TQLineEdit;
class TQTextEdit;
class KDateWidget;
class KIntSpinBox;
template <typename T> class TQValueList;

namespace KMime {
  namespace Types {
    struct AddrSpec;
    typedef TQValueList<AddrSpec> AddrSpecList;
  }
}

namespace KMail {

  class VacationDialog : public KDialogBase {
    Q_OBJECT
  public:
    VacationDialog( const TQString & caption, TQWidget * parent=0,
		    const char * name=0, bool modal=true );
    virtual ~VacationDialog();

    virtual void enableDomainAndSendForSpam( bool enable = true );

    bool activateVacation() const;
    virtual void setActivateVacation( bool activate );

    bool domainCheck() const;
    virtual void setDomainCheck( bool check );

    TQString messageText() const;
    virtual void setMessageText( const TQString & text );

    int notificationInterval() const;
    virtual void setNotificationInterval( int days );

    KMime::Types::AddrSpecList mailAliases() const;
    virtual void setMailAliases( const KMime::Types::AddrSpecList & aliases );
    virtual void setMailAliases( const TQString & aliases );

    TQString domainName() const;
    virtual void setDomainName( const TQString & domain );

    bool sendForSpam() const;
    virtual void setSendForSpam( bool enable );


  private slots:
    void slotIntervalSpinChanged( int value );

  protected:
    TQCheckBox   * mActiveCheck;
    KIntSpinBox * mIntervalSpin;
    TQLineEdit   * mMailAliasesEdit;
    TQTextEdit   * mTextEdit;
    TQCheckBox   * mSpamCheck;
    TQCheckBox   * mDomainCheck;
    TQLineEdit   * mDomainEdit;

  };

} // namespace KMail

#endif // __KMAIL_VACATIONDIALOG_H__
