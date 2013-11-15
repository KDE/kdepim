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

#include "importabstractautocorrection.h"

using namespace PimCommon;

ImportAbstractAutocorrection::ImportAbstractAutocorrection(QWidget *parent)
    : mParent(parent)
{
}

ImportAbstractAutocorrection::~ImportAbstractAutocorrection()
{
}

QSet<QString> ImportAbstractAutocorrection::upperCaseExceptions() const
{
    return mUpperCaseExceptions;
}

QSet<QString> ImportAbstractAutocorrection::twoUpperLetterExceptions() const
{
    return mTwoUpperLetterExceptions;
}

QHash<QString, QString> ImportAbstractAutocorrection::autocorrectEntries() const
{
    return mAutocorrectEntries;
}

AutoCorrection::TypographicQuotes ImportAbstractAutocorrection::typographicSingleQuotes() const
{
    return mTypographicSingleQuotes;
}

AutoCorrection::TypographicQuotes ImportAbstractAutocorrection::typographicDoubleQuotes() const
{
    return mTypographicDoubleQuotes;
}

QHash<QString, QString> ImportAbstractAutocorrection::superScriptEntries() const
{
    return mSuperScriptEntries;
}
