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
#ifndef ADDRESSDETAILS_H
#define ADDRESSDETAILS_H

#include <khtml_part.h>

#include <kabc/addressee.h>

/**
@author Marco Gulino
*/
class addressDetails : public KHTMLPart
{
Q_OBJECT
public:
    addressDetails(QWidget *parentWidget, const QString &objectname, QObject *parent = 0);
    ~addressDetails();
    void showAddressee(const KABC::Addressee &addressee, bool readOnly=false);
    static const QString getTemplate();

public slots:
    void popupMenu ( const QString &url, const QPoint &point);
    void showHP();
protected slots:
        void openUrlRequest(const KUrl &url);

private:
    int timerpolls;
    KABC::Addressee p_addressee;
    bool ro;
    signals:
        void refreshClicked();
        void editClicked(const KABC::Addressee&);
        void addContact();
        void delContact();
        void exportPB();
        void importPB();
        void dial(const QString &);
};

#endif
