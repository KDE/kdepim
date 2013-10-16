/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "imagescaling_gui.h"
#include "messagecomposer/imagescaling/imagescalingwidget.h"

#include <kapplication.h>
#include <KCmdLineArgs>
#include <KLocale>


#include <QPointer>
#include <QVBoxLayout>


ImageScalingTestWidget::ImageScalingTestWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->addWidget(new MessageComposer::ImageScalingWidget);
    setLayout(lay);
}

ImageScalingTestWidget::~ImageScalingTestWidget()
{
}

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "imagescaling_gui", 0, ki18n("ImageScalingTest_Gui"),
                       "1.0", ki18n("Test for imagescaling widget"));
    KApplication app;

    ImageScalingTestWidget *w = new ImageScalingTestWidget();
    w->resize(800,600);

    w->show();
    app.exec();
    delete w;
    return 0;
}

