/*
    This file is part of kdepim.
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef DEBUGDIALOG_H
#define DEBUGDIALOG_H

#include <kdialogbase.h>

class KTextBrowser;

/**
  A dialog that parses chunks of XML documents and
  displays them in a treeview.
 */
class DebugDialog : public KDialogBase
{
  Q_OBJECT

  public:
    /**
      The type of the message.
     */
    enum Type { Input, Output };

    /**
      Starts the debug dialog depending on the presence
      of the environment variable EGROUPWARE_DEBUG.
     */
    static void init();

    /**
      Destructor.
     */
    ~DebugDialog();

    /**
      Adds a message, which will be shown by the dialog.
     */
    static void addMessage( const QString &msg, Type type );

  private slots:
    void clear();
    void save();

  protected slots:
    virtual void slotUser1();
    virtual void slotUser2();

  private:
    DebugDialog();
    static DebugDialog *mSelf;

    void addText( const QString&, Type );

    QStringList mMessages;
    QStringList mHTMLMessages;

    KTextBrowser *mView;
};

#endif
