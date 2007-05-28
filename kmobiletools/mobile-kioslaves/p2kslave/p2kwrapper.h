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
#ifndef P2KWRAPPER_H
#define P2KWRAPPER_H

#include <qobject.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <p2kmoto.h>
#include <qstringlist.h>
#include <q3memarray.h>

/**
@author Marco Gulino
*/
class P2KWrapper : public QObject
{
Q_OBJECT
public:
    P2KWrapper(int vendor, int at_prodID, int p2k_prodID, const QString &acmDevice, QObject *parent = 0, const char *name = 0);
    explicit P2KWrapper(QObject *parent = 0, const char *name = 0);

    ~P2KWrapper();
    bool connected() { return b_connected; }
    Q3ValueList<p2k_fileInfo> getFiles() { if (countFiles() != (int) files.count() ) fetchFileList(); return files; }
    QStringList getDirs() { return dirs; }
    bool findDevice(const QString &acmDevice, const QString &rawVendor, const QString &rawProduct);
    void setATConfig(int v, int p);
    void setP2KConfig(int v, int p);

    private:
        int i_vendor, i_at_prodID, i_p2k_prodID;
        bool b_connected;
        QString s_acmDevice;
        Q3ValueList<p2k_fileInfo> files;
        QStringList dirs;
        int n_files;

public slots:
    bool connectPhone();
    void closePhone();
    int countFiles();
    QStringList getRoot();
    void fetchFileList();
    int getFile(const QString &name, char* buffer);
    bool putFile( const QString &name, char *buffer, int size);
    bool deleteFile( const QString &name );
};

#endif
