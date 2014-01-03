/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "themeeditorutil.h"

QString themeeditorutil::defaultMail()
{
    const QString mail = QString::fromLatin1("From: montel@example.com\n"
                                             "To: kde@example.com\n"
                                             "Sender: montel@example.com\n"
                                             "MIME-Version: 1.0\n"
                                             "Date: 28 Apr 2013 23:58:21 -0000\n"
                                             "Subject: Test message\n"
                                             "Content-Type: text/plain\n"
                                             "X-Length: 0\n"
                                             "X-UID: 6161\n"
                                             "\n"
                                             "Hello this is a test mail\n");
    return mail;
}
