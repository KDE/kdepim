/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef KNOTEFINDDIALOG_H
#define KNOTEFINDDIALOG_H

#include <QDialog>
#include <AkonadiCore/Item>
#include "knotes_export.h"
class QPushButton;
class QLineEdit;
class QListWidgetItem;
class QListWidget;
class QLabel;
namespace NoteShared
{
class NoteListWidget;
}
class KNoteFindWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KNoteFindWidget(QWidget *parent = Q_NULLPTR);
    ~KNoteFindWidget();

    void setExistingNotes(const QHash<Akonadi::Item::Id, Akonadi::Item> &notes);

Q_SIGNALS:
    void noteSelected(Akonadi::Item::Id);

private Q_SLOTS:
    void slotTextChanged(const QString &);
    void slotSearchNote();
    void slotItemDoubleClicked(QListWidgetItem *);

private:
    QHash<Akonadi::Item::Id, Akonadi::Item> mNotes;
    QLabel *mResultSearch;
    QLineEdit *mSearchLineEdit;
    QPushButton *mSearchButton;
    NoteShared::NoteListWidget *mNoteList;
};

class KNOTES_EXPORT KNoteFindDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KNoteFindDialog(QWidget *parent = Q_NULLPTR);
    ~KNoteFindDialog();
    void setExistingNotes(const QHash<Akonadi::Item::Id, Akonadi::Item> &notes);

Q_SIGNALS:
    void noteSelected(Akonadi::Item::Id);

private:
    void writeConfig();
    void readConfig();
    KNoteFindWidget *mNoteFindWidget;
};

#endif // KNOTEFINDDIALOG_H
