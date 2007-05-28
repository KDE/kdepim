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
#ifndef FIRSTPAGE_IMPL_H
#define FIRSTPAGE_IMPL_H

#include "ui_firstpage.h"
#include <QWizardPage>

class FirstPagePrivate;

class FirstPage : public QWizardPage, public Ui::FirstPage
{
Q_OBJECT
Q_PROPERTY( QString engineLibrary READ engineLibrary WRITE setEngineLibrary )
public:
    FirstPage(QWidget *parent=0);
    bool isFinalPage() const;
    void initializePage();
    bool validatePage();
    QString engineLibrary() const;
    void setEngineLibrary(const QString &) { /* @TODO implement me */ }

public slots:
    void engineSelected(int index);
    virtual void slotCompleteChanged();
private:
    FirstPagePrivate *d;
signals:
    QString engineLibraryChanged(const QString &);
};

#endif
