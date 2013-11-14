/*
    This file is part of Akregator2.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "addfeeddialog.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <kurl.h>

#include <QCheckBox>

namespace Akregator2 {

AddFeedWidget::AddFeedWidget(QWidget *parent)
   : QWidget(parent)
{
    setupUi(this);
    pixmapLabel1->setPixmap(KIconLoader::global()->loadIcon( "applications-internet",KIconLoader::Desktop,KIconLoader::SizeHuge, KIconLoader::DefaultState, QStringList(), 0, true));
    statusLabel->setText(QString());
}

AddFeedWidget::~AddFeedWidget()
{}

QSize AddFeedDialog::sizeHint() const
{
    QSize sh = KDialog::sizeHint();
    sh.setHeight(minimumSize().height());
    sh.setWidth(sh.width() * 1.2);
    return sh;
}


AddFeedDialog::AddFeedDialog(QWidget *parent)
   : KDialog(parent
     /*Qt::WStyle_DialogBorder*/)
{
    widget = new AddFeedWidget(this);
    setCaption(i18n("Add Feed"));
    setButtons(KDialog::Ok|KDialog::Cancel);
    setDefaultButton(KDialog::Ok);
    widget->urlEdit->setFocus();
    connect(widget->urlEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
    enableButtonOk(false);
    setMainWidget(widget);
}

AddFeedDialog::~AddFeedDialog()
{}

void AddFeedDialog::setUrl(const QString& t)
{
    widget->urlEdit->setText(t);
}

QString AddFeedDialog::url() const
{
    return widget->urlEdit->text();
}

void AddFeedDialog::textChanged(const QString& text)
{
    enableButtonOk(!text.isEmpty());
}

} // namespace Akregator2

