/* -*- mode: c++; c-basic-offset:4 -*-
    utils/types.h

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

#ifndef __KLEOPATRA_UTILS_TYPES_H__
#define __KLEOPATRA_UTILS_TYPES_H__

#include <utils/pimpl_ptr.h>

class QWidget;

namespace boost
{
template <typename T> class shared_ptr;
}

namespace Kleo
{
enum DecryptVerifyOperation {
    Decrypt,
    Verify,
    DecryptVerify/*,
        VerifyOpaque,
        VerifyDetached*/
};

enum VerificationMode {
    Opaque,
    Detached
};

enum Policy {
    NoPolicy,
    Allow,
    Force,
    Deny
};

class ExecutionContext
{
public:
    virtual ~ExecutionContext() {}
    virtual void applyWindowID(QWidget *widget) const = 0;
};

class ExecutionContextUser
{
public:
    ExecutionContextUser();
    explicit ExecutionContextUser(const boost::shared_ptr<const ExecutionContext> &ec);
    virtual ~ExecutionContextUser();

    void setExecutionContext(const boost::shared_ptr<const ExecutionContext> &ec);
    boost::shared_ptr<const ExecutionContext> executionContext() const;

protected:
    void bringToForeground(QWidget *wid);
    void applyWindowID(QWidget *wid);
private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

}

#endif // __KLEOPATRA_UTILS_TYPES_H__
