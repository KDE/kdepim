/*
  This file is part of KTnef.

  Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#ifndef KTNEFMAIN_H
#define	KTNEFMAIN_H

#include <KXmlGuiWindow>

class QContextMenuEvent;
class QTreeWidgetItem;

namespace KTnef {
  class KTNEFParser;
  class KTNEFAttach;
}
using namespace KTnef;

class KTNEFView;

class KTNEFMain : public KXmlGuiWindow
{
  Q_OBJECT

  public:
    KTNEFMain( QWidget *parent = 0 );
    ~KTNEFMain();

    void loadFile( const QString &filename );

  protected:
    /** @reimp */
    virtual void contextMenuEvent( QContextMenuEvent *event );

  protected slots:
    void openFile();
    void viewFile();
    void viewFileAs();
    void extractFile();
    void extractFileTo();
    void propertiesFile();
    void optionDefaultDir();
    void extractAllFiles();
    void slotEditToolbars();
    void slotNewToolbarConfig();
    void slotShowMessageProperties();
    void slotShowMessageText();
    void slotSaveMessageText();

    void viewSelectionChanged();
    void viewDoubleClicked( QTreeWidgetItem * );
    void viewDragRequested( const QList<KTNEFAttach *> &list );
    void slotConfigureKeys();

//  protected:
//    void closeEvent( QCloseEvent *e );

  private:
    void setupStatusbar();
    void setupActions();
    void setupTNEF();
    void enableExtractAll( bool on = true );
    void enableSingleAction( bool on = true );
    void cleanup();

    void extractTo( const QString &dirname );
    QString extractTemp( KTNEFAttach *att );

  private:
    KTNEFView *mView;
    KTNEFParser *mParser;
    QString mFilename;
    QString mDefaultDir;
    QString mLastDir;
};

#endif
