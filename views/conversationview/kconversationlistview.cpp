/*
 * copyright (c) Aron Bostrom <Aron.Bostrom at gmail.com>, 2006 
 *
 * this library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

ConversationListView::ConversationListView(QWidget *parent) : QListView(parent)
{
}

ConversationListView::~ConversationListView()
{
}

/*!
  \reimp
*/
void QListView::paintEvent(QPaintEvent *e)
{
    Q_D(QListView);
    QStyleOptionViewItem option = viewOptions();
    QPainter painter(d->viewport);
    QRect area = e->rect();

    // if there's nothing to do, clear the area and return
    if (!model()) {
        painter.fillRect(area, option.palette.brush(QPalette::Base));
        return;
    }

    QVector<QModelIndex> toBeRendered;
//     QVector<QRect> rects = e->region().rects();
//     for (int i = 0; i < rects.size(); ++i) {
//         d->intersectingSet(rects.at(i).translated(horizontalOffset(), verticalOffset()));
//         toBeRendered += d->intersectVector;
//     }
    d->intersectingSet(e->rect().translated(horizontalOffset(), verticalOffset()), false);
    toBeRendered = d->intersectVector;

    const QPoint offset = d->scrollDelayOffset;
    const QModelIndex current = currentIndex();
    const QModelIndex hover = d->hover;
    const QAbstractItemModel *itemModel = model();
    const QAbstractItemDelegate *delegate = itemDelegate();
    const QItemSelectionModel *selections = selectionModel();
    const bool focus = (hasFocus() || d->viewport->hasFocus()) && current.isValid();
    const bool alternate = d->alternatingColors;
    const QStyle::State state = option.state;
    const QAbstractItemView::State viewState = this->state();
    const bool enabled = (state & QStyle::State_Enabled) != 0;

    bool alternateBase = false;
    if (alternate && !toBeRendered.isEmpty()) {
        int first = toBeRendered.first().row();
        if (!d->hiddenRows.isEmpty()) {
            for (int row = 0; row < first; ++row) {
                if (!d->hiddenRows.contains(row))
                    alternateBase = !alternateBase;
            }
        } else {
            alternateBase = (first & 1) != 0;
        }
    }

    QVector<QModelIndex>::const_iterator end = toBeRendered.constEnd();
    for (QVector<QModelIndex>::const_iterator it = toBeRendered.constBegin(); it != end; ++it) {
        Q_ASSERT((*it).isValid());
        option.rect = visualRect(*it).translated(offset);
        option.state = state;
        if (selections && selections->isSelected(*it))
            option.state |= QStyle::State_Selected;
        if (enabled) {
            QPalette::ColorGroup cg;
            if ((itemModel->flags(*it) & Qt::ItemIsEnabled) == 0) {
                option.state &= ~QStyle::State_Enabled;
                cg = QPalette::Disabled;
            } else {
                cg = QPalette::Normal;
            }
            option.palette.setCurrentColorGroup(cg);
        }
        if (focus && current == *it) {
            option.state |= QStyle::State_HasFocus;
            if (viewState == EditingState)
                option.state |= QStyle::State_Editing;
        }
        if (*it == hover)
            option.state |= QStyle::State_MouseOver;
        else
            option.state &= ~QStyle::State_MouseOver;

        if (alternate) {
            QBrush fill = alternateBase
                          ? option.palette.brush(QPalette::AlternateBase)
                          : option.palette.brush(QPalette::Base);
            alternateBase = !alternateBase;
            painter.fillRect(option.rect, fill);
        }
        delegate->paint(&painter, option, *it);
    }

#ifndef QT_NO_DRAGANDDROP
    if (!d->draggedItems.isEmpty() && d->viewport->rect().contains(d->draggedItemsPos)) {
        QPoint delta = d->draggedItemsDelta();
        painter.translate(delta.x(), delta.y());
        d->drawItems(&painter, d->draggedItems);
    }

    // Paint the dropIndicator
    d_func()->paintDropIndicator(&painter);
#endif

#ifndef QT_NO_RUBBERBAND
    if (d->elasticBand.isValid()) {
        QStyleOptionRubberBand opt;
        opt.initFrom(this);
        opt.shape = QRubberBand::Rectangle;
        opt.opaque = false;
        opt.rect = d->mapToViewport(d->elasticBand).intersect(
            d->viewport->rect().adjusted(-16, -16, 16, 16));
        painter.save();
        style()->drawControl(QStyle::CE_RubberBand, &opt, &painter);
        painter.restore();
    }
#endif
}
