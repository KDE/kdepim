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
# pragma interface "EmpathEditorProcess.h"
#endif

// Qt includes
#include <qcstring.h>
#include <qobject.h>
#include <qdatetime.h>

// KDE includes
#include <kprocess.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathConfig.h"

/**
 * An external editor process.
 * @author Rikkus
 */
class EmpathEditorProcess : public QObject
{
    Q_OBJECT
    
    public:
        
        /**
         * Create an editor process that works on the given text.
         */
        EmpathEditorProcess(const QCString &);
        ~EmpathEditorProcess();
        /**
         * Start editing.
         */
        void go();
        
    protected slots:

        /**
         * Called when the external process has finished.
         */
        void s_composeFinished(KProcess *);
        
    signals:
        
        /**
         * Signals that the external process ended. If the editor
         * returned with a positive response, ok is true. editedText
         * is the edited text.
         */
        void done(bool ok, QCString editedText);
        
    private:
        
        QCString text_;
        QString fileName_;
        QDateTime myModTime_;
        KProcess p;
};

// vim:ts=4:sw=4:tw=78
