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
#include <sys/types.h>
#include <stdlib.h>
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

// -----------------------------------------------------------------------------

QString generateUnique();
bool writeMessage(const QString & s);

// -----------------------------------------------------------------------------

void checkDir(QString path) 
{
    if(path[path.length()-1] == '/')
        path.truncate(path.length()-1);
    
    QDir d;
    
    if (!d.cd(path)) {
        if(d.mkdir(path)) {
            chmod(path, S_IRUSR|S_IWUSR|S_IXUSR);
            cout << "created dir: " << path << endl;
        }
        else {
            cerr << "Can't create missing directory \"" << path << "\"" << endl;
            exit(1);
        }
    }
}

    
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

    QFileInfo fi(mboxPath);

    if(fi.isDir()) {
        cerr << "\"" << mboxPath << "\" should be a file, not a directory!" << endl;
        exit(1);
    }
    
    QFile mboxFile(mboxPath);
    
    if (!mboxFile.open(IO_ReadOnly)) {
        cerr << "Can't open mbox file \"" << mboxPath.ascii() << "\"" << endl;
        exit(1);
    }
    
    path = QDir(maildirPath).absPath();

    checkDir(path);
    checkDir(path + "/tmp");
    checkDir(path + "/new");
    checkDir(path + "/cur");
    
    QString maildirFullPath(path + "/cur");

    QTextStream str(&mboxFile);
    
    QString buf;    
    
    int n = 0;
    int failures = 0;
    
    cerr << "Copying messages" << endl <<
        "From: " << QFileInfo(mboxFile).absFilePath().ascii() << endl <<
        "To:   " << path.ascii() << endl;
        
    while (!str.eof()) {
            
        QString s = str.readLine() + "\n";

        if (s.contains(QRegExp("^From[ \\t]"))) {    

            QString unique = generateUnique();

            cerr << "copying " << n << " message (" << failures << " failures)\r";
            
            QFile f(maildirFullPath + "/" + unique);
            
            if (0 != n++)
                if (!writeMessage(buf))
                    ++failures;
            
            buf.truncate(0);
        }
        else
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
        cerr << "Couldn't open mail file (" << _path << ") for writing." << endl;
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
        cout << "link from: " << _path.ascii() << endl;
        cout << "link to  : " << linkTarget.ascii() << endl;
        perror("link");
        f.close();
        f.remove();
        return false;
    }

    return true;
}


// vim:ts=4:sw=4:tw=78
