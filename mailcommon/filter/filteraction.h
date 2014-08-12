/*
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

#ifndef MAILCOMMON_FILTERACTION_H
#define MAILCOMMON_FILTERACTION_H

#include "mailcommon_export.h"
#include "../search/searchpattern.h"

#include "itemcontext.h"

#include <Collection>
#include <Item>

#include <KMime/MDN>
#include <KMime/KMimeMessage>

#include <QtCore/QList>
#include <QtCore/QObject>

class QWidget;

namespace MailCommon {

/**
 * @short Abstract base class for mail filter actions.
 *
 * Abstract base class for mail filter actions. All it can do is
 * hold a name (ie. type-string). There are several sub-classes that
 * inherit form this and are capable of providing parameter handling
 * (import/export as string, a widget to allow editing, etc.)
 *
 * @author Marc Mutz <mutz@kde.org>, based on work by Stefan Taferner <taferner@kde.org>.
 * @see Filter FilterMgr
 */
class MAILCOMMON_EXPORT FilterAction : public QObject
{
    Q_OBJECT
public:

    /**
     * Describes the possible return codes of filter processing:
     */
    enum ReturnCode {
        ErrorNeedComplete = 0x1, ///< Could not process because a complete message is needed.
        GoOn = 0x2,              ///< Go on with applying filter actions.
        ErrorButGoOn = 0x4,      ///< A non-critical error occurred
        ///  (e.g. an invalid address in the 'forward' action),
        ///   but processing should continue.
        CriticalError = 0x8      ///< A critical error occurred during processing
        ///  (e.g. "disk full").
    };


    /**
     * Creates a new filter action.
     *
     * The action is initialized with an identifier @p name and
     * an i18n'd @p label.
     */
    FilterAction( const QString &name, const QString &label, QObject *parent = 0 );

    /**
     * Destroys the filter action.
     */
    virtual ~FilterAction();

    /**
     * Returns i18n'd label, ie. the one which is presented in
     * the filter dialog.
     */
    QString label() const;

    /**
     * Returns identifier name, ie. the one under which it is
     * known in the config.
     */
    QString name() const;

    virtual QStringList sieveRequires() const;

    virtual QString sieveCode() const;

    /**
     * Execute action on given message (inside the item context).
     * Returns @p CriticalError if a
     * critical error has occurred (eg. disk full), @p ErrorButGoOn if
     * there was a non-critical error (e.g. invalid address in
     * 'forward' action), @p ErrorNeedComplete if a complete message
     * is required, @p GoOn if the message shall be processed by
     * further filters and @p Ok otherwise.
     */
    virtual ReturnCode process( ItemContext &context, bool applyOnOutbound ) const = 0;

    /**
     * Returns the required part from the item that is needed for the action to
     * operate. See @ref SearchRule::RequiredPart */
    virtual SearchRule::RequiredPart requiredPart() const = 0;
    /**
     * Determines whether this action is valid. But this is just a
     * quick test. Eg., actions that have a mail address as parameter
     * shouldn't try real address validation, but only check if the
     * string representation is empty.
     */
    virtual bool isEmpty() const;

    /**
     * Creates a widget for setting the filter action parameter. Also
     * sets the value of the widget.
     */
    virtual QWidget *createParamWidget( QWidget *parent ) const;

    /**
     * The filter action shall set it's parameter from the widget's
     * contents. It is allowed that the value is read by the action
     * before this function is called.
     */
    virtual void applyParamWidgetValue( QWidget *paramWidget );

    /**
     * The filter action shall set it's widget's contents from it's
     * parameter.
     */
    virtual void setParamWidgetValue( QWidget *paramWidget ) const;

    /**
     * The filter action shall clear it's parameter widget's
     * contents.
     */
    virtual void clearParamWidget( QWidget *paramWidget ) const;

    /**
     * Read extra arguments from given string.
     */
    virtual void argsFromString( const QString &argsStr ) = 0;

    /**
     * Read extra arguments from given string.
     * Return true if we need to update config file
     */
    virtual bool argsFromStringInteractive( const QString &argsStr, const QString &filterName );

    virtual QString argsAsStringReal() const;

    /**
     * Return extra arguments as string. Must not contain newlines.
     */
    virtual QString argsAsString() const = 0;

    /**
     * Returns a translated string describing this filter for visualization
     * purposes, e.g. in the filter log.
     */
    virtual QString displayString() const = 0;

    /**
     * Called from the filter when a folder is removed. Tests if the
     * folder @p aFolder is used and changes to @p aNewFolder in this
     * case. Returns true if a change was made.
     */
    virtual bool folderRemoved( const Akonadi::Collection &aFolder,
                                const Akonadi::Collection &aNewFolder );

    /**
     * Static function that creates a filter action of this type.
     */
    static FilterAction *newAction();

    /**
     * Automates the sending of MDNs from filter actions.
     */
    static void sendMDN( const Akonadi::Item &item,
                         KMime::MDN::DispositionType d,
                         const QList<KMime::MDN::DispositionModifier> &m =
            QList<KMime::MDN::DispositionModifier>() );

signals:
    /**
     * Called to notify that the current FilterAction has had some
     * value modification
     */
    void filterActionModified();

private:
    QString mName;
    QString mLabel;
};

}

#endif
