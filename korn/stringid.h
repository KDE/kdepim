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

/*
 * This class provides a identification with string,
 * because in KIO, I don't know if emails return in the same order.
 * Author Mart Kelder
 */

#include<qstring.h>
#include"mailid.h"

class KornStringId : public KornMailId
{
private:
	QString _id;
public:
	KornStringId( const QString & id );
	KornStringId( const KornStringId & src );
	~KornStringId() {}

	QString getId() const { return _id; }
	virtual QString toString() const { return _id; }

	virtual KornMailId *clone() const;
};

#endif //MK_STRINGID_H
