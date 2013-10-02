/*
  Copyright (c) 2011, 2012, 2013 Montel Laurent <montel@kde.org>

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

#include "filterimporterthunderbird_p.h"
#include "mailfilter.h"

#include <KDebug>

#include <QFile>
#include <QDir>

using namespace MailCommon;

FilterImporterThunderbird::FilterImporterThunderbird( QFile *file )
    :FilterImporterAbstract()
{
    QTextStream stream(file);
    MailFilter *filter = 0;
    while ( !stream.atEnd() ) {
        QString line = stream.readLine();
        kDebug() << " line :" << line << " filter " << filter;
        filter = parseLine( stream, line, filter );
    }
    appendFilter(filter);
}

FilterImporterThunderbird::~FilterImporterThunderbird()
{
}

QString FilterImporterThunderbird::defaultFiltersSettingsPath()
{
    return QString::fromLatin1( "%1/.thunderbird/" ).arg( QDir::homePath() );
}

MailCommon::MailFilter *FilterImporterThunderbird::parseLine( QTextStream &stream,
                                                              QString line,
                                                              MailCommon::MailFilter *filter )
{
    if ( line.startsWith( QLatin1String( "name=" ) ) ) {
        appendFilter(filter);
        filter = new MailFilter();
        line = cleanArgument( line, QLatin1String( "name=" ) );
        filter->pattern()->setName( line );
        filter->setToolbarName( line );
    } else if ( line.startsWith( QLatin1String( "action=" ) ) ) {
        line = cleanArgument( line, QLatin1String( "action=" ) );
        QString value;
        QString actionName = extractActions( line, filter, value );
        if ( !stream.atEnd() ) {
            line = stream.readLine();
            if ( line.startsWith( QLatin1String( "actionValue=" ) ) ) {
                value = cleanArgument( line, QLatin1String( "actionValue=" ) );
                //change priority
                if (actionName == QLatin1String("Change priority")) {
                    QStringList lstValue;
                    lstValue << QLatin1String("X-Priority");
                    if (value == QLatin1String("Highest")) {
                        value = QLatin1String("1 (Highest)");
                    } else if (value == QLatin1String("High")) {
                        value = QLatin1String("2 (High)");
                    } else if (value == QLatin1String("Normal")) {
                        value = QLatin1String("3 (Normal)");
                    } else if (value == QLatin1String("Low")) {
                        value = QLatin1String("4 (Low)");
                    } else if (value == QLatin1String("Lowest")) {
                        value = QLatin1String("5 (Lowest)");
                    }
                    lstValue << value;
                    value = lstValue.join(QLatin1String("\t"));
                    actionName = QLatin1String("add header");
                } else if (actionName == QLatin1String("copy") || actionName == QLatin1String("transfer")) {
                    KUrl url(value);
                    if (url.isValid()) {
                        QString path = url.path();
                        if (path.startsWith(QLatin1Char('/'))) {
                            path.remove(0,1); //Remove '/'
                        }
                        value = path;
                    }
                }
                createFilterAction( filter, actionName, value );
            } else {
                createFilterAction( filter, actionName, value );
                filter = parseLine( stream, line, filter );
            }
        } else {
            createFilterAction( filter, actionName, value );
        }
    } else if ( line.startsWith( QLatin1String( "enabled=" ) ) ) {
        line = cleanArgument( line, QLatin1String( "enabled=" ) );
        if ( line == QLatin1String( "no" ) ) {
            filter->setEnabled( false );
        }
    } else if ( line.startsWith( QLatin1String( "condition=" ) ) ) {
        line = cleanArgument( line, QLatin1String( "condition=" ) );
        extractConditions( line, filter );
    } else if ( line.startsWith( QLatin1String( "type=" ) ) ) {
        line = cleanArgument( line, QLatin1String( "type=" ) );
        extractType( line, filter );
    } else if ( line.startsWith( QLatin1String( "version=" ) ) ) {
        line = cleanArgument( line, QLatin1String( "version=" ) );
        if ( line.toInt() != 9 ) {
            kDebug() << " thunderbird filter version different of 9 need to look at if it changed";
        }
    } else if ( line.startsWith( QLatin1String( "logging=" ) ) ) {
        line = cleanArgument( line, QLatin1String( "logging=" ) );
        if ( line == QLatin1String( "no" ) ) {
            //TODO
        } else if ( line == QLatin1String( "yes" ) ) {
            //TODO
        } else {
            kDebug() << " Logging option not implemented " << line;
        }
    } else {
        kDebug() << "unknown tag : " << line;
    }
    return filter;
}

void FilterImporterThunderbird::extractConditions( const QString &line,
                                                   MailCommon::MailFilter *filter )
{
    if ( line.startsWith( QLatin1String( "AND" ) ) ) {
        filter->pattern()->setOp( SearchPattern::OpAnd );
        const QStringList conditionsList = line.split( QLatin1String( "AND " ) );
        const int numberOfCond( conditionsList.count() );
        for ( int i = 0; i < numberOfCond; ++i ) {
            if ( !conditionsList.at( i ).trimmed().isEmpty() ) {
                splitConditions( conditionsList.at( i ), filter );
            }
        }
    } else if ( line.startsWith( QLatin1String( "OR" ) ) ) {
        filter->pattern()->setOp( SearchPattern::OpOr );
        const QStringList conditionsList = line.split( QLatin1String( "OR " ) );
        const int numberOfCond( conditionsList.count() );
        for ( int i = 0; i < numberOfCond; ++i ) {
            if ( !conditionsList.at( i ).trimmed().isEmpty() ) {
                splitConditions( conditionsList.at( i ), filter );
            }
        }
    } else if ( line.startsWith( QLatin1String( "ALL" ) ) ){
        filter->pattern()->setOp( SearchPattern::OpAll );
    } else {
        kDebug() << " missing extract condition" << line;
    }
}

bool FilterImporterThunderbird::splitConditions( const QString &cond,
                                                 MailCommon::MailFilter *filter )
{
    /*
   *    {nsMsgSearchAttrib::Subject,    "subject"},
    {nsMsgSearchAttrib::Sender,     "from"},
    {nsMsgSearchAttrib::Body,       "body"},
    {nsMsgSearchAttrib::Date,       "date"},
    {nsMsgSearchAttrib::Priority,   "priority"},
    {nsMsgSearchAttrib::MsgStatus,  "status"},
    {nsMsgSearchAttrib::To,         "to"},
    {nsMsgSearchAttrib::CC,         "cc"},
    {nsMsgSearchAttrib::ToOrCC,     "to or cc"},
    {nsMsgSearchAttrib::AllAddresses, "all addresses"},
    {nsMsgSearchAttrib::AgeInDays,  "age in days"},
    {nsMsgSearchAttrib::Label,      "label"},
    {nsMsgSearchAttrib::Keywords,   "tag"},
    {nsMsgSearchAttrib::Size,       "size"},
    // this used to be nsMsgSearchAttrib::SenderInAddressBook
    // we used to have two Sender menuitems
    // for backward compatibility, we can still parse
    // the old style.  see bug #179803
    {nsMsgSearchAttrib::Sender,     "from in ab"},
    {nsMsgSearchAttrib::JunkStatus, "junk status"},
    {nsMsgSearchAttrib::JunkPercent, "junk percent"},
    {nsMsgSearchAttrib::JunkScoreOrigin, "junk score origin"},
    {nsMsgSearchAttrib::HasAttachmentStatus, "has attachment status"},

  */

    QString str = cond.trimmed();
    str.remove( '(' );
    str.remove( str.length() - 1, 1 ); //remove last )

    const QStringList listOfCond = str.split( QLatin1Char( ',' ) );
    if ( listOfCond.count() < 3 ) {
        kDebug() << "We have a pb in cond:" << cond;
        return false;
    }
    const QString field = listOfCond.at( 0 );
    const QString function = listOfCond.at( 1 );
    const QString contents = listOfCond.at( 2 );

    QByteArray fieldName;
    if ( field == QLatin1String( "subject" ) ) {
        fieldName = "subject";
    } else if ( field == QLatin1String( "from" ) ) {
        fieldName = "from";
    } else if ( field == QLatin1String( "body" ) ) {
        fieldName = "<body>";
    } else if ( field == QLatin1String( "date" ) ) {
        fieldName = "<date>";
    } else if ( field == QLatin1String( "priority" ) ) {
        //TODO
    } else if ( field == QLatin1String( "status" ) ) {
        fieldName = "<status>";
    } else if ( field == QLatin1String( "to" ) ) {
        fieldName = "to";
    } else if ( field == QLatin1String( "cc" ) ) {
        fieldName = "cc";
    } else if ( field == QLatin1String( "to or cc" ) ) {
        fieldName = "<recipients>";
    } else if ( field == QLatin1String( "all addresses" ) ) {
        fieldName = "<recipients>";
    } else if ( field == QLatin1String( "age in days" ) ) {
        fieldName = "<age in days>";
    } else if ( field == QLatin1String( "label" ) ) {
        //TODO
    } else if ( field == QLatin1String( "tag" ) ) {
        fieldName = "<tag>";
    } else if ( field == QLatin1String( "size" ) ) {
        fieldName = "<size>";
    } else if ( field == QLatin1String( "from in ab" ) ) {
        //TODO
    } else if ( field == QLatin1String( "junk status" ) ) {
        //TODO
    } else if ( field == QLatin1String( "junk percent" ) ) {
        //TODO
    } else if ( field == QLatin1String( "junk score origin" ) ) {
        //TODO
    } else if ( field == QLatin1String( "has attachment status" ) ) {
        //TODO
    }

    if ( fieldName.isEmpty() ) {
        kDebug() << " Field not implemented: " << field;
    }
    /*
  {nsMsgSearchOp::Contains, "contains"},
  {nsMsgSearchOp::DoesntContain,"doesn't contain"},
  {nsMsgSearchOp::Is,"is"},
  {nsMsgSearchOp::Isnt,  "isn't"},
  {nsMsgSearchOp::IsEmpty, "is empty"},
  {nsMsgSearchOp::IsntEmpty, "isn't empty"},
  {nsMsgSearchOp::IsBefore, "is before"},
  {nsMsgSearchOp::IsAfter, "is after"},
  {nsMsgSearchOp::IsHigherThan, "is higher than"},
  {nsMsgSearchOp::IsLowerThan, "is lower than"},
  {nsMsgSearchOp::BeginsWith, "begins with"},
  {nsMsgSearchOp::EndsWith, "ends with"},
  {nsMsgSearchOp::IsInAB, "is in ab"},
  {nsMsgSearchOp::IsntInAB, "isn't in ab"},
  {nsMsgSearchOp::IsGreaterThan, "is greater than"},
  {nsMsgSearchOp::IsLessThan, "is less than"},
  {nsMsgSearchOp::Matches, "matches"},
  {nsMsgSearchOp::DoesntMatch, "doesn't match"}
*/
    SearchRule::Function functionName = SearchRule::FuncNone;

    if ( function == QLatin1String( "contains" ) ) {
        functionName = SearchRule::FuncContains;
    } else if ( function == QLatin1String( "doesn't contain" ) ) {
        functionName = SearchRule::FuncContainsNot;
    } else if ( function == QLatin1String( "is" ) ) {
        functionName = SearchRule::FuncEquals;
    } else if ( function == QLatin1String( "isn't" ) ) {
        functionName = SearchRule::FuncNotEqual;
    } else if ( function == QLatin1String( "is empty" ) ) {
        //TODO
    } else if ( function == QLatin1String( "isn't empty" ) ) {
        //TODO
    } else if ( function == QLatin1String( "is before" ) ) {
        functionName = SearchRule::FuncIsLess;
    } else if ( function == QLatin1String( "is after" ) ) {
        functionName = SearchRule::FuncIsGreater;
    } else if ( function == QLatin1String( "is higher than" ) ) {
        functionName = SearchRule::FuncIsGreater;
    } else if ( function == QLatin1String( "is lower than" ) ) {
        functionName = SearchRule::FuncIsLess;
    } else if ( function == QLatin1String( "begins with" ) ) {
        functionName = SearchRule::FuncStartWith;
    } else if ( function == QLatin1String( "ends with" ) ) {
        functionName = SearchRule::FuncEndWith;
    } else if ( function == QLatin1String( "is in ab" ) ) {
        functionName = SearchRule::FuncIsInAddressbook;
    } else if ( function == QLatin1String( "isn't in ab" ) ) {
        functionName = SearchRule::FuncIsNotInAddressbook;
    } else if ( function == QLatin1String( "is greater than" ) ) {
        functionName = SearchRule::FuncIsGreater;
    } else if ( function == QLatin1String( "is less than" ) ) {
        functionName = SearchRule::FuncIsLess;
    } else if ( function == QLatin1String( "matches" ) ) {
        functionName = SearchRule::FuncEquals;
    } else if ( function == QLatin1String( "doesn't match" ) ) {
        functionName = SearchRule::FuncNotEqual;
    }

    if ( functionName == SearchRule::FuncNone ) {
        kDebug() << " functionName not implemented: " << function;
    }
    QString contentsName;
    if ( fieldName == "<status>" ) {
        if ( contents == QLatin1String( "read" ) ) {
            contentsName = QLatin1String( "Read" );
        } else if ( contents == QLatin1String( "unread" ) ) {
            contentsName = QLatin1String( "Unread" );
        } else if ( contents == QLatin1String( "new" ) ) {
            contentsName = QLatin1String( "New" );
        } else if ( contents == QLatin1String( "forwarded" ) ) {
            contentsName = QLatin1String( "Forwarded" );
        } else {
            kDebug() << " contents for status not implemented " << contents;
        }
    } else if ( fieldName == "<size>" ) {
        int value = contents.toInt();
        value = value * 1024; //Ko
        contentsName = QString::number( value );
    } else if ( fieldName == "<date>" ) {
        QLocale locale(QLocale::C);
        const QDate date = locale.toDate(contents,QString::fromLatin1("dd-MMM-yyyy"));
        contentsName = date.toString(Qt::ISODate);
    } else {
        contentsName = contents;
    }

    SearchRule::Ptr rule = SearchRule::createInstance( fieldName, functionName, contentsName );
    filter->pattern()->append( rule );
    //kDebug() << " field :" << field << " function :" << function
    //         << " contents :" << contents << " cond :" << cond;
    return true;
}

QString FilterImporterThunderbird::extractActions( const QString &line,
                                                   MailCommon::MailFilter *filter,
                                                   QString &value )
{
    /*
  { nsMsgFilterAction::MoveToFolder,            "Move to folder"},
  { nsMsgFilterAction::CopyToFolder,            "Copy to folder"},
  { nsMsgFilterAction::ChangePriority,          "Change priority"},
  { nsMsgFilterAction::Delete,                  "Delete"},
  { nsMsgFilterAction::MarkRead,                "Mark read"},
  { nsMsgFilterAction::KillThread,              "Ignore thread"},
  { nsMsgFilterAction::KillSubthread,           "Ignore subthread"},
  { nsMsgFilterAction::WatchThread,             "Watch thread"},
  { nsMsgFilterAction::MarkFlagged,             "Mark flagged"},
  { nsMsgFilterAction::Label,                   "Label"},
  { nsMsgFilterAction::Reply,                   "Reply"},
  { nsMsgFilterAction::Forward,                 "Forward"},
  { nsMsgFilterAction::StopExecution,           "Stop execution"},
  { nsMsgFilterAction::DeleteFromPop3Server,    "Delete from Pop3 server"},
  { nsMsgFilterAction::LeaveOnPop3Server,       "Leave on Pop3 server"},
  { nsMsgFilterAction::JunkScore,               "JunkScore"},
  { nsMsgFilterAction::FetchBodyFromPop3Server, "Fetch body from Pop3Server"},
  { nsMsgFilterAction::AddTag,                  "AddTag"},
  { nsMsgFilterAction::Custom,                  "Custom"},
     */

    QString actionName;
    if ( line == QLatin1String( "Move to folder" ) ) {
        actionName = QLatin1String( "transfer" );
    } else if ( line == QLatin1String( "Forward" ) ) {
        actionName = QLatin1String( "forward" );
    } else if ( line == QLatin1String( "Mark read" ) ) {
        actionName = QLatin1String( "set status" );
        value = QLatin1String( "R" );
    } else if ( line == QLatin1String( "Mark unread" ) ) {
        actionName = QLatin1String( "set status" );
        value = QLatin1String( "U" ); //TODO verify
    } else if ( line == QLatin1String( "Copy to folder" ) ) {
        actionName = QLatin1String( "copy" );
    } else if ( line == QLatin1String( "AddTag" ) ) {
        actionName = QLatin1String( "add tag" );
    } else if ( line == QLatin1String( "Delete" ) ) {
        actionName = QLatin1String( "delete" );
    } else if ( line == QLatin1String( "Change priority" ) ) {
        actionName = QLatin1String("Change priority"); //Doesn't exist in kmail but we help us to importing
    } else if ( line == QLatin1String( "Ignore thread" ) ) {
    } else if ( line == QLatin1String( "Ignore subthread" ) ) {
    } else if ( line == QLatin1String( "Watch thread" ) ) {
    } else if ( line == QLatin1String( "Mark flagged" ) ) {
    } else if ( line == QLatin1String( "Label" ) ) {
    } else if ( line == QLatin1String( "Reply" ) ) {
        actionName = QLatin1String( "set Reply-To" );
    } else if ( line == QLatin1String( "Stop execution" ) ) {
        filter->setStopProcessingHere( true );
        return QString();
    } else if ( line == QLatin1String( "Delete from Pop3 server" ) ) {
    } else if ( line == QLatin1String( "JunkScore" ) ) {
    } else if ( line == QLatin1String( "Fetch body from Pop3Server" ) ) {
    } else if ( line == QLatin1String( "Custom" ) ) {
    }
    if ( actionName.isEmpty() ) {
        kDebug() << QString::fromLatin1( " missing convert method: %1" ).arg( line );
    }
    return actionName;
}

void FilterImporterThunderbird::extractType( const QString &line, MailCommon::MailFilter *filter )
{
    const int value = line.toInt();
    if ( value == 1 ) {
        filter->setApplyOnInbound( true );
        filter->setApplyOnExplicit( false );
        //Checking mail
    } else if ( value == 16 ) {
        filter->setApplyOnInbound( false );
        filter->setApplyOnExplicit( true );
        //Manual mail
    } else if ( value == 17 ) {
        filter->setApplyOnInbound( true );
        filter->setApplyOnExplicit( true );
        //Checking mail or manual
    } else if ( value == 32 ) {
        filter->setApplyOnExplicit( false );
        filter->setApplyOnOutbound( true );
        filter->setApplyOnInbound( false );
        //checking mail after classification
    } else if ( value == 48 ) {
        filter->setApplyOnExplicit( true );
        filter->setApplyOnOutbound( true );
        filter->setApplyOnInbound( false );
        //checking mail after classification or manual check
    } else {
        kDebug() << " type value is not valid :" << value;
    }
}

QString FilterImporterThunderbird::cleanArgument( const QString &line, const QString &removeStr )
{
    QString str = line;
    str.remove( removeStr );
    str.remove( QLatin1String( "\"" ) );
    str.remove( str.length(), 1 ); //remove last "
    return str;
}
