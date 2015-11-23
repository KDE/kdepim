/*

  Author: Marc Mutz <mutz@kde.org>

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

#ifndef MAILCOMMON_SEARCHPATTERN_H
#define MAILCOMMON_SEARCHPATTERN_H

#include "mailcommon_export.h"
#include <KLocalizedString>
#include "mailcommon/searchrule.h"

#include <QList>
#include <QString>

#include <AkonadiCore/searchquery.h>

namespace Akonadi
{
class Item;
}

namespace KMime
{
class Message;
}

class KConfigGroup;

namespace MailCommon
{

// Needed for MSVC 2010, as it seems to not implicit cast for a pointer anymore
#ifdef _MSC_VER
uint qHash(SearchRule::Ptr sr);
#endif

// ------------------------------------------------------------------------

/** This class is an abstraction of a search over messages.  It is
    intended to be used inside a KFilter (which adds KFilterAction's),
    as well as in KMSearch. It can read and write itself into a
    KConfig group and there is a constructor, mainly used by KMFilter
    to initialize from a preset KConfig-Group.

    From a class hierarchy point of view, it is a QPtrList of
    SearchRule's that adds the boolean operators (see Operator)
    'and' and 'or' that connect the rules logically, and has a name
    under which it could be stored in the config file.

    As a QPtrList with autoDelete enabled, it assumes that it is the
    central repository for the rules it contains. So if you want to
    reuse a rule in another pattern, make a deep copy of that rule.

    @short An abstraction of a search over messages.
    @author Marc Mutz <mutz@kde.org>
*/
class MAILCOMMON_EXPORT SearchPattern : public QList<SearchRule::Ptr>
{

public:
    /**
     * Boolean operators that connect the return values of the
     * individual rules. A pattern with @p OpAnd will match iff all
     *  it's rules match, whereas a pattern with @p OpOr will match if
     *  any of it's rules matches.
     */
    enum Operator {
        OpAnd,
        OpOr,
        OpAll
    };

    enum SparqlQueryError {
        NoError = 0,
        MissingCheck,
        FolderEmptyOrNotIndexed,
        EmptyResult,
        NotEnoughCharacters
    };

    /**
     * Constructor which provides a pattern with minimal, but
     * sufficient initialization. Unmodified, such a pattern will fail
     * to match any KMime::Message. You can query for such an empty
     * rule by using isEmpty, which is inherited from QPtrList.
     */
    SearchPattern();

    /**
     * Constructor that initializes from a given KConfig group, if
     * given. This feature is mainly (solely?) used in KMFilter,
     * as we don't allow to store search patterns in the config (yet).
     */
    explicit SearchPattern(const KConfigGroup &config);

    /** Destructor. Deletes all stored rules! */
    ~SearchPattern();

    /**
     * The central function of this class. Tries to match the set of
     * rules against a KMime::Message. It's virtual to allow derived
     * classes with added rules to reimplement it, yet reimplemented
     * methods should and (&&) the result of this function with their
     * own result or else most functionality is lacking, or has to be
     * reimplemented, since the rules are private to this class.
     *
     * @return true if the match was successful, false otherwise.
     */
    bool matches(const Akonadi::Item &item, bool ignoreBody = false) const;

    /**
     * Returns the required part from the item that is needed for the search to
     * operate. See @ref RequiredPart */
    SearchRule::RequiredPart requiredPart() const;

    /**
     * Removes all empty rules from the list. You should call this
     * method whenever the user had had control of the rules outside of
     * this class. (e.g. after editing it with SearchPatternEdit).
     */
    QString purify(bool removeAction = true);

    /**
     * Reads a search pattern from a KConfigGroup. If it does not find
     * a valid saerch pattern in the preset group, initializes the pattern
     * as if it were constructed using the default constructor.
     *
     * For backwards compatibility with previous versions of KMail, it
     * checks for old-style filter rules (e.g. using @p OpIgnore)
     * in @p config und converts them to the new format on writeConfig.
     *
     * Derived classes reimplementing readConfig() should also call this
     * method, or else the rules will not be loaded.
     */
    void readConfig(const KConfigGroup &config);

    /**
     * Writes itself into @p config. Tries to delete old-style keys by
     * overwriting them with QString().
     *
     * Derived classes reimplementing writeConfig() should also call this
     * method, or else the rules will not be stored.
     */
    void writeConfig(KConfigGroup &config) const;

    /**
     * Returns the name of the search pattern.
     */
    QString name() const
    {
        return mName;
    }

    /**
     * Sets the name of the search pattern. KMFilter uses this to
     * store it's own name, too.
     */
    void setName(const QString &newName)
    {
        mName = newName;
    }

    /**
     * Returns the filter operator.
     */
    SearchPattern::Operator op() const
    {
        return mOperator;
    }

    /**
     * Sets the filter operator.
     */
    void setOp(SearchPattern::Operator aOp)
    {
        mOperator = aOp;
    }

    static int filterRulesMaximumSize();
    /**
     * Returns the pattern as string. For debugging.
     */
    QString asString() const;

    /**
     * Returns the pattern as akonadi query
     */
    SparqlQueryError asAkonadiQuery(Akonadi::SearchQuery &) const;

    /**
     * Overloaded assignment operator. Makes a deep copy.
     */
    const SearchPattern &operator=(const SearchPattern &aPattern);

    /**
     * Writes the pattern into a byte array for persistance purposes.
     */
    QByteArray serialize() const;

    /**
     * Constructs the pattern from a byte array serialization.
     */
    void deserialize(const QByteArray &);

    QDataStream &operator>>(QDataStream &s) const;
    QDataStream &operator<<(QDataStream &s);

    void generateSieveScript(QStringList &requires, QString &code);

private:
    /**
     * Tries to import a legacy search pattern, ie. one that still has
     * e.g. the @p unless or @p ignore operator which were useful as long as
     * the number of rules was restricted to two. This method is called from
     * readConfig, which detects legacy configurations and also makes sure
     * that this method is called from an initialized object.
     */
    void importLegacyConfig(const KConfigGroup &config);

    /**
     * Initializes the object. Clears the list of rules, sets the name
     * to "<i18n("unnamed")>", and the boolean operator to @p OpAnd.
     */
    void init();
    QString  mName;
    Operator mOperator;
};

}

Q_DECLARE_METATYPE(MailCommon::SearchRule::RequiredPart)

#endif /* MAILCOMMON_SEARCHPATTERN_H_ */
