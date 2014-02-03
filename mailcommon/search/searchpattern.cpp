/* -*- mode: C++; c-file-style: "gnu" -*-

  Author: Marc Mutz <mutz@kde.org>
  Copyright (C) 2012 Andras Mantia <amantia@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "searchpattern.h"
#include "filterlog.h"
using MailCommon::FilterLog;

#include <Akonadi/Contact/ContactSearchJob>

#include <Akonadi/SearchQuery>

#include <KMime/KMimeMessage>

#include <KPIMUtils/Email>

//note: lowercase include for compatibility
#include <kascii.h>
#include <KDebug>
#include <KConfigGroup>
#include <KLocale>
#include <KGlobal>

#include <QDataStream>
#include <QRegExp>
#include <QXmlStreamWriter>

#include <algorithm>
#include <boost/bind.hpp>
#include <akonadi/searchquery.h>


using namespace MailCommon;

static const char *funcConfigNames[] =
{
  "contains", "contains-not",
  "equals", "not-equal",
  "regexp", "not-regexp",
  "greater", "less-or-equal", "less", "greater-or-equal",
  "is-in-addressbook", "is-not-in-addressbook",
  "is-in-category", "is-not-in-category",
  "has-attachment", "has-no-attachment",
  "start-with", "not-start-with",
  "end-with", "not-end-with"
};

static const int numFuncConfigNames =
  sizeof funcConfigNames / sizeof *funcConfigNames;

struct _statusNames {
  const char *name;
  Akonadi::MessageStatus status;
};

static struct _statusNames statusNames[] =
{
  { "Important", Akonadi::MessageStatus::statusImportant() },
  { "Unread", Akonadi::MessageStatus::statusUnread() },
  { "Read", Akonadi::MessageStatus::statusRead() },
  { "Deleted", Akonadi::MessageStatus::statusDeleted() },
  { "Replied", Akonadi::MessageStatus::statusReplied() },
  { "Forwarded", Akonadi::MessageStatus::statusForwarded() },
  { "Queued", Akonadi::MessageStatus::statusQueued() },
  { "Sent", Akonadi::MessageStatus::statusSent() },
  { "Watched", Akonadi::MessageStatus::statusWatched() },
  { "Ignored", Akonadi::MessageStatus::statusIgnored() },
  { "Action Item", Akonadi::MessageStatus::statusToAct() },
  { "Spam", Akonadi::MessageStatus::statusSpam() },
  { "Ham", Akonadi::MessageStatus::statusHam() },
  { "Has Attachment", Akonadi::MessageStatus::statusHasAttachment() }
};

static const int numStatusNames =
  sizeof statusNames / sizeof ( struct _statusNames );

//==================================================
//
// class SearchRule (was: KMFilterRule)
//
//==================================================

SearchRule::SearchRule( const QByteArray &field, Function func, const QString &contents )
  : mField( field ),
    mFunction( func ),
    mContents( contents )
{
}

SearchRule::SearchRule( const SearchRule &other )
  : mField( other.mField ),
    mFunction( other.mFunction ),
    mContents( other.mContents )
{
}

const SearchRule & SearchRule::operator=( const SearchRule &other )
{
  if ( this == &other ) {
    return *this;
  }

  mField = other.mField;
  mFunction = other.mFunction;
  mContents = other.mContents;

  return *this;
}

SearchRule::Ptr SearchRule::createInstance( const QByteArray &field,
                                            Function func,
                                            const QString &contents )
{
  SearchRule::Ptr ret;
  if ( field == "<status>" ) {
    ret = SearchRule::Ptr( new SearchRuleStatus( field, func, contents ) );
  } else if ( field == "<age in days>" || field == "<size>" ) {
    ret = SearchRule::Ptr( new SearchRuleNumerical( field, func, contents ) );
  } else if ( field == "<date>" ) {
    ret = SearchRule::Ptr( new SearchRuleDate( field, func, contents ) );
  } else {
    ret = SearchRule::Ptr( new SearchRuleString( field, func, contents ) );
  }

  return ret;
}

SearchRule::Ptr SearchRule::createInstance( const QByteArray &field,
                                            const char *func,
                                            const QString &contents )
{
  return ( createInstance( field, configValueToFunc( func ), contents ) );
}

SearchRule::Ptr SearchRule::createInstance( const SearchRule &other )
{
  return ( createInstance( other.field(), other.function(), other.contents() ) );
}

SearchRule::Ptr SearchRule::createInstanceFromConfig( const KConfigGroup &config, int aIdx )
{
  const char cIdx = char( int( 'A' ) + aIdx );

  static const QString & field = KGlobal::staticQString( "field" );
  static const QString & func = KGlobal::staticQString( "func" );
  static const QString & contents = KGlobal::staticQString( "contents" );

  const QByteArray &field2 = config.readEntry( field + cIdx, QString() ).toLatin1();
  Function func2 = configValueToFunc( config.readEntry( func + cIdx, QString() ).toLatin1() );
  const QString & contents2 = config.readEntry( contents + cIdx, QString() );

  if ( field2 == "<To or Cc>" ) { // backwards compat
    return SearchRule::createInstance( "<recipients>", func2, contents2 );
  } else {
    return SearchRule::createInstance( field2, func2, contents2 );
  }
}

SearchRule::Ptr SearchRule::createInstance( QDataStream &s )
{
  QByteArray field;
  s >> field;
  QString function;
  s >> function;
  Function func = configValueToFunc( function.toUtf8() );
  QString contents;
  s >> contents;
  return createInstance( field, func, contents );
}

SearchRule::~SearchRule()
{
}

SearchRule::Function SearchRule::configValueToFunc( const char *str )
{
  if ( !str ) {
    return FuncNone;
  }

  for ( int i = 0; i < numFuncConfigNames; ++i ) {
    if ( qstricmp( funcConfigNames[i], str ) == 0 ) {
      return (Function)i;
    }
  }

  return FuncNone;
}

QString SearchRule::functionToString( Function function )
{
  if ( function != FuncNone ) {
    return funcConfigNames[int( function )];
  } else {
    return "invalid";
  }
}

void SearchRule::writeConfig( KConfigGroup &config, int aIdx ) const
{
  const char cIdx = char( 'A' + aIdx );
  static const QString & field = KGlobal::staticQString( "field" );
  static const QString & func = KGlobal::staticQString( "func" );
  static const QString & contents = KGlobal::staticQString( "contents" );

  config.writeEntry( field + cIdx, QString(mField) );
  config.writeEntry( func + cIdx, functionToString( mFunction ) );
  config.writeEntry( contents + cIdx, mContents );
}

QString SearchRule::conditionToString(Function function)
{
    QString str;
    switch(function) {
    case FuncEquals:
        str = i18n("equal");
        break;
    case FuncNotEqual:
        str = i18n("not equal");
        break;
    case FuncIsGreater:
        str = i18n("is greater");
        break;
    case FuncIsLessOrEqual:
        str = i18n("is less or equal");
        break;
    case FuncIsLess:
        str = i18n("is less");
        break;
    case FuncIsGreaterOrEqual:
        str = i18n("is greater or equal");
        break;
    case FuncIsInAddressbook:
        str = i18n("is in addressbook");
        break;
    case FuncIsNotInAddressbook:
        str = i18n("is not in addressbook");
        break;
    case FuncIsInCategory:
        str = i18n("is in category");
        break;
    case FuncIsNotInCategory:
        str = i18n("is in category");
        break;
    case FuncHasAttachment:
        str = i18n("has an attachment");
        break;
    case FuncHasNoAttachment:
        str = i18n("has not an attachment");
        break;
    case FuncStartWith:
        str = i18n("start with");
        break;
    case FuncNotStartWith:
        str = i18n("not start with");
        break;
    case FuncEndWith:
        str = i18n("end with");
        break;
    case FuncNotEndWith:
        str = i18n("not end with");
        break;
    case FuncNone:
        str = i18n("none");
        break;
    case FuncContains:
        str = i18n("contains");
        break;
    case FuncContainsNot:
        str = i18n("not contains");
        break;
    case FuncRegExp:
        str = i18n("has regexp");
        break;
    case FuncNotRegExp:
        str = i18n("not regexp");
        break;
    }
    return str;
}

void SearchRule::generateSieveScript(QStringList &requires, QString &code)
{
    QString contentStr = mContents;
    if (mField == "<size>") {
        QString comparaison;
        int offset = 0;
        switch(mFunction) {
        case FuncEquals:
            comparaison = QLatin1Char('"') + i18n("size equals not supported") + QLatin1Char('"');
            break;
        case FuncNotEqual:
            comparaison = QLatin1Char('"') + i18n("size not equals not supported") + QLatin1Char('"');
            break;
        case FuncIsGreater:
            comparaison = QLatin1String(":over");
            break;
        case FuncIsLessOrEqual:
            comparaison = QLatin1String(":under");
            offset = 1;
            break;
        case FuncIsLess:
            comparaison = QLatin1String(":under");
            break;
        case FuncIsGreaterOrEqual:
            comparaison = QLatin1String(":over");
            offset = -1;
            break;
        case FuncIsInAddressbook:
        case FuncIsNotInAddressbook:
        case FuncIsInCategory:
        case FuncIsNotInCategory:
        case FuncHasAttachment:
        case FuncHasNoAttachment:
        case FuncStartWith:
        case FuncNotStartWith:
        case FuncEndWith:
        case FuncNotEndWith:
        case FuncNone:
        case FuncContains:
        case FuncContainsNot:
        case FuncRegExp:
        case FuncNotRegExp:
            code += QLatin1Char('"') + i18n("\"%1\" is not supported with condition \"%2\"", QLatin1String(mField), conditionToString(mFunction)) + QLatin1Char('"');
            return;
        }
        code += QString::fromLatin1("size %1 %2K").arg(comparaison).arg(QString::number(mContents.toInt() + offset));
    } else if (mField == "<status>") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<status> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "<any header>") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<any header> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "contents") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<contents> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "<age in days>") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<age in days> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "<date>") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<date> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "<recipients>") {
        //TODO ?
        code += QLatin1Char('"') + i18n("<recipients> not implemented/supported") + QLatin1Char('"');
    } else if (mField == "<tag>") {
        code += QLatin1Char('"') + i18n("<Tag> is not supported") + QLatin1Char('"');
    } else if (mField == "<message>") {
        //TODO ?
        code += i18n("<message> not implemented/supported");
    } else if (mField == "<body>") {
        if (!requires.contains(QLatin1String("body")))
            requires << QLatin1String("body");
        QString comparaison;
        bool negative = false;
        switch(mFunction) {
        case FuncNone:
            break;
        case FuncContains:
            comparaison = QLatin1String(":contains");
            break;
        case FuncContainsNot:
            negative = true;
            comparaison = QLatin1String(":contains");
            break;
        case FuncEquals:
            comparaison = QLatin1String(":is");
            break;
        case FuncNotEqual:
            comparaison = QLatin1String(":is");
            negative = true;
            break;
        case FuncRegExp:
            comparaison = QLatin1String(":regex");
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            break;
        case FuncNotRegExp:
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            comparaison = QLatin1String(":regex");
            negative = true;
            break;
        case FuncStartWith:
            comparaison = QLatin1String(":regex");
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            contentStr = QLatin1Char('^') + contentStr;
            break;
        case FuncNotStartWith:
            comparaison = QLatin1String(":regex");
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            comparaison = QLatin1String(":regex");
            contentStr = QLatin1Char('^') + contentStr;
            negative = true;
            break;
        case FuncEndWith:
            comparaison = QLatin1String(":regex");
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            comparaison = QLatin1String(":regex");
            contentStr = contentStr + QLatin1Char('$');
            break;
        case FuncNotEndWith:
            comparaison = QLatin1String(":regex");
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            comparaison = QLatin1String(":regex");
            contentStr = contentStr + QLatin1Char('$');
            negative = true;
            break;
        case FuncIsGreater:
        case FuncIsLessOrEqual:
        case FuncIsLess:
        case FuncIsGreaterOrEqual:
        case FuncIsInAddressbook:
        case FuncIsNotInAddressbook:
        case FuncIsInCategory:
        case FuncIsNotInCategory:
        case FuncHasAttachment:
        case FuncHasNoAttachment:
            code += QLatin1Char('"') + i18n("\"%1\" is not supported with condition \"%2\"", QLatin1String(mField), conditionToString(mFunction)) + QLatin1Char('"');
            return;
        }
        code += (negative ? QLatin1String("not ") : QString()) + QString::fromLatin1("body :text %1 \"%2\"").arg(comparaison).arg(contentStr);
    } else {
        QString comparaison;
        bool negative = false;
        switch(mFunction) {
        case FuncNone:
            break;
        case FuncContains:
            comparaison = QLatin1String(":contains");
            break;
        case FuncContainsNot:
            negative = true;
            comparaison = QLatin1String(":contains");
            break;
        case FuncEquals:
            comparaison = QLatin1String(":is");
            break;
        case FuncNotEqual:
            comparaison = QLatin1String(":is");
            negative = true;
            break;
        case FuncRegExp:
            comparaison = QLatin1String(":regex");
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            break;
        case FuncNotRegExp:
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            comparaison = QLatin1String(":regex");
            negative = true;
            break;
        case FuncStartWith:
            comparaison = QLatin1String(":regex");
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            contentStr = QLatin1Char('^') + contentStr;
            break;
        case FuncNotStartWith:
            comparaison = QLatin1String(":regex");
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            comparaison = QLatin1String(":regex");
            contentStr = QLatin1Char('^') + contentStr;
            negative = true;
            break;
        case FuncEndWith:
            comparaison = QLatin1String(":regex");
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            comparaison = QLatin1String(":regex");
            contentStr = contentStr + QLatin1Char('$');
            break;
        case FuncNotEndWith:
            comparaison = QLatin1String(":regex");
            if (!requires.contains(QLatin1String("regex")))
                requires << QLatin1String("regex");
            comparaison = QLatin1String(":regex");
            contentStr = contentStr + QLatin1Char('$');
            negative = true;
            break;

        case FuncIsGreater:
        case FuncIsLessOrEqual:
        case FuncIsLess:
        case FuncIsGreaterOrEqual:
        case FuncIsInAddressbook:
        case FuncIsNotInAddressbook:
        case FuncIsInCategory:
        case FuncIsNotInCategory:
        case FuncHasAttachment:
        case FuncHasNoAttachment:
            code += QLatin1Char('"') + i18n("\"%1\" is not supported with condition \"%2\"", QLatin1String(mField), conditionToString(mFunction)) + QLatin1Char('"');
            return;
        }
        code += (negative ? QLatin1String("not ") : QString()) + QString::fromLatin1("header %1 \"%2\" \"%3\"").arg(comparaison).arg(QLatin1String(mField)).arg(contentStr);
    }
}

void SearchRule::setFunction( Function function )
{
  mFunction = function;
}

SearchRule::Function SearchRule::function() const
{
  return mFunction;
}

void SearchRule::setField( const QByteArray &field )
{
  mField = field;
}

QByteArray SearchRule::field() const
{
  return mField;
}

void SearchRule::setContents( const QString &contents )
{
  mContents = contents;
}

QString SearchRule::contents() const
{
  return mContents;
}

const QString SearchRule::asString() const
{
  QString result  = QLatin1String("\"") + mField + QLatin1String("\" <");
  result += functionToString( mFunction );
  result += "> \"" + mContents + "\"";

  return result;
}


Akonadi::SearchTerm::Condition SearchRule::akonadiComparator() const
{
  switch ( function() ) {
  case SearchRule::FuncContains:
  case SearchRule::FuncContainsNot:
    return Akonadi::SearchTerm::CondContains;

  case SearchRule::FuncEquals:
  case SearchRule::FuncNotEqual:
    return Akonadi::SearchTerm::CondEqual;

  case SearchRule::FuncIsGreater:
    return Akonadi::SearchTerm::CondGreaterThan;

  case SearchRule::FuncIsGreaterOrEqual:
    return Akonadi::SearchTerm::CondGreaterOrEqual;

  case SearchRule::FuncIsLess:
    return Akonadi::SearchTerm::CondLessThan;

  case SearchRule::FuncIsLessOrEqual:
    return Akonadi::SearchTerm::CondLessOrEqual;

  case SearchRule::FuncRegExp:
  case SearchRule::FuncNotRegExp:
    //TODO is this sufficient?
    return Akonadi::SearchTerm::CondContains;

  case SearchRule::FuncStartWith:
  case SearchRule::FuncNotStartWith:
  case SearchRule::FuncEndWith:
  case SearchRule::FuncNotEndWith:
    //TODO is this sufficient?
    return Akonadi::SearchTerm::CondContains;
  default:
    kDebug() << "Unhandled function type: " << function();
  }

  return Akonadi::SearchTerm::CondEqual;
}

bool SearchRule::isNegated() const
{
  bool negate = false;
  switch ( function() ) {
  case SearchRule::FuncContainsNot:
  case SearchRule::FuncNotEqual:
  case SearchRule::FuncNotRegExp:
  case SearchRule::FuncHasNoAttachment:
  case SearchRule::FuncIsNotInCategory:
  case SearchRule::FuncIsNotInAddressbook:
  case SearchRule::FuncNotStartWith:
  case SearchRule::FuncNotEndWith:
    negate = true;
  default:
    break;
  }
  return negate;
}

QDataStream &SearchRule::operator >>( QDataStream &s ) const
{
  s << mField << functionToString( mFunction ) << mContents;
  return s;
}

//==================================================
//
// class SearchRuleString
//
//==================================================

SearchRuleString::SearchRuleString( const QByteArray &field,
                                    Function func,
                                    const QString &contents )
  : SearchRule( field, func, contents )
{
}

SearchRuleString::SearchRuleString( const SearchRuleString &other )
  : SearchRule( other )
{
}

const SearchRuleString &SearchRuleString::operator=( const SearchRuleString &other )
{
  if ( this == &other ) {
    return *this;
  }

  setField( other.field() );
  setFunction( other.function() );
  setContents( other.contents() );

  return *this;
}

SearchRuleString::~SearchRuleString()
{
}

bool SearchRuleString::isEmpty() const
{
  return field().trimmed().isEmpty() || contents().isEmpty();
}

SearchRule::RequiredPart SearchRuleString::requiredPart() const
{
  const QByteArray f = field();
  SearchRule::RequiredPart part = Header;
  if ( kasciistricmp( f, "<recipients>" ) == 0 ||
       kasciistricmp( f, "<status>" ) == 0 ||
       kasciistricmp( f, "<tag>" ) == 0 ||
       kasciistricmp( f, "Subject" ) == 0 ||
       kasciistricmp( f, "From" ) == 0 ) {
      part = Envelope;
  } else if ( kasciistricmp( f, "<message>" ) == 0 ||
       kasciistricmp( f, "<body>" ) == 0 ) {
      part = CompleteMessage;
  }

  return part;
}


bool SearchRuleString::matches( const Akonadi::Item &item ) const
{
  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
  Q_ASSERT( msg.get() );

  if ( isEmpty() ) {
    return false;
  }

  if ( !msg->hasHeader( "From" ) ) {
    msg->parse(); // probably not parsed yet: make sure we can access all headers
  }

  QString msgContents;
  // Show the value used to compare the rules against in the log.
  // Overwrite the value for complete messages and all headers!
  bool logContents = true;

  if ( kasciistricmp( field(), "<message>" ) == 0 ) {
    msgContents = msg->encodedContent();
    logContents = false;
  } else if ( kasciistricmp( field(), "<body>" ) == 0 ) {
    msgContents = msg->body();
    logContents = false;
  } else if ( kasciistricmp( field(), "<any header>" ) == 0 ) {
    msgContents = msg->head();
    logContents = false;
  } else if ( kasciistricmp( field(), "<recipients>" ) == 0 ) {
    // (mmutz 2001-11-05) hack to fix "<recipients> !contains foo" to
    // meet user's expectations. See FAQ entry in KDE 2.2.2's KMail
    // handbook
    if ( function() == FuncEquals || function() == FuncNotEqual ) {
      // do we need to treat this case specially? Ie.: What shall
      // "equality" mean for recipients.
      return
        matchesInternal( msg->to()->asUnicodeString() ) ||
        matchesInternal( msg->cc()->asUnicodeString() ) ||
        matchesInternal( msg->bcc()->asUnicodeString() ) ;
    }
    msgContents = msg->to()->asUnicodeString();
    msgContents += ", " + msg->cc()->asUnicodeString();
    msgContents += ", " + msg->bcc()->asUnicodeString();
  } else if ( kasciistricmp( field(), "<tag>" ) == 0 ) {
    //port?
//     const Nepomuk2::Resource res( item.url() );
//     foreach ( const Nepomuk2::Tag &tag, res.tags() ) {
//       msgContents += tag.label();
//     }
    logContents = false;
  } else {
    // make sure to treat messages with multiple header lines for
    // the same header correctly
    msgContents = msg->headerByType( field() ) ?
                    msg->headerByType( field() )->asUnicodeString() :
                    "";
  }

  if ( function() == FuncIsInAddressbook ||
       function() == FuncIsNotInAddressbook ) {
    // I think only the "from"-field makes sense.
    msgContents = msg->headerByType( field() ) ?
                    msg->headerByType( field() )->asUnicodeString() :
                    "";

    if ( msgContents.isEmpty() ) {
      return ( function() == FuncIsInAddressbook ) ? false : true;
    }
  }

  // these two functions need the kmmessage therefore they don't call matchesInternal
  if ( function() == FuncHasAttachment ) {
    return ( msg->attachments().size() > 0 );
  }
  if ( function() == FuncHasNoAttachment ) {
    return ( msg->attachments().size() == 0 );
  }

  bool rc = matchesInternal( msgContents );
  if ( FilterLog::instance()->isLogging() ) {
    QString msg = ( rc ? "<font color=#00FF00>1 = </font>" : "<font color=#FF0000>0 = </font>" );
    msg += FilterLog::recode( asString() );
    // only log headers bcause messages and bodies can be pretty large
    if ( logContents ) {
      msg += " (<i>" + FilterLog::recode( msgContents ) + "</i>)";
    }
    FilterLog::instance()->add( msg, FilterLog::RuleResult );
  }
  return rc;
}

void SearchRuleString::addQueryTerms(Akonadi::SearchTerm &groupTerm , bool &emptyIsNotAnError) const
{
  using namespace Akonadi;
  emptyIsNotAnError = false;
  SearchTerm termGroup(SearchTerm::RelOr);
  if ( kasciistricmp( field(), "subject" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Subject, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "reply-to" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderReplyTo, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "<message>" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Message, contents(), akonadiComparator()) );
  } else if ( field() == "<body>" ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Body, contents(), akonadiComparator()) );
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Attachment, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "<recipients>" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderTo, contents(), akonadiComparator()) );
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderCC, contents(), akonadiComparator()) );
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderBCC, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "<any header>" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Headers, contents(), akonadiComparator()) );
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::Subject, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "to" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderTo, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "cc" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderCC, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "bcc" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderBCC, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "from" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderFrom, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "list-id" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderListId, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "resent-from" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderResentFrom, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "x-loop" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderXLoop, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "x-mailing-list" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderXMailingList, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "x-spam-flag" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderXSpamFlag, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "organization" )  == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::HeaderOrganization, contents(), akonadiComparator()) );
  } else if ( kasciistricmp( field(), "<tag>" ) == 0 ) {
    termGroup.addSubTerm(EmailSearchTerm(EmailSearchTerm::MessageTag, contents(), akonadiComparator()) );
  }

  // TODO complete for other headers, generic headers

  if ( !termGroup.subTerms().isEmpty() ) {
    termGroup.setIsNegated( isNegated() );
    groupTerm.addSubTerm( termGroup );
  }
}

// helper, does the actual comparing
bool SearchRuleString::matchesInternal( const QString &msgContents ) const
{
  if( msgContents.isEmpty()) {
    return false;
  }

  switch ( function() ) {
  case SearchRule::FuncEquals:
    return ( QString::compare( msgContents.toLower(), contents().toLower() ) == 0 );

  case SearchRule::FuncNotEqual:
    return ( QString::compare( msgContents.toLower(), contents().toLower() ) != 0 );

  case SearchRule::FuncContains:
    return ( msgContents.contains( contents(), Qt::CaseInsensitive ) );

  case SearchRule::FuncContainsNot:
    return ( !msgContents.contains( contents(), Qt::CaseInsensitive ) );

  case SearchRule::FuncRegExp:
  {
    QRegExp regexp( contents(), Qt::CaseInsensitive );
    return ( regexp.indexIn( msgContents ) >= 0 );
  }

  case SearchRule::FuncNotRegExp:
  {
    QRegExp regexp( contents(), Qt::CaseInsensitive );
    return ( regexp.indexIn( msgContents ) < 0 );
  }

  case SearchRule::FuncStartWith:
  {
    return msgContents.startsWith( contents() );
  }

  case SearchRule::FuncNotStartWith:
  {
    return !msgContents.startsWith( contents() );
  }

  case SearchRule::FuncEndWith:
  {
    return msgContents.endsWith( contents() );
  }

  case SearchRule::FuncNotEndWith:
  {
    return !msgContents.endsWith( contents() );
  }

  case FuncIsGreater:
    return ( QString::compare( msgContents.toLower(), contents().toLower() ) > 0 );

  case FuncIsLessOrEqual:
    return ( QString::compare( msgContents.toLower(), contents().toLower() ) <= 0 );

  case FuncIsLess:
    return ( QString::compare( msgContents.toLower(), contents().toLower() ) < 0 );

  case FuncIsGreaterOrEqual:
    return ( QString::compare( msgContents.toLower(), contents().toLower() ) >= 0 );

  case FuncIsInAddressbook:
  {
    const QStringList addressList = KPIMUtils::splitAddressList( msgContents.toLower() );
    QStringList::ConstIterator end( addressList.constEnd() );
    for ( QStringList::ConstIterator it = addressList.constBegin(); ( it != end ); ++it ) {
      Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob();
      job->setLimit( 1 );
      job->setQuery( Akonadi::ContactSearchJob::Email, KPIMUtils::extractEmailAddress( *it ).toLower() );
      job->exec();

      if ( !job->contacts().isEmpty() ) {
        return true;
      }
    }
    return false;
  }

  case FuncIsNotInAddressbook:
  {
    const QStringList addressList = KPIMUtils::splitAddressList( msgContents.toLower() );
    QStringList::ConstIterator end( addressList.constEnd() );

    for ( QStringList::ConstIterator it = addressList.constBegin(); ( it != end ); ++it ) {
      Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob();
      job->setLimit( 1 );
      job->setQuery( Akonadi::ContactSearchJob::Email, KPIMUtils::extractEmailAddress( *it ).toLower() );
      job->exec();

      if ( job->contacts().isEmpty() ) {
        return true;
      }
    }
    return false;
  }

  case FuncIsInCategory:
  {
    QString category = contents();
    const QStringList addressList =  KPIMUtils::splitAddressList( msgContents.toLower() );

    QStringList::ConstIterator end( addressList.constEnd() );
    for ( QStringList::ConstIterator it = addressList.constBegin(); it != end; ++it ) {
      Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob();
      job->setQuery( Akonadi::ContactSearchJob::Email, KPIMUtils::extractEmailAddress( *it ).toLower() );
      job->exec();

      const KABC::Addressee::List contacts = job->contacts();

      foreach ( const KABC::Addressee &contact, contacts ) {
        if ( contact.hasCategory( category ) ) {
          return true;
        }
      }
    }
    return false;
  }

  case FuncIsNotInCategory:
  {
    QString category = contents();
    const QStringList addressList =  KPIMUtils::splitAddressList( msgContents.toLower() );

    QStringList::ConstIterator end( addressList.constEnd() );
    for ( QStringList::ConstIterator it = addressList.constBegin(); it != end; ++it ) {
      Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob();
      job->setQuery( Akonadi::ContactSearchJob::Email, KPIMUtils::extractEmailAddress( *it ).toLower() );
      job->exec();

      const KABC::Addressee::List contacts = job->contacts();

      foreach ( const KABC::Addressee &contact, contacts ) {
        if ( contact.hasCategory( category ) ) {
          return false;
        }
      }

    }
    return true;
  }
  default:
    ;
  }

  return false;
}

//==================================================
//
// class SearchRuleNumerical
//
//==================================================

SearchRuleNumerical::SearchRuleNumerical( const QByteArray &field,
                                          Function func,
                                          const QString &contents )
  : SearchRule( field, func, contents )
{
}

bool SearchRuleNumerical::isEmpty() const
{
  bool ok = false;
  contents().toInt( &ok );

  return !ok;
}

bool SearchRuleNumerical::matches( const Akonadi::Item &item ) const
{
  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();

  QString msgContents;
  qint64 numericalMsgContents = 0;
  qint64 numericalValue = 0;

  if ( kasciistricmp( field(), "<size>" ) == 0 ) {
    numericalMsgContents = item.size();
    numericalValue = contents().toLongLong();
    msgContents.setNum( numericalMsgContents );
  } else if ( kasciistricmp( field(), "<age in days>" ) == 0 ) {
    QDateTime msgDateTime = msg->date()->dateTime().dateTime();
    numericalMsgContents = msgDateTime.daysTo( QDateTime::currentDateTime() );
    numericalValue = contents().toInt();
    msgContents.setNum( numericalMsgContents );
  }
  bool rc = matchesInternal( numericalValue, numericalMsgContents, msgContents );
  if ( FilterLog::instance()->isLogging() ) {
    QString msg = ( rc ? "<font color=#00FF00>1 = </font>"
                       : "<font color=#FF0000>0 = </font>" );
    msg += FilterLog::recode( asString() );
    msg += " ( <i>" + QString::number( numericalMsgContents ) + "</i> )";
    FilterLog::instance()->add( msg, FilterLog::RuleResult );
  }
  return rc;
}

SearchRule::RequiredPart SearchRuleNumerical::requiredPart() const
{
  return SearchRule::Envelope;
}


bool SearchRuleNumerical::matchesInternal( long numericalValue,
    long numericalMsgContents, const QString & msgContents ) const
{
  switch ( function() ) {
  case SearchRule::FuncEquals:
    return ( numericalValue == numericalMsgContents );

  case SearchRule::FuncNotEqual:
    return ( numericalValue != numericalMsgContents );

  case SearchRule::FuncContains:
    return ( msgContents.contains( contents(), Qt::CaseInsensitive ) );

  case SearchRule::FuncContainsNot:
    return ( !msgContents.contains( contents(), Qt::CaseInsensitive ) );

  case SearchRule::FuncRegExp:
  {
    QRegExp regexp( contents(), Qt::CaseInsensitive );
    return ( regexp.indexIn( msgContents ) >= 0 );
  }

  case SearchRule::FuncNotRegExp:
  {
    QRegExp regexp( contents(), Qt::CaseInsensitive );
    return ( regexp.indexIn( msgContents ) < 0 );
  }

  case FuncIsGreater:
    return ( numericalMsgContents > numericalValue );

  case FuncIsLessOrEqual:
    return ( numericalMsgContents <= numericalValue );

  case FuncIsLess:
    return ( numericalMsgContents < numericalValue );

  case FuncIsGreaterOrEqual:
    return ( numericalMsgContents >= numericalValue );

  case FuncIsInAddressbook:  // since email-addresses are not numerical, I settle for false here
    return false;

  case FuncIsNotInAddressbook:
    return false;

  default:
    ;
  }

  return false;
}

void SearchRuleNumerical::addQueryTerms( Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError ) const
{
  using namespace Akonadi;
  emptyIsNotAnError = false;
  if ( kasciistricmp( field(), "<size>" ) == 0 ) {
    EmailSearchTerm term(EmailSearchTerm::ByteSize, contents().toInt(), akonadiComparator());
    term.setIsNegated( isNegated() );
    groupTerm.addSubTerm(term);
  } else if ( kasciistricmp( field(), "<age in days>" ) == 0 ) {
    QDateTime date(QDate::currentDate());
    date = date.addDays( contents().toInt() );
    EmailSearchTerm term(EmailSearchTerm::HeaderDate, date, akonadiComparator());
    term.setIsNegated( isNegated() );
    groupTerm.addSubTerm(term);
  }
}

//==================================================
//
// class SearchRuleDate
//
//==================================================

SearchRuleDate::SearchRuleDate( const QByteArray &field,
                                          Function func,
                                          const QString &contents )
  : SearchRule( field, func, contents )
{
}

bool SearchRuleDate::isEmpty() const
{
  return !QDate::fromString( contents(), Qt::ISODate ).isValid();
}

bool SearchRuleDate::matches( const Akonadi::Item &item ) const
{
  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();


  QDate msgDate = msg->date()->dateTime().date();
  QDate dateValue = QDate::fromString( contents(), Qt::ISODate );
  bool rc = matchesInternal( dateValue, msgDate );
  if ( FilterLog::instance()->isLogging() ) {
    QString msg = ( rc ? "<font color=#00FF00>1 = </font>"
                       : "<font color=#FF0000>0 = </font>" );
    msg += FilterLog::recode( asString() );
    msg += " ( <i>" + contents() + "</i> )"; //TODO change with locale?
    FilterLog::instance()->add( msg, FilterLog::RuleResult );
  }
  return rc;
}

bool SearchRuleDate::matchesInternal( const QDate& dateValue,
    const QDate& msgDate ) const
{
  switch ( function() ) {
  case SearchRule::FuncEquals:
    return ( dateValue == msgDate );

  case SearchRule::FuncNotEqual:
    return ( dateValue != msgDate );

  case FuncIsGreater:
    return ( msgDate > dateValue );

  case FuncIsLessOrEqual:
    return ( msgDate <= dateValue );

  case FuncIsLess:
    return ( msgDate < dateValue );

  case FuncIsGreaterOrEqual:
    return ( msgDate >= dateValue );

  default:
    ;
  }
  return false;
}

SearchRule::RequiredPart SearchRuleDate::requiredPart() const
{
  return SearchRule::Envelope;
}



void SearchRuleDate::addQueryTerms( Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError ) const
{
  using namespace Akonadi;
  emptyIsNotAnError = false;
  const QDateTime date = QDateTime::fromString( contents(), Qt::ISODate );
  EmailSearchTerm term(EmailSearchTerm::HeaderDate, date, akonadiComparator());
  term.setIsNegated( isNegated() );
  groupTerm.addSubTerm(term);
}


//==================================================
//
// class SearchRuleStatus
//
//==================================================
QString englishNameForStatus( const Akonadi::MessageStatus &status )
{
  for ( int i=0; i< numStatusNames; ++i ) {
    if ( statusNames[i].status == status ) {
      return statusNames[i].name;
    }
  }
  return QString();
}

SearchRuleStatus::SearchRuleStatus( const QByteArray &field,
                                    Function func, const QString &aContents )
  : SearchRule( field, func, aContents )
{
  // the values are always in english, both from the conf file as well as
  // the patternedit gui
  mStatus = statusFromEnglishName( aContents );
}

SearchRuleStatus::SearchRuleStatus( Akonadi::MessageStatus status, Function func )
 : SearchRule( "<status>", func, englishNameForStatus( status ) )
{
  mStatus = status;
}

Akonadi::MessageStatus SearchRuleStatus::statusFromEnglishName( const QString &aStatusString )
{
  for ( int i=0; i< numStatusNames; ++i ) {
    if ( !aStatusString.compare( statusNames[i].name ) ) {
      return statusNames[i].status;
    }
  }
  Akonadi::MessageStatus unknown;
  return unknown;
}

bool SearchRuleStatus::isEmpty() const
{
  return field().trimmed().isEmpty() || contents().isEmpty();
}

bool SearchRuleStatus::matches( const Akonadi::Item &item ) const
{
  Akonadi::MessageStatus status;
  status.setStatusFromFlags( item.flags() );
  bool rc = false;
  switch ( function() ) {
    case FuncEquals: // fallthrough. So that "<status> 'is' 'read'" works
    case FuncContains:
      if ( status & mStatus ) {
        rc = true;
      }
      break;
    case FuncNotEqual: // fallthrough. So that "<status> 'is not' 'read'" works
    case FuncContainsNot:
      if ( ! ( status & mStatus ) ) {
        rc = true;
      }
      break;
    // FIXME what about the remaining funcs, how can they make sense for
    // stati?
    default:
      break;
  }
  if ( FilterLog::instance()->isLogging() ) {
    QString msg = ( rc ? "<font color=#00FF00>1 = </font>" : "<font color=#FF0000>0 = </font>" );
    msg += FilterLog::recode( asString() );
    FilterLog::instance()->add( msg, FilterLog::RuleResult );
  }
  return rc;
}

SearchRule::RequiredPart SearchRuleStatus::requiredPart() const
{
  return SearchRule::Envelope;
}

void SearchRuleStatus::addQueryTerms( Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError ) const
{
  using namespace Akonadi;
  emptyIsNotAnError = true;
  //TODO double check that isRead also works
  if (!mStatus.statusFlags().isEmpty()) {
    EmailSearchTerm term(EmailSearchTerm::MessageStatus, mStatus.statusFlags().toList().first(), akonadiComparator());
    term.setIsNegated( isNegated() );
    groupTerm.addSubTerm(term);
  }
}

// ----------------------------------------------------------------------------

//==================================================
//
// class SearchPattern
//
//==================================================

SearchPattern::SearchPattern()
  : QList<SearchRule::Ptr>()
{
  init();
}

SearchPattern::SearchPattern( const KConfigGroup &config )
  : QList<SearchRule::Ptr>()
{
  readConfig( config );
}

SearchPattern::~SearchPattern()
{
}

bool SearchPattern::matches( const Akonadi::Item &item, bool ignoreBody ) const
{
  if ( isEmpty() ) {
    return true;
  }
  if ( !item.hasPayload<KMime::Message::Ptr>() ) {
    return false;
  }

  QList<SearchRule::Ptr>::const_iterator it;
  QList<SearchRule::Ptr>::const_iterator end( constEnd() );
  switch ( mOperator ) {
  case OpAnd: // all rules must match
    for ( it = constBegin(); it != end; ++it ) {
      if ( !( (*it)->requiredPart() == SearchRule::CompleteMessage && ignoreBody ) ) {
        if ( !(*it)->matches( item ) ) {
          return false;
        }
      }
    }
    return true;

  case OpOr:  // at least one rule must match
    for ( it = constBegin(); it != end; ++it ) {
      if ( !( (*it)->requiredPart() == MailCommon::SearchRule::CompleteMessage && ignoreBody ) ) {
        if ( (*it)->matches( item ) ) {
          return true;
        }
      }
    }
    return false;

  case OpAll:
    return true;

  default:
    return false;
  }
}

SearchRule::RequiredPart SearchPattern::requiredPart() const
{
  SearchRule::RequiredPart reqPart = SearchRule::Envelope;

  if (!isEmpty()) {
    reqPart = (*std::max_element(constBegin(), constEnd(),
                                 boost::bind(&MailCommon::SearchRule::requiredPart, _1) <
                                 boost::bind(&MailCommon::SearchRule::requiredPart, _2) ))->requiredPart();
  }
  return reqPart;
}


void SearchPattern::purify()
{
  QList<SearchRule::Ptr>::iterator it = end();
  while ( it != begin() ) {
    --it;
    if ( (*it)->isEmpty() ) {
#ifndef NDEBUG
      kDebug() << "Removing" << (*it)->asString();
#endif
      erase( it );
      it = end();
    }
  }
}

void SearchPattern::readConfig( const KConfigGroup &config )
{
  init();

  mName = config.readEntry( "name" );
  if ( !config.hasKey( "rules" ) ) {
    kDebug() << "Found legacy config! Converting.";
    importLegacyConfig( config );
    return;
  }

  const QString op = config.readEntry( "operator" );
  if ( op == QLatin1String( "or" ) ) {
    mOperator = OpOr;
  } else if ( op == QLatin1String( "and" ) ) {
    mOperator = OpAnd;
  } else if ( op == QLatin1String( "all" ) ) {
    mOperator = OpAll;
  }

  const int nRules = config.readEntry( "rules", 0 );

  for ( int i = 0; i < nRules; ++i ) {
    SearchRule::Ptr r = SearchRule::createInstanceFromConfig( config, i );
    if ( !r->isEmpty() ) {
      append( r );
    }
  }
}

void SearchPattern::importLegacyConfig( const KConfigGroup & config )
{
  SearchRule::Ptr rule =
    SearchRule::createInstance(
      config.readEntry( "fieldA" ).toLatin1(),
      config.readEntry( "funcA" ).toLatin1(),
      config.readEntry( "contentsA" ) );

  if ( rule->isEmpty() ) {
    // if the first rule is invalid,
    // we really can't do much heuristics...
    return;
  }
  append( rule );

  const QString sOperator = config.readEntry( "operator" );
  if ( sOperator == "ignore" ) {
    return;
  }

  rule =
    SearchRule::createInstance(
      config.readEntry( "fieldB" ).toLatin1(),
      config.readEntry( "funcB" ).toLatin1(),
      config.readEntry( "contentsB" ) );

  if ( rule->isEmpty() ) {
    return;
  }
  append( rule );

  if ( sOperator == QLatin1String( "or" ) ) {
    mOperator = OpOr;
    return;
  }
  // This is the interesting case...
  if ( sOperator == QLatin1String( "unless" ) ) { // meaning "and not", ie we need to...
    // ...invert the function (e.g. "equals" <-> "doesn't equal")
    // We simply toggle the last bit (xor with 0x1)... This assumes that
    // SearchRule::Function's come in adjacent pairs of pros and cons
    SearchRule::Function func = last()->function();
    unsigned int intFunc = (unsigned int)func;
    func = SearchRule::Function( intFunc ^ 0x1 );

    last()->setFunction( func );
  }

  // treat any other case as "and" (our default).
}

void SearchPattern::writeConfig( KConfigGroup &config ) const
{
  config.writeEntry( "name", mName );
  switch( mOperator ) {
  case OpOr:
    config.writeEntry( "operator", "or" );
    break;
  case OpAnd:
    config.writeEntry( "operator", "and" );
    break;
  case OpAll:
    config.writeEntry( "operator", "all" );
    break;
  }

  int i = 0;
  QList<SearchRule::Ptr>::const_iterator it;
  QList<SearchRule::Ptr>::const_iterator endIt( constEnd() );

  for ( it = constBegin(); it != endIt && i < FILTER_MAX_RULES; ++i, ++it ) {
    // we could do this ourselves, but we want the rules to be extensible,
    // so we give the rule it's number and let it do the rest.
    (*it)->writeConfig( config, i );
  }

  // save the total number of rules.
  config.writeEntry( "rules", i );
}

void SearchPattern::init()
{
  clear();
  mOperator = OpAnd;
  mName = '<' + i18nc( "name used for a virgin filter", "unknown" ) + '>';
}

QString SearchPattern::asString() const
{
  QString result;
  switch( mOperator ) {
  case OpOr:
    result = i18n( "(match any of the following)" );
    break;
  case OpAnd:
    result = i18n( "(match all of the following)" );
    break;
  case OpAll:
    result = i18n( "(match all messages)" );
    break;
  }

  QList<SearchRule::Ptr>::const_iterator it;
  QList<SearchRule::Ptr>::const_iterator endIt = constEnd();
  for ( it = constBegin(); it != endIt; ++it ) {
    result += "\n\t" + FilterLog::recode( (*it)->asString() );
  }

  return result;
}

SearchPattern::SparqlQueryError SearchPattern::asAkonadiQuery( Akonadi::SearchQuery& query ) const
{
  query = Akonadi::SearchQuery(Akonadi::SearchTerm::RelOr);

  Akonadi::SearchTerm term(Akonadi::SearchTerm::RelAnd);
//   if ( op == SearchPattern::OpOr ) {
//     term = Akonadi::SearchTerm(Akonadi::SearchTerm::RelOr);
//   }

  const_iterator end( constEnd() );
  bool emptyIsNotAnError = false;
  bool resultAddQuery = false;
  for ( const_iterator it = constBegin(); it != end; ++it ) {
    (*it)->addQueryTerms( term, emptyIsNotAnError );
    resultAddQuery &= emptyIsNotAnError;
  }

  if ( term.subTerms().isEmpty() ) {
      if (resultAddQuery) {
          qDebug()<<" innergroup is Empty. Need to report bug";
          return MissingCheck;
      } else {
          return EmptyResult;
      }
  }
  query.setTerm(term);

  return NoError;
}

const SearchPattern & SearchPattern::operator=( const SearchPattern &other )
{
  if ( this == &other ) {
    return *this;
  }

  setOp( other.op() );
  setName( other.name() );

  clear(); // ###
  QList<SearchRule::Ptr>::const_iterator it;
  QList<SearchRule::Ptr>::const_iterator end( other.constEnd() );
  for ( it = other.constBegin(); it != end; ++it ) {
    append( SearchRule::createInstance( **it ) ); // deep copy
  }

  return *this;
}

QByteArray SearchPattern::serialize() const
{
  QByteArray out;
  QDataStream stream( &out, QIODevice::WriteOnly );
  *this >> stream;
  return out;
}

void SearchPattern::deserialize( const QByteArray &str )
{
  QDataStream stream( str );
  *this << stream;
}

QDataStream & SearchPattern::operator>>( QDataStream &s ) const
{
  switch( op() ) {
  case SearchPattern::OpAnd:
    s << QString::fromLatin1( "and" );
    break;
  case SearchPattern::OpOr:
    s << QString::fromLatin1( "or" );
    break;
  case SearchPattern::OpAll:
    s << QString::fromLatin1( "all" );
    break;
  }

  Q_FOREACH ( const SearchRule::Ptr rule, *this ) {
    *rule >> s;
  }
  return s;
}

QDataStream &SearchPattern::operator<<( QDataStream &s )
{
  QString op;
  s >> op;
  if ( op == QLatin1String( "and" ) ) {
    setOp( OpAnd );
  } else if ( op == QLatin1String( "or" ) ) {
    setOp( OpOr );
  } else if ( op == QLatin1String( "all" ) ) {
    setOp( OpAll );
  }

  while ( !s.atEnd() ) {
    SearchRule::Ptr rule = SearchRule::createInstance( s );
    append( rule );
  }
  return s;
}

void SearchPattern::generateSieveScript(QStringList &requires, QString &code)
{
    code += QLatin1String("\n#") + mName + QLatin1Char('\n');
    switch( mOperator ) {
    case OpOr:
        code += QLatin1String("if anyof (");
      break;
    case OpAnd:
        code += QLatin1String("if allof (");
      break;
    case OpAll:
        code += QLatin1String("if (true) {");
        return;
    }

    QList<SearchRule::Ptr>::const_iterator it;
    QList<SearchRule::Ptr>::const_iterator endIt( constEnd() );
    int i = 0;
    for ( it = constBegin(); it != endIt && i < FILTER_MAX_RULES; ++i, ++it ) {
        if (i != 0) {
            code += QLatin1String("\n, ");
        }
        (*it)->generateSieveScript(requires, code);
    }
}

// Needed for MSVC 2010, as it seems to not implicit cast for a pointer anymore
#ifdef _MSC_VER
namespace MailCommon {
uint qHash( SearchRule::Ptr sr )
{
  return ::qHash( sr.get() );
}
}
#endif
