/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef ADDHOSTDIALOG_H
#define ADDHOSTDIALOG_H

#include <kdialogbase.h>
#include <kabc/ldapconfigwidget.h>
#include <libkdepim/ldapclient.h>

class KLineEdit;
class QPushButton;
class QSpinBox;

class AddHostDialog : public KDialogBase
{
    Q_OBJECT

  public:
    AddHostDialog( KPIM::LdapServer* server, QWidget* parent = 0, const char* name = 0 );
    ~AddHostDialog();

  signals:
    void changed( bool );

  private slots:
    void slotHostEditChanged( const QString& );
    virtual void slotOk();
  private:
    KABC::LdapConfigWidget *mCfg;
    KPIM::LdapServer *mServer;
};

#endif // ADDHOSTDIALOG_H
