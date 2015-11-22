/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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
#include "pimsettingexportcore_debug.h"
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
            if (!doc.setContent(&file, &errorMsg, &errorRow, &errorCol)) {
                qCDebug(PIMSETTINGEXPORTERCORE_LOG) << "Unable to load document.Parse error in line " << errorRow
                                                    << ", col " << errorCol << ": " << errorMsg;
            } else {
                mDocument = doc;
            }
        } else {
            qCDebug(PIMSETTINGEXPORTERCORE_LOG) << "Unable to load file:" << path;
        }
    }
}

TemplateSelection::~TemplateSelection()
{

}

Utils::StoredTypes TemplateSelection::loadStoredTypes(const QDomElement &element, int &numberOfStep)
{
    Utils::StoredTypes types = Utils::None;
    QDomNode n = element.firstChild();
    while (!n.isNull())  {
        QDomElement e = n.toElement();
        if (!e.isNull())  {
            const QString tagName(e.tagName());
            if (tagName == QLatin1String("mailtransport")) {
                types |= Utils::MailTransport;
                numberOfStep++;
            } else if (tagName == QLatin1String("mail")) {
                types |= Utils::Mails;
                numberOfStep++;
            } else if (tagName == QLatin1String("resources")) {
                types |= Utils::Resources;
                numberOfStep++;
            } else if (tagName == QLatin1String("identity")) {
                types |= Utils::Identity;
                numberOfStep++;
            } else if (tagName == QLatin1String("config")) {
                types |= Utils::Config;
                numberOfStep++;
            } else if (tagName == QLatin1String("data")) {
                types |= Utils::Data;
                numberOfStep++;
            } else if (tagName == QLatin1String("akonadidb")) {
                types |= Utils::AkonadiDb;
                numberOfStep++;
            }
        }
        n = n.nextSibling();
    }
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
    while (!n.isNull())  {
        QDomElement e = n.toElement();
        if (!e.isNull())  {
            const QString tagName(e.tagName());
            //qCDebug(PIMSETTINGEXPORTERCORE_LOG) << "tag :" << tagName;
            Utils::AppsType type = Utils::Unknown;
            if (tagName == QLatin1String("kmail")) {
                type = Utils::KMail;
            } else if (tagName == QLatin1String("kaddressbook")) {
                type = Utils::KAddressBook;
            } else if (tagName == QLatin1String("kalarm")) {
                type = Utils::KAlarm;
            } else if (tagName == QLatin1String("korganizer")) {
                type = Utils::KOrganizer;
            } else if (tagName == QLatin1String("knotes")) {
                type = Utils::KNotes;
            } else if (tagName == QLatin1String("akregator")) {
                type = Utils::Akregator;
            } else if (tagName == QLatin1String("blogilo")) {
                type = Utils::Blogilo;
            }
            if (type != Utils::Unknown) {
                int numberOfSteps = 0;
                Utils::StoredTypes storedType = loadStoredTypes(e, numberOfSteps);
                if (storedType != Utils::None) {
                    Utils::importExportParameters utils;
                    utils.types = storedType;
                    utils.numberSteps = numberOfSteps;
                    value.insert(type, utils);
                }
            }
        }
        n = n.nextSibling();
    }
    return value;
}

void TemplateSelection::saveParameters(Utils::StoredTypes type, QDomElement &elem)
{
    if (type & Utils::MailTransport) {
        QDomElement tag = mDocument.createElement(QStringLiteral("mailtransport"));
        elem.appendChild(tag);
    }
    if (type & Utils::Mails) {
        QDomElement tag = mDocument.createElement(QStringLiteral("mail"));
        elem.appendChild(tag);
    }
    if (type & Utils::Resources) {
        QDomElement tag = mDocument.createElement(QStringLiteral("resources"));
        elem.appendChild(tag);
    }
    if (type & Utils::Identity) {
        QDomElement tag = mDocument.createElement(QStringLiteral("identity"));
        elem.appendChild(tag);
    }
    if (type & Utils::Config) {
        QDomElement tag = mDocument.createElement(QStringLiteral("config"));
        elem.appendChild(tag);
    }
    if (type & Utils::Data) {
        QDomElement tag = mDocument.createElement(QStringLiteral("data"));
        elem.appendChild(tag);
    }
    if (type & Utils::AkonadiDb) {
        QDomElement tag = mDocument.createElement(QStringLiteral("akonadidb"));
        elem.appendChild(tag);
    }

}

void TemplateSelection::createTemplate(const QHash<Utils::AppsType, Utils::importExportParameters> &stored)
{
    mDocument = QDomDocument();
    QDomProcessingInstruction xmlDeclaration = mDocument.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\""));
    mDocument.appendChild(xmlDeclaration);

    QDomElement root = mDocument.createElement(QStringLiteral("pimsettingexporter"));
    mDocument.appendChild(root);

    QHash<Utils::AppsType, Utils::importExportParameters>::const_iterator i = stored.constBegin();
    while (i != stored.constEnd())  {
        switch (i.key()) {
        case Utils::KMail: {
            QDomElement tag = mDocument.createElement(QStringLiteral("kmail"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::KAddressBook: {
            QDomElement tag = mDocument.createElement(QStringLiteral("kaddressbook"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::KAlarm: {
            QDomElement tag = mDocument.createElement(QStringLiteral("kalarm"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::KOrganizer: {
            QDomElement tag = mDocument.createElement(QStringLiteral("korganizer"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::KNotes: {
            QDomElement tag = mDocument.createElement(QStringLiteral("knotes"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::Akregator: {
            QDomElement tag = mDocument.createElement(QStringLiteral("akregator"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::Blogilo: {
            QDomElement tag = mDocument.createElement(QStringLiteral("blogilo"));
            root.appendChild(tag);
            saveParameters(i.value().types, tag);
            break;
        }
        case Utils::Unknown: {
            qCCritical(PIMSETTINGEXPORTERCORE_LOG) << "Code must not use this enum here";
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
