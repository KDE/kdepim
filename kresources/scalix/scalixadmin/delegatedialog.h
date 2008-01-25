/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DELEGATEDIALOG_H
#define DELEGATEDIALOG_H

#include <qmap.h>

#include <kdialogbase.h>

class QCheckBox;
class QLineEdit;

namespace Scalix {
class Delegate;
}

class DelegateDialog : public KDialogBase
{
  Q_OBJECT

  public:
    DelegateDialog( QWidget *parent = 0 );

    void setDelegate( const Scalix::Delegate &delegate );
    Scalix::Delegate delegate() const;

  private slots:
    void selectEmail();

  private:
    QLineEdit *mEmail;
    QMap<int, QCheckBox*> mRights;
};

#endif
