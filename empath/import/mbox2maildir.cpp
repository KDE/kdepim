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

// System includes
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>

#include <iostream.h>

// Qt includes
#include <qstring.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtextstream.h>

unsigned long startSec;
unsigned long seq;
QString hostName;
unsigned long pid;
QString path;

QString generateUnique();
bool writeMessage(const QString & s);
    
int main(int argc, char * argv[])
{
    if (argc != 3) {
        cerr << "usage: mbox2maildir <mbox> <Maildir>" << endl;
        exit(1);
    }
    
    QString mboxPath(argv[1]);
    QString maildirPath(argv[2]);
    
    struct utsname utsName;
    
    if (uname(&utsName) != 0) {
        cerr << "Can't get hostname" << endl;
        exit(1);
    }
    
    struct timeval timeVal;
    struct timezone timeZone;
    gettimeofday(&timeVal, &timeZone);
    
    hostName = utsName.nodename;
    startSec = timeVal.tv_sec;
    pid = (unsigned long)getpid();
    seq = 0;

    QFile mboxFile(mboxPath);
    
    if (!mboxFile.open(IO_ReadOnly)) {
        cerr << "Can't open mbox file \"" << mboxPath.ascii() << "\"" << endl;
        exit(1);
    }
    
    path = QDir(maildirPath).absPath();
    
    QDir d;
    
    if (!d.cd(path)) {
        cerr << "Can't go to directory \"" << maildirPath.ascii() <<
            "\"" << endl;
        exit(1);
    }
    
    QString maildirFullPath(path + "/cur/");
    
    if (!d.cd(maildirFullPath)) {
        cerr << "Can't go to directory \"" << maildirFullPath.ascii()
            << "\"" << endl;
        exit(1);
    }
    
    QTextStream str(&mboxFile);
    
    QString buf;    
    
    int n = 0;
    int failures = 0;
    
    cerr << "Copying messages" << endl <<
        "From: " << QFileInfo(mboxFile).absFilePath().ascii() << endl <<
        "To:   " << path.ascii() << endl;
        
    while (!str.eof()) {
            
        QString s = str.readLine();

        if (s.find(QRegExp("^From[ \\t]"))) {    

            QString unique = generateUnique();
            
            QFile f(maildirFullPath + unique);
            
            if (0 != n++)
                if (!writeMessage(buf))
                    ++failures;
            
            buf.truncate(0);
            
            cerr << ".";
        }
        
        buf += s;
    }

    cerr << endl;
    
    cerr << "Copied " << n << " messages OK" << endl;

    if (failures != 0)
        cerr << "Errors: " << failures << endl;
}

    QString
generateUnique()
{
    QString unique;
    
    unique = QString().setNum(startSec);
    unique += ".";
    unique += QString().setNum(getpid());
    unique += "_";
    unique += QString().setNum(seq);
    unique += ".";
    unique += hostName;
    
    ++seq;

    return unique;
}

    bool
writeMessage(const QString & s)
{
    QString canonName = generateUnique();
    QString _path = path + "/tmp/" + canonName;
    QFile f(_path);

    if (f.exists()) {
        
        cerr << "Couldn't get a free filename for writing mail file." << endl;
        cerr << "Timing out for 2 seconds." << endl;
        
        usleep(2000);
    
        if (f.exists()) {
            cerr << "Still couldn't get a free filename." << endl;
            cerr << "Giving up on this message." << endl;
            return false;
        }
    }

    if (!f.open(IO_WriteOnly)) {
        cerr <<
            "Couldn't open mail file for writing." << endl;
        return false;
    }

    QDataStream outputStream(&f);
    outputStream.writeRawBytes(s.ascii(), s.length());
    
    f.flush();
    f.close();
    
    if (f.status() != IO_Ok) {
        
        cerr << "Couldn't close() file";
        f.close();
        f.remove();
        return false;
    }

    QString linkTarget(path + "/new/" + canonName);
    
    if (::link(_path.ascii(), linkTarget.ascii()) != 0) {
        cerr << "Couldn't successfully link mail file - giving up" << endl;
        perror("link");
        f.close();
        f.remove();
        return false;
    }

    return true;
}


// vim:ts=4:sw=4:tw=78
