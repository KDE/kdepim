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
# pragma interface "EmpathTask.h"
#endif

#ifndef EMPATH_TASK_H
#define EMPATH_TASK_H

#include <qstring.h>
#include <qobject.h>

/**
 * @short Task progress notifier
 * Create one of these using empath->addTask().
 * Call the methods at appropriate times during your task.
 * @author Rikkus
 */
class EmpathTask : public QObject
{
    Q_OBJECT
        
    public:
        
        EmpathTask(const QString &);
        ~EmpathTask();
        
        /**
         * Set the maximum number of steps this task will take to complete.
         */
        void setMax(int);
        
        /**
         * Set the current number of steps this task has taken.
         */
        void setPos(int);
        
        /**
         * Signal that another item has been processed (increment the
         * position).
         */
        void doneOne();
        
        /**
         * Signal that the task has completed. You may go away now.
         */
        void done();
        
        int max() { return max_; }
        int pos() { return pos_; }
        
        bool isDone() { return done_; }
        
        QString name() { return name_; }
        
    signals:
        
        void maxChanged(int);
        void posChanged(int);
        void addOne();
        void finished();
        
    private:
        
        QString name_;
        int max_;
        int pos_;
        bool done_;
};

#endif

// vim:ts=4:sw=4:tw=78
