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


#include "themedefaulttemplate.h"

#include <KLocale>

QList<PimCommon::defaultTemplate> ThemeDefaultTemplate::defaultTemplates()
{
    QList<PimCommon::defaultTemplate> lst;
    PimCommon::defaultTemplate tmp;
    tmp.name = i18n("Subject");
    tmp.text = QString::fromLatin1("{% if header.subject %}\n"
                                   "   {{ header.subject|safe }}\n"
                                   "{% endif %}\n");
    lst << tmp;

    tmp.name = i18n("From");
    tmp.text = QString::fromLatin1("{% if header.from %}\n"
                                   "   {{ header.from|safe }}\n"
                                   "{% endif %}\n");
    lst << tmp;

    tmp.name = i18n("To");
    tmp.text = QString::fromLatin1("{% if header.to %}\n"
                                   "   {{ header.to|safe }}\n"
                                   "{% endif %}\n");
    lst << tmp;

    tmp.name = i18n("Cc");
    tmp.text = QString::fromLatin1("{% if header.cc %}\n"
                                   "   {{ header.cc|safe }}\n"
                                   "{% endif %}\n");
    lst << tmp;

    return lst;
}

