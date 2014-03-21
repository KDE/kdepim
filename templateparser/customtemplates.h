/*
 * Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
 * Copyright (C) 2011, 2012 Laurent Montel <montel@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TEMPLATEPARSER_CUSTOMTEMPLATES_H
#define TEMPLATEPARSER_CUSTOMTEMPLATES_H

#include "templateparser_export.h"

#include <QStyledItemDelegate>
#include <QTreeWidgetItem>
#include <QWidget>

class KActionCollection;

class Ui_CustomTemplatesBase;

namespace TemplateParser {
class CustomTemplateItem;

class TEMPLATEPARSER_EXPORT CustomTemplates : public QWidget
{
    Q_OBJECT
public:
    enum Type {
        TUniversal,
        TReply,
        TReplyAll,
        TForward
    };

public:
    explicit CustomTemplates( const QList<KActionCollection*> &actionCollection,
                              QWidget *parent = 0 );
    ~CustomTemplates();

    void load();
    void save();

signals:
    void changed();
    void templatesUpdated();

private Q_SLOTS:
    void slotInsertCommand( const QString &cmd, int adjustCursor = 0 );
    void slotTextChanged();
    void slotAddClicked();
    void slotRemoveClicked();
    void slotListSelectionChanged();
    void slotTypeActivated( int index );
    void slotShortcutChanged( const QKeySequence &newSeq );
    void slotItemChanged( QTreeWidgetItem *item, int column );
    void slotHelpLinkClicked( const QString & );
    void slotNameChanged( const QString &text );
    void slotDuplicateClicked();

private:
    bool nameAlreadyExists( const QString &str, QTreeWidgetItem *item = 0 );
    QString indexToType( int index );
    QString createUniqueName( const QString &name ) const;
    void iconFromType( CustomTemplates::Type type, CustomTemplateItem *item );

    /// These templates will be deleted when we're saving.
    QStringList mItemsToDelete;

    QPixmap mReplyPix;
    QPixmap mReplyAllPix;
    QPixmap mForwardPix;

    /// Whether or not to emit the changed() signal. This is useful to disable when loading
    /// templates, which changes the UI without user action
    bool mBlockChangeSignal;

    Ui_CustomTemplatesBase *mUi;
};

class CustomTemplateItem : public QTreeWidgetItem
{
public:
    explicit CustomTemplateItem( QTreeWidget *parent,
                                 const QString &name,
                                 const QString &content,
                                 const QKeySequence &shortcut,
                                 CustomTemplates::Type type,
                                 const QString &to,
                                 const QString &cc );
    ~CustomTemplateItem();
    void setCustomType( CustomTemplates::Type type );
    CustomTemplates::Type customType() const;

    QString to() const;
    QString cc() const;

    void setTo( const QString & );
    void setCc( const QString & );

    QString content() const;
    void setContent( const QString & );

    QKeySequence shortcut() const;
    void setShortcut( const QKeySequence & );

    QString oldName() const;
    void setOldName( const QString & );

private:
    QString mName, mContent;
    QKeySequence mShortcut;
    CustomTemplates::Type mType;
    QString mTo, mCC;
};

class CustomTemplateItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CustomTemplateItemDelegate( QObject *parent = 0 );
    ~CustomTemplateItemDelegate();
    QWidget *createEditor ( QWidget *parent, const QStyleOptionViewItem &option,
                            const QModelIndex &index ) const;

    void setModelData( QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index ) const;

};

}

#endif
