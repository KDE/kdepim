/* -*- mode: C++; c-file-style: "gnu" -*-
 *
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef MAILCOMMON_MAILFILTER_H
#define MAILCOMMON_MAILFILTER_H

#include "mailcommon_export.h"
#include "filteraction.h"
#include "searchpattern.h"

#include <Akonadi/Collection>

#include <KShortcut>

class KConfigGroup;

namespace MailCommon {

// maximum number of filter actions per filter
const int FILTER_MAX_ACTIONS = 8;

class MAILCOMMON_EXPORT MailFilter
{
  friend MAILCOMMON_EXPORT QDataStream &operator<<( QDataStream &stream, const MailFilter &filter );
  friend MAILCOMMON_EXPORT QDataStream &operator>>( QDataStream &stream, MailFilter &filter );

  public:
    /**
     * Result codes returned by process.
     */
    enum ReturnCode {
      NoResult,     //< For internal use only!
      GoOn,         //< Everything is OK. You are still the owner of the
                    //  message and you should continue applying filter
                    //  actions to this message.
      CriticalError //<  A critical error occurred (e.g. "disk full").
    };

    /**
     * Account type codes used by setApplicability.
     */
    enum AccountType {
        All,        //< Apply to all accounts
        ButImap,    //< Apply to all but IMAP accounts
        Checked     //< Apply to all accounts specified by setApplyOnAccount
    };

    /**
     * Constructor that initializes basic settings.
     */
    MailFilter();

    /**
     * Constructor that initializes from given config group. Filters are
     * stored one by one in config groups, i.e. one filter, one group.
     */
    explicit MailFilter( const KConfigGroup &aConfig, bool internal, bool &needUpdate );

    /**
     * Copy constructor. Constructs a deep copy of @p aFilter.
     */
    MailFilter( const MailFilter &other );

    /**
     * Destructor.
     */
    ~MailFilter();

    /**
     * Returns the unique identifier of this filter.
     */
    QString identifier() const;

    /**
     * Returns the file name.
     * Equivalent to @pattern()->name().
     */
    QString name() const;

    /**
     * Execute the filter action(s) on the given message.
     * Returns:
     *  @li 2 if a critical error occurred,
     *  @li 1 if the caller is still the owner of the message,
     *  @li 0 if processed successfully.
     *
     * @param context The context that contains the item to which the actions should be applied.
     * @param stopIt Contains true if the caller may apply other filters
     *               and false if he shallstop the filtering of this message.
     */
    ReturnCode execActions( ItemContext &context, bool &stopIt ) const;

    /**
     * Determines if the filter depends on the body of the message.
     */
    bool requiresBody();

    /**
     * Writes contents to the specified config group.
     */
    void writeConfig( KConfigGroup &config, bool exportFilter ) const;

    /**
     * Initializes from the given specified group.
     */
    bool readConfig( const KConfigGroup &config, bool interactive = false );

    /**
     * Removes empty rules (and actions one day).
     */
    void purify();

    /**
     * Checks for empty pattern and action list.
     */
    bool isEmpty() const;

    /**
     * Provides a reference to the internal action list. If you used
     * the @p setAction() and @p action() functions before, please
     * convert to using myFilter->actions()->at() and friends now.
     */
    QList<FilterAction*> *actions();

    /**
     * Provides a reference to the internal action list. Const version.
     */
    const QList<FilterAction*> *actions() const;

    /**
     * Provides a reference to the internal pattern. If you used the
     * @p matches() function before, please convert to using
     * myFilter->pattern()->matches() now.
     */
    SearchPattern *pattern();

    /**
     * Provides a reference to the internal pattern. If you used the
     *  @p matches() function before, please convert to using
     * myFilter->pattern()->matches() now.
     */
    const SearchPattern *pattern() const;

    /**
     * Sets whether this filter should be applied on
     * outbound messages (@p aApply == true) or not.
     * @see applyOnOutbound applyOnInbound setApplyOnInbound
     */
    void setApplyOnOutbound( bool aApply = true );

    /**
     * Set whether this filter should be applied on
     * outbound messages before sending (@p aApply == TRUE) or not.
     * @see applyOnOutbound applyOnInbound setApplyOnInbound
     */
    void setApplyBeforeOutbound( bool aApply = true );

    /**
     * Returns true if this filter should be applied on
     *      outbound messages, false otherwise.
     *  @see setApplyOnOutbound applyOnInbound setApplyOnInbound
     */
    bool applyOnOutbound() const;

    /**
     * Returns true if this filter should be applied on
     * outbound messages before they are sent, FALSE otherwise.
     * @see setApplyOnOutbound applyOnInbound setApplyOnInbound
     */
    bool applyBeforeOutbound() const;

    /**
     * Sets whether this filter should be applied on
     * inbound messages (@p aApply == true) or not.
     * @see setApplyOnOutbound applyOnInbound applyOnOutbound
     */
    void setApplyOnInbound( bool aApply = true );

    /**
     * Returns true if this filter should be applied on
     * inbound messages, false otherwise.
     * @see setApplyOnOutbound applyOnOutbound setApplyOnInbound
     */
    bool applyOnInbound() const;

    /**
     * Sets whether this filter should be applied on
     * explicit (CTRL-J) filtering (@p aApply == true) or not.
     * @see setApplyOnOutbound applyOnInbound applyOnOutbound
     */
    void setApplyOnExplicit( bool aApply = true );

    /**
     * Returns true if this filter should be applied on
     * explicit (CTRL-J) filtering, false otherwise.
     * @see setApplyOnOutbound applyOnOutbound setApplyOnInbound
     */
    bool applyOnExplicit() const;

    /**
     * Sets whether this filter should be applied on
     * inbound messages for all accounts (@p aApply == All) or
     * inbound messages for all but IMAP accounts (@p aApply == ButImap) or
     * for a specified set of accounts only.
     * Only applicable to filters that are applied on inbound messages.
     * @see setApplyOnInbound setApplyOnAccount
     */
    void setApplicability( AccountType aApply = All );

    /**
     * Returns true if this filter should be applied on
     * inbound messages for all accounts, or false if this filter
     * is to be applied on a specified set of accounts only.
     * Only applicable to filters that are applied on inbound messages.
     * @see setApplicability
     */
    AccountType applicability() const;

    /**
     * Sets whether this filter should be applied on
     * inbound messages for the account with id (@p id).
     * Only applicable to filters that are only applied to a specified
     * set of accounts.
     * @see setApplicability applyOnAccount
     */
    void setApplyOnAccount( const QString &id, bool aApply = true );

    /**
     * Returns true if this filter should be applied on
     * inbound messages from the account with id (@p id), false otherwise.
     * @see setApplicability
     */
    bool applyOnAccount( const QString &id ) const;

    void setStopProcessingHere( bool aStop );
    bool stopProcessingHere() const;

    /**
     * Sets whether this filter should be plugged into the filter menu.
     */
    void setConfigureShortcut( bool aShort );

    /**
     * Returns true if this filter should be plugged into the filter menu,
     * false otherwise.
     * @see setConfigureShortcut
     */
    bool configureShortcut() const;

    /**
     * Sets whether this filter should be plugged into the toolbar.
     * This can be done only if a shortcut is defined.
     * @see setConfigureShortcut
     */
    void setConfigureToolbar( bool aTool );

    /**
     * Returns true if this filter should be plugged into the toolbar,
     * false otherwise.
     * @see setConfigureToolbar
     */
    bool configureToolbar() const;

    /**
     * Returns the toolbar name of this filter.
     * @see setToolbarName
     */
    QString toolbarName() const;

    /**
     * Sets the toolbar name for this filter.
     * The toolbar name is the text to be displayed underneath the toolbar icon
     * for this filter. This is usually the same as name(),  expect when
     * explicitly set by this function.
     * This is useful if the normal filter mame is too long for the toolbar.
     * @see toolbarName, name
     */
    void setToolbarName( const QString &toolbarName );

    /**
     * Sets the shortcut to be used if plugged into the filter menu
     * or toolbar. Default is no shortcut.
     * @see setConfigureShortcut setConfigureToolbar
     */
    void setShortcut( const KShortcut &shortcut );

    /**
     * Returns the shortcut assigned to the filter.
     * @see setShortcut
     */
    const KShortcut &shortcut() const;

    /**
     * Sets the icon to be used if plugged into the filter menu
     * or toolbar. Default is the gear icon.
     * @see setConfigureShortcut setConfigureToolbar
     */
    void setIcon( const QString &icon );

    /**
     * Returns the name of the icon to be used.
     * @see setIcon
     */
    QString icon() const;

    /**
     * Tests if the folder aFolder is used in any action.
     * Changes it to aNewFolder folder in this case.
     * Called from the filter manager when a folder is moved.
     * @return true if a change in some action occurred,
     * false if no action was affected.
     */
    bool folderRemoved( const Akonadi::Collection &Folder, const Akonadi::Collection &aNewFolder );

    /**
     * Returns the filter in a human-readable form. useful for
     * debugging but not much else. Don't use, as it may well go away
     * in the future...
     */
#ifndef NDEBUG
    const QString asString() const;
#endif

    /**
     * Sets the mode for using automatic naming for the filter.
     * If the feature is enabled, the name is derived from the
     * first filter rule.
     */
    void setAutoNaming( bool useAutomaticNames );

    /**
     * Returns if an automatic name is used for the filter.
     */
    bool isAutoNaming() const;

    /**
     * Return if filter is enabled or not.
     */
    bool isEnabled() const;
    void setEnabled( bool );

  private:
    QString mIdentifier;
    SearchPattern mPattern;
    QList<FilterAction*> mActions;
    QStringList mAccounts;
    QString mIcon;
    QString mToolbarName;
    KShortcut mShortcut;
    bool bApplyOnInbound : 1;
    bool bApplyBeforeOutbound : 1;
    bool bApplyOnOutbound : 1;
    bool bApplyOnExplicit : 1;
    bool bStopProcessingHere : 1;
    bool bConfigureShortcut : 1;
    bool bConfigureToolbar : 1;
    bool bAutoNaming : 1;
    bool bEnabled : 1;
    AccountType mApplicability;
};

MAILCOMMON_EXPORT QDataStream &operator<<( QDataStream &stream, const MailFilter &filter );
MAILCOMMON_EXPORT QDataStream &operator>>( QDataStream &stream, MailFilter &filter );

}

#endif /*MAILCOMMON_MAILFILTER_H*/
