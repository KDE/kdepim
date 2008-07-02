/* -*- mode: c++; c-basic-offset:4 -*-
    command_p.h

    This file is part of KleopatraClient, the Kleopatra interface library
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    KleopatraClient is free software; you can redistribute it and/or modify
    it under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KleopatraClient is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef __LIBKLEOPATRACLIENT_CORE_COMMAND_P_H__
#define __LIBKLEOPATRACLIENT_CORE_COMMAND_P_H__

#include "command.h"

#include <QThread>
#include <QMutex>

#include <QString>
#include <QByteArray>
#include <QVariant>

#include <map>
#include <string>

class KLEOPATRACLIENT_NAMESPACE::Command::Private : public QThread {
    Q_OBJECT
private:
    friend class ::KLEOPATRACLIENT_NAMESPACE::Command;
    Command * const q;
public:
    explicit Private( Command * qq )
        : QThread(),
          q( qq ),
          mutex( QMutex::Recursive ),
          inputs(),
          outputs()
    {

    }
    ~Private() {}

private:
    void init();

private:
    /* reimp */ void run();

private:
    QMutex mutex;
    struct Option {
        QVariant value;
        bool hasValue : 1;
        bool isCritical : 1;
    };
    struct Inputs {
        Inputs() : parentWId( 0 ) {}
        std::map<std::string,Option> options;
        WId parentWId;
        QByteArray command;
    } inputs;
    struct Outputs {
        Outputs() : canceled( false ), serverPid( 0 ) {}
        QString errorString;
        bool canceled : 1;
        QByteArray data;
        qint64 serverPid;
        QString serverLocation;
    } outputs;
};

#endif /* __LIBKLEOPATRACLIENT_CORE_COMMAND_P_H__ */
