/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>
  based on code:
Copyright 2009 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef SPLITTERCOLLAPSER_H
#define SPLITTERCOLLAPSER_H

// Qt
#include <QToolButton>
#include "pimcommon_export.h"

class QSplitter;

namespace PimCommon
{
/**
 * A button which appears on the side of a splitter handle and allows easy
 * collapsing of the widget on the opposite side
 */
class PIMCOMMON_EXPORT SplitterCollapser : public QToolButton
{
    Q_OBJECT
public:
    /**
     * @brief SplitterCollapser create a splitter collapser
     * @param splitter the splitter where we put the splitter collapser
     * @param widget the widget which be associate with splitter collapser.
     * @param parent the parent widget.
     */
    explicit SplitterCollapser(QSplitter *splitter, QWidget *widget, QWidget *parent = 0);

    ~SplitterCollapser();

    /**
     * @brief isCollapsed
     * @return true if splitter is collapsed.
     */
    bool isCollapsed() const;

public Q_SLOTS:
    void collapse();
    void restore();
    void setCollapsed(bool collapsed);

private Q_SLOTS:
    void slotClicked();

protected:
    bool eventFilter(QObject *, QEvent *) Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

private:
    class Private;
    Private *const d;
};

} // namespace

#endif /* SPLITTERCOLLAPSER_H */
