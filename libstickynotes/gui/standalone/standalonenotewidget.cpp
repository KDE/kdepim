/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. <gamaral@amaral.com.mx>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#include "standalonenotewidget.h"

#include "standalonenoteeditor.h"
#include "standalonenoteitem.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QLabel>
#include <QMouseEvent>

using namespace StickyNotes;

namespace StickyNotes {

/* StandaloneNoteWidgetPrivate */

class StandaloneNoteWidgetPrivate
{
Q_DECLARE_PUBLIC(StandaloneNoteWidget)

public:
	StandaloneNoteWidgetPrivate(StandaloneNoteWidget *_q);

private:
	void setupUi(void);

private:
	StandaloneNoteWidget *q_ptr;
	bool   _dragging;
	QPoint _dragLastPos;
	bool   _readonly;
	bool   _stretching;
	QLabel *content;
	StandaloneNoteItem *item;
	QLabel *subject;
};

} // namespace StickyNotes

static float d_scaleFactor = 1.00;

StandaloneNoteWidgetPrivate::StandaloneNoteWidgetPrivate(StandaloneNoteWidget *_q)
: q_ptr(_q), _dragging(false), _readonly(false), _stretching(false),
    content(0), item(0), subject(0)
{
	setupUi();
}

void
StandaloneNoteWidgetPrivate::setupUi(void)
{
	QSvgWidget *window;
	QBoxLayout *wlayout;

	window = q_func();
	window->load(QString(":/data/standalonenotewidget/background.svg"));

	wlayout = new QBoxLayout(QBoxLayout::TopToBottom, window);
	subject = new QLabel(window);
	content = new QLabel(window);

	content->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	content->setScaledContents(true);
	content->setTextInteractionFlags(Qt::NoTextInteraction);
	{
		QPalette cp = content->palette();

		cp.setColor(QPalette::Active, QPalette::WindowText,
		    QColor(55,55,0));
		cp.setColor(QPalette::Inactive, QPalette::WindowText,
		    QColor(105,105,4));

		content->setPalette(cp);
	}

	subject->setAlignment(Qt::AlignRight | Qt::AlignBottom);
	subject->setTextFormat(Qt::PlainText);
	subject->setTextInteractionFlags(Qt::NoTextInteraction);
	{
		QFont cf    = subject->font();
		QPalette cp = subject->palette();

		cf.setPointSize(cf.pointSize() - 2);
		cp.setColor(QPalette::Active, QPalette::WindowText,
		    QColor(105,105,4));
		cp.setColor(QPalette::Inactive, QPalette::WindowText,
		    QColor(185,185,84));

		subject->setFont(cf);
		subject->setPalette(cp);
	}

	wlayout->setContentsMargins(9,9,9,9);
	wlayout->setSpacing(6);

	wlayout->addWidget(subject, 26);
	wlayout->addWidget(content, 223);
}

/* StandaloneNoteWidget */

StandaloneNoteWidget::StandaloneNoteWidget(StandaloneNoteItem &_item,
    QWidget *_parent, Qt::WindowFlags _f)
: QSvgWidget(_parent), d_ptr(new StandaloneNoteWidgetPrivate(this))
{
	Q_D(StandaloneNoteWidget);

	d->item = &_item;

	connect(d->item, SIGNAL(appliedAttribute(const QString &, const QVariant &)),
	    this, SLOT(on_item_appliedAttribute(const QString &, const QVariant &)));
	connect(d->item, SIGNAL(appliedContent(const QString &)),
	    this, SLOT(on_item_appliedContent(const QString &)));
	connect(d->item, SIGNAL(appliedSubject(const QString &)),
	    this, SLOT(on_item_appliedSubject(const QString &)));
	connect(d->item, SIGNAL(bound(void)),
	    this, SLOT(on_item_bound(void)));

	// TODO: Play with later on
	// setAttribute(Qt::WA_TranslucentBackground);
	setWindowFlags(_f | Qt::FramelessWindowHint);

	// TODO: Check meta-data for predefined positions and dimensions.
	scaleToScreen();

	// Listen to mouse move events
	d->subject->installEventFilter(this);
	d->content->installEventFilter(this);
}

StandaloneNoteWidget::~StandaloneNoteWidget(void)
{
	delete d_ptr;
}

bool
StandaloneNoteWidget::eventFilter(QObject *_watched, QEvent *_event)
{
	Q_D(StandaloneNoteWidget);
	QMouseEvent *me;

	if ((me = dynamic_cast<QMouseEvent *>(_event))) {
			switch (me->type()) {
				case QEvent::MouseButtonPress:
					if (Qt::LeftButton == me->button())
						d->_dragging = true;
					else if (Qt::RightButton == me->button())
						d->_stretching = true;

					d->_dragLastPos	= me->globalPos();
					break;
				case QEvent::MouseButtonRelease:
					d->_dragging   = false;
					d->_stretching = false;
					break;
				case QEvent::MouseMove:
					if (d->_dragging) {
						move(pos() + (me->globalPos() - d->_dragLastPos));
						d->_dragLastPos	= me->globalPos();
					} else if (d->_stretching) {
						QSize csize = size();
						QPoint cdiff = me->globalPos()
						     - d->_dragLastPos;

						csize.rheight() += 
						    ((cdiff.rx() + cdiff.ry()) / 2);
						csize.rwidth() = 
						    csize.rheight() / d_scaleFactor;

						resize(csize);

						d->_dragLastPos	= me->globalPos();
					}
					break;
				case QEvent::MouseButtonDblClick:
					if (!d->_readonly)
						d->item->edit();
					break;
				default: break;
			}
	} else {
		return (QWidget::eventFilter(_watched, _event));
	}

	return (false);
}

void
StandaloneNoteWidget::scaleToScreen(void)
{
	QRect dag(QApplication::desktop()->availableGeometry(this));
	int h;
	int w;

	h = (dag.height() / 4.5); // Make note 4 1/2 the height of the screen.
	w = (h / d_scaleFactor);  // Make the weight a frac (d_scaleFactor) of the height.

	resize(w, h);
}

void
StandaloneNoteWidget::on_item_appliedAttribute(const QString &_name, const QVariant &_value)
{
	if (0 == _name.compare("hidden")) {
		setVisible(!_value.toBool());
	} else if (0 == _name.compare("readonly")) {
		d_func()->_readonly = _value.toBool();
	}
}

void
StandaloneNoteWidget::on_item_appliedContent(const QString &_content)
{
	d_func()->content->setText(_content);
}

void
StandaloneNoteWidget::on_item_appliedSubject(const QString &_subject)
{
	d_func()->subject->setText(_subject);
}

void
StandaloneNoteWidget::on_item_bound(void)
{
	Q_D(StandaloneNoteWidget);

	// set read-only state
	d->_readonly = d->item->attribute("readonly").toBool();

	// set content
	d->subject->setText(d->item->subject());
	d->content->setText(d->item->content());

	// set visibility
	setVisible(!d->item->attribute("hidden").toBool());
}

#include "include/standalonenotewidget.moc"

