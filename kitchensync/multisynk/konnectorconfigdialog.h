/*
    This file is part of KitchenSync.
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    Based on the code of KRES::ConfigDialog from kdelibs

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KONNECTORCONFIGDIALOG_H
#define KONNECTORCONFIGDIALOG_H

#include <kdialogbase.h>

class QCheckBox;
class QWidget;

class KLineEdit;

namespace KRES {
class ConfigWidget;
}

namespace KSync {
class Konnector;
}

class KonnectorConfigDialog : public KDialogBase
{
  Q_OBJECT

  public:
    KonnectorConfigDialog( QWidget *parent, KSync::Konnector* konnector );

    void setInEditMode( bool value );

  protected slots:
    void accept();
    void setReadOnly( bool value );
    void slotNameChanged( const QString &text );

  private:
    QWidget *createGeneralPage( QWidget* );
    QWidget *createFilterPage( QWidget* );

    KRES::ConfigWidget *mConfigWidget;
    KSync::Konnector* mKonnector;

    KLineEdit *mName;
    QCheckBox *mReadOnly;
};

#endif
