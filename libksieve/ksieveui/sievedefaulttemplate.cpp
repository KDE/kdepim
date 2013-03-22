/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include <KLocale>

QList<KSieveUi::SieveDefaultTemplate::defaultTemplate> KSieveUi::SieveDefaultTemplate::defaultTemplates()
{
    QList<KSieveUi::SieveDefaultTemplate::defaultTemplate> lst;
    KSieveUi::SieveDefaultTemplate::defaultTemplate tmp;
    tmp.name = i18n("Move to folder");
    tmp.text = QString::fromLatin1("require \"fileinto\";\n"
                                   "if header :contains \"List-ID\" [ \"examples.com\", \"examples.mail.com\" ] {\n"
                                   "    fileinto \"list-example/examples\"; \n"
                                   "    stop;\n"
                                   "});\n");
    lst << tmp;
    return lst;
}

