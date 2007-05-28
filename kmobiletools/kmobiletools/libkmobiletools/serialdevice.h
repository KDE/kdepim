/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef DEVICE_H
#define DEVICE_H
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <libkmobiletools/kmobiletools_export.h>


#define MAX_TIMEOUT 6000

/**
@author Marco Gulino
*/
class QTimer;
namespace KMobileTools{
    class Job;
    class SerialManagerPrivate;
    class QSerial;
    /**
     \class SerialManager serialdevice.h
     \brief This class can manage a QSerial object, initialize the modem, sending AT commands, parsing answers.
    */
    class KMOBILETOOLS_EXPORT SerialManager : public QObject
    {
    Q_OBJECT
    public:
        /**
         * Creates a new SerialManager object.
         * @param parent the parent object.
         * @param name the name of this object.
         * @param devicePath path to use while initializing the serial device.
         * @param initStrings AT commands to send to the serial port while initializing.
         */
        SerialManager(QObject * parent, const QString &objname, const QString &devicePath=QString(), const QStringList &initStrings=QStringList() );
        /**
         * Destroys a SerialManager object.
         */
        ~SerialManager();
        /**
         * Opens the serial port for read/write commands.
         * @return true if the port was opened, false otherwise.
         */
        bool open(KMobileTools::Job *job);
        /**
         * Close the serial device, freeing resources.
         */
        void close();
        /**
         * Sends an AT command to the serial port.
         * @param cmd the command to send.
         * @param timeout wait for the answer until timeout expires. A value of 0 will loop forever. Timeout is in milliseconds.
         * @return the answer to cmd, or QString() if an error occurred.
         */
        QString sendATCommand(KMobileTools::Job *job, const QString &cmd, uint timeout=MAX_TIMEOUT, bool tryBreakingTimeout=true);
        /**
         * Returns the QSerial class managed by this object.
         * @return the managed QSerial object if is valid, 0 otherwise.
         */
        QSerial *qserial();
        /**
         * Convenience method to see if the Serial object is connected to the device.
         * @return true if the device is connected, false otherwise.
         */
        bool isConnected();
        /**
         * Change the device this class is pointing to.
         * @param path the path to the device to manage.
         */
        void setDevicePath(const QString &path);
        void setSpeed(int value);

        void lockMutex();
        void unlockMutex();
        QString devicePath() const;
        static QString decodePDU( const QString &text );
    static bool ATError(const QString &buffer);

    private:
        SerialManagerPrivate *const d;

    public Q_SLOTS:
        void gotData();
        void gotError(int);
    void log(bool incoming, const QString &data);

    Q_SIGNALS:
        void disconnected();
        void connected();
        void error();
        void invalidLockFile(const QString &);
    };
}

#endif
