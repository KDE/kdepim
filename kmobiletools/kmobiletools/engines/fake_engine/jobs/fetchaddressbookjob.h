/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef FETCHADDRESSBOOKJOB_H
#define FETCHADDRESSBOOKJOB_H

#include <libkmobiletools/jobxp.h>

using namespace KMobileTools;

/**
 * The job that fetches the phone's address book
 *
 * @author Matthias Lechner <matthias@lmme.de>
 */
class FetchAddressbookJob : public JobXP
{
    Q_OBJECT
public:
    FetchAddressbookJob( QObject* parent = 0 );
    ~FetchAddressbookJob();

    void run();

public Q_SLOTS:
    void requestAbort();
};

#endif
