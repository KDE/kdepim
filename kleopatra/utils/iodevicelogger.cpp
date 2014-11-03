/* -*- mode: c++; c-basic-offset:4 -*-
    iodevicelogger.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

#include <config-kleopatra.h>

#include "iodevicelogger.h"

#include <cassert>

using namespace boost;
using namespace Kleo;

class IODeviceLogger::Private
{
    IODeviceLogger *const q;
public:

    static bool write(const shared_ptr<QIODevice> &dev, const char *data, qint64 max);

    explicit Private(const shared_ptr<QIODevice> &io_, IODeviceLogger *qq) : q(qq), io(io_), writeLog(), readLog()
    {
        assert(io);
        connect(io.get(), SIGNAL(aboutToClose()), q, SIGNAL(aboutToClose()));
        connect(io.get(), SIGNAL(bytesWritten(qint64)), q, SIGNAL(bytesWritten(qint64)));
        connect(io.get(), SIGNAL(readyRead()), q, SIGNAL(readyRead()));
        q->setOpenMode(io->openMode());
    }

    ~Private()
    {
    }

    const shared_ptr<QIODevice> io;
    shared_ptr<QIODevice> writeLog;
    shared_ptr<QIODevice> readLog;
};

bool IODeviceLogger::Private::write(const shared_ptr<QIODevice> &dev, const char *data, qint64 max)
{
    assert(dev);
    assert(data);
    assert(max >= 0);
    qint64 toWrite = max;
    while (toWrite > 0) {
        const qint64 written = dev->write(data, toWrite);
        if (written < 0) {
            return false;
        }
        toWrite -= written;
    }
    return true;
}

IODeviceLogger::IODeviceLogger(const shared_ptr<QIODevice> &iod, QObject *parent) : QIODevice(parent), d(new Private(iod, this))
{
}

IODeviceLogger::~IODeviceLogger()
{
}

void IODeviceLogger::setWriteLogDevice(const shared_ptr<QIODevice> &dev)
{
    d->writeLog = dev;
}

void IODeviceLogger::setReadLogDevice(const shared_ptr<QIODevice> &dev)
{
    d->readLog = dev;
}

bool IODeviceLogger::atEnd() const
{
    return d->io->atEnd();
}

qint64 IODeviceLogger::bytesAvailable() const
{
    return d->io->bytesAvailable();
}

qint64 IODeviceLogger::bytesToWrite() const
{
    return d->io->bytesToWrite();
}

bool IODeviceLogger::canReadLine() const
{
    return d->io->canReadLine();
}

void IODeviceLogger::close()
{
    d->io->close();
}

bool IODeviceLogger::isSequential() const
{
    return d->io->isSequential();
}

bool IODeviceLogger::open(OpenMode mode)
{
    QIODevice::open(mode);
    return d->io->open(mode);
}

qint64 IODeviceLogger::pos() const
{
    return d->io->pos();
}

bool IODeviceLogger::reset()
{
    return d->io->reset();
}

bool IODeviceLogger::seek(qint64 pos)
{
    return d->io->seek(pos);
}

qint64 IODeviceLogger::size() const
{
    return d->io->size();
}

bool IODeviceLogger::waitForBytesWritten(int msecs)
{
    return d->io->waitForBytesWritten(msecs);
}

bool IODeviceLogger::waitForReadyRead(int msecs)
{
    return d->io->waitForReadyRead(msecs);
}

qint64 IODeviceLogger::readData(char *data, qint64 maxSize)
{
    const qint64 num = d->io->read(data, maxSize);
    if (num > 0 && d->readLog) {
        Private::write(d->readLog, data, num);
    }
    return num;
}

qint64 IODeviceLogger::writeData(const char *data, qint64 maxSize)
{
    const qint64 num = d->io->write(data, maxSize);
    if (num > 0 && d->writeLog) {
        Private::write(d->writeLog, data, num);
    }
    return num;
}

qint64 IODeviceLogger::readLineData(char *data, qint64 maxSize)
{
    const qint64 num = d->io->readLine(data, maxSize);
    if (num > 0 && d->readLog) {
        Private::write(d->readLog, data, num);
    }
    return num;
}

