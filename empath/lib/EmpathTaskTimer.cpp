/*
    Empath - Mailer for KDE

    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>

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


#include "EmpathTaskTimer.h"

EmpathTaskTimer::EmpathTaskTimer(EmpathTask * t)
    :    QObject(),
        task_(t)
{
    empathDebug("");
    QObject::connect(
        task_,    SIGNAL(finished()),
        this,    SLOT(s_done()));

    QObject::connect(
        &timer_,SIGNAL(timeout()),
        this,    SLOT(s_timeout()));

    QObject::connect(
        this,    SIGNAL(newTask(EmpathTask *)),
        empath,    SLOT(s_newTask(EmpathTask *)));

    timer_.start(100, true); // 0.1 s
}

EmpathTaskTimer::~EmpathTaskTimer()
{
//    delete task_;
//    task_ = 0;
}

    void
EmpathTaskTimer::s_done()
{
    empathDebug("");
    QObject::disconnect(
        task_,    SIGNAL(finished()),
        this,    SLOT(s_done()));
    timer_.stop();
    delete this;
}

    void
EmpathTaskTimer::s_timeout()
{
    empathDebug("");
    emit(newTask(task_));
}

// vim:ts=4:sw=4:tw=78
