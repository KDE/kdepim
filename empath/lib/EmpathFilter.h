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
# pragma interface "EmpathFilter.h"
#endif

#ifndef EMPATHFILTER_H
#define EMPATHFILTER_H

// Qt includes
#include <qobject.h>
#include <qlist.h>

// Local includes
#include "RMM_MessageID.h"
#include "EmpathMatcher.h"
#include "EmpathFilterEventHandler.h"

typedef QListIterator<EmpathMatcher> EmpathMatcherListIterator;

/**
 * @short Used to control movement of messages
 * 
 * A filter has a URL which tells it when it'll work on a message.
 * For instance, if a message is passed that is in empath://Mailbox/Inbox
 * and this filter's URL is empath://Mailbox/Inbox then it'll try to match.
 * 
 * Each filter has a set of match expressions which it uses to try and match
 * the given message. If any of these expressions match then the event handler
 * is called.
 * 
 * The event handler is simply passed the URL via
 * EmpathFilterEventHandler::handleMessage(const EmpathURL &);
 * 
 * @author Rikkus
 */
class EmpathFilter : public QObject
{
    Q_OBJECT

    public:
        
        /**
         * Create a new filter with the specified name.
         * You must use empath->filterList().append(filter) to make
         * Empath use the filter.
         */
        EmpathFilter(const QString & name);

        virtual ~EmpathFilter();
        /**
         * The filter must use this to load information about itself
         * on startup. Empath will call it automatically if it's in
         * the filter list.
         */
        void load();
        /**
         * The filter must use this to save information about itself
         * before shutdown. Empath will call it automatically if it's
         * in the filter list.
         */
        void save();
        
        /**
         * Rename this filter. Names must be unique.
         */
        void setName(const QString & name) { name_ = name; }
        
        /**
         * The name of this filter.
         */
        QString name() { return name_; }
        
        /**
         * Perform filtering on the given URL (pointing to an RMM::RMessage).
         */
        void filter(const EmpathURL & source);

        /**
         * Add a new match expression.
         */
        void addMatchExpr(EmpathMatcher * matcher);

        /**
         * Description of what this filter does, in a human readable form.
         */
        QString        description() const;
        
        /**
         * Description of what the filter event handler will do, in a human
         * readable form.
         */
        QString        actionDescription() const;
        
        /**
         * Tell this filter to use the specified event handler.
         */
        void        setEventHandler(EmpathFilterEventHandler *);
        
        /**
         * Set the URL that this filter filters messages from.
         */
        void        setURL(const EmpathURL & url);
        
        /**
         * The URL that this filter filters messages from.
         */
        EmpathURL    url() const;

        /**
         * A list of all match expressions used by this filter.
         */
        QList<EmpathMatcher> *        matchExprList();
        
        /**
         * Pointer to the event handler used when a match expression hits.
         */
        EmpathFilterEventHandler *    eventHandler();
        
        /**
         * Alter the priority of this filter.
         */
        void        setPriority(Q_UINT32 priority)
        { priority_ = priority; }
        
        /**
         * The priority of this filter.
         */
        Q_UINT32    priority()
        { return priority_; }
        
    private:

        bool match(const EmpathURL & id);
        void loadMatchExpr(Q_UINT32 matchExprID);
        void loadEventHandler();

        Q_UINT32 id_;
        Q_UINT32 priority_;
        
        EmpathURL                    url_;
        QList<EmpathMatcher>        matchExprs_;
        EmpathFilterEventHandler    * fEventHandler_;
        
        QString    name_;
};

#endif // EMPATHFILTER_H

// vim:ts=4:sw=4:tw=78
