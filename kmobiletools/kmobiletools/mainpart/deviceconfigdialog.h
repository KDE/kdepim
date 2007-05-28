/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef DEVICECONFIGDIALOG_H
#define DEVICECONFIGDIALOG_H

#include <kconfigdialog.h>
#include <libkmobiletools/devicesconfig.h>

#include <config-kmobiletools.h>
//Added by qt3to4:
#include <QLabel>

namespace Ui {
class wizDeviceFirst;

class genericDeviceOptions;
class cfgFilesystem;
}
/**
@author Marco Gulino
*/
class deviceConfigDialog : public KConfigDialog
{
Q_OBJECT
public:
    deviceConfigDialog (QWidget *parent, const QString &name, KConfigSkeleton *config);

    ~deviceConfigDialog();
    
protected slots:
    void slotOk();
    void slotApply();
    void updateSettings();
    void slotEngineChanged(const QString &);
    void readEngine(const QString &);
    void slotPollEnabled(bool poll);
    void fs_selected(int);

private:
    QString currentEngine;
    QWidget* w_firstPage;
    QWidget* w_genOptions;
    QWidget* w_fsConfig;
    QList<QWidget*> enginepages;
    Ui::wizDeviceFirst *firstPage;
    Ui::genericDeviceOptions *genOptions;
#ifdef ENABLE_FS
    Ui::cfgFilesystem *fsConfig;
#else
    QLabel *fsConfig;
#endif

public slots:
    void chooseSMSCenter();
};

#endif
