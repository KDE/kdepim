/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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

// System includes
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <errno.h>

// Qt includes
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>

// KDE includes
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

// Local includes
#include "EmpathEditorProcess.h"

unsigned int EmpathEditorProcess::seq_ = 0;

EmpathEditorProcess::EmpathEditorProcess(const QCString & text)
    :   QObject(),
        text_(text)
{
    pidStr_.setNum(getpid());
 
    struct timeval timeVal;
    struct timezone timeZone;
    
    gettimeofday(&timeVal, &timeZone);
    startupSecondsStr_.setNum(timeVal.tv_sec);

    struct utsname utsName;
    if (uname(&utsName) == 0)
        hostName_ = utsName.nodename;

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
    // Add 'empath_' prefix to allow editor to recognise.
    fileName_ = "/tmp/empath_" + _generateUnique();

    QFile f(fileName_);

    if (!f.open(IO_WriteOnly)) {
        
        QString warning = i18n("Couldn't open the temporary file `%1'");
        
        QMessageBox::warning(
            (QWidget *)0, "Empath",
            warning.arg(fileName_),
            i18n("OK"));
        return;
    }
    
    f.writeBlock(text_.data(), text_.length());
    f.flush();
    f.close();
    
    if (f.status() != IO_Ok) {
        
        QString warning = i18n("Couldn't write to the temporary file `%1'");
        
        QMessageBox::warning(0, "Empath", warning.arg(fileName_), i18n("OK"));
    }
    
    QFileInfo fi(f);
    myModTime_ = fi.lastModified();
    
    KConfig * config = KGlobal::config();
    
    config->setGroup("EmpathEditorProcess");
    QString externalEditor = config->readEntry("EditorName");

    p << externalEditor << fileName_;

    if (!p.start(KProcess::NotifyOnExit, KProcess::All)) {

        QString warning = i18n("Couldn't start the editor `%1'");
        
        QMessageBox::warning
            (0, "Empath", warning.arg(externalEditor), i18n("OK"));
        return;
    }
}

    void
EmpathEditorProcess::s_composeFinished(KProcess *)
{
    QFileInfo finfo(fileName_);

    QDateTime modTime = finfo.lastModified();

    if (myModTime_ == modTime) {
        // File was NOT modified.
        emit(done(false, ""));
        return;
    }

    QFile f(fileName_);

    if (!f.open(IO_ReadOnly)) {

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

    QString
EmpathEditorProcess::_generateUnique()
{
    return (
        startupSecondsStr_ + '.' + pidStr_ + '_' +
        QString::number(seq_++) + '.' + hostName_);
}


// vim:ts=4:sw=4:tw=78
#include "EmpathEditorProcess.moc"
