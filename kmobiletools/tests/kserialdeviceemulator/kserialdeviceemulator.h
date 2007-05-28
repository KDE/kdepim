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


#ifndef _KSERIALDEVICEEMULATOR_H_
#define _KSERIALDEVICEEMULATOR_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/Job.h>
#include "commandslist.h"
#include <kmainwindow.h>

/**
 * @short Application Main Window
 * @author Marco Gulino <marco@kmobiletools.org>
 * @version 0.1
 */
 namespace KMobileTools{
    class QSerial;
 }
 using namespace ThreadWeaver;
 class QMutex;
 class KSerialDeviceEmulatorWidget;
class CommandJob : public ThreadWeaver::Job
{
    Q_OBJECT
    public:
        CommandJob(KMobileTools::QSerial *serial, QObject* parent = 0 , const char* name = 0);
        Command command() const { return cmd; }
    private:
        Command cmd;
        KMobileTools::QSerial *serial;
        QString s_buffer;
    protected:
        void run();
        QString getAnswer(const QString &cmd);
        void gotCMD(const QString &cmd);
};

class LoadFileJob : public ThreadWeaver::Job
{
    Q_OBJECT
    public:
        LoadFileJob(const QString &filename, QObject* parent = 0 , const char* name = 0)
    :ThreadWeaver::Job("loadfile", parent, name)
        { this->filename=filename; }
    private:
        QString filename;
    protected:
        void run()
        { CommandsList::instance()->loadFile(filename); }
};

class SendEventJob : public ThreadWeaver::Job
{
    Q_OBJECT
    public:
        SendEventJob(KMobileTools::QSerial *serial, const QString &event, QObject* parent = 0 , const char* name = 0)
    :ThreadWeaver::Job("sendevent", parent, name)
        { this->serial=serial; this->event=event; }
    private:
        KMobileTools::QSerial *serial;
        QString event;
    protected:
        void run();
};

class KSerialDeviceEmulator : public KMainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    KSerialDeviceEmulator();

    /**
     * Default Destructor
     */
    virtual ~KSerialDeviceEmulator();
    private:
        KMobileTools::QSerial *serial;
        QMutex *mutex;
        KSerialDeviceEmulatorWidget* m_widget;
        ThreadWeaver::Weaver *weaver;
public slots:
    void gotData();
    void jobDone(Job *)    ;
    void loadFile(const QString &file);
    void sendEvent(const QString &event);
};

#endif // _KSERIALDEVICEEMULATOR_H_
