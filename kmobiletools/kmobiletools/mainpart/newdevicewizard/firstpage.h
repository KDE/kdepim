/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>
   by Matthias Lechner <matthias@lmme.de>

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
#ifndef FIRSTPAGE_IMPL_H
#define FIRSTPAGE_IMPL_H

#include "ui_firstpage.h"
#include <QWizardPage>

class FirstPagePrivate;

class FirstPage : public QWizardPage, public Ui::FirstPage
{
    Q_OBJECT
    Q_PROPERTY( QString engineName READ engineName WRITE setEngineName )

    public:
        FirstPage( QWidget* parent=0 );

        bool isFinalPage() const;
        void initializePage();
        void cleanupPage();
        bool validatePage();

    public Q_SLOTS:
        void engineSelected( int index );

    protected:
        QString engineName() const;
        void setEngineName( const QString& engineName );

    Q_SIGNALS:
        void engineNameChanged( const QString& engineName );

    private:
        FirstPagePrivate *d;
};

#endif
