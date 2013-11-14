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

#ifndef IMPORTABSTRACTAUTOCORRECTION_H
#define IMPORTABSTRACTAUTOCORRECTION_H

#include <QSet>
#include <QString>
#include <QHash>

#include "autocorrection/composerautocorrection.h"

namespace PimCommon {
class ImportAbstractAutocorrection
{
public:
    enum LoadAttribute {
        All = 0,
        SuperScript
    };


    explicit ImportAbstractAutocorrection(QWidget *parent);
    virtual ~ImportAbstractAutocorrection();

    virtual bool import(const QString& fileName, ImportAbstractAutocorrection::LoadAttribute loadAttribute = All ) = 0;

    QSet<QString> upperCaseExceptions() const;
    QSet<QString> twoUpperLetterExceptions() const;
    QHash<QString, QString> autocorrectEntries() const;
    QHash<QString, QString> superScriptEntries() const;

    ComposerAutoCorrection::TypographicQuotes typographicSingleQuotes() const;
    ComposerAutoCorrection::TypographicQuotes typographicDoubleQuotes() const;

protected:
    QSet<QString> mUpperCaseExceptions;
    QSet<QString> mTwoUpperLetterExceptions;
    QHash<QString, QString> mAutocorrectEntries;
    QHash<QString, QString> mSuperScriptEntries;
    ComposerAutoCorrection::TypographicQuotes mTypographicSingleQuotes;
    ComposerAutoCorrection::TypographicQuotes mTypographicDoubleQuotes;
    QWidget *mParent;
};
}

#endif // IMPORTABSTRACTAUTOCORRECTION_H
