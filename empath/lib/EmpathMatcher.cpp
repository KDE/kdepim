/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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

#ifdef __GNUG__
# pragma implementation "EmpathMatcher.h"
#endif

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
#include "EmpathConfig.h"

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
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_EXPR + parentid + "_" + QString().setNum(id));
    
    c->writeEntry(F_MATCH_TYPE, (unsigned int)type_);
    
    switch (type_) {
        
        case Size:
            c->writeEntry(F_MATCH_SIZE, size_);
            break;
            
        case BodyExpr:
            c->writeEntry(F_MATCH_EXPR, matchExpr_);
            break;
            
        case HeaderExpr:
            c->writeEntry(F_MATCH_HEADER, matchHeader_);
            c->writeEntry(F_MATCH_EXPR, matchExpr_);
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

    using namespace EmpathConfig;
    
    c->setGroup(GROUP_EXPR + parentName + "_" + QString().setNum(id));
    
    MatchExprType t = (MatchExprType)(c->readNumEntry(F_MATCH_TYPE));
    
    setType(t);
    
    switch (t) {

        case Size:
            size_ = c->readNumEntry(F_MATCH_SIZE);
            break;

        case BodyExpr:
            matchExpr_ = c->readEntry(F_MATCH_EXPR);
            break;

        case HeaderExpr:
            matchExpr_ = c->readEntry(F_MATCH_EXPR);
            matchHeader_ = c->readEntry(F_MATCH_HEADER);
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
                RMM::RMessage * m(empath->message(id, xinfo));
                if (m == 0)
                    return false;
                
                Q_UINT32 sizeOfMessage = m->size();

                empath->finishedWithMessage(id, xinfo);

                empathDebug("size of message is " +
                    QString().setNum(sizeOfMessage));

                // Size * 1024 as it's specified in Kb.
                return (sizeOfMessage > (size_ * 1024));
            }
            break;
            
        case BodyExpr:
            {
                RMM::RMessage * m(empath->message(id, xinfo));
                
                if (m == 0)
                    return false;
                
                empath->finishedWithMessage(id, xinfo);

                QString s; // FIXME -- = m->firstPlainBodyPart ?
                
                QRegExp r(matchExpr_);
                if (!r.isValid()) return false;
                return (s.find(r) != -1);
            }
            break;
            
        case HeaderExpr:
            {
                RMM::RMessage * m(empath->message(id, xinfo));
                
                if (m == 0)
                    return false;
                
                QString s = m->envelope().asString();
                empath->finishedWithMessage(id, xinfo);

                QRegExp r(matchExpr_);
                if (!r.isValid()) return false;
                return (s.find(r) != -1);
            }
            break;
            
        case HasAttachments:    
            {
                RMM::RMessage * m(empath->message(id, xinfo));
                if (m == 0)
                    return false;
                
                bool ok = m->type() == RMM::RBodyPart::Mime;

                empath->finishedWithMessage(id, xinfo);

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
