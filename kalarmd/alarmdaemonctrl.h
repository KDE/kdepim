/*
    This file is part of the KDE alarm daemon.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef ALARMDAEMONCTRL_H
#define ALARMDAEMONCTRL_H

#include <kcmodule.h>

class QCheckBox;
class QSpinBox;

class AlarmDaemonCtrl : public KCModule
{
    Q_OBJECT
  public:
    AlarmDaemonCtrl( QWidget *parent, const char *name);
    ~AlarmDaemonCtrl();

    virtual void load();
    virtual void save();
    virtual void defaults();
    virtual QString quickHelp() const;

    int buttons();

  signals:
    void changed(bool);

  protected slots:
    void changed();

  private:
    QCheckBox *mAutoStartCheck;
    QSpinBox *mIntervalSpin;
};

#endif

