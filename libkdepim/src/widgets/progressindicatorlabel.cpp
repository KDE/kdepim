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

#include "progressindicatorlabel.h"
#include "progressindicatorwidget.h"

#include <QHBoxLayout>

namespace KPIM
{
class ProgressIndicatorLabelPrivate
{
public:
    ProgressIndicatorLabelPrivate(const QString &_label, ProgressIndicatorLabel *qq)
        : labelStr(_label),
          q(qq)
    {
        QHBoxLayout *lay = new QHBoxLayout;
        lay->setMargin(0);
        q->setLayout(lay);
        indicator = new ProgressIndicatorWidget;
        lay->addWidget(indicator);
        label = new QLabel;
        lay->addWidget(label);
    }

    ~ProgressIndicatorLabelPrivate()
    {
    }

    void setActiveLabel(const QString &str)
    {
        if (indicator->isActive()) {
            label->setText(str);
        }
    }

    void start()
    {
        indicator->start();
        label->setText(labelStr);
    }

    void stop()
    {
        indicator->stop();
        label->clear();
    }

    QString labelStr;
    QLabel *label;
    ProgressIndicatorWidget *indicator;
    ProgressIndicatorLabel *q;
};

ProgressIndicatorLabel::ProgressIndicatorLabel(const QString &label, QWidget *parent)
    : QWidget(parent),
      d(new ProgressIndicatorLabelPrivate(label, this))
{
}

ProgressIndicatorLabel::ProgressIndicatorLabel(QWidget *parent)
    : QWidget(parent),
      d(new ProgressIndicatorLabelPrivate(QString(), this))
{
}

ProgressIndicatorLabel::~ProgressIndicatorLabel()
{
    delete d;
}

void ProgressIndicatorLabel::start()
{
    d->start();
}

void ProgressIndicatorLabel::stop()
{
    d->stop();
}

void ProgressIndicatorLabel::setActiveLabel(const QString &label)
{
    d->setActiveLabel(label);
}

}

