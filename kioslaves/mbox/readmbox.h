/*
 * This is a simple kioslave to handle mbox-files.
 * Copyright (C) 2004 Mart Kelder (mart.kde@hccnet.nl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef READMBOX_H
#define READMBOX_H

#include "mboxfile.h"

class UrlInfo;
class MBox;

class QFile;
class QString;
class QTextStream;

/**
 * This class handels reading from a mbox-file.
 */
class ReadMBox : public MBoxFile
{
public:
	/**
	 * Constructor
	 *
	 * @param info The information of the file to read
	 * @param parent The instance of the parent MBoxProtocol.
	 */
	ReadMBox( const UrlInfo* info, MBoxProtocol* parent );

	/**
	 * Destructor
	 */
	~ReadMBox();

	/**
	 * This functions return the current line
	 * @return The line last read, or QString::null if there wasn't such last line
	 */
	QString currentLine() const;

	/**
	 * This function returns the current id. The id is the first line of an email,
	 * and is used in filenaming. The id normally starts with "From ".
	 * @return The current ID, or QString::null if no id was found yet.
	 */
	QString currentID() const;

	/**
	 * This function reads the next line. The next line can be read by the @see currentLine()
	 * function call.
	 *
	 * @return true if succesfull, otherwise false.
	 */
	bool nextLine();

	/**
	 * This function search the file for a certain id.
	 * If not found, the position is EOF.
	 * @param id The id of the message to be found.
	 * @return true if the message was found, false otherwise.
	 */
	bool searchMessage( const QString& id );

	/**
	 * Skips all lines which belongs to the current message. The cursor is on the first line
	 * of a new message message at the end of this function, or at EOF if the cursor was already
	 * on the last message.
	 * @return The number of bytes read while skipping the message.
	 */
	unsigned int skipMessage();

	/**
	 * Sets the cursor back to the beginning of the file
	 */
	void rewind();

	/**
	 * Returns true if the cursor is at EOF.
	 * @return true if and only if the cursor is at EOF.
	 */
	bool atEnd() const;
private:
	/**
	 * Opens a file
	 * @return true Returns true if opening was succesful.
	 */
	bool open();

	/**
	 * Closes a file.
	 */
	void close();

private:
	QFile* m_file;
	QTextStream* m_stream;
	QString* m_current_line;
	QString* m_current_id;
};
#endif
