/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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

#include <qcheckbox.h>

#include <klocale.h>

#include "syncconfig.h"

using namespace KSync;

SyncConfig::SyncConfig( bool confirmDelete, bool confirmSync )
    : QVBox()
{
    m_sync = new QCheckBox(i18n("Confirm before writing synced data back") , this );
    m_sync->setChecked( confirmSync );

    m_del  = new QCheckBox(i18n("Confirm before deleting"), this );
    m_del->setChecked( confirmDelete );

    QWidget* dummy = new QWidget(this);
    dummy->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

}
SyncConfig::~SyncConfig() {
}
bool SyncConfig::confirmDelete()const {
    return m_del->isChecked();
}
bool SyncConfig::confirmSync()const {
    return m_sync->isChecked();
}

#include "syncconfig.moc"
