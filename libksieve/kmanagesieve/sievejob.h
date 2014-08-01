/*  -*- c++ -*-
    sievejob.h

    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KSIEVE_KMANAGESIEVE_SIEVEJOB_H
#define KSIEVE_KMANAGESIEVE_SIEVEJOB_H

#include "kmanagesieve_export.h"

#include <QtCore/QObject>
#include <QtCore/QStack>
#include <QtCore/QStringList>

#include <QUrl>
#include <kio/global.h>
#include <kio/udsentry.h>

namespace KIO {
class Job;
}

class KJob;

namespace KManageSieve {

class Session;

/**
 * @short A job to manage sieve scripts.
 *
 * This class provides functionality to manage sieve scripts
 * on an IMAP server.
 */
class KMANAGESIEVE_EXPORT SieveJob : public QObject
{
    Q_OBJECT

public:
    /**
     * Stores a sieve script on an IMAP server.
     *
     * @param destination The sieve URL that describes the destination.
     * @param script The raw sieve script.
     * @param makeActive If @c true, the script will be marked as active.
     * @param wasActive If @c true, the script will be marked as inactive.
     */
    static SieveJob* put( const QUrl &destination, const QString &script,
                          bool makeActive, bool wasActive );

    /**
     * Gets a sieve script from an IMAP server.
     *
     * @param source The sieve URL that describes the source.
     */
    static SieveJob* get( const QUrl &source );

    /**
     * Lists all available scripts at the given sieve @p url.
     */
    static SieveJob* list( const QUrl &url );

    /**
     * Deletes the script with the given sieve @p url.
     */
    static SieveJob* del( const QUrl &url );

    /**
     * Activates the script with the given sieve @p url.
     */
    static SieveJob* activate( const QUrl &url );

    /**
     * Deactivates the script with the given sieve @p url.
     */
    static SieveJob* deactivate( const QUrl &url );

    /**
     * Kills the sieve job.
     */
    void kill( KJob::KillVerbosity verbosity = KJob::Quietly );

    /**
     * Sets whether the sieve job shall be interactive.
     */
    void setInteractive( bool interactive );

    /**
     * Returns the sieve capabilities of the IMAP server.
     */
    QStringList sieveCapabilities() const;

    /**
     * Returns whether the requested sieve script exists on
     * the IMAP server.
     */
    bool fileExists() const;

Q_SIGNALS:
    /**
     * This signal is emitted when a get job has finished.
     *
     * @param job The job that has finished
     * @param success Whether the job was successfully.
     * @param script The downloaded sieve script.
     * @param active Whether the script is active on the server.
     */
    void gotScript( KManageSieve::SieveJob *job, bool success,
                    const QString &script, bool active );

    /**
     * This signal is emitted when a list job has finished.
     *
     * @param job The job that has finished.
     * @param success Whether the job was successfully.
     * @param scriptList The list of script filenames on the server.
     * @param activeScript The filename of the active script, or an
     *                     empty string if no script is active.
     */
    void gotList( KManageSieve::SieveJob *job, bool success,
                  const QStringList &scriptList, const QString &activeScript );

    /**
     * This signal is emitted for all kind of jobs when they have finished.
     *
     * @param job The job that has finished.
     * @param success Whether the job was successfully.
     * @param script The script the action was about.
     * @param active The filename of the active script, or an
     * @param active Whether the script is active on the server.
     */
    void result( KManageSieve::SieveJob *job, bool success,
                 const QString &script, bool active);

    void errorMessage( KManageSieve::SieveJob *job, bool success,
                       const QString& errMsg );
    /**
     * This signal is emitted for each result entry of a list job.
     *
     * @param job The job the result belongs to.
     * @param filename The filename of the sieve script on the server.
     * @param active Whether the script is active on the server.
     */
    void item( KManageSieve::SieveJob *job, const QString &filename, bool active );

private:
    //@cond PRIVATE
    SieveJob( QObject *parent = 0 );
    ~SieveJob();

    class Private;
    Private* const d;
    friend class Session;
    //@endcond
};

}

#endif
