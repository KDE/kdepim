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

#ifndef MAILCOMMON_SNIPPETDIALOG_P_H
#define MAILCOMMON_SNIPPETDIALOG_P_H

#include <kdialog.h>

namespace Ui {
class SnippetDialog;
}

class KActionCollection;

class QAbstractItemModel;
class QModelIndex;

class SnippetDialog : public KDialog
{
    Q_OBJECT

public:
    explicit SnippetDialog( KActionCollection *actionCollection, bool inGroupMode, QWidget *parent = 0 );
    ~SnippetDialog();

    void setName( const QString &name );
    QString name() const;

    void setText( const QString &text );
    QString text() const;

    void setKeySequence( const QKeySequence &sequence );
    QKeySequence keySequence() const;

    void setGroupModel( QAbstractItemModel *model );

    void setGroupIndex( const QModelIndex &index );
    QModelIndex groupIndex() const;

private Q_SLOTS:
    void slotTextChanged();
    void slotGroupChanged();

private:
    bool snippetIsValid() const;

    KActionCollection *mActionCollection;
    Ui::SnippetDialog *mUi;
};

#endif
