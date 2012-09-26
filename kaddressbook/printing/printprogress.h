/*
  This file is part of KAddressBook.
  Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef PRINTPROGRESS_H
#define PRINTPROGRESS_H

#include <QWidget>

class QProgressBar;
class QString;

class KTextBrowser;

namespace KABPrinting {

/**
  This defines a simple widget to display print progress
  information. It is provided to all print styles during a print
  process. It displays messages and a a progress bar.
 */
class PrintProgress : public QWidget
{
  Q_OBJECT

  public:
    explicit PrintProgress( QWidget *parent, const char *name = 0 );
    ~PrintProgress();

    /**
      Add a message to the message log. Give the user something to admire :-)
     */
    void addMessage( const QString & );

    /**
      Set the progress to a certain amount. Steps are from 0 to 100.
     */
    void setProgress( int );

  private:
    QStringList mMessages;

    KTextBrowser *mLogBrowser;
    QProgressBar *mProgressBar;
};

}

#endif
