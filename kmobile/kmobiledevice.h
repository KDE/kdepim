/*  This file is part of the KDE kmobile library.
    Copyright (C) 2003 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef LIB_KMOBILEDEVICE_H
#define LIB_KMOBILEDEVICE_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmutex.h>

#include <kdepimmacros.h>
#include <klibloader.h>

#include <kabc/addressee.h>
#include <kabc/addresseelist.h>

#include <kio/global.h>
#include <kio/authinfo.h>

class KConfig;

#define KMOBILE_MIMETYPE_DEVICE			"kdedevice/mobiledevice"
#define KMOBILE_MIMETYPE_DEVICE_KONQUEROR(name) QString("kdedevice/kmobile_%1").arg(name)
#define KMOBILE_MIMETYPE_INODE			"inode/"
#define KMOBILE_ICON_UNKNOWN			"mobile_unknown"

/**
 * @short Represents the base class for dynamically loaded mobile device drivers.
 *
 * KMobileDevice is the base class for all hardware device drivers.
 * Every derived class has to add additional functionality.
 *
 * For a KMobileSomeDevice driver you have to write the following code:
 * <pre>
 * K_EXPORT_COMPONENT_FACTORY( libkmobile_somedevice, KMobileSomeDevice() );
 * QObject *KMobileSomeDevice::createObject( QObject *parent, const char *name,
 *      const char *, const QStringList &args )
 * {
 *    return new KMobileSomeDevice( parent, name, args );
 * }
 * </pre>
 *
 * @see KLibFactory
 * @author Helge Deller <deller@kde.org>
 */

class KDE_EXPORT KMobileDevice : public KLibFactory
{
Q_OBJECT
    friend class KMobileView;

public:
    /**
     * Construct a new KMobileDevice.
     *
     * @param obj The parent object. This is usually 0.
     * @param name The object name. For session management and window management to work.
     * @param args Additional commandline parameters - the first entry has the config file name.
     */
    KMobileDevice(QObject *obj, const char *name, const QStringList &args );
    virtual ~KMobileDevice();


    /**
     * Connect to the device.
     *
     * @param parent The parent widget. It will be used as parent for message boxes.
     */
    virtual bool connectDevice( QWidget *parent = 0 ) = 0;

    /**
     * Disconnect from the device.
     *
     * @param parent The parent widget. It will be used as parent for message boxes.
     */
    virtual bool disconnectDevice( QWidget *parent = 0 ) = 0;

    /**
     * Returns true, if the device is currently connected and the link is online.
     */
    virtual bool connected();

    /**
     * Returns the classname, to which the device belongs. Examples are e.g.
     *  "Nokia mobile phone", "MP3 Player", "Handspring Organizer"
     */
    virtual QString deviceClassName() const;

    /**
     * Returns the real devices name, e.g. "Nokia 6310" or "Rio MP3 Player"
     */
    virtual QString deviceName() const;

    /**
     * Returns the hardware revision of the devices, e.g. "Revision 1.2"
     */
    virtual QString revision() const;

    /**
     * Returns an unique ID for the device, e.g. IMEI number on phones, or serial number.
     * The returned String is used to have a unique identification for syncronisation.
     */
    virtual QString deviceUniqueID() = 0;

    /**
     * Returns true, if the device is connected via a slow connection.
     * Good examples for slow connections are serial or infrared ports.
     */
    virtual bool isSlowDevice() const;

    /**
     * Returns true, if this is a read-only device.
     */
    virtual bool isReadOnly() const;

    /**
     * Pop-up a device-specific configuration dialog.
     *
     * @param parent The parent widget. It will be used as parent for the configuration dialog.
     */
    virtual bool configDialog(QWidget *parent);

    // The ClassType may be used e.g. to select an suitable icon
    enum ClassType {
	Unclassified = 0,
	Phone        = 1,
        Organizer    = 2,
        Camera       = 3,
	MusicPlayer  = 4, // e.g. MP3Players, CDPlayers
        LastClassType = MusicPlayer
    };
    enum ClassType classType() const;

    // you may provide your own icon() implementation to display
    // an appropriate Pixmap (e.g. a Palm Pilot or a Zaurus image).
    virtual QString iconFileName() const;

    // the default Icon set
    static QString defaultIconFileName( ClassType ct = Unclassified );
    static QString defaultClassName( ClassType ct = Unclassified );

    // The capabilities of this device (bitmapped value)
    enum Capabilities {
	hasNothing     = 0,	// not supported
	hasAddressBook = 1,	// mobile phones, organizers, ...
	hasCalendar    = 2,	// organizers, mobile phones, ...
	hasNotes       = 4,	// organizers, mobile phones, ...
	hasFileStorage = 8,	// organizers, handhelds, mp3-player, ...
	hasAnyCapability = 0xffff // used to select devices independent of the capatibilities
    };
    int capabilities() const;
    const QString nameForCap(int cap) const;

    // returns an error string for the given error code
    // @see KIO::buildErrorString()
    QString buildErrorString(KIO::Error err, const QString &errorText) const;

public:
    /*
     * Addressbook / Phonebook support
     */
    virtual int numAddresses();
    virtual int readAddress( int index, KABC::Addressee &adr );
    virtual int storeAddress( int index, const KABC::Addressee &adr, bool append = false );

    /*
     * Calendar support
     */
    // TODO: TBD
    virtual int numCalendarEntries();
//    virtual int readCalendarEntry( int index, <type> &entry );
//    virtual int storeCalendarEntry( int index, <type> &entry );

    /*
     * Notes support
     */
    virtual int numNotes();
    virtual int readNote( int index, QString &note );
    virtual int storeNote( int index, const QString &note );



    /*
     **********************
     * FILE STORAGE SUPPORT
     **********************
     * mostly compatible to the kioslave base class <kio/slavebase.h>
     */

    /**
     * helper functions for the kmobile device drivers
     */
    void createDirEntry(KIO::UDSEntry& entry, const QString& name,
		const QString& url, const QString& mime) const;
    void createFileEntry(KIO::UDSEntry& entry, const QString& name,
		const QString& url, const QString& mime,
		const unsigned long size = 0) const;
    /**
     * Lists the contents of @p path.
     * The slave should emit ERR_CANNOT_ENTER_DIRECTORY if it doesn't exist,
     * if we don't have enough permissions, or if it is a file
     * It should also emit @ref #totalFiles as soon as it knows how many
     * files it will list.
     */
    virtual void listDir( const QString &url );

    /**
     * Create a directory
     * @param path path to the directory to create
     * @param permissions the permissions to set after creating the directory
     * (-1 if no permissions to be set)
     * The slave emits ERR_COULD_NOT_MKDIR if failure.
     */
    virtual void mkdir( const QString &url, int permissions );

    /**
     * Rename @p oldname into @p newname.
     * If the slave returns an error ERR_UNSUPPORTED_ACTION, the job will
     * ask for copy + del instead.
     * @param src where to move the file from
     * @param dest where to move the file to
     * @param overwrite if true, any existing file will be overwritten
     */
    virtual void rename( const QString &src, const QString &dest, bool overwrite );

    /**
     * Creates a symbolic link named @p dest, pointing to @p target, which
     * may be a relative or an absolute path.
     * @param target The string that will become the "target" of the link (can be relative)
     * @param dest The symlink to create.
     * @param overwrite whether to automatically overwrite if the dest exists
     */
    virtual void symlink( const QString &target, const QString &dest, bool overwrite );

    /**
     * Delete a file or directory.
     * @param path file/directory to delete
     * @param isfile if true, a file should be deleted.
     *               if false, a directory should be deleted.
     */
    virtual void del( const QString &url, bool isfile);

    /**
     * Finds all details for one file or directory.
     * The information returned is the same as what @ref #listDir returns,
     * but only for one file or directory.
     */
    virtual void stat( const QString &url );

    /**
     * Change permissions on @p path
     * The slave emits ERR_DOES_NOT_EXIST or ERR_CANNOT_CHMOD
     */
    virtual void chmod( const QString &url, int permissions );

    /**
     * get, aka read.
     * @param url the full url for this request. Host, port and user of the URL
     *        can be assumed to be the same as in the last setHost() call.
     * The slave emits the data through @ref #data
     */
    virtual void get( const QString &url );

    /**
     * put, aka write.
     * @param path where to write the file (decoded)
     * @param permissions may be -1. In this case no special permission mode is set.
     * @param overwrite if true, any existing file will be overwritten.
     * If the file indeed already exists, the slave should NOT apply the
     * permissions change to it.
     * @param resume
     */
    virtual void put( const QString &url, int permissions, bool overwrite, bool resume );

    /**
     * Finds mimetype for one file or directory.
     *
     * This method should either emit 'mimeType' or it
     * should send a block of data big enough to be able
     * to determine the mimetype.
     *
     * If the slave doesn't reimplement it, a @ref #get will
     * be issued, i.e. the whole file will be downloaded before
     * determining the mimetype on it - this is obviously not a
     * good thing in most cases.
     */
    virtual void mimetype( const QString &url );

    /**
     * Used for any command that is specific to this slave (protocol)
     * Examples are : HTTP POST, mount and unmount (kio_file)
     *
     * @param data packed data; the meaning is completely dependent on the
     *        slave, but usually starts with an int for the command number.
     * Document your slave's commands, at least in its header file.
     */
    virtual void special( const QByteArray & );

signals:
    /**
     * Call this from stat() to express details about an object, the
     * UDSEntry customarily contains the atoms describing file name, size,
     * mimetype, etc.
     * @param _entry The UDSEntry containing all of the object attributes.
     */
    void statEntry( const KIO::UDSEntry &_entry );

    /**
     * internal function to be called by the slave.
     * It collects entries and emits them via listEntries
     * when enough of them are there or a certain time
     * frame exceeded (to make sure the app gets some
     * items in time but not too many items one by one
     * as this will cause a drastic performance penalty)
     * @param ready set to true after emitting all items. _entry is not
     *        used in this case
     */
    void listEntry( const KIO::UDSEntry& _entry, bool ready);

    /**
     * Internal function to transmit meta data to the application.
     */
    void sendMetaData();

    /**
     * Prompt the user for Authorization info (login & password).
     *
     * Use this function to request authorization information from
     * the end user. You can also pass an error message which explains
     * why a previous authorization attempt failed. Here is a very
     * simple example:
     *
     * <pre>
     * KIO::AuthInfo authInfo;
     * if ( openPassDlg( authInfo ) )
     * {
     *    kdDebug() << QString::fromLatin1("User: ")
     *              << authInfo.username << endl;
     *    kdDebug() << QString::fromLatin1("Password: ")
     *              << QString::fromLatin1("Not displayed here!") << endl;
     * }
     * </pre>
     *
     * You can also preset some values like the username, caption or
     * comment as follows:
     *
     * <pre>
     * KIO::AuthInfo authInfo;
     * authInfo.caption= "Acme Password Dialog";
     * authInfo.username= "Wile E. Coyote";
     * QString errorMsg = "You entered an incorrect password.";
     * if ( openPassDlg( authInfo, errorMsg ) )
     * {
     *    kdDebug() << QString::fromLatin1("User: ")
     *              << authInfo.username << endl;
     *    kdDebug() << QString::fromLatin1("Password: ")
     *              << QString::fromLatin1("Not displayed here!") << endl;
     * }
     * </pre>
     *
     * NOTE: A call to this function can fail and return @p false,
     * if the UIServer could not be started for whatever reason.
     *
     * @param info  See @ref AuthInfo.
     * @param errorMsg Error message to show
     * @return      @p TRUE if user clicks on "OK", @p FALSE otherwsie.
     * @since 3.1
     */
    bool openPassDlg( KIO::AuthInfo& info, const QString &errorMsg );

    /**
     * Call this in @ref #mimetype, when you know the mimetype.
     * See @ref #mimetype about other ways to implement it.
     */
    void mimeType( const QString &_type );

    /**
     * Call to signal an error.
     * This also finishes the job, no need to call finished.
     *
     * If the Error code is KIO::ERR_SLAVE_DEFINED then the
     * _text should contain the complete translated text of
     * of the error message.  This message will be displayed
     * in an KTextBrowser which allows rich text complete
     * with hyper links.  Email links will call the default
     * mailer, "exec:/command arg1 arg2" will be forked and
     * all other links will call the default browser.
     *
     * @see KIO::Error
     * @see KTextBrowser
     * @param _errid the error code from KIO::Error
     * @param _text the rich text error message
     */
    void error( int _errid, const QString &_text );

    /**
     * Call to signal a warning, to be displayed in a dialog box.
     */
    void warning( const QString &msg );

    /**
     * Call to signal a message, to be displayed if the application wants to,
     * for instance in a status bar. Usual examples are "connecting to host xyz", etc.
     */
    void infoMessage( const QString &msg );

    /**
     * Call to signal successful completion of any command
     * (besides openConnection and closeConnection)
     */
    void finished();


    enum MessageBoxType { QuestionYesNo = 1, WarningYesNo = 2, WarningContinueCancel = 3,
		WarningYesNoCancel = 4, Information = 5, SSLMessageBox = 6 };

    /**
     * Call this to show a message box from the slave (it will in fact be handled
     * by kio_uiserver, so that the progress info dialog for the slave is hidden
     * while this message box is shown)
     * @param type type of message box: QuestionYesNo, WarningYesNo, WarningContinueCancel...
     * @param text Message string. May contain newlines.
     * @param caption Message box title.
     * @param buttonYes The text for the first button.
     *                  The default is i18n("&Yes").
     * @param buttonNo  The text for the second button.
     *                  The default is i18n("&No").
     * Note: for ContinueCancel, buttonYes is the continue button and buttonNo is unused.
     *       and for Information, none is used.
     * @return a button code, as defined in KMessageBox, or 0 on communication error.
     */
    int messageBox( MessageBoxType type, const QString &text,
                    const QString &caption = QString::null,
                    const QString &buttonYes = QString::null,
                    const QString &buttonNo = QString::null );

    /**
     * Call this in @ref #get and @ref #copy, to give the total size
     * of the file
     * Call in @ref listDir too, when you know the total number of items.
     */
    void totalSize( KIO::filesize_t _bytes );
    /**
     * Call this during @ref #get and @ref #copy, once in a while,
     * to give some info about the current state.
     * Don't emit it in @ref #listDir, @ref #listEntries speaks for itself.
     */
    void processedSize( KIO::filesize_t _bytes );


signals:
    void connectionChanged( bool conn_established );

protected:
    // only available to sub-classed device drivers:
    void setClassType( enum ClassType ct );
    void setCapabilities( int caps );
    KConfig *config() const { return m_config; };
    QString configFileName() const { return m_configFileName; };


    /**
     * Lock/Unlock serial ports and other devices
     * @param device Name of a device port (e.g. /dev/ttyS1, ttyS1, /dev/ircomm0)
     * Returns true, if device could be locked or unlocked
     */
    bool lockDevice(const QString &device, QString &err_reason);
    bool unlockDevice(const QString &device);

protected:
    QMutex  m_mutex;		// mutex to syncronize DCOP accesses to this device
    QString m_configFileName;
    KConfig *m_config;		// this is where this device should store it's configuration
    enum ClassType m_classType;
    QString m_deviceClassName;	// e.g. "Nokia mobile phone", "MP3 Player", "Handspring Organizer"
    QString m_deviceName;	// e.g. "Nokia 6310", "Opie"
    QString m_deviceRevision;	// e.g. "Revision 1.2" or "n/a"
    QString m_connectionName;	// e.g. "IRDA", "USB", "Cable", "gnokii", "gammu", ...
    int m_caps;			// @see enum Capabilities
    bool m_connected;

private:
    class KMobileDevicePrivate *d;
};

#endif	/* LIB_KMOBILEDEVICE_H */

