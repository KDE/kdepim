/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "vacationscriptextractor.h"

//TODO: add unittests for VacationDataExtractor

using namespace KSieveUi;
VacationDataExtractor::VacationDataExtractor()
    : KSieve::ScriptBuilder()
    , mContext(None)
    , mNotificationInterval(0)
    , mActive(true)
    , mInIfBlock(false)
    , mFoundInBlock(false)
    , mBlockLevel(0)
    , mLineStart(0)
    , mLineEnd(0)
    , mMailAction(VacationUtils::Keep)
    , mMailActionContext(None)
{
    qCDebug(LIBKSIEVE_LOG);
}

VacationDataExtractor::~VacationDataExtractor()
{

}

void VacationDataExtractor::commandStart(const QString &identifier, int lineNumber)
{
    qCDebug(LIBKSIEVE_LOG) << "(\"" << identifier << "\")";
    if (identifier == QStringLiteral("if") && mContext == None) {
        mContext = IfBlock;
        mLineStart = lineNumber;
        mInIfBlock = true;
    }

    if (commandFound() && (!mFoundInBlock || mBlockLevel > 0)) {
        if (identifier == QStringLiteral("discard")) {
            mMailAction = VacationUtils::Discard;
        } else if (identifier == QStringLiteral("redirect")) {
            mMailAction = VacationUtils::Sendto;
            mMailActionContext = RedirectCommand;
        }
    }

    if (identifier != QStringLiteral("vacation")) {
        return;
    }

    if (mContext != IfBlock) {
        mLineStart = lineNumber;
    }

    reset();
    mContext = VacationCommand;
    mFoundInBlock = (mBlockLevel > 0);
}

void VacationDataExtractor::commandEnd(int lineNumber)
{
    qCDebug(LIBKSIEVE_LOG);
    if (mContext != None && mContext != IfBlock  && mContext != VacationEnd) {
        mContext = VacationEnd;
        mLineEnd = lineNumber;
    }
    mMailActionContext = None;
}

void VacationDataExtractor::error(const KSieve::Error &e)
{
    qCDebug(LIBKSIEVE_LOG) << e.asString() << "@" << e.line() << "," << e.column();
}

void VacationDataExtractor::finished()
{

}

void VacationDataExtractor::testStart(const QString &test)
{
    if (mContext == IfBlock) {
        if (test ==  QStringLiteral("true") || test ==  QStringLiteral("false")) {
            mActive = (test == QStringLiteral("true"));
            mIfComment = QString();
        }
    }
}

void VacationDataExtractor::hashComment(const QString &comment)
{
    if (mContext == IfBlock) {
        mIfComment += comment;
    }
}

void VacationDataExtractor::blockStart(int lineNumber)
{
    Q_UNUSED(lineNumber)
    mBlockLevel++;
}

void VacationDataExtractor::blockEnd(int lineNumber)
{
    mBlockLevel--;
    if (mBlockLevel == 0 && !commandFound()) {      //We are in main level again, and didn't found vacation in block
        mActive = true;
        mIfComment = QString();
    } else if (mInIfBlock && mBlockLevel == 0 && commandFound()) {
        mLineEnd = lineNumber;
        mInIfBlock = false;
    }
}

void VacationDataExtractor::taggedArgument(const QString &tag)
{
    qCDebug(LIBKSIEVE_LOG) << "(\"" << tag << "\")";
    if (mMailActionContext == RedirectCommand) {
        if (tag == QStringLiteral("copy")) {
            mMailAction = VacationUtils::CopyTo;
        }
    }
    if (mContext != VacationCommand) {
        return;
    }
    if (tag == QStringLiteral("days")) {
        mContext = Days;
    } else if (tag == QStringLiteral("addresses")) {
        mContext = Addresses;
    } else if (tag == QStringLiteral("subject")) {
        mContext = Subject;
    }
}

void VacationDataExtractor::stringArgument(const QString &string, bool, const QString &)
{
    qCDebug(LIBKSIEVE_LOG) << "(\"" << string << "\")";
    if (mContext == Addresses) {
        mAliases.push_back(string);
        mContext = VacationCommand;
    } else if (mContext == Subject) {
        mSubject = string;
        mContext = VacationCommand;
    } else if (mContext == VacationCommand) {
        mMessageText = string;
        mContext = VacationCommand;
    }
    if (mMailActionContext == RedirectCommand) {
        mMailActionRecipient = string;
    }
}

void VacationDataExtractor::numberArgument(unsigned long number, char)
{
    qCDebug(LIBKSIEVE_LOG) << "(\"" << number << "\")";
    if (mContext != Days) {
        return;
    }
    if (number > INT_MAX) {
        mNotificationInterval = INT_MAX;
    } else {
        mNotificationInterval = number;
    }
    mContext = VacationCommand;
}

void VacationDataExtractor::stringListArgumentStart()
{

}
void VacationDataExtractor::stringListEntry(const QString &string, bool, const QString &)
{
    qCDebug(LIBKSIEVE_LOG) << "(\"" << string << "\")";
    if (mContext != Addresses) {
        return;
    }
    mAliases.push_back(string);
}

void VacationDataExtractor::stringListArgumentEnd()
{
    qCDebug(LIBKSIEVE_LOG);
    if (mContext != Addresses) {
        return;
    }
    mContext = VacationCommand;
}

void VacationDataExtractor::reset()
{
    qCDebug(LIBKSIEVE_LOG);
    mContext = None;
    mMailAction = VacationUtils::Keep;
    mMailActionRecipient = QString();
    mNotificationInterval = 0;
    mAliases.clear();
    mMessageText.clear();
}

RequireExtractor::RequireExtractor()
    : KSieve::ScriptBuilder()
    , mContext(None)
    , mLineStart(0)
    , mLineEnd(0)
{

}

RequireExtractor::~RequireExtractor()
{

}

void RequireExtractor::commandStart(const QString &identifier, int lineNumber)
{
    if (identifier == QStringLiteral("require") && mContext == None) {
        mContext = RequireCommand;
        mLineStart = lineNumber;
    }
}

void RequireExtractor::commandEnd(int lineNumber)
{
    if (mContext == RequireCommand) {
        mContext = EndState;
        mLineEnd = lineNumber;
    }
}

void RequireExtractor::error(const KSieve::Error &e)
{
    qCDebug(LIBKSIEVE_LOG) << e.asString() << "@" << e.line() << "," << e.column();
}

void RequireExtractor::finished()
{

}

void RequireExtractor::stringArgument(const QString &string, bool, const QString &)
{
    mRequirements << string;
}

void RequireExtractor::stringListEntry(const QString &string, bool, const QString &)
{
    mRequirements << string;
}
