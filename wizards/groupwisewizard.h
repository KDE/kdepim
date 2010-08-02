/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef GROUPWISEWIZARD_H
#define GROUPWISEWIZARD_H

#include <kconfigwizard.h>
#include <kdepimmacros.h>

class KLineEdit;
class TQCheckBox;
class TQSpinBox;
class TQGroupBox;

class KDE_EXPORT GroupwiseWizard : public KConfigWizard
{
    Q_OBJECT
  public:
    GroupwiseWizard();
    ~GroupwiseWizard();

    TQString validate();
    void usrReadConfig();
    void usrWriteConfig();

  protected slots:
    void slotAboutToShowPage( TQWidget * );

  private:
    KLineEdit *mServerEdit;
    KLineEdit *mPathEdit;
    TQSpinBox *mPortEdit;
    KLineEdit *mUserEdit;
    KLineEdit *mPasswordEdit;
    TQCheckBox *mSavePasswordCheck;
    TQCheckBox *mSecureCheck;

    TQFrame *mEmailPage;
    TQGroupBox *mEmailBox;
    TQWidget *mEmailWidget;
    KLineEdit *mEmailEdit;
    KLineEdit *mFullNameEdit;
};

#endif
