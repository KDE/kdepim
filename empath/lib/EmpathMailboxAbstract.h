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
# pragma interface "EmpathMailboxAbstract.h"
#endif

// These functions are here to be included by all subclasses of EmpathMailbox.
// Include them in the class definition for your subclass, under public:
// XXX Make sure that you put a new public:, protected: etc specifier after the
// #include or you'll turn some stuff into private: without knowing.

public:
    
    virtual void
init();
    
    virtual bool
getMail();
    
    virtual bool
newMail() const;

    virtual void
saveConfig();

    virtual void
loadConfig();
        
    virtual void
sync(const EmpathURL &);
        
protected:
    
    virtual void
_retrieve(const EmpathURL &, QString, QString);

    virtual void
_retrieve(const EmpathURL &, const EmpathURL &, QString, QString);
    
    virtual QString
_write(const EmpathURL &, RMM::RMessage &, QString, QString);
    
    virtual void
_removeMessage(const EmpathURL &, QString, QString);
    
    virtual void
_removeMessage(const EmpathURL &, const QStringList &, QString, QString);
    
    virtual void
_createFolder(const EmpathURL &, QString, QString);

    virtual void
_removeFolder(const EmpathURL &, QString, QString);
    
    virtual void
_mark(const EmpathURL &, RMM::MessageStatus, QString, QString);
    
    virtual void
_mark(const EmpathURL &, const QStringList &, RMM::MessageStatus,
    QString, QString);
        
public slots:

virtual void s_checkMail();

private: // Prefer compiler warnings to public slots.

// vim:ts=4:sw=4:tw=78
