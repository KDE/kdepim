/* -*- mode: C++; c-file-style: "gnu" -*-

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

#include <KLocale>
#include <KUrl>

#include <QList>
#include <QString>

#include <boost/shared_ptr.hpp>
#include <akonadi/searchquery.h>

namespace Akonadi {
class Item;
}

namespace KMime {
class Message;
}

class KConfigGroup;

namespace MailCommon {

// maximum number of filter rules per filter
const int FILTER_MAX_RULES = 8;

/**
 * @short This class represents one search pattern rule.
 * Incoming mail is sent through the list of mail filter
 * rules before it is placed in the associated mail folder (usually "inbox").
 * This class represents one mail filter rule. It is also used to represent
 * a search rule as used by the search dialog and folders.
 */
class MAILCOMMON_EXPORT SearchRule
{
public:
    /**
     * Defines a pointer to a search rule.
     */
    typedef boost::shared_ptr<SearchRule> Ptr;

    /**
     * Describes operators for comparison of field and contents.
     *
     * If you change the order or contents of the enum: do not forget
     * to change funcConfigNames[], sFilterFuncList and matches()
     * in SearchRule, too.
     * Also, it is assumed that these functions come in pairs of logical
     * opposites (ie. "=" <-> "!=", ">" <-> "<=", etc.).
     */
    enum Function {
        FuncNone = -1,
        FuncContains = 0,
        FuncContainsNot,
        FuncEquals,
        FuncNotEqual,
        FuncRegExp,
        FuncNotRegExp,
        FuncIsGreater,
        FuncIsLessOrEqual,
        FuncIsLess,
        FuncIsGreaterOrEqual,
        FuncIsInAddressbook,
        FuncIsNotInAddressbook,
        FuncIsInCategory,
        FuncIsNotInCategory,
        FuncHasAttachment,
        FuncHasNoAttachment,
        FuncStartWith,
        FuncNotStartWith,
        FuncEndWith,
        FuncNotEndWith
    };

    enum RequiredPart {
        Envelope = 0,
        Header,
        CompleteMessage
    };

    /**
     * Creates new new search rule.
     *
     * @param field The field to search in.
     * @param function The function to use for searching.
     * @param contents The contents to search for.
     */
    explicit SearchRule ( const QByteArray &field = 0, Function function = FuncContains,
                          const QString &contents = QString() );

    /**
     * Creates a new search rule from an @p other rule.
     */
    SearchRule( const SearchRule &other );

    /**
     * Initializes this rule with an @p other rule.
     */
    const SearchRule &operator=( const SearchRule &other );

    /**
     * Creates a new search rule of a certain type by instantiating the
     * appropriate subclass depending on the @p field.
     *
     * @param field The field to search in.
     * @param function The function to use for searching.
     * @param contents The contents to search for.
     */
    static SearchRule::Ptr createInstance( const QByteArray &field = 0,
                                           Function function = FuncContains,
                                           const QString &contents = QString() );

    /**
     * Creates a new search rule of a certain type by instantiating the
     * appropriate subclass depending on the @p field.
     *
     * @param field The field to search in.
     * @param function The name of the function to use for searching.
     * @param contents The contents to search for.
     */
    static SearchRule::Ptr createInstance( const QByteArray &field,
                                           const char *function,
                                           const QString &contents );

    /**
     * Creates a new search rule by cloning an @p other rule.
     */
    static SearchRule::Ptr createInstance( const SearchRule &other );

    /**
     * Creates a new search rule by deseralizing its structure from a data @p stream.
     */
    static SearchRule::Ptr createInstance( QDataStream &stream );

    /**
     * Creates a new search rule from a given config @p group.
     *
     * @param group The config group to read the structure from.
     * @param index The identifier that is used to distinguish
     *              rules within a single config group.
     *
     * @note This function does no validation of the data obtained
     *       from the config file. You should call isEmpty yourself
     *       if you need valid rules.
     */
    static SearchRule::Ptr createInstanceFromConfig( const KConfigGroup &group, int index );

    /**
     * Destroys the search rule.
     */
    virtual ~SearchRule();

    /**
     * Tries to match the rule against the KMime::Message in the
     * given @p item.
     *
     * @return true if the rule matched, false otherwise.
     *
     * @note Must be implemented by subclasses.
     */
    virtual bool matches( const Akonadi::Item &item ) const = 0;

    /**
     * Determines whether the rule is worth considering.
     * It isn't if either the field is not set or the contents is empty.
     * The calling code should make sure that it's rule list contains
     * only non-empty rules, as matches doesn't check this.
     */
    virtual bool isEmpty() const = 0;

    /**
     * Returns the required part from the item that is needed for the search to
     * operate. See @ref RequiredPart */
    virtual SearchRule::RequiredPart requiredPart() const = 0;

    /**
     * Saves the object into a given config @p group.
     *
     * @param index The identifier that is used to distinguish
     *              rules within a single config group.
     *
     * @note This function will happily write itself even when it's
     *       not valid, assuming higher layers to Do The Right Thing(TM).
     */
    void writeConfig( KConfigGroup &group, int index ) const;

    void generateSieveScript(QStringList &requires, QString &code);

    /**
     * Sets the filter @p function of the rule.
     */
    void setFunction( Function function );

    /**
     * Returns the filter function of the rule.
     */
    Function function() const;

    /**
     * Sets the message header field @p name.
     *
     * @note Make sure the name contains no trailing ':'.
     */
    void setField( const QByteArray &name );

    /**
     * Returns the message header field name (without the trailing ':').
     *
     * There are also six pseudo-headers:
     * @li \<message\>: Try to match against the whole message.
     * @li \<body\>: Try to match against the body of the message.
     * @li \<any header\>: Try to match against any header field.
     * @li \<recipients\>: Try to match against both To: and Cc: header fields.
     * @li \<size\>: Try to match against size of message (numerical).
     * @li \<age in days\>: Try to match against age of message (numerical).
     * @li \<status\>: Try to match against status of message (status).
     * @li \<tag\>: Try to match against message tags.
     */
    QByteArray field() const;

    /**
     * Set the @p contents of the rule.
     *
     * This can be either a substring to search for in
     * or a regexp pattern to match against the header.
     */
    void setContents( const QString &contents );

    /**
     * Returns the contents of the rule.
     */
    QString contents() const;

    /**
     * Returns the rule as string for debugging purpose
     */
    const QString asString() const;

    /**
     * Adds query terms to the given term group.
     */
    virtual void addQueryTerms( Akonadi::SearchTerm &groupTerm, bool &emptyIsNotAnError ) const {  }


    QDataStream &operator>>( QDataStream & ) const;

protected:
    /**
     * Helper that returns whether the rule has a negated function.
     */
    bool isNegated() const;

    /**
     * Converts the rule function into the corresponding Akonadi query operator.
     */
    Akonadi::SearchTerm::Condition akonadiComparator() const;

protected:
    QString quote( const QString &content ) const;
private:
    static Function configValueToFunc( const char * );
    static QString functionToString( Function );
    QString conditionToString(Function function);

    QByteArray mField;
    Function mFunction;
    QString  mContents;
};

// Needed for MSVC 2010, as it seems to not implicit cast for a pointer anymore
#ifdef _MSC_VER
uint qHash( SearchRule::Ptr sr );
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
    explicit SearchPattern( const KConfigGroup &config );

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
    bool matches( const Akonadi::Item &item, bool ignoreBody = false ) const;

    /**
     * Returns the required part from the item that is needed for the search to
     * operate. See @ref RequiredPart */
    SearchRule::RequiredPart requiredPart() const;

    /**
     * Removes all empty rules from the list. You should call this
     * method whenever the user had had control of the rules outside of
     * this class. (e.g. after editing it with SearchPatternEdit).
     */
    void purify();

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
    void readConfig( const KConfigGroup &config );

    /**
     * Writes itself into @p config. Tries to delete old-style keys by
     * overwriting them with QString().
     *
     * Derived classes reimplementing writeConfig() should also call this
     * method, or else the rules will not be stored.
     */
    void writeConfig( KConfigGroup &config ) const;

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
    void setName( const QString &newName )
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
    void setOp( SearchPattern::Operator aOp )
    {
        mOperator = aOp;
    }

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
    const SearchPattern &operator=( const SearchPattern &aPattern );

    /**
     * Writes the pattern into a byte array for persistance purposes.
     */
    QByteArray serialize() const;

    /**
     * Constructs the pattern from a byte array serialization.
     */
    void deserialize( const QByteArray & );

    QDataStream &operator>>( QDataStream &s ) const;
    QDataStream &operator<<( QDataStream &s );


    void generateSieveScript(QStringList &requires, QString &code);

private:
    /**
     * Tries to import a legacy search pattern, ie. one that still has
     * e.g. the @p unless or @p ignore operator which were useful as long as
     * the number of rules was restricted to two. This method is called from
     * readConfig, which detects legacy configurations and also makes sure
     * that this method is called from an initialized object.
     */
    void importLegacyConfig( const KConfigGroup &config );

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
