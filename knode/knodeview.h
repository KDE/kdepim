/***************************************************************************
                     knodeview.h - description
 copyright            : (C) 1999 by Christian Thurner
 email                : cthurner@freepage.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNODEVIEW_H
#define KNODEVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qsplitter.h>
#include <kaction.h>

class KNArticleWidget;
class KNListView;
class KNFocusWidget;


class KNodeView : public QSplitter
{
  Q_OBJECT

  friend class KNodeApp;
  
  public:
      
    KNodeView(QWidget *parent=0,const char * name=0);
    ~KNodeView();
    
    const KActionCollection& actions()      { return actionCollection; }    
    
    void readOptions();
    void saveOptions();

    void updateViewMode();   // switch between long & short group list

  protected:
    void initCollectionView();
    void initHdrView();
    
    QSplitter *secSplitter;
    KNArticleWidget *artView;
    KNListView *hdrView;
    KNListView *collectionView;
    KNFocusWidget *colFocus, *hdrFocus, *artFocus;
    bool longView;

    KSelectAction *actSortSelect;
    KAction *actNextArt, *actPrevArt, *actNextUnreadArt, *actReadThrough, *actNextUnreadThread,
            *actNextGroup, *actPrevGroup, *actToggleThread;
    KActionCollection actionCollection;

  protected slots:
    void slotSortMenuSelect(int newCol);   // select from KSelectAction
    void slotSortHdrSelect(int newCol);    // select from QListView header
    void slotNextArticle();
    void slotPrevArticle();
    void slotNextUnreadArticle();
    void slotReadThrough();
    void slotNextUnreadThread();
    void slotNextGroup();
    void slotPrevGroup();
    void slotToggleThread();
    
};

#endif // KNODEVIEW_H

