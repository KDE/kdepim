/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "pagecolorbackgrounddialog.h"
#include "pagecolorbackgroundwidget.h"

#include <KLocale>
#include <KSeparator>

#include <QVBoxLayout>
#include <QWebFrame>
#include <QWebElement>

namespace ComposerEditorNG {

class PageColorBackgroundDialogPrivate
{
public:
    PageColorBackgroundDialogPrivate(QWebFrame *frame, PageColorBackgroundDialog *qq)
        :webFrame(frame), q(qq)
    {
        q->setCaption( i18n( "Page Color and Background" ) );
        q->setButtons( KDialog::Ok | KDialog::Apply | KDialog::Cancel );

        QVBoxLayout *layout = new QVBoxLayout( q->mainWidget() );
        pageColorWidget = new PageColorBackgroundWidget;
        layout->addWidget(pageColorWidget);

        KSeparator *sep = new KSeparator;
        layout->addWidget( sep );

        q->connect(q, SIGNAL(okClicked()), q, SLOT(_k_slotOkClicked()));
        q->connect(q, SIGNAL(applyClicked()), q, SLOT(_k_slotApplyClicked()));
        initSettings();
    }

    void initSettings();

    void applyChanges();

    void _k_slotOkClicked();
    void _k_slotApplyClicked();

    PageColorBackgroundWidget *pageColorWidget;
    QWebFrame *webFrame;
    PageColorBackgroundDialog *q;
};

void PageColorBackgroundDialogPrivate::initSettings()
{
    if(webFrame) {
        const QWebElement element = webFrame->findFirstElement(QLatin1String("body"));
        if(!element.isNull()) {
            if(element.hasAttribute(QLatin1String("bgcolor"))||
                    element.hasAttribute(QLatin1String("text"))||
                    element.hasAttribute(QLatin1String("link"))||
                    element.hasAttribute(QLatin1String("vlink"))||
                    element.hasAttribute(QLatin1String("alink"))) {
                pageColorWidget->setUseDefaultColor(false);
                pageColorWidget->setPageBackgroundColor(QColor(element.attribute(QLatin1String("bgcolor"))));
                pageColorWidget->setTextColor(QColor(element.attribute(QLatin1String("text"))));
                pageColorWidget->setLinkColor(QColor(element.attribute(QLatin1String("link"))));
                pageColorWidget->setActiveLinkColor(QColor(element.attribute(QLatin1String("alink"))));
                pageColorWidget->setVisitedLinkColor(QColor(element.attribute(QLatin1String("vlink"))));
            } else {
                pageColorWidget->setUseDefaultColor(true);
            }
            if(element.hasAttribute(QLatin1String("background"))) {
                pageColorWidget->setBackgroundImageUrl(KUrl(element.attribute(QLatin1String("background"))));
            }
        }
    }
}

void PageColorBackgroundDialogPrivate::_k_slotOkClicked()
{
    applyChanges();
    q->accept();
}

void PageColorBackgroundDialogPrivate::_k_slotApplyClicked()
{
    applyChanges();
}

void PageColorBackgroundDialogPrivate::applyChanges()
{
    if (webFrame) {
        QWebElement element = webFrame->findFirstElement(QLatin1String("body"));
        if (!element.isNull()) {
            if (pageColorWidget->useDefaultColor()) {
                element.removeAttribute(QLatin1String("bgcolor"));
                element.removeAttribute(QLatin1String("text"));
                element.removeAttribute(QLatin1String("link"));
                element.removeAttribute(QLatin1String("alink"));
                element.removeAttribute(QLatin1String("vlink"));
            } else {
                QColor col = pageColorWidget->pageBackgroundColor();
                if(col.isValid())
                    element.setAttribute(QLatin1String("bgcolor"),col.name());
                col = pageColorWidget->textColor();
                if(col.isValid())
                    element.setAttribute(QLatin1String("text"),col.name());
                col = pageColorWidget->linkColor();
                if(col.isValid())
                    element.setAttribute(QLatin1String("link"),col.name());
                col = pageColorWidget->activeLinkColor();
                if(col.isValid())
                    element.setAttribute(QLatin1String("alink"),col.name());
                col = pageColorWidget->visitedLinkColor();
                if(col.isValid())
                    element.setAttribute(QLatin1String("vlink"),col.name());
            }
            if(pageColorWidget->backgroundImageUrl().isEmpty()) {
                element.removeAttribute(QLatin1String("background"));
            } else {
                //FIX IT
                element.setAttribute(QLatin1String("background"),QString::fromLatin1("file://%1").arg(pageColorWidget->backgroundImageUrl().path()));
            }
        }
    }
}


PageColorBackgroundDialog::PageColorBackgroundDialog(QWebFrame *frame, QWidget *parent)
    : KDialog(parent), d(new PageColorBackgroundDialogPrivate(frame, this))
{
}

PageColorBackgroundDialog::~PageColorBackgroundDialog()
{
}

}

#include "pagecolorbackgrounddialog.moc"
