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

#ifdef __GNUG__
# pragma implementation "EmpathFilterEventHandler.h"
#endif

// KDE includes
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "EmpathFilterEventHandler.h"
#include "EmpathDefines.h"
#include "Empath.h"

EmpathFilterEventHandler::EmpathFilterEventHandler()
    :    actionType_(MoveFolder)
{
    moveCopyFolder_ = EmpathURL("empath://local/orphaned");
}

EmpathFilterEventHandler::~EmpathFilterEventHandler()
{
    // Empty.
}

    EmpathFilterEventHandler::ActionType
EmpathFilterEventHandler::actionType() const
{
    return actionType_;
}

    EmpathURL
EmpathFilterEventHandler::moveOrCopyFolder() const
{
    return moveCopyFolder_;
}

    QString
EmpathFilterEventHandler::forwardAddress() const
{
    return forwardAddress_;
}
        
    void
EmpathFilterEventHandler::setMoveFolder(const EmpathURL & folder)
{
    actionType_ = MoveFolder;
    moveCopyFolder_ = folder;
}

    void
EmpathFilterEventHandler::setCopyFolder(const EmpathURL & folder)
{
    actionType_ = CopyFolder;
    moveCopyFolder_ = folder;
}

    void
EmpathFilterEventHandler::setDelete()
{
    actionType_ = Delete;
}

    void
EmpathFilterEventHandler::setIgnore()
{
    actionType_ = Ignore;
}

    void
EmpathFilterEventHandler::setForward(const QString & address)
{
    actionType_ = Forward;
    forwardAddress_ = address;
}

    void
EmpathFilterEventHandler::handleMessage(const EmpathURL & id)
{
    EmpathFolder * folder = empath->folder(id);
    if (folder == 0) return;
    
    switch (actionType_) {

        case MoveFolder:
            empath->move(id, moveCopyFolder_);
            break;
            
        case CopyFolder:
            empath->copy(id, moveCopyFolder_);
            break;

        case Forward:
            empath->s_forward(id);
            break;
            
        case Delete:
            empath->remove(id);
            break;
    
        case Ignore:
            break;
            
        default:
            break;
    }
}

    bool
EmpathFilterEventHandler::load(const QString & filterID)
{
    KConfig * c = KGlobal::config();

    c->setGroup("Filter_" + filterID);

    actionType_ = (ActionType)c->readNumEntry("Type");

    switch (actionType_) {

        case MoveFolder:
            setMoveFolder(EmpathURL(c->readEntry("Folder")));
            break;
            
        case CopyFolder:

            setCopyFolder(EmpathURL(c->readEntry("Folder")));
            break;

        case Forward:

            setForward(c->readEntry("Address"));
            break;

        case Delete:
        case Ignore:
            break;
        default:
            return false;
            break;
    }
    return true;
}

    void
EmpathFilterEventHandler::save(const QString & filterID)
{
    KConfig * c = KGlobal::config();

    c->setGroup("Filter_" + filterID);

    c->writeEntry("Type", (int)actionType_);

    switch (actionType_) {

        case MoveFolder:
        case CopyFolder:
            
            c->writeEntry("Folder", moveCopyFolder_.asString());
            break;
        
        case Forward:
        
            c->writeEntry("Address", forwardAddress_);
        
            break;
        
        case Delete:
        case Ignore:
        default:
            break;
    }
    c->sync();
}

    QString
EmpathFilterEventHandler::description() const
{
    QString action;
    
    switch (actionType_) {
        
        case MoveFolder:
            action = i18n("Move to folder");
            action += " ";
            action += moveCopyFolder_.mailboxName();
            action += "/";
            action += moveCopyFolder_.folderPath();
            break;
            
        case CopyFolder:
            action = i18n("Copy to folder");
            action += " ";
            action += moveCopyFolder_.mailboxName();
            action += "/";
            action += moveCopyFolder_.folderPath();
            break;

        case Delete:
            action = i18n("Delete");
            break;
            
        case Ignore:
            action = i18n("Ignore");
            break;
            
        case Forward:
            action = i18n("Forward to");
            action += " ";
            action += forwardAddress_;
            break;
            
        default:
            action = i18n("No action defined");
            break;
    }
    
    return action;
}

// vim:ts=4:sw=4:tw=78
