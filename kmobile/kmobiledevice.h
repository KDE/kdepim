/* This file is part of the KDE kmobile library.
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

#include <klibloader.h>
#include <kconfig.h>

#include <kabc/addressee.h>
#include <kabc/addresseelist.h>

#include <kio/global.h>

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
 *    return new KMobileSkeleton( parent, name, args );
 * }
 * </pre>
 *
 * @see KLibFactory
 * @author Helge Deller <deller@kde.org>
 */

class KMobileDevice : public KLibFactory
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
    virtual bool connectDevice( QWidget *parent = 0 ) = 0L;

    /**
     * Disconnect from the device.
     *
     * @param parent The parent widget. It will be used as parent for message boxes.
     */
    virtual bool disconnectDevice( QWidget *parent = 0 ) = 0L;

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
    // an appropiate Pixmap (e.g. a Palm Pilot or a Zaurus image).
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
	hasAnyCapability = 0xffff // used to select devices independend of the capatibilities
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
     * File storage support
     * @param fileName  path and name of a file in the mobile device, e.g. "/MYFILE.TXT", "/mp3/song1.mp3"
     */
    virtual int listDir( const QString &fileName );
    virtual int stat( const QString &pathName, KIO::UDSEntry &entry );
    virtual int deleteFile( const QString &fileName );
    virtual int readFile( const QString &fileName, QByteArray &content );
    virtual int storeFile( const QString &fileName, const QByteArray &content, const KIO::UDSEntry &entry );
    virtual int mkDir( const QString &fileName, const KIO::UDSEntry &entry );
    virtual int rmDir( const QString &fileName );


    // debugging levels
    enum MsgLevel { info=0, warning=1, error=2 };

signals:
    virtual void connectionChanged( bool conn_established );
    virtual void message( int msgLevel, const QString &msg );

private slots:
    // called whenever the connection status changes
    void slotConnectionChanged( bool connected );

    void slotMessage( int msgLevel, const QString &msg );

protected:
    // only available to sub-classed device drivers:
    void setClassType( enum ClassType ct );
    void setCapabilities( int caps );
    KConfig *config() const { return m_config; };
    QString configFileName() const { return m_configFileName; };

protected:
    QMutex  m_mutex;
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

