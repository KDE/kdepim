/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#ifndef PIMCOMMON_SETTINGS_H
#define PIMCOMMON_SETTINGS_H

#include "pimcommon_export.h"
#include "pimcommonsetting_base.h"


class QTimer;

namespace PimCommon {

class PIMCOMMON_EXPORT PimCommonSettings : public PimCommon::PimCommonSettingsBase
{
    Q_OBJECT
public:
    static PimCommonSettings *self();

    /** Call this slot instead of directly @ref KConfig::sync() to
      minimize the overall config writes. Calling this slot will
      schedule a sync of the application config file using a timer, so
      that many consecutive calls can be condensed into a single
      sync, which is more efficient. */
    void requestSync();

private slots:
    void slotSyncNow();

private:
    PimCommonSettings();
    virtual ~PimCommonSettings();
    static PimCommonSettings *mSelf;

    QTimer *mConfigSyncTimer;

};

}

#endif /* PIMCOMMON_SETTINGS_H */
