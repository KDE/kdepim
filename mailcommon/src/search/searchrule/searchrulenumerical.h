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

#ifndef SEARCHRULENUMERICAL_H
#define SEARCHRULENUMERICAL_H

#include "searchpattern.h"
#include <AkonadiCore/Item>
namespace MailCommon
{
/**
 * @short This class represents a search pattern rule operating on numerical values.
 *
 * This class represents a search to be performed against a numerical value,
 * such as the age of the message in days or its size.
 */
class SearchRuleNumerical : public SearchRule
{
public:
    /**
     * Creates new numerical search rule.
     *
     * @param field The field to search in.
     * @param function The function to use for searching.
     * @param contents The contents to search for.
     */
    explicit SearchRuleNumerical(const QByteArray &field = QByteArray(),
                                 Function function = FuncContains,
                                 const QString &contents = QString());

    /**
     * @copydoc SearchRule::isEmpty()
     */
    bool isEmpty() const  Q_DECL_OVERRIDE;

    /**
     * @copydoc SearchRule::matches()
     */
    bool matches(const Akonadi::Item &item) const Q_DECL_OVERRIDE;

    /**
     * @copydoc SearchRule::requiredPart()
     */
    RequiredPart requiredPart() const Q_DECL_OVERRIDE;

    // Optimized matching not implemented, will use the unoptimized matching
    // from SearchRule
    using SearchRule::matches;

    /**
     * A helper method for the main matches() method.
     * Does the actual comparing.
     */
    bool matchesInternal(long numericalValue, long numericalContents,
                         const QString &contents) const;

    /**
     * @copydoc SearchRule::addQueryTerms()
     */
    void addQueryTerms(Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError) const Q_DECL_OVERRIDE;
    QString informationAboutNotValidRules() const Q_DECL_OVERRIDE;

};
}
#endif // SEARCHRULENUMERICAL_H
