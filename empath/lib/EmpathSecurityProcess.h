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

#ifdef __GNUG__
# pragma interface "EmpathSecurityProcess.h"
#endif

// Qt includes
#include <qcstring.h>
#include <qobject.h>

// KDE includes
#include <kprocess.h>

/**
 * This is a temporary wrapper for PGP 5.0
 * It will disappear when PGP 5.5 is out and we can link to a library.
 * If there's an usable API for doing this within KDE already, we'll
 * just scrap this.
 * 
 * @short PGP 5.0 wrapper
 * @author Rik Hemsley <rik@kde.org>
 */
class EmpathSecurityProcess : public QObject
{
	Q_OBJECT
	
	public:
		
		~EmpathSecurityProcess();

		/**
		 * Encrypt s to r. This will not ask for passphrase.
		 * A connection will be made to parent's slot 's_encryptDone()'
		 */
		static void encrypt
			(const QCString & s, const QCString & r, QObject * parent);

		/**
		 * Encrypt and sign s to r. This will ask for passphrase.
		 * A connection will be made to parent's slot 's_encryptAndSignDone()'
		 */
		static void encryptAndSign
			(const QCString & s, const QCString & r, QObject * parent);
		
		/**
		 * Decrypt s.
		 * A connection will be made to parent's slot 's_decryptDone()'
		 */
		static void decrypt
			(const QCString & s, QObject * parent);
		
	protected slots:

		void s_pgpFinished(KProcess *);
		void s_pgpSentOutput(KProcess *, char *, int);
		void s_pgpSentError(KProcess *, char *, int);
		
	signals:
		
		void done(bool, QCString);
		
	private:
		
		EmpathSecurityProcess();

		void _encrypt			(const QCString &, const QCString &, QObject *);
		void _encryptAndSign	(const QCString &, const QCString &, QObject *);
		void _decrypt			(const QCString &, QObject *);

		KShellProcess p;
		
		QCString outputStr_;
		QCString errorStr_;
};

