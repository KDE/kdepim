/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef TEMPLATESELECTION_H
#define TEMPLATESELECTION_H

#include <QHash>
#include <QDomDocument>
#include "utils.h"

class TemplateSelection
{
public:
    TemplateSelection();
    ~TemplateSelection();

    void createTemplate(const QHash<Utils::AppsType, Utils::importExportParameters> &stored);

    QDomDocument document() const;

    QHash<Utils::AppsType, Utils::importExportParameters> loadTemplate(const QDomDocument &doc);
private:
    void saveParameters(Utils::StoredTypes type, QDomElement &elem);
    QDomDocument mDocument;
};

#endif // TEMPLATESELECTION_H
