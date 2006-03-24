/*
    This file is part of libkpimexchange.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEPIM_RESOURCECALENDAREXCHANGECONFIG_H
#define KDEPIM_RESOURCECALENDAREXCHANGECONFIG_H

#include <qcheckbox.h>

#include <kcombobox.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <knuminput.h>

#include <kresources/resource.h>
#include <kresources/configwidget.h>

namespace KCal {

class ResourceExchangeConfig : public KRES::ConfigWidget
{ 
  Q_OBJECT

public:
  ResourceExchangeConfig( QWidget* parent = 0, const char* name = 0 );

public slots:
  virtual void loadSettings( KRES::Resource *resource);
  virtual void saveSettings( KRES::Resource *resource );

protected slots:
  void slotToggleAuto( bool on );
  void slotUserChanged( const QString& text );
  void slotFindClicked();
  void slotCacheEditChanged( int value );

private:
  KLineEdit* mHostEdit;
  KLineEdit* mPortEdit;
  KLineEdit* mAccountEdit;
  KLineEdit* mPasswordEdit;
  QCheckBox *mAutoMailbox;
  KLineEdit* mMailboxEdit;
  QPushButton* mTryFindMailbox;
  KIntNumInput* mCacheEdit;
};

}
#endif
