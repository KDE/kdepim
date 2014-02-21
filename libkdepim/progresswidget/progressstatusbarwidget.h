/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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


#ifndef PROGRESSSTATUSBARWIDGET_H
#define PROGRESSSTATUSBARWIDGET_H

#include <QObject>
#include "kdepim_export.h"

namespace KPIM {
class StatusbarProgressWidget;
class KDEPIM_EXPORT ProgressStatusBarWidget : public QObject
{
    Q_OBJECT
public:
    explicit ProgressStatusBarWidget(QWidget *alignWidget, QWidget *parent = 0, unsigned int showTypeProgressItem = 0);
    ~ProgressStatusBarWidget();

    KPIM::StatusbarProgressWidget *littleProgress() const;

private:
    KPIM::StatusbarProgressWidget *mLittleProgress;
};
}

#endif // PROGRESSSTATUSBARWIDGET_H
