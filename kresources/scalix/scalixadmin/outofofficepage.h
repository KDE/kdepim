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

#ifndef OUTOFOFFICEPAGE_H
#define OUTOFOFFICEPAGE_H

#include <qwidget.h>

class QLabel;
class QPushButton;
class QRadioButton;
class QTextEdit;

class OutOfOfficePage : public QWidget
{
  Q_OBJECT

  public:
    OutOfOfficePage( QWidget *parent = 0 );
    ~OutOfOfficePage();

  private slots:
    void load();
    void loaded( KIO::Job* );
    void store();
    void stored( KIO::Job* );
    void statusChanged();
    void changed();

  private:
    QRadioButton *mEnabled;
    QRadioButton *mDisabled;
    QLabel *mLabel;
    QTextEdit *mMessage;
    QPushButton *mSaveButton;

    bool mChanged;
};

#endif
