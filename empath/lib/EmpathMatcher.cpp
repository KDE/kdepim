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


// Qt includes
#include <qregexp.h>

// KDE includes
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathMatcher.h"

EmpathMatcher::EmpathMatcher()
{
    // Empty.
}

EmpathMatcher::~EmpathMatcher()
{
    // Empty.
}

    void
EmpathMatcher::save(const QString & parentid, Q_UINT32 id)
{
    KConfig * c = KGlobal::config();

    c->setGroup("Expr_" + parentid + "_" + QString().setNum(id));

    c->writeEntry("Type", (unsigned int)type_);

    switch (type_) {

        case Size:
            c->writeEntry("Size", size_);
            break;

        case BodyExpr:
            c->writeEntry("Expr", matchExpr_);
            break;

        case HeaderExpr:
            c->writeEntry("Header", matchHeader_);
            c->writeEntry("Expr", matchExpr_);
            break;

        case HasAttachments:
        case AnyMessage:
        default:
            break;
    }
    c->sync();
}

    void
EmpathMatcher::load(const QString & parentName, Q_UINT32 id)
{
    KConfig * c = KGlobal::config();

    c->setGroup("Expr_" + parentName + "_" + QString().setNum(id));

    MatchExprType t = (MatchExprType)(c->readNumEntry("Type"));

    setType(t);

    switch (t) {

        case Size:
            size_ = c->readNumEntry("Size");
            break;

        case BodyExpr:
            matchExpr_ = c->readEntry("Expr");
            break;

        case HeaderExpr:
            matchExpr_ = c->readEntry("Expr");
            matchHeader_ = c->readEntry("Header");
            break;

        case HasAttachments:
        case AnyMessage:
            break;

        default:
            // FIXME: ERROR ! We're supposed to have written the type of the
            // matcher to the config !
            return;
            break;
    }
}


    bool
EmpathMatcher::match(const EmpathURL & id)
{
    // FIXME FIXME FIXME FIXME FIXME !!!
    QString xinfo;

    switch (type_) {

        case Size:
            {
                RMM::Message m(empath->message(id));
                if (!m)
                    return false;

                Q_UINT32 sizeOfMessage = m.size();

                empathDebug("size of message is " +
                    QString().setNum(sizeOfMessage));

                // Size * 1024 as it's specified in Kb.
                return (sizeOfMessage > (size_ * 1024));
            }
            break;

        case BodyExpr:
            {
                RMM::Message m(empath->message(id));

                if (!m)
                    return false;

                QString s; // FIXME: iterate through parts = m.body().asString();

                QRegExp r(matchExpr_);
                if (!r.isValid()) return false;
                return (s.find(r) != -1);
            }
            break;

        case HeaderExpr:
            {
                RMM::Message m(empath->message(id));

                if (!m)
                    return false;

                QString s = m.envelope().asString();

                QRegExp r(matchExpr_);
                if (!r.isValid()) return false;
                return (s.find(r) != -1);
            }
            break;

        case HasAttachments:    
            {
                RMM::Message m(empath->message(id));
                if (!m)
                    return false;

                bool ok = m.type() == RMM::BodyPart::Mime;

                return ok;
            }
            break;

        case AnyMessage:
            return true;
            break;

        default:
            return false;
            break;
    }

    return false;
}

    QString
EmpathMatcher::description() const
{
    QString desc;

    switch (type_) {

        case Size:
            desc =
                i18n("Message is larger than %1 Kb")
                .arg(size_);
            break;

        case BodyExpr:
            desc =
                i18n("Expression `%1' in message body")
                .arg(matchExpr_);
            break;

        case HeaderExpr:
            desc =
                i18n("Expression `%1' in message header `%2'")
                .arg(matchExpr_)
                .arg(matchHeader_);
            break;

        case HasAttachments:
            desc = i18n("Message has attachments");
            break;

        case AnyMessage:
            desc = i18n("Any message");
            break;

        default:
            break;
    }

    return desc;
}

// vim:ts=4:sw=4:tw=78
