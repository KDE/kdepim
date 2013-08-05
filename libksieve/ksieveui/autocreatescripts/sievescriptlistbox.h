/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SIEVESCRIPTLISTBOX_H
#define SIEVESCRIPTLISTBOX_H

#include <QGroupBox>
#include <QListWidgetItem>
class QListWidget;
class QDomDocument;
class QDomNode;
class KPushButton;

namespace KSieveUi {
class SieveScriptPage;
class SieveScriptListItem : public QListWidgetItem
{
public:
    SieveScriptListItem( const QString &text, QListWidget *parent );
    ~SieveScriptListItem();

    void setDescription(const QString & desc);
    QString description() const;

    SieveScriptPage *scriptPage() const;
    void setScriptPage(SieveScriptPage *page);

    QString generatedScript(QStringList &requires) const;

private:
    QString mDescription;
    SieveScriptPage *mScriptPage;
};

class SieveScriptListBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit SieveScriptListBox(const QString &title, QWidget *parent = 0);
    ~SieveScriptListBox();

    QString generatedScript(QString &requires) const;
    void loadScript(const QDomDocument &doc);

Q_SIGNALS:
    void addNewPage(QWidget *);
    void removePage(QWidget *);
    void activatePage(QWidget *);
    void enableButtonOk(bool);

private Q_SLOTS:
    void slotNew();
    void slotDelete();
    void slotRename();
    void updateButtons();
    void slotEditDescription();
    void slotItemActived(QListWidgetItem*);
    void slotTop();
    void slotBottom();
    void slotDown();
    void slotUp();

private:
    enum ParseSieveScriptTypeBlock {
        TypeUnknown = 0,
        TypeBlockIf,
        TypeBlockElsif,
        TypeBlockElse,
        TypeBlockInclude,
        TypeBlockGlobal,
        TypeBlockAction,
        TypeBlockForeachBlock
    };

    void loadBlock(QDomNode &n);
    void clear();
    SieveScriptPage *createNewScript(const QString &newName, const QString &description = QString());
    QString createUniqName();
    QListWidget *mSieveListScript;
    KPushButton *mBtnNew;
    KPushButton *mBtnDelete;
    KPushButton *mBtnRename;
    KPushButton *mBtnDescription;
    KPushButton *mBtnTop;
    KPushButton *mBtnUp;
    KPushButton *mBtnDown;
    KPushButton *mBtnBottom;
    int mScriptNumber;
};
}

#endif // SIEVESCRIPTLISTBOX_H
