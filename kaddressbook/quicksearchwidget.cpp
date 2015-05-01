/*
  This file is part of KAddressBook.

  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "quicksearchwidget.h"

#include <QLineEdit>
#include <KLocalizedString>

#include <QtCore/QTimer>
#include <QKeyEvent>
#include <QVBoxLayout>

QuickSearchWidget::QuickSearchWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    mEdit = new QLineEdit;
    //If change shortcut changes it in mainwidget
    mEdit->setClearButtonEnabled(true);
    mEdit->setToolTip(
        i18nc("@info:tooltip", "Search contacts in list"));
    mEdit->setWhatsThis(
        i18nc("@info:whatsthis",
              "Start typing a search string in this box and the list of contacts "
              "matching that string will be displayed.  This is a quick way of searching "
              "for contacts of interest."));
    mEdit->installEventFilter(this);

    layout->addWidget(mEdit);

    mTimer = new QTimer(this);

    connect(mEdit, &QLineEdit::textChanged, this, &QuickSearchWidget::resetTimer);
    connect(mTimer, &QTimer::timeout, this, &QuickSearchWidget::delayedTextChanged);
}

QuickSearchWidget::~QuickSearchWidget()
{
}

QSize QuickSearchWidget::sizeHint() const
{
    const QSize size = mEdit->sizeHint();
    return QSize(200, size.height());
}

void QuickSearchWidget::slotFocusQuickSearch()
{
    mEdit->setFocus();
}

void QuickSearchWidget::resetTimer()
{
    mTimer->stop();
    mTimer->start(500);
}

void QuickSearchWidget::delayedTextChanged()
{
    mTimer->stop();
    Q_EMIT filterStringChanged(mEdit->text());
}

void QuickSearchWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Down) {
        event->accept();
        delayedTextChanged();
        Q_EMIT arrowDownKeyPressed();
        return;
    }

    QWidget::keyPressEvent(event);
}

void QuickSearchWidget::updateQuickSearchText(const QString &text)
{
    mEdit->setPlaceholderText(text);
}
