/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef QUICKSEARCHLINE_H
#define QUICKSEARCHLINE_H

#include <QWidget>
class KLineEdit;
class KComboBox;
class QToolButton;

class QuickSearchLine : public QWidget
{
    Q_OBJECT
public:
    explicit QuickSearchLine(QWidget *parent=0);
    ~QuickSearchLine();

    void focusQuickSearch();

    KComboBox *statusFilterComboBox() const;
    KLineEdit *searchEdit() const;
    QToolButton *openFullSearchButton() const;
    QToolButton *lockSearch() const;
    int firstTagInComboIndex() const;

Q_SIGNALS:
    void fullSearchRequest();
    void clearButtonClicked();
    void searchEditTextEdited(const QString &);

private slots:
    void slotLockSearchClicked(bool locked);

private:
    void defaultFilterStatus();
    KLineEdit *mSearchEdit;
    KComboBox *mStatusFilterCombo;
    QToolButton *mOpenFullSearchButton;
    QToolButton *mLockSearch;
    int mFirstTagInComboIndex;
};

#endif // QUICKSEARCHLINE_H
