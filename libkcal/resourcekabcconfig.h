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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KCAL_RESOURCEKABCCONFIG_H
#define KCAL_RESOURCEKABCCONFIG_H

#include <qcheckbox.h>
#include <qlabel.h>

#include <krestrictedline.h>
#include <kresources/resource.h>
#include <kresources/resourceconfigwidget.h>

namespace KCal {

class ResourceKABCConfig : public KRES::ResourceConfigWidget
{
    Q_OBJECT
  public:
    ResourceKABCConfig( QWidget* parent = 0, const char* name = 0 );

  public slots:
    virtual void loadSettings( KRES::Resource *resource);
    virtual void saveSettings( KRES::Resource *resource );

  private slots:
    void alarmClicked();

  private:
    QCheckBox *mAlarm;
    KRestrictedLine *mAlarmTimeEdit;
    QLabel *mALabel;
};

}

#endif
