/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "composerimageresizetooltip.h"

#include <KLocale>

#include <QHBoxLayout>
#include <QLabel>

namespace ComposerEditorNG
{

class ComposerImageResizeToolTipPrivate
{
public:
    ComposerImageResizeToolTipPrivate(ComposerImageResizeToolTip *qq)
        : q(qq)
    {
        QHBoxLayout *lay = new QHBoxLayout;
        lay->setMargin(0);
        lay->setSpacing(0);
        lab = new QLabel;
        lay->addWidget(lab);
        q->setLayout(lay);
    }
    void displaySize(const QSize& s);

    QLabel *lab;
    ComposerImageResizeToolTip *q;
};

void ComposerImageResizeToolTipPrivate::displaySize(const QSize& s)
{
    lab->setText( i18n("%1x%2").arg(s.width()).arg(s.height()));
}

ComposerImageResizeToolTip::ComposerImageResizeToolTip(QWidget *parent)
    : QWidget(parent), d(new ComposerImageResizeToolTipPrivate(this))
{
    setWindowFlags(Qt::ToolTip);
}

ComposerImageResizeToolTip::~ComposerImageResizeToolTip()
{
    delete d;
}

void ComposerImageResizeToolTip::displaySize(const QSize &s)
{
    d->displaySize(s);
}

}

#include "composerimageresizetooltip.moc"
