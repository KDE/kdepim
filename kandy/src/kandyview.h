/*
    This file is part of Kandy.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KANDYVIEW_H
#define KANDYVIEW_H

#include <qwidget.h>

class QPainter;
class QTextStream;
class QDataStream;
class QDomDocument;
class QDomElement;
class QTextEdit;
class QListView;

class KURL;

class Modem;
class ATCommand;
class MobileGui;
class CommandScheduler;
class CmdPropertiesDialog;

/**
  This is the main view class for Kandy.
 
  @short Main view
  @author Cornelius Schumacher <schumacher@kde.org>
*/
class KandyView : public QWidget
{
    Q_OBJECT
  public:
	/**
	 * Default constructor
	 */
    KandyView(CommandScheduler *,QWidget *parent);

	/**
	 * Destructor
	 */
    virtual ~KandyView();

    /**
      Import phonebook from mobile phone and save it to Kab. This function
      returns before the job is actually done.
    */
    void importPhonebook();

    /**
     * Print this view to any medium -- paper or not
     */
    void print(QPainter *, int height, int width);

    bool loadFile(const QString& filename);
    bool saveFile(const QString& filename);

    void setModified(bool modified=true);
    bool isModified();

  public slots:
    void addCommand();
    void executeCommand();
    void deleteCommand();
    void editCommand();

  signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString& text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString& text);

    void modifiedChanged(bool);

  protected slots:
    void appendOutput(const char *line);

    void setResult(ATCommand *);

  private slots:
    void slotSetTitle(const QString& title);
    void processLastLine();

  private:
    QString mLastInput;

    CommandScheduler *mScheduler;

    bool mModified;

    QListView *mCommandList;

    QTextEdit *mInput;
    QTextEdit *mOutput;
    QTextEdit *mResultView;
};

#endif // KANDYVIEW_H
