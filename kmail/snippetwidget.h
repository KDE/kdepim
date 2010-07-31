/***************************************************************************
 *   snippet feature from kdevelop/plugins/snippet/                        *
 *                                                                         * 
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __SNIPPET_WIDGET_H__
#define __SNIPPET_WIDGET_H__

#include <tqwidget.h>
#include <tqstring.h>
#include <klistview.h>
#include <tqtooltip.h>
#include <tqrect.h>

#include <ktexteditor/editinterface.h>
#include <ktexteditor/view.h>
#include "snippetconfig.h"

class KDevProject;
class SnippetPart;
class QPushButton;
class KListView;
class QListViewItem;
class QPoint;
class SnippetDlg;
class SnippetItem;
class KTextEdit;
class KConfig;
class KMEdit;
class KActionCollection;

/**
This is the widget which gets added to the right TreeToolView.
It inherits KListView and TQToolTip which is needed for showing the
tooltips which contains the text of the snippet
@author Robert Gruber
*/
class SnippetWidget : public KListView, public QToolTip
{
  Q_OBJECT

public:
    SnippetWidget(KMEdit* editor, KActionCollection* actionCollection, TQWidget* parent = 0);
    ~SnippetWidget();
    TQPtrList<SnippetItem> * getList() { return (&_list); }
    void writeConfig();
    SnippetConfig *  getSnippetConfig() { return (&_SnippetConfig); }


private slots:
    void initConfig();

protected:
    void maybeTip( const TQPoint & );
    bool acceptDrag (TQDropEvent *event) const;

private:
    void insertIntoActiveView( const TQString &text );
    TQString parseText(TQString text, TQString del="$");
    bool showMultiVarDialog(TQMap<TQString, TQString> * map, TQMap<TQString, TQString> * mapSave,
                            int & iWidth, int & iBasicHeight, int & iOneHeight);
    TQString showSingleVarDialog(TQString var, TQMap<TQString, TQString> * mapSave, TQRect & dlgSize);
    SnippetItem* makeItem( SnippetItem* parent, const TQString& name, const TQString& text, const KShortcut& shortcut );

    TQPtrList<SnippetItem> _list;
    TQMap<TQString, TQString> _mapSaved;
    KConfig * _cfg;
    SnippetConfig _SnippetConfig;
    KMEdit* mEditor;
    KActionCollection* mActionCollection;

public slots:
    void slotRemove();
    void slotEdit( TQListViewItem* item_ = 0 );
    void slotEditGroup();
    void slotAdd();
    void slotAddGroup();
    void slotExecute();

protected slots:
    void showPopupMenu( TQListViewItem * item, const TQPoint & p, int );
    void slotExecuted(TQListViewItem * item =  0);
    void slotDropped(TQDropEvent *e, TQListViewItem *after);
    void startDrag();
};


#endif
