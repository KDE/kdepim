/*
  Copyright (c) 2003 Andreas Gungl <a.gungl@gmx.de>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#ifndef MAILCOMMON_FILTERLOG_H
#define MAILCOMMON_FILTERLOG_H

#include "mailcommon_export.h"

#include <QObject>
#include <QStringList>

namespace MailCommon
{

/**
 * @short KMail Filter Log Collector.
 *
 * The filter log helps to collect log information about the
 * filter process in KMail. It's implemented as singleton,
 * so it's easy to direct pieces of information to a unique
 * instance.
 * It's possible to activate / deactivate logging. All
 * collected log information can get thrown away, the
 * next added log entry is the first one until another
 * clearing.
 * A signal is emitted whenever a new logentry is added,
 * when the log was cleared or any log state was changed.
 *
 * @author Andreas Gungl <a.gungl@gmx.de>
 */
class MAILCOMMON_EXPORT FilterLog : public QObject
{
    Q_OBJECT

public:
    /**
     * Destroys the filter log.
     */
    virtual ~FilterLog();

    /**
     * Returns the single global instance of the filter log.
     */
    static FilterLog *instance();

    /**
     * Describes the type of content that will be logged.
     */
    enum ContentType {
        Meta               = 1, ///< Log all meta data.
        PatternDescription = 2, ///< Log all pattern description.
        RuleResult         = 4, ///< Log all rule matching results.
        PatternResult      = 8, ///< Log all pattern matching results.
        AppliedAction      = 16 ///< Log all applied actions.
    };

    /**
     * Sets whether the filter log is currently @p active.
     */
    void setLogging(bool active);

    /**
     * Returns whether the filter log is currently active.
     */
    bool isLogging() const;

    /**
     * Sets the maximum @p size of the log in bytes.
     */
    void setMaxLogSize(long size = -1);

    /**
     * Returns the maximum size of the log in bytes.
     */
    long maxLogSize() const;

    /**
     * Sets whether a given content @p type will be @p enabled for logging.
     */
    void setContentTypeEnabled(ContentType type, bool enabled);

    /**
     * Returns whether the given content @p type is enabled for logging.
     */
    bool isContentTypeEnabled(ContentType type) const;

    /**
     * Adds the given log @p entry under the given content @p type to the log.
     */
    void add(const QString &entry, ContentType type);

    /**
     * Adds a separator line to the log.
     */
    void addSeparator();

    /**
     * Clears the log.
     */
    void clear();

    /**
     * Returns the list of log entries.
     */
    QStringList logEntries() const;

    /**
     * Saves the log to the file with the given @p fileName.
     *
     * @return @c true on success or @c false on failure.
     */
    bool saveToFile(const QString &fileName) const;

    /**
     * Returns an escaped version of the log which can be used
     * in a HTML document.
     */
    static QString recode(const QString &plain);

    /**
     * Dumps the log to console. Used for debugging.
     */
    void dump();

Q_SIGNALS:
    /**
     * This signal is emitted whenever a new @p entry has been added to the log.
     */
    void logEntryAdded(const QString &entry);

    /**
     * This signal is emitted whenever the log has shrinked.
     */
    void logShrinked();

    /**
     * This signal is emitted whenever the activity of the filter log has been changed.
     */
    void logStateChanged();

private:
    //@cond PRIVATE
    FilterLog();

    class Private;
    Private *const d;
    //@endcond
};

}

#endif
