/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef TEXTFINDREPLACEWIDGET_H
#define TEXTFINDREPLACEWIDGET_H

#include <QWidget>
#include <QTextDocument>

class QAction;
class QLineEdit;
class QPushButton;
namespace PimCommon
{

class TextFindWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TextFindWidget(QWidget *parent = 0);
    ~TextFindWidget();

    QTextDocument::FindFlags searchOptions() const;

    QLineEdit *search() const;

    void setFoundMatch(bool match);
    QRegExp findRegExp() const;

private Q_SLOTS:
    void slotAutoSearch(const QString &str);

Q_SIGNALS:
    void findNext();
    void findPrev();
    void clearSearch();
    void autoSearch(const QString &);
    void updateSearchOptions();
    void searchStringEmpty(bool);

private:
    QLineEdit *mSearch;
    QAction *mCaseSensitiveAct;
    QAction *mWholeWordAct;

    QPushButton *mFindPrevBtn;
    QPushButton *mFindNextBtn;
};

class TextReplaceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TextReplaceWidget(QWidget *parent = 0);
    ~TextReplaceWidget();

    QLineEdit *replace() const;

public Q_SLOTS:
    void slotSearchStringEmpty(bool);

Q_SIGNALS:
    void replaceText();
    void replaceAllText();

private:
    QLineEdit *mReplace;
    QPushButton *mReplaceBtn;
    QPushButton *mReplaceAllBtn;
};
}
#endif // TEXTFINDREPLACEWIDGET_H
