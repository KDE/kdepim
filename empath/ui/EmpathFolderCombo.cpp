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

// Local includes
#include "Empath.h"
#include "EmpathMailboxList.h"
#include "EmpathMailbox.h"
#include "EmpathFolder.h"
#include "EmpathFolderCombo.h"

EmpathFolderCombo::EmpathFolderCombo(QWidget * parent)
    :   QComboBox(parent, "EmpathFolderCombo")
{
    connect(empath, SIGNAL(updateFolderLists()), this, SLOT(s_update()));

    connect(this, SIGNAL(activated(const QString &)),
            this, SLOT(s_activated(const QString &)));
}

EmpathFolderCombo::~EmpathFolderCombo()
{
    // Empty.
}

    void
EmpathFolderCombo::s_update()
{
    clear();

    EmpathMailboxListIterator mit(*(empath->mailboxList()));

    for (; mit.current(); ++mit) {

        if (mit.current()->type() != EmpathMailbox::POP3) {

            EmpathFolderListIterator fit(mit.current()->folderList());

            for (; fit.current(); ++fit) {
                EmpathURL url = fit.current()->url();
                if (url.isFolder())
                    insertItem(url.mailboxName() + "/" + url.folderPath());
            }
        }
    }
}

    void
EmpathFolderCombo::s_activated(const QString & s)
{
    empathDebug("Activated: " + s);
    emit(folderSelected(EmpathURL("empath://" + s + "/")));
}

    void
EmpathFolderCombo::activate(const EmpathURL & url)
{
    for (int i = 0; i < count(); i++)
    {
        if (url == QString("empath://" + text(i) + "/"))
        {
            setCurrentItem(i);
            emit(folderSelected(url));
            return;
        }
    }

    empathDebug("Can't find item " + url.asString());
}

// vim:ts=4:sw=4:tw=78
#include "EmpathFolderCombo.moc"
