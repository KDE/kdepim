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


#include "templateselection.h"
#include <QDomDocument>

TemplateSelection::TemplateSelection(const QString &path)
{
    if (!path.isEmpty()) {
        //TODO load domdocument
    }
}

TemplateSelection::~TemplateSelection()
{

}

Utils::StoredTypes TemplateSelection::loadStoredTypes(const QDomElement &element)
{
    Utils::StoredTypes types;
    return types;
}

QHash<Utils::AppsType, Utils::importExportParameters> TemplateSelection::loadTemplate(const QDomDocument &doc)
{
    QHash<Utils::AppsType, Utils::importExportParameters> value;
    if (!doc.isNull()) {
        mDocument = doc;
    }
    QDomElement docElem = mDocument.documentElement();
    QDomNode n = docElem.firstChild();
    while(!n.isNull())  {
        QDomElement e = n.toElement();
        if(!e.isNull())  {
            qDebug()<<"tag :"<< e.tagName();
        }
        n = n.nextSibling();
    }
    //TODO
    return value;
}

void TemplateSelection::saveParameters(Utils::StoredTypes type, QDomElement &elem)
{
    if (type & Utils::MailTransport) {
        QDomElement tag = mDocument.createElement(QLatin1String("mailtransport"));
        elem.appendChild(tag);
    }
    if (type & Utils::Mails) {
        QDomElement tag = mDocument.createElement(QLatin1String("mail"));
        elem.appendChild(tag);
    }
    if (type & Utils::Resources) {
        QDomElement tag = mDocument.createElement(QLatin1String("resources"));
        elem.appendChild(tag);
    }
    if (type & Utils::Identity) {
        QDomElement tag = mDocument.createElement(QLatin1String("identity"));
        elem.appendChild(tag);
    }
    if (type & Utils::Config) {
        QDomElement tag = mDocument.createElement(QLatin1String("config"));
        elem.appendChild(tag);
    }
    if (type & Utils::AkonadiDb) {
        QDomElement tag = mDocument.createElement(QLatin1String("akonadidb"));
        elem.appendChild(tag);
    }

}

void TemplateSelection::createTemplate(const QHash<Utils::AppsType, Utils::importExportParameters> &stored)
{
    mDocument = QDomDocument();
    QDomProcessingInstruction xmlDeclaration = mDocument.createProcessingInstruction(QLatin1String("xml"), QLatin1String("version=\"1.0\""));
    mDocument.appendChild(xmlDeclaration);

    QDomElement root = mDocument.createElement(QLatin1String("pimsettingexporter"));
    mDocument.appendChild(root);

    QHash<Utils::AppsType, Utils::importExportParameters>::const_iterator i = stored.constBegin();
    while (i != stored.constEnd())  {
        switch(i.key()) {
        case Utils::KMail: {
            QDomElement tag = mDocument.createElement(QLatin1String("kmail"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::KAddressBook: {
            QDomElement tag = mDocument.createElement(QLatin1String("kaddressbook"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::KAlarm: {
            QDomElement tag = mDocument.createElement(QLatin1String("kalarm"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::KOrganizer: {
            QDomElement tag = mDocument.createElement(QLatin1String("korganizer"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::KJots: {
            QDomElement tag = mDocument.createElement(QLatin1String("kjots"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::KNotes: {
            QDomElement tag = mDocument.createElement(QLatin1String("knotes"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::Akregator: {
            QDomElement tag = mDocument.createElement(QLatin1String("akregator"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::Blogilo: {
            QDomElement tag = mDocument.createElement(QLatin1String("blogilo"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::KNode: {
            QDomElement tag = mDocument.createElement(QLatin1String("knode"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        }
        ++i;
    }
}

QDomDocument TemplateSelection::document() const
{
    return mDocument;
}
