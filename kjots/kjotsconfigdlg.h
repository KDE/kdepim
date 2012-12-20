/*
    Copyright (c) 2009 Montel Laurent <montel@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KJOTSCONFIGDLG_H
#define KJOTSCONFIGDLG_H

#include <kcmodule.h>
#include <kcmultidialog.h>
#include "ui_confpagemisc.h"

class confPageMisc : public QWidget, public Ui::confPageMisc
{
public:
    explicit confPageMisc( QWidget *parent ) : QWidget( parent ) {
        setupUi( this );
    }
};


class KJotsConfigMisc : public KCModule
{
    Q_OBJECT

  public:
    KJotsConfigMisc( const KComponentData &inst, QWidget *parent );

    /** Reimplemented from KCModule. */
    virtual void load();

    /** Reimplemented form KCModule. */
    virtual void save();
  private slots:
    void modified();
  private:
    confPageMisc *miscPage;
};

class KJotsConfigDlg : public KCMultiDialog
{
    Q_OBJECT

public:
    KJotsConfigDlg( const QString & title, QWidget *parent );
    ~KJotsConfigDlg();

public slots:
    void slotOk();
};

#endif /* KJOTSCONFIGDLG_H */

