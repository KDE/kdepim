/*
    Twister - PIM app for KDE
    
    Copyright 2000
        Rik Hemsley <rik@kde.org>
    
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

#ifndef TWISTER_SIDEBAR_H
#define TWISTER_SIDEBAR_H

// Qt includes
#include <qiconview.h>

class TwisterSideBar : public QIconView
{
    Q_OBJECT

    public:
        
        TwisterSideBar(QWidget * parent);
        ~TwisterSideBar();

    protected slots:

        void s_currentChanged(QIconViewItem *);

    signals:

        void switchWidget(const QString &);
};


#endif
// vim:ts=4:sw=4:tw=78
