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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef GROUPWISEWIZARD_H
#define GROUPWISEWIZARD_H

#include <kconfigwizard.h>
#include <kdepimmacros.h>

class KLineEdit;
class QCheckBox;
class QSpinBox;
class QGroupBox;

class KDE_EXPORT GroupwiseWizard : public KConfigWizard
{
    Q_OBJECT
  public:
    GroupwiseWizard();
    ~GroupwiseWizard();

    QString validate();
    void usrReadConfig();
    void usrWriteConfig();

  protected slots:
    void slotAboutToShowPage( QWidget * );

  private:
    KLineEdit *mServerEdit;
    QSpinBox *mPortEdit;
    KLineEdit *mUserEdit;
    KLineEdit *mPasswordEdit;
    QCheckBox *mSavePasswordCheck;
    QCheckBox *mSecureCheck;

    QFrame *mEmailPage;
    QGroupBox *mEmailBox;
    QWidget *mEmailWidget;
    KLineEdit *mEmailEdit;
    KLineEdit *mFullNameEdit;
};

#endif
