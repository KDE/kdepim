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

#ifndef SEARCHRULESTRING_H
#define SEARCHRULESTRING_H

#include "searchpattern.h"
#include <AkonadiCore/Item>

/**
 * @short This class represents a search pattern rule operating on a string.
 *
 * This class represents a search to be performed against a string.
 * The string can be either a message header, or a pseudo header, such
 * as \<body\>
 */
namespace MailCommon
{
class SearchRuleString : public SearchRule
{
public:
    /**
     * Creates new new string search rule.
     *
     * @param field The field to search in.
     * @param function The function to use for searching.
     * @param contents The contents to search for.
     */
    explicit SearchRuleString(const QByteArray &field = QByteArray(),
                              Function function = FuncContains,
                              const QString &contents = QString());

    /**
     * Creates a new string search rule from an @p other rule.
     */
    SearchRuleString(const SearchRuleString &other);

    /**
     * Initializes this rule with an @p other rule.
     */
    const SearchRuleString &operator=(const SearchRuleString &other);

    /**
     * Destroys the string search rule.
     */
    virtual ~SearchRuleString();

    /**
     * @copydoc SearchRule::isEmpty()
     */
    bool isEmpty() const Q_DECL_OVERRIDE;

    /**
     * @copydoc SearchRule::requiredPart()
     */
    RequiredPart requiredPart() const Q_DECL_OVERRIDE;

    /**
     * @copydoc SearchRule::matches()
     */
    bool matches(const Akonadi::Item &item) const Q_DECL_OVERRIDE;

    /**
     * A helper method for the main matches() method.
     * Does the actual comparing.
     */
    bool matchesInternal(const QString &contents) const;

    /**
     * @copydoc SearchRule::addQueryTerms()
     */
    void addQueryTerms(Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError) const Q_DECL_OVERRIDE;
    QString informationAboutNotValidRules() const Q_DECL_OVERRIDE;
};
}
#endif // SEARCHRULESTRING_H
