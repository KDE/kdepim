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
class QPushButton;
class QButtonGroup;
class QuickSearchLine : public QWidget
{
    Q_OBJECT
public:
    explicit QuickSearchLine(QWidget *parent=0);
    ~QuickSearchLine();

    enum SearchOption {
        SearchNoOption = 1,
        SearchAgainstBody = 2,
        SearchAgainstSubject = 4,
        SearchAgainstFrom = 8,
        SearchAgainstBcc = 16
    };

    Q_ENUMS(SearchOption)
    Q_DECLARE_FLAGS(SearchOptions, SearchOption)

    SearchOptions searchOptions() const;

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
    void searchOptionChanged(SearchOptions searchOption);

private slots:
    void slotLockSearchClicked(bool locked);
    void slotSearchOptionChanged();
    void slotSearchEditTextEdited(const QString &text);
    void slotClearButtonClicked();
private:
    void defaultFilterStatus();
    KLineEdit *mSearchEdit;
    KComboBox *mStatusFilterCombo;
    QToolButton *mOpenFullSearchButton;
    QToolButton *mLockSearch;
    QPushButton *mSearchAgainstBody;
    QPushButton *mSearchAgainstSubject;
    QPushButton *mSearchAgainstFrom;
    QPushButton *mSearchAgainstBcc;
    QWidget *mExtraOption;
    QButtonGroup *mButtonGroup;
    int mFirstTagInComboIndex;
};

#endif // QUICKSEARCHLINE_H
