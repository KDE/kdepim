/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#ifndef KS_PLUCKER_PROCESS_HANDLER_H
#define KS_PLUCKER_PROCESS_HANDLER_H

#include <kprocess.h>

#include <qobject.h>
#include <qstringlist.h>

namespace KSPlucker {
class PluckerProcessHandler : public QObject 
{
    Q_OBJECT
public:
    enum Mode {
        Configure,
        Convert
    };
    PluckerProcessHandler( enum Mode, bool forget,
                    const QString& file, QObject* parent = 0);
    PluckerProcessHandler( enum Mode, bool forget,
                    const QStringList& file, const QString& dest,
                    QObject* parent = 0);
    ~PluckerProcessHandler();

    void run();
signals:
    void sigProgress( const QString& );
    void sigFinished( PluckerProcessHandler* );

private:
    void runConfig (KProcess*);
    void runConvert(KProcess*);
    void popFirst();
    Mode m_mode;
    bool m_forget : 1;
    QString m_file;
    QString m_dir;
    QStringList m_files;
    bool m_useList : 1;
private slots:
    void slotExited( KProcess* proc );
    void slotStdOutput(KProcess* proc, char *buffer,  int buflen );
};
}

#endif
