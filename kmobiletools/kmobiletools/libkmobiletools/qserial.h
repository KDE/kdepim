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
#ifndef KMOBILETOOLSQSERIAL_H
#define KMOBILETOOLSQSERIAL_H

#include <QtCore/QIODevice>
#include <termios.h>

namespace KMobileTools {

/**
	@author Marco Gulino <marco@kmobiletools.org>
*/
class QSerialPrivate;
class QSerial : public QIODevice
{
    Q_OBJECT
//     Q_PROPERTY( QString device READ device WRITE setDevice )
//     Q_PROPERTY( Baud baud READ baud WRITE setBaud )
//     Q_PROPERTY( Parity parity READ parity WRITE setParity )
//     Q_PROPERTY( StopBits stopbits READ stopBits WRITE setStopbits)
//     Q_PROPERTY( FlowControl flowcontrol READ flowControl WRITE setFlowControl )
//     Q_ENUMS( Baud )
//     Q_ENUMS( Parity )
//     Q_ENUMS( StopBits )
//     Q_ENUMS( FlowControl )
public:
    /**
     * Creates a new QSerial object.
     */
    QSerial();
    /**
     * Convenience method for the above.
     * Creates a new QSerial object, and sets the device path to deviceName.
     * @param deviceName path to the device to use.
     */
    QSerial( const QString &deviceName );
    ~QSerial();

    /**
     * Serial Port baud rates.
     */
    enum Baud {
        BAUD_50      = B50,
        BAUD_75      = B75,
        BAUD_110     = B110,
        BAUD_134     = B134,
        BAUD_150     = B150,
        BAUD_200     = B200,
        BAUD_300     = B300,
        BAUD_600     = B600,
        BAUD_1200    = B1200,
        BAUD_1800    = B1800,
        BAUD_2400    = B2400,
        BAUD_4800    = B4800,
        BAUD_9600    = B9600,
        BAUD_19200   = B19200,
        BAUD_38400   = B38400,
        BAUD_57600   = B57600,
        BAUD_115200  = B115200,
        BAUD_230400  = B230400,
        BAUD_DEFAULT = BAUD_57600
    };
    /**
     * Serial Port parity.
     */
    enum Parity {
        /**
         * Enable parity
         */
        PARITY_NONE = 0x0,
        PARITY_ODD = 0x1,
        PARITY_EVEN = 0x2,
        PARITY_MARK = 0x3,
        PARITY_SPACE = 0x4,
        PARITY_DEFAULT = PARITY_NONE
    };
    /**
     * Serial port data bits.
     */
    enum DataBits {
        DATABITS_5=5,
        DATABITS_6=6,
        DATABITS_7=7,
        DATABITS_8=8,
        DATABITS_DEFAULT=DATABITS_8
    };
    /**
     * Serial Port stop bits.
     */
    enum StopBits {
        STOP_BITS_1=0x1,
        STOP_BITS_2=0x2,
        STOP_BITS_DEFAULT = STOP_BITS_1
    };
    /**
     * Serial Port Flowcontrol.
     */
    enum FlowControl {
        /**
         * Enable RTS/CTS (hardware) flow control.
         */
        FLOW_CONTROL_HARD=0x1,
        /**
         * Enable XON/XOFF flow control.
         */
        FLOW_CONTROL_XONXOFF=0x2,
        /**
         * No flow control.
         */
        FLOW_CONTROL_NONE=0x0,
        /**
         * Default flow control value (hardware)
         */
        FLOW_CONTROL_DEFAULT = FLOW_CONTROL_HARD
    };


    /**
     * Current serial port baudrate.
     * @return the current serial port baudrate.
     */
    Baud baud();
    /**
     * Sets the current serial port baudrate.
     * This will work only if the serial port is not already opened.
     * @param  baudrate the baudrate to use.
     */
    void setBaud( Baud baudrate);
    /**
     * Current serial port parity check settings.
     * @return the current serial port parity check settings.
     */
    Parity parity();
    /**
     * Sets the current serial port parity check settings.
     * This will work only if the serial port is not already opened.
     * @param  parity the parity setting to use.
     */
    void setParity( Parity parity);
    /**
     * Current serial port databits settings.
     * @return the current serial port databits settings.
     */
    DataBits databits();
    /**
     * Sets the current serial port databits settings.
     * @param databits the databits setting to use.
     */
    void setDatabits( DataBits databits);
    /**
     * Current serial port stop bits.
     * @return the current serial port stop bits.
     */
    StopBits stopBits();
    /**
     * Sets the current serial port stop bits.
     * This will work only if the serial port is not already opened.
     * @param  stopbits the stop bits settings to use.
     */
    void setStopBits( StopBits stopbits);
    /**
     * Current serial port flow control settings.
     * @return the current serial port flow control settings.
     */
    FlowControl flowControl();
    /**
     * Sets the current serial port flow control settings.
     * This will work only if the serial port is not already opened.
     * @param  flowcontrol the flow control settings to use.
     */
    void setFlowControl( FlowControl flowcontrol);
    /**
     * Current serial port device path.
     * @return the current serial port device path.
     * @see QSerial::setName(const QString &)
     */
    QString name() const;
    /**
     * Sets the current serial device path to use.
     * This will work only if the serial port is not already opened.
     * @param name the serial device path to use.
     * @see QSerial::name()
     */
    void setName(const QString &name);
    /**
     * Opens the serial device using the specified mode. Returns TRUE if the device was successfully opened; otherwise returns FALSE.
     * @param mode Flags for opening the device.
     * @param createLockFile True if a lock file has to be created, false otherwise.
     * @return true if the device was opened, otherwise false.
     * @see QIODevice::open(int m), close()
     */
    bool open( OpenMode mode, bool createLockFile );
    /**
     * Convenience method for the above.
     * Opens the serial device with mode, creating automatically a lock file.
     * @param mode Flags for opening the device.
     * @param createLockFile True if a lock file has to be created, false otherwise.
     * @return true if the device was opened, otherwise false.
     * @see QIODevice::open(int m), close()
         */
    bool open( OpenMode mode );
    /**
     * Closes the serial device.
     * @see open(int)
     */
    void close();

    /**
     * Flushes data to the serial device.
     */
    void flush();
    /**
     * Checks the serial device input buffer for incoming bytes.
     * @return the number of incoming bytes that can be read right now.
     */
    qint64 size() const;
    /**
     * Reads a single byte/character from the serial device.
     * @return the byte/character read, or -1 if an error occurred.
     * @see putch()
     */
    int getch();
    /**
     * Writes the character ch to the I/O device.
     * @param ch the byte to write.
     * @return ch, or -1 if an error occurred.
     * @see getch()
     */
    int putch( int ch );

    /**
     * Unimplemented.
     * @param ch character to write back to the device
     * @return always -1 (error) at this time.
     */
    int  ungetch( int ch );
    /**
     * Reset the serial port sending a 0x1A character.
     * @return true if the reset was sent correctly, false otherwise.
     */
    bool reset();
    /**
     * Returns if the device was opened or not
     * @return true if the device was opened, false otherwise.
    */
    bool isOpen() const;
    bool isSequential() const;

protected:
    /**
     * Reads at most maxlen bytes from the I/O device into data and returns the number of bytes actually read. 
    The device must be opened for reading, and data must not be 0.
     * @param data buffer for receiving bytes.
     * @param maxlen ,maximum number of bytes that should be read.
     * @return -1 if a fatal error occurs, otherwise the number of bytes read (0 if no data is available).
     * @see writeBlock()
     */
    qint64 readData( char *data, qint64 maxSize );
    /**
     * Writes len bytes from data to the serial device and returns the number of bytes actually written.
    This function should return -1 if a fatal error occurs.
     * @param data Buffer containing data to write.
     * @param len length of the buffer.
     * @return -1 if an error occurred.
     * @see readBlock()
     */
    qint64 writeData( const char *data, qint64 maxSize );
private:
    bool lockFile(bool lock);
    const QString lockFileName();
    void setupParameters();
    void createObject();
    QSerialPrivate *d;
private Q_SLOTS:
    void slotNotifierData(int);
Q_SIGNALS:
    /**
     * This signal is emitted when the serial port has received data ready to be read.
     */
    void gotData();
};

}

#endif
