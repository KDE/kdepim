/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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

// Pure virtual methods
public:
virtual bool getMail();
virtual bool newMail() const;

virtual void saveConfig();
virtual void readConfig();
		
virtual bool writeMessage(const EmpathURL & folder, const RMessage &);
		
virtual Q_UINT32				sizeOfMessage		(const EmpathURL &);
virtual QString					plainBodyOfMessage	(const EmpathURL &);
virtual REnvelope *				envelopeOfMessage	(const EmpathURL &);
virtual RMessage::MessageType	typeOfMessage		(const EmpathURL &);
virtual RMessage *				message				(const EmpathURL &);
virtual bool 					removeMessage		(const EmpathURL &);
virtual bool					addFolder			(const EmpathURL &);
virtual bool					removeFolder		(const EmpathURL &);
virtual void					syncIndex			(const EmpathURL &);

virtual void init();
		
public slots:

virtual void s_checkNewMail();
virtual void s_getNewMail();

private: // Just to make sure we don't accidentally make lots of public slots.
// End pure virtual methods
