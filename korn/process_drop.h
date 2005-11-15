/*
 * Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef PROCESS_DROP_H
#define PROCESS_DROP_H

/** @file
 * In this file, a class ProcessDrop is defined.
 * That class is used to get numbers out of running processes.
 * That number is considered the number of new 'emails'.
 */

#include "polldrop.h"

class KProcess;
class KShellProcess;

class QByteArray;
class QString;

/**
 * This class makes a maildrop for processes.
 * Normally, processes aren't a source for email, but if can often be used to walk around limitiations of the program.
 *
 * This class starts a process and waits for data on stdOut. If some data is received, it search for the last number in the output.
 * That number is considered to be the number of emails in this mailbox.
 *
 * This maildrop is pollable which means that it is checked periodically. If it checks for new 'mail', it checks
 * if the process is still running. If the process is running, nothing is done: it keeps waiting for number on stdOut.
 * If the process isn't running, it is (re)started.
 *
 * It is not neccesairy to recheck this account to update the number: if the process keeps running, the mail count keeps updated
 * if new values arrive.
 *
 * @author Mart Kelder <mart.kde@hccnet.nl>
 */
class ProcessDrop : public KPollableDrop
{ Q_OBJECT
public:
	/**
	 * Constructor. It does almost nothing except initialisation of members.
	 */
	ProcessDrop();
	/**
	 * Destructor. It does almost nothing except destructing members.
	 */
	~ProcessDrop();

	/**
	 * This function return true if this box is still valid.
	 * A box can get invalid if an error occures. See KMailDrop::valid().
	 *
	 * In this case, a box is considered valid if the command could be executed.
	 * If it is valid, is does not imply that a number is retrieved.
	 *
	 * @return true is this box is valid
	 */
	virtual bool valid();
	/**
	 * This function is called to recheck this box. See KPollableDrop::recheck().
	 *
	 * In this particular box, it will check if the process is still running.
	 * If not, it will start it.
	 */
	virtual void recheck();

	/**
	 * This function makes another object of the same type.
	 * The retriever of the return value is responsible for deleting the pointer.
	 *
	 * @return pointer to a new ProcessDrop
	 */
	virtual KMailDrop *clone() const;

	/**
	 * This function returns a string which only depends on the type of the box (see MailDrop::type()).
	 * In this class, it always return "process".
	 *
	 * @return the type of this box; in this class always "process"
	 */
	virtual QString type() const;

	/**
	 * This function reads the config from the configuration file.
	 * See KMailDrop::readConfigGroup.
	 *
	 * In this class, it only reads the "program" entry.
	 *
	 * @param cfg the configuration
	 * @return true if reading was succesfull; in this class, it is always true
	 */
	virtual bool readConfigGroup ( const KConfigBase& cfg );
	/**
	 * This function writes the configuration to the configuration file.
	 * See KMailDrop::writeConfigGroup
	 *
	 * In this class, this function does nothing, because the settings are saved through another method.
	 *
	 * @param cfg the configuration to put the settings in.
	 * @return true if writing was succesfull, false otherwise (in this class, it is always true)
	 */
        virtual bool writeConfigGroup ( KConfigBase& cfg ) const;
private slots:
	/**
	 * This function is called when the process is terminated.
	 * At the moment, it doesn't do much with that information.
	 *
	 * @param proc the process which is terminated
	 */
	void slotExited( KProcess *proc );
	
	/**
	 * This function is called when data from the process is received.
	 * The data is append to _receivedBuffer, and checked for numbers after that.
	 *
	 * @param proc the process which send the data
	 * @param data the data iself
	 * @param length the length of the data
	 */
	void slotDataReceived( KProcess *proc, char* data, int length );
private:
	bool _valid;
	bool _waitForRechecked;
	KShellProcess *_process;
	QString *_program;
	QByteArray *_receivedBuffer;
};

#endif //PROCESS_DROP_H

