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

#include "vacationscriptextractor.h"

using namespace KSieveUi;
VacationDataExtractor::VacationDataExtractor()
    : KSieve::ScriptBuilder(),
      mContext( None ),
      mNotificationInterval( 0 )
{
    kDebug();
}

VacationDataExtractor::~VacationDataExtractor()
{

}

void VacationDataExtractor::commandStart( const QString & identifier ) {
    kDebug() << "( \"" << identifier <<"\" )";
    if ( identifier != QLatin1String("vacation") )
        return;
    reset();
    mContext = VacationCommand;
}

void VacationDataExtractor::commandEnd() {
    kDebug();
    mContext = None;
}

void VacationDataExtractor::error( const KSieve::Error & e )
{
    kDebug() << e.asString() << "@" << e.line() << "," << e.column();
}

void VacationDataExtractor::finished()
{

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
}

void VacationDataExtractor::stringArgument( const QString & string, bool, const QString & )
{
    kDebug() << "( \"" << string <<"\" )";
    if ( mContext == Addresses ) {
        mAliases.push_back( string );
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
