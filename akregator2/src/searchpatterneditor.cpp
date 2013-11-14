/*
    This file is part of Akregator2.

    Copyright (C) 2012 Dan Vrátil <dvratil@redhat.com>

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

#include "searchpatterneditor.h"
#include "searchpatternrow.h"

#include <QButtonGroup>
#include <QRadioButton>
#include <QLayout>
#include <KLocalizedString>
#include <Akonadi/ItemSearchJob>

#include <Nepomuk2/Query/Query>
#include <Nepomuk2/Query/ComparisonTerm>
#include <Nepomuk2/Query/LiteralTerm>
#include <Nepomuk2/Query/AndTerm>
#include <Nepomuk2/Query/OrTerm>
#include <Nepomuk2/Query/NegationTerm>
#include <Nepomuk2/Query/ResourceTerm>
#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Resource>

#include <QDebug>
#include <QDate>

using namespace Akregator2;

SearchPatternEditor::SearchPatternEditor( QWidget* parent ):
  QWidget(parent)
{
  m_mainLayout = new QVBoxLayout( this );

  m_operator = new QButtonGroup( this );
  m_operator->addButton( new QRadioButton( i18n( "Match a&ll of the following"), this ), 0 );
  m_operator->addButton( new QRadioButton( i18n( "Match an&y of the following"), this ), 1 );
  m_operator->button( 0 )->setChecked( true );

  m_mainLayout->addWidget( m_operator->button( 0 ) );
  m_mainLayout->addWidget( m_operator->button( 1 ) );

  slotAddPatternRow();
  slotAddPatternRow();
}

SearchPatternEditor::~SearchPatternEditor()
{
  qDeleteAll( m_patterns );
  m_patterns.clear();
}

QVariant SearchPatternEditor::pattern() const
{
  QVariantMap map;
  map[ "mode" ] = m_operator->checkedId();

  QVariantList patterns;
  Q_FOREACH( SearchPatternRow* row, m_patterns ) {
    QVariantMap pattern;
    pattern[ "field" ] = row->field();
    pattern[ "comparator" ] = row->comparator();
    pattern[ "expression" ] = row->expression();

    patterns << pattern;
  }

  map[ "patterns" ] = patterns;

  return map;
}

void SearchPatternEditor::setPattern(const QVariant& _pattern)
{
  const QVariantMap &map = _pattern.toMap();

  m_operator->button( map[ "mode "].toInt() )->setChecked( true );

  qDeleteAll(m_patterns);
  m_patterns.clear();

  QVariantList patterns = map[ "patterns" ].toList();
  Q_FOREACH( const QVariant &v, patterns ) {
    const QVariantMap &pattern = v.toMap();

    slotAddPatternRow();
    SearchPatternRow *row = m_patterns.last();

    row->setField( (SearchPatternRow::Field) pattern[ "field" ].toInt() );
    row->setComparator( (SearchPatternRow::Comparator) pattern[ "comparator" ].toInt() );
    row->setExpression( pattern[ "expression" ].toString() );
  }
}

void SearchPatternEditor::clear()
{

}

void SearchPatternEditor::slotAddPatternRow()
{
  SearchPatternRow *patternRow = new SearchPatternRow( );
  connect( patternRow, SIGNAL(patternAddClicked()), SLOT(slotAddPatternRow()) );
  connect( patternRow, SIGNAL(patternRemoveClicked()), SLOT(slotRemovePatternRow()) );

  m_patterns << patternRow;

  /* Insert below the pattern in which user clicked "Add" or at the end, if
   * previous is null */
  SearchPatternRow *previous = qobject_cast<SearchPatternRow*>( sender() );
  int index = m_mainLayout->indexOf( previous );
  m_mainLayout->insertWidget( ( index > -1 ? index + 1 : -1 )  , patternRow );

  updatePatternsButtons();
}

void SearchPatternEditor::slotRemovePatternRow()
{
  SearchPatternRow *patternRow = qobject_cast<SearchPatternRow*>( sender() );
  if (!patternRow) {
    return;
  }

  m_mainLayout->removeWidget( patternRow );
  m_patterns.removeOne( patternRow );
  delete patternRow;

  updatePatternsButtons();
}

void SearchPatternEditor::updatePatternsButtons()
{
  bool enableRemove = true;
  if ( m_patterns.count() <= 2) {
    enableRemove = false;
  }

  Q_FOREACH( SearchPatternRow *patternRow, m_patterns ) {
    patternRow->setRemoveButtonEnabled( enableRemove );
  }
}

static Nepomuk2::Query::GroupTerm makeGroupTerm( bool orTerm )
{
  if ( orTerm ) {
    return Nepomuk2::Query::OrTerm();
  } else {
    return Nepomuk2::Query::AndTerm();
  }
}

QString SearchPatternEditor::nepomukQuery( const KUrl::List &parentFolders ) const
{
  Nepomuk2::Query::Query query;
  const Nepomuk2::Query::Query::RequestProperty itemIdProperty(
    Akonadi::ItemSearchJob::akonadiItemIdUri(), false );

  Nepomuk2::Query::OrTerm parentsTerm;
  // Constraint results only to selected feeds
  Q_FOREACH( const KUrl &url, parentFolders ) {
    const Nepomuk2::Resource parentResource( url );
    Nepomuk2::Query::ComparisonTerm parentTerm(
      Nepomuk2::Vocabulary::NIE::isPartOf(),
      Nepomuk2::Query::ResourceTerm( parentResource ),
      Nepomuk2::Query::ComparisonTerm::Equal );

    parentsTerm.addSubTerm( parentTerm );
  }

  Nepomuk2::Query::GroupTerm paramsTerm = makeGroupTerm( m_operator->checkedId() == 1 );
  Q_FOREACH( SearchPatternRow *patternRow, m_patterns ) {
    Nepomuk2::Query::Term rowTerm = patternRow->asTerm();
    if ( rowTerm.isValid() ) {
      paramsTerm.addSubTerm( rowTerm );
    }
  }

  Nepomuk2::Query::AndTerm mainTerm;
  mainTerm.addSubTerm( parentsTerm );
  mainTerm.addSubTerm( paramsTerm );

  query.setTerm( mainTerm );
  query.addRequestProperty( itemIdProperty );
  return query.toSparqlQuery();
}
