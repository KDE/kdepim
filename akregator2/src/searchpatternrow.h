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

#ifndef SEARCHPATTERNROW_H
#define SEARCHPATTERNROW_H

#include <QtGui/QWidget>

#include <Nepomuk2/Query/ComparisonTerm>

class QHBoxLayout;
class QWidget;
class KPushButton;
class KComboBox;
class KLineEdit;

namespace Akregator2 {

class SearchPatternRow : public QWidget
{
    Q_OBJECT

  public:
    enum Comparator {
      Contains,
      ContainsNot,
      Equals,
      EqualsNot,
      IsAfter,
      IsAfterOrEqual,
      IsBefore,
      IsBeforeOrEqual,
      StartsWith,
      StartsWithNot,
      EndsWith,
      EndsWithNot,
      MatchesRegex,
      MatchesRegexNot
    };

    enum Field {
      AnyField,
      Title,
      Author,
      Content,
      PublishDate
    };

    explicit SearchPatternRow( QWidget* parent = 0 );
    virtual ~SearchPatternRow();

    void setAddButtonEnabled( bool enable );
    void setRemoveButtonEnabled( bool enable );

    Field field() const;
    void setField( Field field );

    QString expression() const;
    void setExpression( const QString &expression );

    Comparator comparator() const;
    void setComparator( Comparator comparator );

    Nepomuk2::Query::ComparisonTerm::Comparator nepomukComparator() const;

    Nepomuk2::Query::Term asTerm() const;

  Q_SIGNALS:
    void patternAddClicked();
    void patternRemoveClicked();

  private Q_SLOTS:
    void slotFieldChanged( int index );

  private:
    bool isNegation() const;
    QUrl fieldUrl( Field field ) const;

    Nepomuk2::Query::Term stringAsTerm( const QUrl &property ) const;
    Nepomuk2::Query::Term dateAsTerm( const QUrl &property ) const;
    Nepomuk2::Query::Term contactAsTerm( const QUrl &property ) const;

    QHBoxLayout *mMainLayout;
    KComboBox *mFields;
    KComboBox *mComparator;
    QWidget *mExpression;

    KPushButton *mAddButton;
    KPushButton *mRemoveButton;

};

} /* namespace Akregator2 */

#endif // SEARCHPATTERNROW_H
