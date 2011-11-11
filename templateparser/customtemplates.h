/*   -*- mode: C++; c-file-style: "gnu" -*-
 *   kmail: KDE mail client
 *   Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
 *   Copyright (C) 2011 Laurent Montel <montel@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#ifndef CUSTOMTEMPLATES_H
#define CUSTOMTEMPLATES_H

#include <QHash>
#include <QPixmap>
#include <QWidget>
#include <QItemDelegate>
#include <QTreeWidgetItem>
#include <KShortcut>

#include "templateparser_export.h"
class KActionCollection;

class Ui_CustomTemplatesBase;

class TEMPLATEPARSER_EXPORT CustomTemplates : public QWidget
{
  Q_OBJECT

  public:

    enum Type { TUniversal, TReply, TReplyAll, TForward };

  public:

    explicit CustomTemplates( const QList<KActionCollection*>& actionCollection, QWidget *parent = 0 );
    ~CustomTemplates();

    void load();
    void save();

    QString indexToType( int index );

  public slots:
    void slotInsertCommand( const QString &cmd, int adjustCursor = 0 );
    void slotTextChanged();
    void slotAddClicked();
    void slotRemoveClicked();
    void slotListSelectionChanged();
    void slotTypeActivated( int index );
    void slotShortcutChanged( const QKeySequence &newSeq );

  signals:
    void changed();
    void templatesUpdated();

  protected:
    /// These templates will be deleted when we're saving.
    QStringList mItemsToDelete;

    QPixmap mReplyPix;
    QPixmap mReplyAllPix;
    QPixmap mForwardPix;

  private slots:

    void slotHelpLinkClicked( const QString& );
    void slotNameChanged( const QString & text );

  private:

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
                               KShortcut &shortcut,
                               CustomTemplates::Type type,
                               const QString& to,
                               const QString& cc );
  ~CustomTemplateItem();
  void setCustomType( CustomTemplates::Type type );
  CustomTemplates::Type customType() const;

  QString to() const;
  QString cc() const;

  void setTo(const QString&);
  void setCc(const QString&);

  QString content() const;
  void setContent(const QString&);

  KShortcut shortcut() const;
  void setShortcut(const KShortcut&);

  QString name() const;
  void setName(const QString&);
  
private:
  QString mName, mContent;
  //QKeySequence might do the trick too, check the rest of your code. --ahartmetz
  KShortcut mShortcut;
  CustomTemplates::Type mType;
  QString mTo, mCC;
};

class CustomTemplateItemDelegate : public QItemDelegate
{
  Q_OBJECT
  
public:
  explicit CustomTemplateItemDelegate(QObject *parent = 0);
  ~CustomTemplateItemDelegate();
  
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;
};

#endif // CUSTOMTEMPLATES_H
