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
    fileName_ = "/tmp/empath_" + empath->generateUnique();

    empathDebug("Opening file " + fileName_);

    QFile f(fileName_);

    if (!f.open(IO_WriteOnly)) {
        
        empathDebug("Couldn't open the temporary file " + fileName_);
        
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
        
        empathDebug("Couldn't write to the temporary file " + fileName_);

        QString warning = i18n("Couldn't write to the temporary file `%1'");
        
        QMessageBox::warning(0, "Empath", warning.arg(fileName_), i18n("OK"));
    }
    
    QFileInfo fi(f);
    myModTime_ = fi.lastModified();
    
    KConfig * config = KGlobal::config();
    
    using namespace EmpathConfig;
    
    config->setGroup(GROUP_COMPOSE);
    QString externalEditor = config->readEntry(C_EXT_EDIT);

    p << externalEditor << fileName_;

    if (!p.start(KProcess::NotifyOnExit, KProcess::All)) {
        empathDebug("Couldn't start process");
        
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

// vim:ts=4:sw=4:tw=78
