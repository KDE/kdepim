/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KSYNC_QTOPIACONFIG_H
#define KSYNC_QTOPIACONFIG_H

#include <qlayout.h>
#include <qlineedit.h>

#include <kresources/configwidget.h>

class QComboBox;
class QLabel;

namespace OpieHelper {

class QtopiaConfig : public KRES::ConfigWidget
{
    Q_OBJECT
  public:
    QtopiaConfig( QWidget *parent, const char *name = 0 );
    ~QtopiaConfig();

  public slots:
    void loadSettings( KRES::Resource *resource );
    void saveSettings( KRES::Resource *resource );

  private slots:
    void slotTextChanged( const QString & );

  private:
    void initUI();
    QString name()const;

    QGridLayout *m_layout;
    QLabel *m_lblUser, *m_lblPass, *m_lblName, *m_lblIP;
    QLabel *m_lblDev;
    QComboBox *m_cmbUser, *m_cmbPass, *m_cmbIP, *m_cmbDev;
    QLineEdit *m_name;
};

}


#endif
