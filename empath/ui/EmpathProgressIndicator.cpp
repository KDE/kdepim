/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Qt includes
#include <qlayout.h>
#include <qlabel.h>
#include <qwidgetstack.h>

// KDE includes
#include <kprogress.h>

// Local includes
#include "EmpathProgressIndicator.h"
#include "EmpathTask.h"

EmpathProgressIndicator::EmpathProgressIndicator
    (EmpathTask * t, QWidgetStack * parent)
    :   QWidget(parent, "ProgressIndicator")
{
    parent->addWidget(this, (int)this);

    QHBoxLayout * layout = new QHBoxLayout(this, 0, 6);

    progress_ =
        new KProgress(t->pos(), t->max(), 0, KProgress::Horizontal, this);

    progress_->setFixedWidth(120);

    QLabel * l = new QLabel(t->name(), this);

    layout->addWidget(progress_);
    layout->addWidget(l);
    layout->addStretch(10);

    QObject::connect(
        t,          SIGNAL(posChanged(int)),
        progress_,  SLOT(setValue(int))
    );

    QObject::connect(
        t,          SIGNAL(maxChanged(int)),
        SLOT(s_setMaxValue(int))
    );

    QObject::connect(
        t,          SIGNAL(addOne()),
        this,       SLOT(s_incValue())
    );

    QObject::connect(
        t,          SIGNAL(finished()),
        this,       SLOT(s_finished())
    );

    show();
}

EmpathProgressIndicator::~EmpathProgressIndicator()
{
    // Empty.
}

    void
EmpathProgressIndicator::s_setMaxValue(int v)
{
    progress_->setRange(progress_->minValue(), v);
}

    void
EmpathProgressIndicator::s_incValue()
{
    progress_->advance(1);
}

    void
EmpathProgressIndicator::s_finished()
{
    delete this;
}


