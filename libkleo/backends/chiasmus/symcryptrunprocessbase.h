/*
    symcryptrunbackend.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2005 Klar�vdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KLEO_BACKEND_CHIASMUS__SYMCRYPTRUNPROCESSBASE_H__
#define __KLEO_BACKEND_CHIASMUS__SYMCRYPTRUNPROCESSBASE_H__

#include <kprocess.h>

class QString;

namespace Kleo
{

class SymCryptRunProcessBase : public KProcess
{
    Q_OBJECT
public:
    enum Operation {
        Encrypt, Decrypt
    };
    SymCryptRunProcessBase(const QString &class_, const QString &program,
                           const QString &keyFile, const QString &options,
                           Operation op,
                           QObject *parent = 0);
    virtual ~SymCryptRunProcessBase();

    bool launch(const QByteArray &input, bool block = true);

    const QByteArray &output() const
    {
        return mOutput;
    }
    const QString &stdErr() const
    {
        return mStderr;
    }

private slots:
    void slotReadyReadStandardError();
    void slotReadyReadStandardOutput();

private:
    void addOptions();

    QByteArray mInput;
    QByteArray mOutput;
    QString mStderr;
    const Operation mOperation;
    QString mOptions;
};

}

#endif // __KLEO_BACKEND_CHIASMUS__SYMCRYPTRUNPROCESSBASE_H__
