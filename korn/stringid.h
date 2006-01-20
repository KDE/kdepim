/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
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

#ifndef MK_STRINGID_H
#define MK_STRINGID_H

/**
 * @file
 *
 * This file contains the class KornStringId
 */

#include<qstring.h>
#include"mailid.h"

/**
 * This class provides a identification with string.
 * 
 * @author Mart Kelder <mart.kde@hccnet.nl>
 */
class KornStringId : public KornMailId
{
private:
	QString _id;
public:
	/**
	 * Constructor, where the id is given with a parameter
	 *
	 * @param id the name for this identification
	 */
	KornStringId( const QString & id );
	/**
	 * Constructor, where the id is obtained from another KornStringId
	 * 
	 * @param src another KornStringId containing the name of the identification
	 */
	KornStringId( const KornStringId & src );
	/**
	 * Empty destructor
	 */
	~KornStringId() {}
	
	/**
	 * Returns the id as given to the class in the constructor
	 *
	 * @return the id
	 */
	QString getId() const { return _id; }
	/**
	 * Return the id, but converted to a QString.
	 * Because this id already is a QString, convertion is easy.
	 *
	 * @return the id
	 */
	virtual QString toString() const { return _id; }

	/**
	 * This function clones the id.
	 *
	 * @return a new KornStringId containing the same id
	 */
	virtual KornMailId *clone() const;
};

#endif //MK_STRINGID_H
