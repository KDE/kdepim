/*
    This file is part of Akregator2.

    Copyright (C) 2013 Dan Vrátil <dvratil@redhat.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "searchpatternrow.h"

#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>
#include <KPushButton>
#include <KIcon>
#include <KDateComboBox>
#include <QHBoxLayout>
#include <KDebug>

#include <Nepomuk2/Types/Property>
#include <Nepomuk2/Query/OrTerm>
#include <Nepomuk2/Query/AndTerm>
#include <Nepomuk2/Query/LiteralTerm>
#include <Nepomuk2/Query/NegationTerm>
#include <Nepomuk2/Vocabulary/NCO>
#include <Nepomuk2/Vocabulary/NIE>

using namespace Akregator2;

static const struct ComparatorField {
  SearchPatternRow::Comparator index;
  QString label;
} comparatorFields[] = {
  { SearchPatternRow::Contains, I18N_NOOP( "contains" ) },
  { SearchPatternRow::ContainsNot, I18N_NOOP( "does not contain" ) },
  { SearchPatternRow::Equals, I18N_NOOP( "is equal to" ) },
  { SearchPatternRow::EqualsNot, I18N_NOOP( "is not equal to" ) },
  { SearchPatternRow::IsAfter, I18N_NOOP( "is after" ) },
  { SearchPatternRow::IsAfterOrEqual, I18N_NOOP( "is after or equal to" ) },
  { SearchPatternRow::IsBefore, I18N_NOOP( "it before" ) },
  { SearchPatternRow::IsBeforeOrEqual, I18N_NOOP( "is before or equal to" ) },
  { SearchPatternRow::StartsWith, I18N_NOOP( "starts with" ) },
  { SearchPatternRow::StartsWithNot, I18N_NOOP( "does not start with" ) },
  { SearchPatternRow::EndsWith, I18N_NOOP( "ends with" ) },
  { SearchPatternRow::EndsWithNot, I18N_NOOP( "does not end with" ) },
  { SearchPatternRow::MatchesRegex, I18N_NOOP( "matches regular expr." ) },
  { SearchPatternRow::MatchesRegexNot, I18N_NOOP( "does not match reg, expr." ) },
};


SearchPatternRow::SearchPatternRow( QWidget* parent )
  : QWidget(parent)
  , mExpression(0)
{
  mMainLayout = new QHBoxLayout( this );
  setLayout( mMainLayout );

  mFields = new KComboBox( parent );
  mFields->addItem( i18n( "Title" ), Title );
  mFields->addItem( i18n( "Author" ), Author );
  mFields->addItem( i18n( "Content" ), Content );
  mFields->addItem( i18n( "Publish date" ), PublishDate );
  mFields->addItem( i18n( "Any field" ), AnyField );
  connect( mFields, SIGNAL(currentIndexChanged(int)), SLOT(slotFieldChanged(int)) );
  mMainLayout->addWidget( mFields );

  mComparator = new KComboBox( parent );
  mMainLayout->addWidget( mComparator );

  slotFieldChanged( 0 ); /* Force populate mComparator and mExpression */

  mAddButton = new KPushButton( KIcon( "list-add" ), QString(), parent );
  connect( mAddButton, SIGNAL(clicked(bool)), SIGNAL(patternAddClicked()) );
  mMainLayout->addWidget( mAddButton );

  mRemoveButton = new KPushButton( KIcon( "list-remove" ), QString(), parent );
  connect( mRemoveButton, SIGNAL(clicked(bool)), SIGNAL(patternRemoveClicked()) );
  mMainLayout->addWidget( mRemoveButton );
}

SearchPatternRow::~SearchPatternRow()
{

}

void SearchPatternRow::setAddButtonEnabled( bool enable )
{
  mAddButton->setEnabled( enable );
}

void SearchPatternRow::setRemoveButtonEnabled( bool enable )
{
  mRemoveButton->setEnabled( enable );
}

void SearchPatternRow::slotFieldChanged( int index )
{
  mComparator->clear();
  if ( mFields->itemData( index ).toInt() == PublishDate ) {
    mComparator->addItem( comparatorFields[Equals].label, Equals);
    mComparator->addItem( comparatorFields[EqualsNot].label, EqualsNot);
    mComparator->addItem( comparatorFields[IsAfter].label, IsAfter);
    mComparator->addItem( comparatorFields[IsAfterOrEqual].label, IsAfterOrEqual);
    mComparator->addItem( comparatorFields[IsBefore].label, IsBefore);
    mComparator->addItem( comparatorFields[IsBeforeOrEqual].label, IsBeforeOrEqual);

    KDateComboBox *combo = new KDateComboBox( this );
    combo->setDate( QDate::currentDate() );

    delete mExpression;
    mExpression = combo;
    mMainLayout->insertWidget( 2, mExpression, 1 );

  } else {
    mComparator->addItem( comparatorFields[Contains].label, Contains);
    mComparator->addItem( comparatorFields[ContainsNot].label, ContainsNot);
    mComparator->addItem( comparatorFields[Equals].label, Equals);
    mComparator->addItem( comparatorFields[EqualsNot].label, EqualsNot);
    mComparator->addItem( comparatorFields[StartsWith].label, StartsWith);
    mComparator->addItem( comparatorFields[StartsWithNot].label, StartsWithNot);
    mComparator->addItem( comparatorFields[EndsWith].label, EndsWith);
    mComparator->addItem( comparatorFields[EndsWithNot].label, EndsWithNot);
    mComparator->addItem( comparatorFields[MatchesRegex].label, MatchesRegex);
    mComparator->addItem( comparatorFields[MatchesRegexNot].label, MatchesRegexNot);

    KLineEdit *edit = new KLineEdit( this );
    edit->setClearButtonShown( true );

    delete mExpression;
    mExpression = edit;
    mMainLayout->insertWidget( 2, mExpression, 1 );
  }
}

SearchPatternRow::Comparator SearchPatternRow::comparator() const
{
  return (Comparator) mComparator->itemData( mComparator->currentIndex() ).toInt();
}

void SearchPatternRow::setComparator(SearchPatternRow::Comparator comparator)
{
  for (int i = 0; i < mComparator->count(); i++) {
    if ( mComparator->itemData( i ).toInt() == comparator ) {
      mComparator->setCurrentIndex( i );
      return;
    }
  }
}

Nepomuk2::Query::ComparisonTerm::Comparator SearchPatternRow::nepomukComparator() const
{
  switch ( (Comparator) mComparator->itemData( mComparator->currentIndex() ).toInt() ) {
    case Contains:
    case ContainsNot:
      return Nepomuk2::Query::ComparisonTerm::Contains;

    case Equals:
    case EqualsNot:
      return Nepomuk2::Query::ComparisonTerm::Equal;

    case IsAfter:
      return Nepomuk2::Query::ComparisonTerm::Greater;

    case IsAfterOrEqual:
      return Nepomuk2::Query::ComparisonTerm::GreaterOrEqual;

    case IsBefore:
      return Nepomuk2::Query::ComparisonTerm::Smaller;

    case IsBeforeOrEqual:
      return Nepomuk2::Query::ComparisonTerm::SmallerOrEqual;

    default:
      kWarning() << "We don't support these yet!";
      return Nepomuk2::Query::ComparisonTerm::Contains;
  }
}

bool SearchPatternRow::isNegation() const
{
  Comparator cmp = (Comparator) mComparator->itemData( mComparator->currentIndex() ).toInt();

  return ( ( cmp == ContainsNot ) || ( cmp == EqualsNot ) || ( cmp == StartsWithNot ) ||
           ( cmp == EndsWithNot ) || ( cmp == MatchesRegexNot ));
}

SearchPatternRow::Field SearchPatternRow::field() const
{
  return (Field) mFields->itemData( mFields->currentIndex() ).toInt();
}

void SearchPatternRow::setField(SearchPatternRow::Field field)
{
  for (int i = 0; i < mFields->count(); i++) {
    if ( mFields->itemData( i ).toInt() == field ) {
      mFields->setCurrentIndex( i );
      return;
    }
  }
}

QString SearchPatternRow::expression() const
{
  if ( field() == PublishDate ) {
    return qobject_cast<KDateComboBox*>( mExpression )->date().toString();
  } else {
    return qobject_cast<KLineEdit*>( mExpression )->text();
  }
}

void SearchPatternRow::setExpression(const QString& expression)
{
  if ( field() == PublishDate ) {
    qobject_cast<KDateComboBox*>( mExpression )->setDate( QDate::fromString( expression ) );
  } else {
    qobject_cast<KLineEdit*>( mExpression )->setText( expression );
  }
}

Nepomuk2::Query::Term SearchPatternRow::asTerm() const
{
  if ( expression().isEmpty() ) {
    return Nepomuk2::Query::Term();
  }

  Nepomuk2::Query::Term term;
  Field currentField = (Field) mFields->itemData( mFields->currentIndex() ).toUInt();

  if ( currentField == PublishDate ) {
    term = dateAsTerm( fieldUrl( PublishDate ) );
  } else if ( currentField == Author ) {
    term = contactAsTerm( fieldUrl( Author ) );
  } else if ( currentField == AnyField ) {
    Nepomuk2::Query::OrTerm orTerm;
    orTerm.addSubTerm( dateAsTerm( fieldUrl( PublishDate ) ) );
    orTerm.addSubTerm( stringAsTerm( fieldUrl( Title ) ) );
    orTerm.addSubTerm( stringAsTerm( fieldUrl( Content ) ) );
    orTerm.addSubTerm( contactAsTerm( fieldUrl( Author ) ) );
    term = orTerm;
  } else {
    term = stringAsTerm( fieldUrl( currentField ) );
  }

  kDebug() << term;
  return term;
}

QUrl SearchPatternRow::fieldUrl( SearchPatternRow::Field field ) const
{
  switch (field) {
    case Author:
      return Nepomuk2::Vocabulary::NCO::creator();
    case Title:
      return Nepomuk2::Vocabulary::NIE::title();
    case Content:
      return Nepomuk2::Vocabulary::NIE::description();
    case PublishDate:
      // FIXME: Use a generated Vocabulary once NRSS is shipped publicly somewhere
      return QUrl::fromEncoded( "http://www.kde.org/ontologies/nrss#publishTime", QUrl::StrictMode );
    default:
      return QUrl();
  }

  return QUrl();
}

Nepomuk2::Query::Term SearchPatternRow::dateAsTerm( const QUrl& property ) const
{
  Nepomuk2::Query::Term term;

  const QDate date = QDate::fromString( expression() );

  if ( nepomukComparator() == Nepomuk2::Query::ComparisonTerm::Greater ) {
    term = Nepomuk2::Query::ComparisonTerm(
            property,
            Nepomuk2::Query::LiteralTerm( QDateTime( date ).addDays( 1 ) ),
            Nepomuk2::Query::ComparisonTerm::GreaterOrEqual );
  } else if ( nepomukComparator() == Nepomuk2::Query::ComparisonTerm::GreaterOrEqual ) {
    term = Nepomuk2::Query::ComparisonTerm(
            property,
            Nepomuk2::Query::LiteralTerm( QDateTime( date ) ),
            Nepomuk2::Query::ComparisonTerm::GreaterOrEqual );
  } else if ( nepomukComparator() == Nepomuk2::Query::ComparisonTerm::Smaller ) {
    term = Nepomuk2::Query::ComparisonTerm(
            property,
            Nepomuk2::Query::LiteralTerm( QDateTime( date ) ),
            Nepomuk2::Query::ComparisonTerm::Smaller );
  } else if ( nepomukComparator() == Nepomuk2::Query::ComparisonTerm::SmallerOrEqual ) {
    term = Nepomuk2::Query::ComparisonTerm(
            property,
            Nepomuk2::Query::LiteralTerm( QDateTime( date ).addDays( 1 ) ),
            Nepomuk2::Query::ComparisonTerm::Smaller );
  } else if ( nepomukComparator() == Nepomuk2::Query::ComparisonTerm::Equal ) {
    Nepomuk2::Query::AndTerm andTerm;
    andTerm.addSubTerm( Nepomuk2::Query::ComparisonTerm(
            property,
            Nepomuk2::Query::LiteralTerm( QDateTime( date ) ),
            Nepomuk2::Query::ComparisonTerm::GreaterOrEqual ) );
    andTerm.addSubTerm( Nepomuk2::Query::ComparisonTerm(
            property,
            Nepomuk2::Query::LiteralTerm( QDateTime( date ).addDays( 1 ) ),
            Nepomuk2::Query::ComparisonTerm::Smaller ) );

    term = andTerm;
  }

  if ( isNegation() ) {
    term = Nepomuk2::Query::NegationTerm::negateTerm( term );

  }

  return term;
}

Nepomuk2::Query::Term SearchPatternRow::contactAsTerm( const QUrl &property) const
{
  const Nepomuk2::Query::Term emailTerm =
    stringAsTerm( Nepomuk2::Vocabulary::NCO::emailAddress() );

  const Nepomuk2::Query::ComparisonTerm hasEmailTerm(
    Nepomuk2::Vocabulary::NCO::hasEmailAddress(),
    emailTerm, Nepomuk2::Query::ComparisonTerm::Equal );

  const Nepomuk2::Query::Term nameTerm =
    stringAsTerm( Nepomuk2::Vocabulary::NCO::fullname() );

  const Nepomuk2::Query::OrTerm contactTerm( hasEmailTerm, nameTerm );

  Nepomuk2::Query::ComparisonTerm term (
    property,
    contactTerm,
    Nepomuk2::Query::ComparisonTerm::Equal );

  return term;
}

Nepomuk2::Query::Term SearchPatternRow::stringAsTerm( const QUrl& property ) const
{
  Nepomuk2::Query::Term term;

  switch ( (Comparator) mComparator->itemData( mComparator->currentIndex() ).toInt() ) {
    case Contains:
    case ContainsNot:
      term = Nepomuk2::Query::ComparisonTerm(
              property,
              Nepomuk2::Query::LiteralTerm( expression() ),
              Nepomuk2::Query::ComparisonTerm::Contains );
      break;

    case Equals:
    case EqualsNot:
      term = Nepomuk2::Query::ComparisonTerm(
              property,
              Nepomuk2::Query::LiteralTerm( expression() ),
              Nepomuk2::Query::ComparisonTerm::Equal );
      break;

    case StartsWith:
    case StartsWithNot:
      term = Nepomuk2::Query::ComparisonTerm(
              property,
              Nepomuk2::Query::LiteralTerm( QString::fromLatin1( "^%1" ).arg( expression() ) ),
              Nepomuk2::Query::ComparisonTerm::Regexp );
      break;

    case EndsWith:
    case EndsWithNot:
      term = Nepomuk2::Query::ComparisonTerm(
              property,
              Nepomuk2::Query::LiteralTerm( QString::fromLatin1( "%1$" ).arg( expression() ) ),
              Nepomuk2::Query::ComparisonTerm::Regexp );
      break;

    case MatchesRegex:
    case MatchesRegexNot:
      term = Nepomuk2::Query::ComparisonTerm(
              property,
              Nepomuk2::Query::LiteralTerm( QString::fromLatin1( "%1" ).arg( expression() ) ),
              Nepomuk2::Query::ComparisonTerm::Regexp );
      break;

    default:
      term = Nepomuk2::Query::Term();
  }

  if ( isNegation() ) {
    term = Nepomuk2::Query::NegationTerm::negateTerm( term );
  }

  return term;
}
