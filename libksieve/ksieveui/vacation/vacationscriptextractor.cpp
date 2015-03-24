/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

using namespace KSieveUi;
VacationDataExtractor::VacationDataExtractor()
    : KSieve::ScriptBuilder(),
      mContext( None ),
      mNotificationInterval( 0 )
    , mActive(true)
    , mInIfBlock(false)
    , mBlockLevel(0)
    , mLineStart(0)
    , mLineEnd(0)
{
    kDebug();
}

VacationDataExtractor::~VacationDataExtractor()
{

}

void VacationDataExtractor::commandStart( const QString & identifier, int lineNumber ) {
    kDebug() << "( \"" << identifier <<"\" )";
    if (identifier == QLatin1String("if") && mContext == None) {
        mContext = IfBlock;
        mLineStart = lineNumber;
        mInIfBlock = true;
    }
    if ( identifier != QLatin1String("vacation") )
        return;

    if (mContext != IfBlock) {
        mLineStart = lineNumber;
    }

    reset();
    mContext = VacationCommand;
}

void VacationDataExtractor::commandEnd(int lineNumber) {
    if ( mContext != None && mContext != IfBlock  && mContext != VacationEnd) {
        mContext = VacationEnd;
        mLineEnd = lineNumber;
    }
}

void VacationDataExtractor::error( const KSieve::Error & e )
{
    kDebug() << e.asString() << "@" << e.line() << "," << e.column();
}

void VacationDataExtractor::finished()
{

}

void VacationDataExtractor::testStart(const QString &test)
{
    if (mContext == IfBlock) {
        if (test ==  QLatin1String("true") || test ==  QLatin1String("false")) {
            mActive = (test == QLatin1String("true"));
            mIfComment = QString();
            kDebug() << "set active level to" << mActive;
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
    mBlockLevel++;
}

void VacationDataExtractor::blockEnd(int lineNumber)
{
    mBlockLevel--;
    if(mBlockLevel == 0 && !commandFound()) {       //We are in main level again, and didn't found vacation in block
        mActive = true;
        mIfComment = QString();
    } else if (mInIfBlock && mBlockLevel == 0 && commandFound()) {
        mLineEnd = lineNumber;
        mInIfBlock = false;
    }
}

void VacationDataExtractor::taggedArgument( const QString & tag )
{
    kDebug() << "( \"" << tag <<"\" )";
    if ( mContext != VacationCommand )
        return;
    if ( tag == QLatin1String("days") )
        mContext = Days;
    else if ( tag == QLatin1String("addresses") )
        mContext = Addresses;
    else if (tag == QLatin1String("subject")) {
        mContext = Subject;
    }
}

void VacationDataExtractor::stringArgument( const QString & string, bool, const QString & )
{
    kDebug() << "( \"" << string <<"\" )";
    if ( mContext == Addresses ) {
        mAliases.push_back( string );
        mContext = VacationCommand;
    } else if (mContext == Subject) {
        mSubject = string;
        mContext = VacationCommand;
    } else if ( mContext == VacationCommand ) {
        mMessageText = string;
        mContext = VacationCommand;
    }
}

void VacationDataExtractor::numberArgument( unsigned long number, char )
{
    kDebug() << "( \"" << number <<"\" )";
    if ( mContext != Days )
        return;
    if ( number > INT_MAX )
        mNotificationInterval = INT_MAX;
    else
        mNotificationInterval = number;
    mContext = VacationCommand;
}

void VacationDataExtractor::stringListArgumentStart()
{

}
void VacationDataExtractor::stringListEntry( const QString & string, bool, const QString & )
{
    kDebug() << "( \"" << string <<"\" )";
    if ( mContext != Addresses )
        return;
    mAliases.push_back( string );
}

void VacationDataExtractor::stringListArgumentEnd()
{
    kDebug();
    if ( mContext != Addresses )
        return;
    mContext = VacationCommand;
}

void VacationDataExtractor::reset()
{
    kDebug();
    mContext = None;
    mNotificationInterval = 0;
    mAliases.clear();
    mMessageText.clear();
}

RequireExtractor::RequireExtractor()
    : KSieve::ScriptBuilder()
    , mContext( None )
    , mLineStart(0)
    , mLineEnd(0)
{

}

RequireExtractor::~RequireExtractor()
{

}

void RequireExtractor::commandStart(const QString &identifier, int lineNumber)
{
    if (identifier == QLatin1String("require") && mContext == None) {
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
     kDebug() << e.asString() << "@" << e.line() << "," << e.column();
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