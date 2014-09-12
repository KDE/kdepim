/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "pimcommonsettings.h"
#include <QTimer>

using namespace PimCommon;

PimCommonSettings *PimCommonSettings::mSelf = 0;

PimCommonSettings *PimCommonSettings::self()
{
    if ( !mSelf ) {
        mSelf = new PimCommonSettings();
        mSelf->load();
    }

    return mSelf;
}

PimCommonSettings::PimCommonSettings()
{
    mConfigSyncTimer = new QTimer( this );
    mConfigSyncTimer->setSingleShot( true );
    connect(mConfigSyncTimer, &QTimer::timeout, this, &PimCommonSettings::slotSyncNow);
}

void PimCommonSettings::requestSync()
{
    if ( !mConfigSyncTimer->isActive() )
        mConfigSyncTimer->start( 0 );
}

void PimCommonSettings::slotSyncNow()
{
    config()->sync();
}

PimCommonSettings::~PimCommonSettings()
{
}

