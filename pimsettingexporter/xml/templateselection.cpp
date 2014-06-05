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
#include <QDebug>
#include <QFile>

TemplateSelection::TemplateSelection(const QString &path)
{
    if (!path.isEmpty()) {
        QDomDocument doc;
        QString errorMsg;
        int errorRow;
        int errorCol;
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            if ( !doc.setContent( &file, &errorMsg, &errorRow, &errorCol ) ) {
                qDebug() << "Unable to load document.Parse error in line " << errorRow
                         << ", col " << errorCol << ": " << errorMsg;
            } else {
                mDocument = doc;
            }
        }
    }
}

TemplateSelection::~TemplateSelection()
{

}

Utils::StoredTypes TemplateSelection::loadStoredTypes(const QDomElement &element)
{
    Utils::StoredTypes types = Utils::None;
    QDomNode n = element.firstChild();
    while(!n.isNull())  {
        QDomElement e = n.toElement();
        if(!e.isNull())  {
            const QString tagName(e.tagName());
            if (tagName == QLatin1String("mailtransport")) {
                types |= Utils::MailTransport;
            } else if (tagName == QLatin1String("mail")) {
                types |= Utils::Mails;
            } else if (tagName == QLatin1String("resources")) {
                types |= Utils::Resources;
            } else if (tagName == QLatin1String("identity")) {
                types |= Utils::Identity;
            } else if (tagName == QLatin1String("config")) {
                types |= Utils::Config;
            } else if (tagName == QLatin1String("akonadidb")) {
                types |= Utils::AkonadiDb;
            }
        }
        n = n.nextSibling();
    }
    return types;
}

QHash<Utils::AppsType, Utils::StoredTypes> TemplateSelection::loadTemplate(const QDomDocument &doc)
{
    QHash<Utils::AppsType, Utils::StoredTypes> value;
    if (!doc.isNull()) {
        mDocument = doc;
    }
    QDomElement docElem = mDocument.documentElement();
    QDomNode n = docElem.firstChild();
    while(!n.isNull())  {
        QDomElement e = n.toElement();
        if(!e.isNull())  {
            const QString tagName(e.tagName());
            qDebug()<<"tag :"<< tagName;
            Utils::AppsType type = Utils::Unknown;
            if (tagName == QLatin1String("kmail"))
                type = Utils::KMail;
            else if (tagName == QLatin1String("kaddressbook"))
                type = Utils::KAddressBook;
            else if (tagName == QLatin1String("kalarm"))
                type = Utils::KAlarm;
            else if (tagName == QLatin1String("korganizer"))
                type = Utils::KOrganizer;
            else if (tagName == QLatin1String("kjots"))
                type = Utils::KJots;
            else if (tagName == QLatin1String("knotes"))
                type = Utils::KNotes;
            else if (tagName == QLatin1String("akregator"))
                type = Utils::Akregator;
            else if (tagName == QLatin1String("blogilo"))
                type = Utils::Blogilo;
            else if (tagName == QLatin1String("knode"))
                type = Utils::KNode;
            if (type != Utils::Unknown) {
                Utils::StoredTypes storedType = loadStoredTypes(e);
                if (storedType != Utils::None)
                    value.insert(type, storedType);
            }
        }
        n = n.nextSibling();
    }
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
        }
        ++i;
    }
}

QDomDocument TemplateSelection::document() const
{
    return mDocument;
}
