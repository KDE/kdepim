/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#ifndef SENDTOBLOGDIALOG_H
#define SENDTOBLOGDIALOG_H

#include <kdialog.h>

class SendToBlogDialog : public KDialog
{
    Q_OBJECT
public:
    explicit SendToBlogDialog( bool isNew, bool isPrivate, QWidget *parent = 0 );
    ~SendToBlogDialog();

    bool isPrivate();
    bool isNew();
public slots:
    virtual void accept();
private:
    class Private;
    Private * const d;
};

#endif // SENDTOBLOGDIALOG_H
