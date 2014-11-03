/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signencryptfilescommand.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#ifndef __KLEO_UISERVER_SIGNENCRYPTFILESCOMMAND_H__
#define __KLEO_UISERVER_SIGNENCRYPTFILESCOMMAND_H__

#include "assuancommand.h"

#include <utils/pimpl_ptr.h>

namespace Kleo
{

class SignEncryptFilesCommand : public Kleo::AssuanCommandMixin<SignEncryptFilesCommand>
{
public:
    SignEncryptFilesCommand();
    virtual ~SignEncryptFilesCommand();

protected:
    enum Operation {
        SignDisallowed = 0,
        SignAllowed = 1,
        SignForced  = 2,

        SignMask = SignAllowed | SignForced,

        EncryptDisallowed = 0,
        EncryptAllowed = 4,
        EncryptForced = 8,

        EncryptMask = EncryptAllowed | EncryptForced
    };

private:
    virtual unsigned int operation() const
    {
        return SignAllowed | EncryptAllowed ;
    }
private:
    int doStart();
    void doCanceled();
public:
    static const char *staticName()
    {
        return "SIGN_ENCRYPT_FILES";
    }

    class Private;
private:
    kdtools::pimpl_ptr<Private> d;
};

class EncryptSignFilesCommand : public Kleo::AssuanCommandMixin<EncryptSignFilesCommand, SignEncryptFilesCommand>
{
public:
    static const char *staticName()
    {
        return "ENCRYPT_SIGN_FILES";
    }
};

class EncryptFilesCommand : public Kleo::AssuanCommandMixin<EncryptFilesCommand, SignEncryptFilesCommand>
{
public:
    static const char *staticName()
    {
        return "ENCRYPT_FILES";
    }
    /* reimp */ unsigned int operation() const
    {
        return SignAllowed | EncryptForced;
    }
};

class SignFilesCommand : public Kleo::AssuanCommandMixin<SignFilesCommand, SignEncryptFilesCommand>
{
public:
    static const char *staticName()
    {
        return "SIGN_FILES";
    }
    /* reimp */ unsigned int operation() const
    {
        return SignForced | EncryptAllowed;
    }
};

}

#endif /*__KLEO_UISERVER_SIGNENCRYPTFILESCOMMAND_H__*/
