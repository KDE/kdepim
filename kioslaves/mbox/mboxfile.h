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
#ifndef MBOXFILE_H
#define MBOXFILE_H
class MBoxProtocol;
class UrlInfo;

/**
 * This class can be used to lock files when implemented.
 * It is a base class for all classes that needs locking and/ir
 * an UrlInfo*.
 */
class MBoxFile
{
public:
	/**
	 * Constructor
	 * @param info The urlinfo which must be used
	 * @param parent The MBoxProtocol parent instance, used to throw errors.
	 */
	MBoxFile( const UrlInfo* info, MBoxProtocol* parent );

	/**
	 * Empty destructor
	 */
	~MBoxFile();

protected:
	/**
	 * When implemented, this function handles the locking of the file.
	 * @return true if the locking was done succesfully.
	 */
	bool lock();

	/**
	 * When implemented, this function unlocks the file.
	 */
	void unlock();

protected:
	/**
	 * This can be used to get information about the file.
	 * The file specified here is the file that must be used.
	 */
	const UrlInfo* const m_info;

	/**
	 * A instance of the parent protocol, meant to throw errors if neccesairy.
	 */
	MBoxProtocol* const m_mbox;
};
#endif
