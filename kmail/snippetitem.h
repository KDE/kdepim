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

#ifndef SNIPPETITEM_H
#define SNIPPETITEM_H

#include <klistview.h>
#include <klocale.h>

#include <tqobject.h>

class TQString;
class KAction;
class SnippetGroup;


/**
This class represents one CodeSnippet-Item in the listview.
It also holds the needed data for one snippet.
@author Robert Gruber
*/
class SnippetItem : public TQObject, public TQListViewItem {
friend class SnippetGroup;

    Q_OBJECT
public:
    SnippetItem(TQListViewItem * parent, TQString name, TQString text);

    ~SnippetItem();
    TQString getName();
    TQString getText();
    using TQListViewItem::parent;
    int getParent() { return iParent; }
    void resetParent();
    void setText(TQString text);
    void setName(TQString name);
    void setAction( KAction* );
    KAction* getAction();
    static SnippetItem * findItemByName(TQString name, TQPtrList<SnippetItem> &list);
    static SnippetGroup * findGroupById(int id, TQPtrList<SnippetItem> &list);
signals:
    void execute( TQListViewItem * );
public slots:
    void slotExecute();
    
private:
  SnippetItem(TQListView * parent, TQString name, TQString text);
  TQString strName;
  TQString strText;
  int iParent;
  KAction *action;
};

/**
This class represents one group in the listview.
It is derived from SnippetItem in order to allow storing 
it in the main TQPtrList<SnippetItem>.
@author Robert Gruber
*/
class SnippetGroup : public SnippetItem {
public:
    SnippetGroup(TQListView * parent, TQString name, int id);
    ~SnippetGroup();

    int getId() { return iId; }
    static int getMaxId() { return iMaxId; }
    
    void setId(int id);

private:
    static int iMaxId;
    int iId;
};

#endif
