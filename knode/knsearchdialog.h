/***************************************************************************
                          knsearchdialog.h  -  description
                             -------------------
    
    copyright            : (C) 2000 by Christian Thurner
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


#ifndef KNSEARCHDIALOG_H
#define KNSEARCHDIALOG_H

#include <qdialog.h>

class QPushButton;

class KNFilterConfigWidget;
class KNArticleFilter;


class KNSearchDialog : public QDialog {

  Q_OBJECT  

  public:
    enum searchType { STfolderSearch, STgroupSearch };
    KNSearchDialog(searchType t=STgroupSearch, QWidget *parent=0);
    ~KNSearchDialog();
    
    KNArticleFilter* filter()   { return f_ilter; }
      
  protected:
    void closeEvent(QCloseEvent *e);
    KNFilterConfigWidget *fcw;
    QPushButton *startBtn, *newBtn,  *closeBtn;
    KNArticleFilter *f_ilter;
    
  protected slots:
    void slotStartClicked();
    void slotNewClicked();
    void slotCloseClicked();
    
  signals:
    void doSearch(KNArticleFilter *);
    void dialogDone();
  
};

#endif
