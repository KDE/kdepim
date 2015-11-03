/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "multilineedit.h"
#include <QStyleOptionFrameV2>
#include <QApplication>

using namespace KSieveUi;

MultiLineEdit::MultiLineEdit(QWidget *parent)
    : KPIMTextEdit::PlainTextEditor(parent)
{
    setSearchSupport(false);
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
    connect(this, &MultiLineEdit::textChanged, this, &MultiLineEdit::valueChanged);
}

MultiLineEdit::~MultiLineEdit()
{

}

QSize MultiLineEdit::sizeHint() const
{
    QFontMetrics fm(font());

    const int h = qMax(document()->size().toSize().height() - fm.descent() + 2 * frameWidth(), 50);

    QStyleOptionFrameV2 opt;
    opt.initFrom(this);
    opt.rect = QRect(0, 0, 100, h);
    opt.lineWidth = lineWidth();
    opt.midLineWidth = 0;
    opt.state |= QStyle::State_Sunken;

    QSize s = style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(100, h).expandedTo(QApplication::globalStrut()), this);

    return s;
}

QSize MultiLineEdit::minimumSizeHint() const
{
    return sizeHint();
}

