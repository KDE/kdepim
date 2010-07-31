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
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNSEARCHDIALOG_H
#define KNSEARCHDIALOG_H

#include <tqdialog.h>

class QPushButton;

class KNFilterConfigWidget;
class KNArticleFilter;


class KNSearchDialog : public TQDialog {

  Q_OBJECT

  public:
    enum searchType { STfolderSearch, STgroupSearch };
    KNSearchDialog(searchType t=STgroupSearch, TQWidget *parent=0);
    ~KNSearchDialog();

    KNArticleFilter* filter() const  { return f_ilter; }

  protected:
    void closeEvent( TQCloseEvent* e );

    KNFilterConfigWidget *fcw;
    TQPushButton *startBtn, *newBtn,  *closeBtn;
    TQCheckBox *completeThreads;
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
