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
#ifndef CALLDIALOGIMPL_H
#define CALLDIALOGIMPL_H

#include "ui_calldialog.h"
#include <qdatetime.h>
#include <kdialog.h>

namespace KMobileTools
{
class Engine;
}
/**
	@author Marco Gulino <marco@kmobiletools.org>
*/
class callDialogImpl : public KDialog
{
Q_OBJECT
public:
    explicit callDialogImpl(KMobileTools::Engine *engine, QWidget *parent = 0, const char *name = 0);

    ~callDialogImpl();

protected:
    void done(int);
public slots:
    int call(const QString &number, const QString &showName=QString() );
    void triggerCall();
    void endCall();
    void slotTimerStart();
    void slotTimerPoll();
private:
    Ui::callDialog ui;
    KMobileTools::Engine *engine;
    bool b_dialing, b_labelSet;
    QString number;
    QTime time;
};

#endif
