/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathEditorProcess.h"
#endif

// Qt includes
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>

// KDE includes
#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

// Local includes
#include "Empath.h"
#include "EmpathEditorProcess.h"

EmpathEditorProcess::EmpathEditorProcess(const QCString & text)
    :   QObject(),
        text_(text)
{
    QObject::connect(&p, SIGNAL(receivedStdout(KProcess *, char *, int)),
            this, SLOT(s_debugExternalEditorOutput(KProcess *, char *, int)));

    QObject::connect(&p, SIGNAL(receivedStderr(KProcess *, char *, int)),
            this, SLOT(s_debugExternalEditorOutput(KProcess *, char *, int)));

    QObject::connect(&p, SIGNAL(processExited(KProcess *)),
        this, SLOT(s_composeFinished(KProcess *)));
}

EmpathEditorProcess::~EmpathEditorProcess()
{
    p.kill(SIGKILL);
}

    void
EmpathEditorProcess::go()
{    
    QString tempName("/tmp/" + empath->generateUnique());

    empathDebug("Opening file " + QString(tempName));
    QFile f(tempName);

    if (!f.open(IO_WriteOnly)) {
        empathDebug("Couldn't open the temporary file " + tempName);
        QMessageBox::warning(
            (QWidget *)0, "Empath",
            i18n("Couldn't open the temporary file") + " " + tempName,
            i18n("OK"));
        return;
    }
    
    f.writeBlock(text_.data(), text_.length());
    f.flush();
    f.close();
    
    if (f.status() != IO_Ok) {
        
        empathDebug("Couldn't write to the temporary file " + tempName);
        
        QMessageBox::warning(
            (QWidget *)0, "Empath",
            i18n("Couldn't write to the temporary file") + " " + tempName,
            i18n("OK"));
    }
    
    QFileInfo fi(f);
    myModTime_ = fi.lastModified();
    
    KConfig * config = KGlobal::config();
    
    using namespace EmpathConfig;
    
    config->setGroup(GROUP_COMPOSE);
    QString externalEditor = config->readEntry(C_EXT_EDIT);

    p << externalEditor << tempName;

    if (!p.start(KProcess::NotifyOnExit, KProcess::All)) {
        empathDebug("Couldn't start process");
        return;
    }
}

    void
EmpathEditorProcess::s_composeFinished(KProcess *)
{
    // Find the process' filename in the process table.
    // Once we have the filename, we can re-read the text from that file and use
    // that text to send the new message. We must check if the file has been
    // modified too. If not, we'll just remove it.

    // Find out when the file was last modified.

    QFileInfo finfo(fileName);

    QDateTime modTime = finfo.lastModified();

    if (myModTime_ == modTime) {
        // File was NOT modified.
        emit(done(false, ""));
        return;
    }

    // Create a new message and send it via the mail sender.

    QFile f(fileName);

    if (!f.open(IO_ReadOnly)) {

        empathDebug("Couldn't reopen the temporary file");
        emit(done(false, ""));
        return;
    }
    
    char buf[1024];

    while (!f.atEnd()) {
        f.readBlock(buf, 1024);
        text_ += buf;
    }
    
    emit(done(true, text_));
}

    void
EmpathEditorProcess::s_debugExternalEditorOutput(
    KProcess *, char * buffer, int)
{
//    empathDebug("Received: " + QString(buffer));
}

// vim:ts=4:sw=4:tw=78
