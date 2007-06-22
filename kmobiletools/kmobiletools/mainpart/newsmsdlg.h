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
#ifndef NEWSMSDLG_H
#define NEWSMSDLG_H

#include "ui_ui_newsms.h"
#include <kdialog.h>

class KStatusBar;
namespace KMobileTools { class SMS; }
/**
	@author Marco Gulino <marco@kmobiletools.org>
*/

class newSMSDlg : public KDialog
{
Q_OBJECT
public:
    explicit newSMSDlg(QWidget *parent = 0, const QString &name=QString() );

    ~newSMSDlg();
    const QStringList numbers() { return sl_numbers;}
    const QString text();
    KMobileTools::SMS *getSMSItem() { return p_sms; }
    enum Actions { Send=0x1, Store=0x2 };
    int action() { return i_action; }
    private:
        Ui::ui_newsms ui;
        KStatusBar *statusBar;
        QStringList sl_numbers;
        void createSMSItem();
        KMobileTools::SMS *p_sms;
        int i_action;

public slots:
    void smsTextChanged();
    void pickPhoneNumber();
    void addNumber( const QString &number);
    void textNumberChanged(const QString &);
    void NumberClicked(Q3ListViewItem *);
    void remClicked();
    void addClicked();
protected slots:
    void slotUser1();
    void slotUser2();
};

#endif
