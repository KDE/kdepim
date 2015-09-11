/* Copyright (C) 2010 Torgny Nyblom <nyblom@kde.org>
 * Copyright (C) 2010-2015 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef FINDBARBASE_H
#define FINDBARBASE_H

#include <QWidget>

class QAction;
class QPushButton;
class QMenu;
class QLabel;
namespace PimCommon
{
class LineEditWithCompleter;
}
namespace MessageViewer
{
class FindBarBase : public QWidget
{
    Q_OBJECT
public:
    explicit FindBarBase(QWidget *parent = Q_NULLPTR);
    virtual ~FindBarBase();

    QString text() const;
    void setText(const QString &text);
    void focusAndSetCursor();

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;
    virtual void clearSelections();
    virtual void updateHighLight(bool);
    virtual void searchText(bool backward, bool isAutoSearch);
    virtual void updateSensitivity(bool);

    void setFoundMatch(bool match);
    QMenu *optionsMenu();

public Q_SLOTS:
    void findNext();
    void findPrev();
    void autoSearch(const QString &str);
    void slotSearchText(bool backward = false, bool isAutoSearch = true);
    void closeBar();

Q_SIGNALS:
    void hideFindBar();

protected Q_SLOTS:
    void caseSensitivityChanged(bool);
    void slotHighlightAllChanged(bool);
    void slotClearSearch();

protected:
    QString mNotFoundString;
    QString mPositiveBackground;
    QString mNegativeBackground;
    QString mLastSearchStr;
    PimCommon::LineEditWithCompleter *mSearch;
    QAction *mCaseSensitiveAct;

    QPushButton *mFindPrevBtn;
    QPushButton *mFindNextBtn;
    QMenu *mOptionsMenu;
    QLabel *mStatus;
    void addToCompletion(const QString &text);
};

}

#endif /* FINDBARBASE_H */

