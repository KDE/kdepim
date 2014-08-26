/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "sievedefaulttemplate.h"
#include "vacation/vacationutils.h"

#include <KLocalizedString>

QList<PimCommon::defaultTemplate> KSieveUi::SieveDefaultTemplate::defaultTemplates()
{
    QList<PimCommon::defaultTemplate> lst;
    PimCommon::defaultTemplate tmp;
    tmp.name = i18n("Filter on Mailing List-ID");
    tmp.text = QString::fromLatin1("require \"fileinto\";\n"
                                   "if header :contains \"List-ID\" [ \"examples.com\", \"examples.mail.com\" ] {\n"
                                   "    fileinto \"list-example/examples\"; \n"
                                   "    stop;\n"
                                   "}\n");
    lst << tmp;

    tmp.name = i18n("Filter on Subject");
    tmp.text = QString::fromLatin1("require \"fileinto\";\n"
                                   "if header :contains \"Subject\" \"Foo Foo\" { \n"
                                   "    fileinto \"INBOX.Foo\"; \n"
                                   "}\n");
    lst << tmp;

    tmp.name = i18n("Filter on Spamassassin");
    tmp.text = QString::fromLatin1("require \"fileinto\";\n"
                                   "if header :contains \"X-Spam-Level\" \"*********\" { \n"
                                   "    fileinto \"Spam\";\n"
                                   "}\n");
    lst << tmp;

    tmp.name = i18n("Flag messages");
    tmp.text = QString::fromLatin1("require [\"imap4flags\"];\n"
                                   "if address \"From\" \"someone@example.org\" { \n"
                                   "    setflag \"\\\\Seen\";\n"
                                   "}\n");
    lst << tmp;

    tmp.name = i18n("Forward Message");
    tmp.text = QString::fromLatin1("require [\"copy\"];\n"
                                   "if header :contains \"Subject\" \"foo\" { \n"
                                   "    redirect :copy \"other@example.net\";\n"
                                   "}\n");
    lst << tmp;

    tmp.name = i18n("Forward Message and add copy");
    tmp.text = QString::fromLatin1("require [\"copy\", \"fileinto\"];\n"
                                   "if header :contains \"Subject\" \"foo\" { \n"
                                   "    redirect :copy \"other@example.net\";\n"
                                   "    fileinto \"Forwarded Messages\"; \n"
                                   "}\n");
    lst << tmp;

    tmp.name = i18n("Destroy mail posted by...");
    tmp.text = QString::fromLatin1("if header :contains [\"from\",\"cc\"]\n"
                                   "[\n"
                                   "\"from-foo@example.net\",\n"
                                   "\"pub@foo.com\"\n"
                                   "]\n"
                                   "{\n"
                                   "    discard;\n"
                                   "    stop;\n"
                                   "}\n");
    lst << tmp;

    tmp.name = i18n("Vacations");

    tmp.text = QString::fromLatin1("require \"vacation\";\n\n"
                                   "if header :contains \"X-Spam-Flag\" \"YES\" { keep; stop; }\n"
                                   "vacation :addresses [ \"me@example.net\", \"other@example.net\" ] :days 7 text: \n%1"
                                   "\n.\n;\n").arg(VacationUtils::defaultMessageText());
    lst << tmp;

    return lst;
}

