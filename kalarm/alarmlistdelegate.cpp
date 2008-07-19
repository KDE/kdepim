/*
 *  alarmlistdelegate.cpp  -  handles editing and display of alarm list
 *  Program:  kalarm
 *  Copyright © 2007,2008 by David Jarvie <djarvie@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kalarm.h"

#include <QAbstractProxyModel>
#include <QMouseEvent>
#include <QApplication>

#include <kdebug.h>

#include "resources/kcalendar.h"
#include "alarmlistview.h"
#include "alarmresources.h"
#include "eventlistmodel.h"
#include "functions.h"
#include "alarmlistdelegate.moc"


void AlarmListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.isValid()  &&  index.column() == EventListModel::TimeColumn)
	{
		QString str = index.data(Qt::DisplayRole).toString();
		// Need to pad out spacing to align times without leading zeroes
		int i = str.indexOf(" ~");    // look for indicator that leading zeroes are omitted
		if (i >= 0)
		{
			QStyleOptionViewItem opt = option;
			QVariant value;
			value = index.data(Qt::ForegroundRole);
			if (value.isValid())
				opt.palette.setColor(QPalette::Text, value.value<QColor>());
			int digitWidth = opt.fontMetrics.width("0");
			QString date = str.left(i + 1);
			int w = opt.fontMetrics.width(date) + digitWidth;
			drawDisplay(painter, opt, opt.rect, date);
			QRect rect(opt.rect);
			rect.setLeft(rect.left() + w);
			drawDisplay(painter, opt, rect, str.mid(i + 2));
			return;
		}
	}
	QItemDelegate::paint(painter, option, index);
}

QSize AlarmListDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.isValid())
	{
		switch (index.column())
		{
			case EventListModel::TimeColumn:
			{
				int h = option.fontMetrics.lineSpacing();
				const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
				int w = 2 * textMargin;
				QString str = index.data(Qt::DisplayRole).toString();
				// Need to pad out spacing to align times without leading zeroes
				int i = str.indexOf(" ~");    // look for indicator that leading zeroes are omitted
				if (i >= 0)
				{
					int digitWidth = option.fontMetrics.width("0");
					QString date = str.left(i + 1);
					w += option.fontMetrics.width(date) + digitWidth + option.fontMetrics.width(str.mid(i + 2));;
				}
				else
					w += option.fontMetrics.width(str);
				return QSize(w, h);
			}
			case EventListModel::ColourColumn:
			{
				int h = option.fontMetrics.lineSpacing();
				return QSize(h * 3 / 4, h);
			}
		}
	}
	return QItemDelegate::sizeHint(option, index);
}

void AlarmListDelegate::edit(KAEvent* event, EventListView* view)
{
	KAlarm::editAlarm(event, static_cast<AlarmListView*>(view));   // edit alarm (view-only mode if archived or read-only)
}	
