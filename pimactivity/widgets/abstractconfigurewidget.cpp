/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "abstractconfigurewidget.h"

#include <KStandardDirs>
#include <QDebug>

using namespace PimActivity;

AbstractConfigureWidget::AbstractConfigureWidget()
{
}

AbstractConfigureWidget::~AbstractConfigureWidget()
{
}

void AbstractConfigureWidget::setCurrentActivity(const QString &id)
{
    if (mActivityId != id) {
        mActivityId = id;
        readConfig(id);
    }
}

void AbstractConfigureWidget::setActivity(const QString &id)
{
    if (mActivityId != id) {
        //Save previous activity
        writeConfig(mActivityId);
        mActivityId = id;
        //read new activity
        readConfig(id);
    }
}

QString AbstractConfigureWidget::activity() const
{
    return mActivityId;
}
