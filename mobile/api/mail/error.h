/*
    Copyright 2014  Michael Bohlender michael.bohlender@kdemail.net

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef ERROR_H
#define ERROR_H

#include <QObject>

#include <QString>

class Error : public QObject
{
    Q_OBJECT
    Q_PROPERTY (int code READ code NOTIFY errorChanged)
    Q_PROPERTY (QString text READ text NOTIFY errorChanged)
    Q_ENUMS (ErrorCode)

public:
    enum ErrorCode {
        OK // everything is alright
    };

    explicit Error( QObject *parent = 0 );

    int code() const;
    QString text() const;

signals:
    void errorChanged();

public slots:
    void clear();
    void setError( ErrorCode code, const QString &text );

private:
    ErrorCode m_code;
    QString m_text;
};

#endif //ERROR_H