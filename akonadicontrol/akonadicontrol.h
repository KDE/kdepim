/*******************************************************************************
**
** Filename   : akonadicontrol.h
** Created on : 27 May, 2006
** Copyright  : (c) 2006 Till Adam
** Email      : adam@kde.org
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
*******************************************************************************/

#ifndef AKONADICONTROL_H
#define AKONADICONTROL_H

#include <QObject>
#include <QProcess>

class AkonadiControl : public QObject
{
    Q_OBJECT
public:
    AkonadiControl();
    ~AkonadiControl();

    void startStorageServer();
    void sanityCheckStorageServer();

private slots:
    void slotServerError( QProcess::ProcessError );
    void slotServerFinshed(  int exitCode,QProcess::ExitStatus );

private:
    QProcess m_server;
}; // End of class AkonadiControl


#endif // AKONADICONTROL_H
