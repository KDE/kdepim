/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "textgotolinewidget.h"

#include <KLocalizedString>
#include <QPushButton>
#include <QIcon>

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>

using namespace PimCommon;

TextGoToLineWidget::TextGoToLineWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(2);
    setLayout(hbox);
    QToolButton *closeBtn = new QToolButton(this);
    closeBtn->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));
    closeBtn->setIconSize(QSize(16, 16));
    closeBtn->setToolTip(i18n("Close"));
    closeBtn->setObjectName(QStringLiteral("closebutton"));
#ifndef QT_NO_ACCESSIBILITY
    closeBtn->setAccessibleName(i18n("Close"));
#endif

    closeBtn->setAutoRaise(true);
    connect(closeBtn, &QToolButton::clicked, this, &TextGoToLineWidget::slotCloseBar);
    hbox->addWidget(closeBtn);

    QLabel *lab = new QLabel(i18n("Go to Line:"));
    hbox->addWidget(lab);
    mSpinbox = new QSpinBox;
    mSpinbox->setMinimum(1);
    mSpinbox->setObjectName(QStringLiteral("line"));
    connect(mSpinbox, &QSpinBox::editingFinished, this, &TextGoToLineWidget::slotGoToLine);
    hbox->addWidget(mSpinbox);
    mGoToLine = new QPushButton(QIcon::fromTheme(QStringLiteral("go-jump")), i18n("Go"));
    mGoToLine->setFlat(true);
    connect(mGoToLine, &QPushButton::clicked, this, &TextGoToLineWidget::slotGoToLine);
    mGoToLine->setObjectName(QStringLiteral("gotoline"));
    hbox->addWidget(mGoToLine);
    hbox->addStretch();
    mSpinbox->setFocus();
}

TextGoToLineWidget::~TextGoToLineWidget()
{

}

void TextGoToLineWidget::goToLine()
{
    show();
    mSpinbox->setFocus();
    mSpinbox->selectAll();
}

void TextGoToLineWidget::slotGoToLine()
{
    Q_EMIT moveToLine(mSpinbox->value());
}

void TextGoToLineWidget::showEvent(QShowEvent *e)
{
    if (!e->spontaneous()) {
        mSpinbox->setFocus();
    }
    QWidget::showEvent(e);
}

void TextGoToLineWidget::slotCloseBar()
{
    hide();
}

bool TextGoToLineWidget::event(QEvent *e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut override we can catch this before it gets to kactions.
    const bool shortCutOverride = (e->type() == QEvent::ShortcutOverride);
    if (shortCutOverride || e->type() == QEvent::KeyPress) {
        QKeyEvent *kev = static_cast<QKeyEvent * >(e);
        if (kev->key() == Qt::Key_Escape) {
            e->accept();
            slotCloseBar();
            return true;
        }
    }
    return QWidget::event(e);
}
