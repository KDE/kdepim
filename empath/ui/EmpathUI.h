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
# pragma interface "EmpathUI.h"
#endif

#ifndef EMPATH_UI_H
#define EMPATH_UI_H

// Qt includes
#include <qobject.h>

// Local includes
#include "Empath.h"
#include "EmpathComposeForm.h"
#include "EmpathURL.h"

class QActionCollection;

class KAction;

/**
 * @short A KDE interface to Empath
 * A KDE interface to Empath.
 */
class EmpathUI : public QObject
{
    Q_OBJECT

    public:

        ~EmpathUI();

        static EmpathUI * instance()
        {
            if (0 == instance_)
                instance_ = new EmpathUI;

            return instance_;
        }

        QActionCollection * actionCollection()
        { return actionCollection_; }

    protected:

        EmpathUI();
        
    protected slots:
    
        void s_setup(Empath::SetupType, QWidget *);
        void s_configureMailbox(const EmpathURL &, QWidget *);
        void s_getSaveName(const EmpathURL &, QWidget *);
        void s_infoMessage(const QString &);
        void s_newComposer(EmpathComposeForm);
        void s_showFolder(const EmpathURL &, unsigned int id);

    signals:

        void showFolder(const EmpathURL &, unsigned int id);

    private:
        
        
        void _init();
        void _connectUp();
        void _initActions();
        void _showWizardIfNeeded();

        static EmpathUI * instance_;
        QActionCollection * actionCollection_;

        KAction	* ac_messageCompose_;

        EmpathURL lastShownFolder_;
};

#endif

// vim:ts=4:sw=4:tw=78
