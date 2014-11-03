/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/decryptverifycommand.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
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

#ifndef __KLEOPATRA_UISERVER_DECRYPTVERIFYCOMMANDEMAILBASE_H__
#define __KLEOPATRA_UISERVER_DECRYPTVERIFYCOMMANDEMAILBASE_H__

#include "assuancommand.h"

#include <utils/pimpl_ptr.h>
#include <utils/types.h>

namespace Kleo
{

class DecryptVerifyCommandEMailBase : public AssuanCommandMixin<DecryptVerifyCommandEMailBase>
{
public:
    explicit DecryptVerifyCommandEMailBase();
    ~DecryptVerifyCommandEMailBase();

private:
    virtual DecryptVerifyOperation operation() const = 0;
    virtual Mode mode() const
    {
        return EMail;
    }

private:
    int doStart();
    void doCanceled();
public:
    static const char *staticName()
    {
        return "";
    }

    class Private;
private:
    kdtools::pimpl_ptr<Private> d;
};

class DecryptVerifyCommand : public AssuanCommandMixin<DecryptVerifyCommand, DecryptVerifyCommandEMailBase>
{
public:
    //DecryptVerifyFilesCommand();
    //~DecryptVerifyFilesCommand();

private:
    DecryptVerifyOperation operation() const
    {
        return DecryptVerify;
    }

public:
    static const char *staticName()
    {
        return "DECRYPT_VERIFY";
    }
};
}

#endif // __KLEOPATRA_UISERVER_DECRYPTCOMMAND_H__
