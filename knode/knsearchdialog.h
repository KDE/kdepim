/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
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

#include <kdialogbase.h>

class QCloseEvent;

class KNFilterConfigWidget;
class KNArticleFilter;

namespace KNode {

/** Article search dialog. */
class SearchDialog : public KDialogBase
{
  Q_OBJECT

  public:
    enum searchType { STfolderSearch, STgroupSearch };
    /** Create a new article search dialog.
     * @param parent The parent widget.
     */
    SearchDialog( searchType t = STgroupSearch, QWidget *parent = 0 );
    ~SearchDialog();

    KNArticleFilter* filter() const  { return f_ilter; }

  protected:
    void closeEvent( QCloseEvent* e );

    KNFilterConfigWidget *fcw;
    QCheckBox *completeThreads;
    KNArticleFilter *f_ilter;

  protected slots:
    /** Search button clicked. */
    void slotUser1();
    /** Clear button clicked. */
    void slotUser2();
    void slotClose();

  signals:
    void doSearch(KNArticleFilter *);
    void dialogDone();

};

}

#endif
