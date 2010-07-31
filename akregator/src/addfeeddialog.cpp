/*
    This file is part of Akregator.

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

#include "feed.h"
#include "addfeeddialog.h"

#include <tqcheckbox.h>

#include <kapplication.h>
#include <kurl.h>
#include <klocale.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <kdebug.h>
#include <ksqueezedtextlabel.h>
#include <kmessagebox.h>

namespace Akregator {

AddFeedWidget::AddFeedWidget(TQWidget *parent, const char *name)
   : AddFeedWidgetBase(parent, name)
{
    pixmapLabel1->setPixmap(kapp->iconLoader()->loadIcon( "package_network",KIcon::Desktop,KIcon::SizeHuge, KIcon::DefaultState, 0, true));
    statusLabel->setText(TQString::null);
}

AddFeedWidget::~AddFeedWidget()
{}

AddFeedDialog::AddFeedDialog(TQWidget *parent, const char *name)
   : KDialogBase(KDialogBase::Swallow, Qt::WStyle_DialogBorder, parent, name, true, i18n("Add Feed"), KDialogBase::Ok|KDialogBase::Cancel)
{
    widget = new AddFeedWidget(this);
    connect(widget->urlEdit, TQT_SIGNAL(textChanged(const TQString&)), this, TQT_SLOT(textChanged(const TQString&)));
    enableButtonOK(false);
    setMainWidget(widget);
}

AddFeedDialog::~AddFeedDialog()
{}

void AddFeedDialog::setURL(const TQString& t)
{
    widget->urlEdit->setText(t);
}

void AddFeedDialog::slotOk( )
{
    enableButtonOK(false);
    feedURL = widget->urlEdit->text().stripWhiteSpace();

    Feed *f=new Feed();

    feed=f;

    // HACK: make weird wordpress links ("feed:http://foobar/rss") work
    if (feedURL.startsWith("feed:"))
        feedURL = feedURL.right( feedURL.length() - 5 );

    if (feedURL.find(":/") == -1)
        feedURL.prepend("http://");
    f->setXmlUrl(feedURL);

    widget->statusLabel->setText( i18n("Downloading %1").arg(feedURL) );

    connect( feed, TQT_SIGNAL(fetched(Feed* )),
             this, TQT_SLOT(fetchCompleted(Feed *)) );
    connect( feed, TQT_SIGNAL(fetchError(Feed* )),
             this, TQT_SLOT(fetchError(Feed *)) );
    connect( feed, TQT_SIGNAL(fetchDiscovery(Feed* )),
             this, TQT_SLOT(fetchDiscovery(Feed *)) );

    f->fetch(true);
}

void AddFeedDialog::fetchCompleted(Feed */*f*/)
{
    KDialogBase::slotOk();
}

void AddFeedDialog::fetchError(Feed *)
{
    KMessageBox::error(this, i18n("Feed not found from %1.").arg(feedURL));
    KDialogBase::slotCancel();
}

void AddFeedDialog::fetchDiscovery(Feed *f)
{
    widget->statusLabel->setText( i18n("Feed found, downloading...") );
    feedURL=f->xmlUrl();
}

void AddFeedDialog::textChanged(const TQString& text)
{
    enableButtonOK(!text.isEmpty());
}

} // namespace Akregator

#include "addfeeddialog.moc"
// vim: ts=4 sw=4 et
