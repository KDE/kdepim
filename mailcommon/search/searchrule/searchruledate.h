/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef SEARCHRULEDATE_H
#define SEARCHRULEDATE_H

#include "searchpattern.h"
#include <Akonadi/Item>
namespace MailCommon {

class SearchRuleDate : public SearchRule
{
public:
    /**
     * Creates new date search rule.
     *
     * @param field The field to search in.
     * @param function The function to use for searching.
     * @param contents The contents to search for.
     */
    explicit SearchRuleDate( const QByteArray &field = QByteArray(),
                             Function function = FuncContains,
                             const QString &contents = QString() );

    /**
     * @copydoc SearchRule::isEmpty()
     */
    virtual bool isEmpty() const ;

    /**
     * @copydoc SearchRule::matches()
     */
    virtual bool matches( const Akonadi::Item &item ) const;

    /**
     * @copydoc SearchRule::requiredPart()
     */
    virtual RequiredPart requiredPart() const;

    // Optimized matching not implemented, will use the unoptimized matching
    // from SearchRule
    using SearchRule::matches;

    /**
     * A helper method for the main matches() method.
     * Does the actual comparing.
     */
    bool matchesInternal( const QDate& dateValue, const QDate& msgDate ) const;

    /**
     * @copydoc SearchRule::addQueryTerms()
     */
    virtual void addQueryTerms( Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError ) const;
    virtual QString informationAboutNotValidRules() const;

};
}

#endif // SEARCHRULEDATE_H
