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

// These functions are here to be included by all subclasses of EmpathMailbox.
// Include them in the class definition for your subclass, under public:
// XXX Make sure that you put a new public:, protected: etc specifier after the
// #include or you'll turn some stuff into private: without knowing.

public:
    
    virtual void
init();
    
    virtual bool
newMail() const;

    virtual void
saveConfig();

    virtual void
loadConfig();
        
    virtual void
sync(const EmpathURL &);

    virtual RMM::Message
retrieveMessage(const EmpathURL &);

    virtual QString
writeMessage(RMM::Message &, const EmpathURL & folder);

    virtual bool
removeMessage(const EmpathURL & url);
        
    virtual EmpathSuccessMap
removeMessage(const EmpathURL & folder, const QStringList & messageIDList);

    virtual bool
markMessage(const EmpathURL & url, EmpathIndexRecord::Status);

    virtual EmpathSuccessMap
markMessage(
    const EmpathURL & folder,
    const QStringList & messageIDList,
    EmpathIndexRecord::Status
);

    virtual bool
createFolder(const EmpathURL & url);

    virtual bool
removeFolder(const EmpathURL & url);
        
public slots:

virtual void s_checkMail();

private: // Prefer compiler warnings to public slots.

// vim:ts=4:sw=4:tw=78
