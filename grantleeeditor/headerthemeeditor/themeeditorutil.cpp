/*
   Copyright (C) 2013-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "themeeditorutil.h"

ThemeEditorUtil::ThemeEditorUtil()
{
}

QString ThemeEditorUtil::defaultMail() const
{
    const QString mail = QStringLiteral("From: montel@example.com\n"
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
