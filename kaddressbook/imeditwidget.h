/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef IMEDITWIDGET_H
#define IMEDITWIDGET_H

#include <kabc/addressee.h>
#include <kdialog.h>

class QGridLayout;
class KLineEdit;

/**
  This widget displays a list box of the instant messaging addresses as well as buttons
  to manipulate them (up, down, add, remove).
*/
class IMEditWidget : public QWidget
{
  Q_OBJECT

  public:
    IMEditWidget(QWidget *parent, KABC::Addressee &addr, const char *name = 0 );

    /*
     * This is a little bit ugly to get the cooperate design conform. But
     * this constructor should only be called in AddresseeEditorWidget.
     * I am not sure, whether this widget is used in another place.
     */
    IMEditWidget( QWidget *parent, KABC::Addressee &addr, QGridLayout *layout, const char *name = 0 );
    ~IMEditWidget();

    void setIMs( const QStringList &list );
    QStringList ims();

    void setPreferredIM( const QString &addr );
    QString preferredIM();
    void setReadOnly( bool readOnly );

  private:
    void initGui();

  signals:
    void modified();

  private slots:
    void edit();
    void textChanged( const QString& );

  private:
    KLineEdit *mIMEdit;
    QPushButton *mEditButton;
    QStringList mIMList;
    bool mReadOnly;
    KABC::Addressee &mAddressee;
    QGridLayout *mParentLayout;
};


#endif
