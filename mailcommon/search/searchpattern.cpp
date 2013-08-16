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

#include <ontologies/nie.h>
#include <ontologies/nmo.h>
#include <ontologies/nco.h>

#include <Nepomuk2/Tag>
#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/AndTerm>
#include <Nepomuk2/Query/OrTerm>
#include <Nepomuk2/Query/LiteralTerm>
#include <Nepomuk2/Query/ResourceTerm>
#include <Nepomuk2/Query/NegationTerm>
#include <Nepomuk2/Query/ResourceTypeTerm>
#include <Nepomuk2/Vocabulary/PIMO>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDF>
#include <Nepomuk2/Vocabulary/NIE>

#include <Akonadi/Contact/ContactSearchJob>

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
        case FuncStartWith:
        case FuncNotStartWith:
        case FuncEndWith:
        case FuncNotEndWith:
            code += QLatin1Char('"') + i18n("\"%1\" is not supported with condition \"%2\"", QLatin1String(mField), conditionToString(mFunction)) + QLatin1Char('"');
            return;
        }
        code += (negative ? QLatin1String("not ") : QString()) + QString::fromLatin1("body :text %1 \"%2\"").arg(comparaison).arg(mContents);
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
        case FuncStartWith:
        case FuncNotStartWith:
        case FuncEndWith:
        case FuncNotEndWith:
            code += QLatin1Char('"') + i18n("\"%1\" is not supported with condition \"%2\"", QLatin1String(mField), conditionToString(mFunction)) + QLatin1Char('"');
            return;
        }
        code += (negative ? QLatin1String("not ") : QString()) + QString::fromLatin1("header %1 \"%2\" \"%3\"").arg(comparaison).arg(QLatin1String(mField)).arg(mContents);
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
  QString result  = "\"" + mField + "\" <";
  result += functionToString( mFunction );
  result += "> \"" + mContents + "\"";

  return result;
}


Nepomuk2::Query::ComparisonTerm::Comparator SearchRule::nepomukComparator() const
{
  switch ( function() ) {
  case SearchRule::FuncContains:
  case SearchRule::FuncContainsNot:
    return Nepomuk2::Query::ComparisonTerm::Contains;

  case SearchRule::FuncEquals:
  case SearchRule::FuncNotEqual:
    return Nepomuk2::Query::ComparisonTerm::Equal;

  case SearchRule::FuncIsGreater:
    return Nepomuk2::Query::ComparisonTerm::Greater;

  case SearchRule::FuncIsGreaterOrEqual:
      return Nepomuk2::Query::ComparisonTerm::GreaterOrEqual;

  case SearchRule::FuncIsLess:
    return Nepomuk2::Query::ComparisonTerm::Smaller;

  case SearchRule::FuncIsLessOrEqual:
    return Nepomuk2::Query::ComparisonTerm::SmallerOrEqual;

  case SearchRule::FuncRegExp:
  case SearchRule::FuncNotRegExp:
    return Nepomuk2::Query::ComparisonTerm::Regexp;

  case SearchRule::FuncStartWith:
  case SearchRule::FuncNotStartWith:
  case SearchRule::FuncEndWith:
  case SearchRule::FuncNotEndWith:
    return Nepomuk2::Query::ComparisonTerm::Regexp;
  default:
    kDebug() << "Unhandled function type: " << function();
  }

  return Nepomuk2::Query::ComparisonTerm::Equal;
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

void SearchRule::addAndNegateTerm( const Nepomuk2::Query::Term &term,
                                   Nepomuk2::Query::GroupTerm &termGroup ) const
{
  if ( isNegated() ) {
    Nepomuk2::Query::NegationTerm neg;
    neg.setSubTerm( term );
    termGroup.addSubTerm( neg );
  } else {
    termGroup.addSubTerm( term );
  }
}


QString SearchRule::xesamComparator() const
{
  switch ( function() ) {
  case SearchRule::FuncContains:
  case SearchRule::FuncContainsNot:
    return QLatin1String( "contains" );

  case SearchRule::FuncEquals:
  case SearchRule::FuncNotEqual:
    return QLatin1String( "equals" );

  case SearchRule::FuncIsGreater:
    return QLatin1String( "greaterThan" );

  case SearchRule::FuncIsGreaterOrEqual:
    return QLatin1String( "greaterThanEquals" );

  case SearchRule::FuncIsLess:
    return QLatin1String( "lessThan" );

  case SearchRule::FuncIsLessOrEqual:
    return QLatin1String( "lessThanEquals" );

  // FIXME how to handle the below? full text?
  case SearchRule::FuncRegExp:
  case SearchRule::FuncNotRegExp:
  case SearchRule::FuncStartWith:
  case SearchRule::FuncNotStartWith:
  case SearchRule::FuncEndWith:
  case SearchRule::FuncNotEndWith:
  default:
    kDebug() << "Unhandled function type: " << function();
  }

  return QLatin1String( "equals" );
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
    const Nepomuk2::Resource res( item.url() );
    foreach ( const Nepomuk2::Tag &tag, res.tags() ) {
      msgContents += tag.label();
    }
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


QString SearchRule::quote( const QString &content ) const
{
   //Without "" nepomuk will search a message containing each individual word
  QString newContent;
  switch( function() ) {
  case SearchRule::FuncRegExp:
  case SearchRule::FuncNotRegExp:
    newContent = content;
    break;
  case SearchRule::FuncStartWith:
  case SearchRule::FuncNotStartWith:
    newContent = QString::fromLatin1( "^%1" ).arg( content );
    break;
  case SearchRule::FuncEndWith:
  case SearchRule::FuncNotEndWith:
    newContent = QString::fromLatin1( "%1$" ).arg( content );;
    break;
  default:
    newContent = QString::fromLatin1( "\'%1\'" ).arg( content );
    break;
  }
  return newContent;
}

void SearchRuleString::addPersonTerm( Nepomuk2::Query::GroupTerm &groupTerm,
                                      const QUrl &field ) const
{
  // TODO split contents() into address/name and adapt the query accordingly
  const Nepomuk2::Query::ComparisonTerm valueTerm(
    Vocabulary::NCO::emailAddress(),
    Nepomuk2::Query::LiteralTerm( contents().toLower() ),
    nepomukComparator() );

  const Nepomuk2::Query::ComparisonTerm addressTerm(
    Vocabulary::NCO::hasEmailAddress(),
    valueTerm,
    Nepomuk2::Query::ComparisonTerm::Equal );

  const Nepomuk2::Query::ComparisonTerm personTerm(
    field,
    addressTerm,
    Nepomuk2::Query::ComparisonTerm::Equal );

  groupTerm.addSubTerm( personTerm );
}

void SearchRuleString::addHeaderTerm( Nepomuk2::Query::GroupTerm &groupTerm,
                                      const Nepomuk2::Query::Term &field ) const
{
  const Nepomuk2::Query::ComparisonTerm headerName(
    Vocabulary::NMO::headerName(),
    field,
    Nepomuk2::Query::ComparisonTerm::Equal );

  const Nepomuk2::Query::ComparisonTerm headerTerm(
    Vocabulary::NMO::headerValue(),
    Nepomuk2::Query::LiteralTerm( quote( contents() ) ),
    nepomukComparator() );

  groupTerm.addSubTerm( headerName );
  groupTerm.addSubTerm( headerTerm );

}

void SearchRuleString::addQueryTerms( Nepomuk2::Query::GroupTerm &groupTerm ) const
{
  Nepomuk2::Query::OrTerm termGroup;
  if ( kasciistricmp( field(), "<message>" ) == 0 ||
       kasciistricmp( field(), "<recipients>" ) == 0  ||
       kasciistricmp( field(), "<any header>" ) == 0 ) {

    const Nepomuk2::Query::ComparisonTerm valueTerm(
      Vocabulary::NCO::emailAddress(),
      Nepomuk2::Query::LiteralTerm( quote( contents() ) ),
      nepomukComparator() );

    const Nepomuk2::Query::ComparisonTerm addressTerm(
      Vocabulary::NCO::hasEmailAddress(),
      valueTerm,
      Nepomuk2::Query::ComparisonTerm::Equal );

    const Nepomuk2::Query::ComparisonTerm personTerm(
      Vocabulary::NMO::to(),
      addressTerm,
      Nepomuk2::Query::ComparisonTerm::Equal );

    const Nepomuk2::Query::ComparisonTerm personTermTo(
      Vocabulary::NMO::cc(),
      personTerm,
      Nepomuk2::Query::ComparisonTerm::Equal );

    const Nepomuk2::Query::ComparisonTerm personTermCC(
      Vocabulary::NMO::bcc(),
      personTermTo,
      Nepomuk2::Query::ComparisonTerm::Equal );

    if ( kasciistricmp( field(), "<any header>" ) == 0 ) {
      const Nepomuk2::Query::ComparisonTerm personTermBCC(
        Vocabulary::NMO::from(),
        personTermTo,
        Nepomuk2::Query::ComparisonTerm::Equal );
      termGroup.addSubTerm( personTermBCC );
    } else {
      termGroup.addSubTerm( personTermCC );
    }
  }

  if ( kasciistricmp( field(), "to" ) == 0 ) {
    addPersonTerm( termGroup, Vocabulary::NMO::to() );
  } else if ( kasciistricmp( field(), "cc" ) == 0 ) {
    addPersonTerm( termGroup, Vocabulary::NMO::cc() );
  } else if ( kasciistricmp( field(), "bcc" ) == 0 ) {
    addPersonTerm( termGroup, Vocabulary::NMO::bcc() );
  } else if ( kasciistricmp( field(), "from" ) == 0 ) {
    addPersonTerm( termGroup, Vocabulary::NMO::from() );
  }

  if ( kasciistricmp( field(), "subject" ) == 0 ||
       kasciistricmp( field(), "<any header>" ) == 0 ||
       kasciistricmp( field(), "<message>" ) == 0 ) {
    const Nepomuk2::Query::ComparisonTerm subjectTerm(
      Vocabulary::NMO::messageSubject(),
      Nepomuk2::Query::LiteralTerm( quote( contents() ) ),
      nepomukComparator() );
    termGroup.addSubTerm( subjectTerm );
  }
  if ( kasciistricmp( field(), "reply-to" ) == 0 ) {
    const Nepomuk2::Query::ComparisonTerm replyToTerm(
      Vocabulary::NMO::messageReplyTo(),
      Nepomuk2::Query::LiteralTerm( quote(contents() ) ),
      nepomukComparator() );
    termGroup.addSubTerm( replyToTerm );
  }

  if ( kasciistricmp( field(), "list-id" ) == 0 ) {
    addHeaderTerm( termGroup, Nepomuk2::Query::LiteralTerm( "List-Id" ) );
  } else if ( kasciistricmp( field(), "resent-from" ) == 0 ) {
    //TODO
  } else if ( kasciistricmp( field(), "x-loop" ) == 0 ) {
    addHeaderTerm( termGroup, Nepomuk2::Query::LiteralTerm( "X-Loop" ) );
  } else if ( kasciistricmp( field(), "x-mailing-list" ) == 0 ) {
    addHeaderTerm( termGroup, Nepomuk2::Query::LiteralTerm( "X-Mailing-List" ) );
  } else if ( kasciistricmp( field(), "x-spam-flag" ) == 0 ) {
    addHeaderTerm( termGroup, Nepomuk2::Query::LiteralTerm( "X-Spam-Flag" ) );
  }

  // TODO complete for other headers, generic headers

  if ( kasciistricmp( field(), "organization" )  == 0 ) {
    addHeaderTerm( termGroup, Nepomuk2::Query::LiteralTerm( "Organization" ) );
  }

  if ( kasciistricmp( field(), "<tag>" ) == 0 ) {
    const Nepomuk2::Tag tag( contents() );
    if ( tag.exists() ) {
       addAndNegateTerm(Nepomuk2::Query::ComparisonTerm(Soprano::Vocabulary::NAO::hasTag(),Nepomuk2::Query::ResourceTerm( tag ), Nepomuk2::Query::ComparisonTerm::Equal ),groupTerm );
    } else {
      foreach ( const Nepomuk2::Tag &tag, Nepomuk2::Tag::allTags() ) {
        if (tag.label() == contents()) {
            addAndNegateTerm(Nepomuk2::Query::ComparisonTerm(Soprano::Vocabulary::NAO::hasTag(),Nepomuk2::Query::ResourceTerm( tag ), Nepomuk2::Query::ComparisonTerm::Equal ),groupTerm );
            break;
        }
      }
    }
  }

  if ( field() == "<body>" || field() == "<message>" ) {
    const Nepomuk2::Query::ComparisonTerm bodyTerm(
      Vocabulary::NMO::plainTextMessageContent(),
      Nepomuk2::Query::LiteralTerm( quote( contents() ) ),
      nepomukComparator() );

    termGroup.addSubTerm( bodyTerm );

    const Nepomuk2::Query::ComparisonTerm attachmentBodyTerm(
      Vocabulary::NMO::plainTextMessageContent(),
      Nepomuk2::Query::LiteralTerm( quote( contents() ) ),
      nepomukComparator() );

    const Nepomuk2::Query::ComparisonTerm attachmentTerm(
      Vocabulary::NIE::isPartOf(),
      attachmentBodyTerm,
      Nepomuk2::Query::ComparisonTerm::Equal );

    termGroup.addSubTerm( attachmentTerm );
  }

  if ( !termGroup.subTerms().isEmpty() ) {
    addAndNegateTerm( termGroup, groupTerm );
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
      job->setQuery( Akonadi::ContactSearchJob::Email, KPIMUtils::extractEmailAddress( *it ) );
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
      job->setQuery( Akonadi::ContactSearchJob::Email, KPIMUtils::extractEmailAddress( *it ) );
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
      job->setQuery( Akonadi::ContactSearchJob::Email, KPIMUtils::extractEmailAddress( *it ) );
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
      job->setQuery( Akonadi::ContactSearchJob::Email, KPIMUtils::extractEmailAddress( *it ) );
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

void SearchRuleString::addXesamClause( QXmlStreamWriter &stream ) const
{
  const QString func = xesamComparator();

  stream.writeStartElement( func );

  if ( field().toLower() == "subject"  ||
       field().toLower() == "to"  ||
       field().toLower() == "cc"  ||
       field().toLower() == "bcc"  ||
       field().toLower() == "from"  ||
       field().toLower() == "sender" ) {
    stream.writeStartElement( QLatin1String( "field" ) );
    stream.writeAttribute( QLatin1String( "name" ), field().toLower() );
  } else {
    stream.writeStartElement( QLatin1String( "fullTextFields" ) );
  }
  stream.writeEndElement();
  stream.writeTextElement( QLatin1String( "string" ), contents() );

  stream.writeEndElement();
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


void SearchRuleNumerical::addQueryTerms( Nepomuk2::Query::GroupTerm &groupTerm ) const
{
  if ( kasciistricmp( field(), "<size>" ) == 0 ) {
    const Nepomuk2::Query::ComparisonTerm sizeTerm(
      Vocabulary::NIE::byteSize(),
      Nepomuk2::Query::LiteralTerm( contents().toInt() ),
      nepomukComparator() );
    addAndNegateTerm( sizeTerm, groupTerm );
  } else if ( kasciistricmp( field(), "<age in days>" ) == 0 ) {
    QDate date = QDate::currentDate();
    date = date.addDays( contents().toInt() );
    const Nepomuk2::Query::ComparisonTerm dateTerm(
      Vocabulary::NMO::sentDate(),
      Nepomuk2::Query::LiteralTerm( date ),
      nepomukComparator() );
    addAndNegateTerm( dateTerm, groupTerm );
  }
}

void SearchRuleNumerical::addXesamClause( QXmlStreamWriter &stream ) const
{
  Q_UNUSED( stream );
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



void SearchRuleDate::addQueryTerms( Nepomuk2::Query::GroupTerm &groupTerm ) const
{
    const QDate date = QDate::fromString( contents(), Qt::ISODate );
    const Nepomuk2::Query::ComparisonTerm dateTerm(
      Vocabulary::NMO::sentDate(),
      Nepomuk2::Query::LiteralTerm( date ),
      nepomukComparator() );
    addAndNegateTerm( dateTerm, groupTerm );
}

void SearchRuleDate::addXesamClause( QXmlStreamWriter &stream ) const
{
  Q_UNUSED( stream );
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
  const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
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


void SearchRuleStatus::addTagTerm( Nepomuk2::Query::GroupTerm &groupTerm,
                                   const QString &tagId ) const
{
  // TODO handle function() == NOT
  const Nepomuk2::Tag tag( tagId );
  if (tag.exists()) {
    addAndNegateTerm(
      Nepomuk2::Query::ComparisonTerm(
        Soprano::Vocabulary::NAO::hasTag(),
        Nepomuk2::Query::ResourceTerm( tag.uri() ),
        Nepomuk2::Query::ComparisonTerm::Equal ),
      groupTerm );
  }
}

void SearchRuleStatus::addQueryTerms( Nepomuk2::Query::GroupTerm &groupTerm ) const
{
  if ( mStatus.isImportant() ) {
    addTagTerm( groupTerm, "important" );
  } else if ( mStatus.isToAct() ) {
    addTagTerm( groupTerm, "todo" );
  } else if ( mStatus.isWatched() ) {
    addTagTerm( groupTerm, "watched" );
  } else if ( mStatus.isDeleted() ) {
    addTagTerm( groupTerm, "deleted" );
  } else if ( mStatus.isSpam() ) {
    addTagTerm( groupTerm, "spam" );
  } else if ( mStatus.isReplied() ) {
    addTagTerm( groupTerm, "replied" );
  } else if ( mStatus.isIgnored() ) {
    addTagTerm( groupTerm, "ignored" );
  } else if ( mStatus.isForwarded() ) {
    addTagTerm( groupTerm, "forwarded" );
  } else if ( mStatus.isSent() ) {
    addTagTerm( groupTerm, "sent" );
  } else if ( mStatus.isQueued() ) {
    addTagTerm( groupTerm, "queued" );
  } else if ( mStatus.isHam() ) {
    addTagTerm( groupTerm, "ham" );
  } else {
      bool read = false;
      if ( function() == FuncContains || function() == FuncEquals ) {
        read = true;
      }

      if ( !mStatus.isRead() ) {
        read = !read;
      }
      groupTerm.addSubTerm(
        Nepomuk2::Query::ComparisonTerm(
          Vocabulary::NMO::isRead(),
          Nepomuk2::Query::LiteralTerm( read ),
          Nepomuk2::Query::ComparisonTerm::Equal ) );

  }

  // TODO
}

void SearchRuleStatus::addXesamClause( QXmlStreamWriter &stream ) const
{
  Q_UNUSED( stream );
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


static Nepomuk2::Query::GroupTerm makeGroupTerm( SearchPattern::Operator op )
{
  if ( op == SearchPattern::OpOr ) {
    return Nepomuk2::Query::OrTerm();
  }
  return Nepomuk2::Query::AndTerm();
}

Nepomuk2::Query::ComparisonTerm SearchPattern::createChildTerm( const KUrl& url, bool& empty ) const
{
  const Nepomuk2::Resource parentResource( url );
  if ( !parentResource.exists() ) {
    empty = true;
    return Nepomuk2::Query::ComparisonTerm();
  }
  empty = false;
  const Nepomuk2::Query::ComparisonTerm isChildTerm( Vocabulary::NIE::isPartOf(), Nepomuk2::Query::ResourceTerm( parentResource ) );
  return isChildTerm;
}

QString SearchPattern::asSparqlQuery(bool &allIsEmpty, const KUrl::List& urlList) const
{

  Nepomuk2::Query::Query query;

  Nepomuk2::Query::AndTerm outerGroup;
  const Nepomuk2::Types::Class cl( Vocabulary::NMO::Email() );
  const Nepomuk2::Query::ResourceTypeTerm typeTerm( cl );
  const Nepomuk2::Query::Query::RequestProperty itemIdProperty(
    Akonadi::ItemSearchJob::akonadiItemIdUri(), false );

  Nepomuk2::Query::GroupTerm innerGroup = makeGroupTerm( mOperator );
  const_iterator end( constEnd() );
  for ( const_iterator it = constBegin(); it != end; ++it ) {
    (*it)->addQueryTerms( innerGroup );
  }


  if ( innerGroup.subTerms().isEmpty() ) {
    qDebug()<<" innergroup is Empty. Need to report bug";
    return QString();
  }
  if ( !urlList.isEmpty() ) {
    const int numberOfUrl = urlList.count();
    if ( numberOfUrl == 1 ) {
      bool empty = false;
      const Nepomuk2::Query::ComparisonTerm isChildTerm = createChildTerm( urlList.at( 0 ), empty );
      if ( empty ) {
        allIsEmpty = true;
        return QString();
      }
      const Nepomuk2::Query::AndTerm andTerm( isChildTerm, innerGroup );
      outerGroup.addSubTerm( andTerm );
    } else {
      QList<Nepomuk2::Query::Term> term;
      bool allFolderIsEmpty = true;
      for ( int i = 0; i < numberOfUrl; ++i ) {
        bool empty = false;
        const Nepomuk2::Query::ComparisonTerm childTerm = createChildTerm( urlList.at( i ), empty );
        if ( !empty ) {
          term<<childTerm;
          allFolderIsEmpty = false;
        }
      }
      if (allFolderIsEmpty) {
        allIsEmpty = true;
        return QString();
      }
      const Nepomuk2::Query::OrTerm orTerm( term );
      const Nepomuk2::Query::AndTerm andTerm( orTerm, innerGroup );
      outerGroup.addSubTerm( andTerm );
    }

  } else {
    outerGroup.addSubTerm( innerGroup );
  }
  outerGroup.addSubTerm( typeTerm );
  query.setTerm( outerGroup );
  query.addRequestProperty( itemIdProperty );
  allIsEmpty = false;
  return query.toSparqlQuery();
}

QString MailCommon::SearchPattern::asXesamQuery() const
{
  QString query;
  QXmlStreamWriter stream( &query );
  stream.setAutoFormatting( true );
  stream.writeStartDocument();
  stream.writeStartElement( QLatin1String( "request" ) );
  stream.writeAttribute( QLatin1String( "xmlns" ),
                         QLatin1String( "http://freedesktop.org/standards/xesam/1.0/query" ) );
  stream.writeStartElement( QLatin1String( "query" ) );

  const bool needsOperator = count() > 1;
  if ( needsOperator ) {
    if ( mOperator == SearchPattern::OpOr ) {
      stream.writeStartElement( QLatin1String( "or" ) );
    } else if ( mOperator == SearchPattern::OpAnd ) {
      stream.writeStartElement( QLatin1String( "and" ) );
    } else {
      Q_ASSERT( false ); // can't happen (TM)
    }
  }

  QListIterator<SearchRule::Ptr> it( *this );
  while ( it.hasNext() ) {
    const SearchRule::Ptr rule = it.next();
    rule->addXesamClause( stream );
  }

  if ( needsOperator ) {
    stream.writeEndElement(); // operator
  }
  stream.writeEndElement(); // query
  stream.writeEndElement(); // request
  stream.writeEndDocument();
  return query;
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
