/*
    knsearchdialog.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

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

    KNArticleFilter* filter() const  { return f_ilter; }

  protected:
    KNFilterConfigWidget *fcw;
    QPushButton *startBtn, *newBtn,  *closeBtn;
    QCheckBox *completeThreads;
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
