/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#include "importkmailautocorrection.h"

#include <QFile>
#include <QDomDocument>
#include <KDebug>
using namespace PimCommon;


ImportKMailAutocorrection::ImportKMailAutocorrection(QWidget *parent)
    : ImportAbstractAutocorrection(parent)
{
}

ImportKMailAutocorrection::~ImportKMailAutocorrection()
{

}


bool ImportKMailAutocorrection::import(const QString& fileName, LoadAttribute loadAttribute)
{
    QFile xmlFile(fileName);
    if (!xmlFile.open(QIODevice::ReadOnly))
        return false;

    QDomDocument doc;
    if (!doc.setContent(&xmlFile))
        return false;
    if (doc.doctype().name() != QLatin1String("autocorrection"))
        return false;

    QDomElement de = doc.documentElement();

    if (loadAttribute == All) {
        const QDomElement upper = de.namedItem(QLatin1String("UpperCaseExceptions")).toElement();
        if (!upper.isNull()) {
            const QDomNodeList nl = upper.childNodes();
            for (int i = 0; i < nl.count(); ++i)
                mUpperCaseExceptions += nl.item(i).toElement().attribute(QLatin1String("exception"));
        }

        const QDomElement twoUpper = de.namedItem(QLatin1String("TwoUpperLetterExceptions")).toElement();
        if (!twoUpper.isNull()) {
            const QDomNodeList nl = twoUpper.childNodes();
            const int numberOfElement(nl.count());
            for(int i = 0; i < numberOfElement; ++i)
                mTwoUpperLetterExceptions += nl.item(i).toElement().attribute(QLatin1String("exception"));
        }

        /* Load advanced autocorrect entry, including the format */
        const QDomElement item = de.namedItem(QLatin1String("items")).toElement();
        if (!item.isNull())
        {
            const QDomNodeList nl = item.childNodes();
            const int numberOfElement(nl.count());
            for (int i = 0; i < numberOfElement; ++i) {
                const QDomElement element = nl.item(i).toElement();
                const QString find = element.attribute(QLatin1String("find"));
                const QString replace = element.attribute(QLatin1String("replace"));
                mAutocorrectEntries.insert(find, replace);
            }
        }

        const QDomElement doubleQuote = de.namedItem(QLatin1String("DoubleQuote")).toElement();
        if (!doubleQuote.isNull()) {
            const QDomNodeList nl = doubleQuote.childNodes();
            if (nl.count()==1) {
                const QDomElement element = nl.item(0).toElement();
                mTypographicDoubleQuotes.begin = element.attribute(QLatin1String("begin")).at(0);
                mTypographicDoubleQuotes.end = element.attribute(QLatin1String("end")).at(0);
            } else {
                kDebug()<<" number of double quote invalid "<<nl.count();
            }
        }

        const QDomElement singleQuote = de.namedItem(QLatin1String("SimpleQuote")).toElement();
        if (!singleQuote.isNull()) {
            const QDomNodeList nl = singleQuote.childNodes();
            if (nl.count()==1) {
                const QDomElement element = nl.item(0).toElement();
                mTypographicSingleQuotes.begin = element.attribute(QLatin1String("begin")).at(0);
                mTypographicSingleQuotes.end = element.attribute(QLatin1String("end")).at(0);
            } else {
                kDebug()<<" number of simple quote invalid "<<nl.count();
            }
        }
    }
    if (loadAttribute == All || loadAttribute == SuperScript) {
        const QDomElement superScript = de.namedItem(QLatin1String("SuperScript")).toElement();
        if (!superScript.isNull()) {
            const QDomNodeList nl = superScript.childNodes();
            const int numberOfNl(nl.count());
            for(int i = 0; i < numberOfNl ; ++i)
                mSuperScriptEntries.insert(nl.item(i).toElement().attribute(QLatin1String("find")), nl.item(i).toElement().attribute(QLatin1String("super")));
        }
    }

    return true;
}

