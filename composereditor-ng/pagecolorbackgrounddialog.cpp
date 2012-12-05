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

#include <QVBoxLayout>
#include <QWebFrame>
#include <QWebElement>

using namespace ComposerEditorNG;

PageColorBackgroundDialog::PageColorBackgroundDialog(QWebFrame *frame, QWidget *parent) :
    KDialog(parent), mFrame(frame)
{
    setCaption( i18n( "Page Color and Background" ) );
    setButtons( Ok | Cancel );

    QVBoxLayout *layout = new QVBoxLayout( mainWidget() );
    mPageColorWidget = new PageColorBackgroundWidget;
    layout->addWidget(mPageColorWidget);
    initSettings();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOkClicked()));
}

PageColorBackgroundDialog::~PageColorBackgroundDialog()
{
}

void PageColorBackgroundDialog::slotOkClicked()
{
    if(mFrame) {
        QWebElement element = mFrame->findFirstElement(QLatin1String("body"));
        if(!element.isNull()) {
            if(mPageColorWidget->useDefaultColor()) {
                element.removeAttribute(QLatin1String("bgcolor"));
                element.removeAttribute(QLatin1String("text"));
            } else {
                element.setAttribute(QLatin1String("bgcolor"),mPageColorWidget->pageBackgroundColor().name());
                element.setAttribute(QLatin1String("text"),mPageColorWidget->textColor().name());
            }
            if(mPageColorWidget->backgroundImageUrl().isEmpty()) {
                element.removeAttribute(QLatin1String("background"));
            } else {
                //TODO
            }
        }
    }
    accept();
}

void PageColorBackgroundDialog::initSettings()
{
    if(mFrame) {
        const QWebElement element = mFrame->findFirstElement(QLatin1String("body"));
        if(!element.isNull()) {
            if(element.hasAttribute(QLatin1String("bgcolor"))||
                    element.hasAttribute(QLatin1String("text"))) {
                mPageColorWidget->setUseDefaultColor(false);
                mPageColorWidget->setPageBackgroundColor(QColor(element.attribute(QLatin1String("bgcolor"))));
                mPageColorWidget->setTextColor(QColor(element.attribute(QLatin1String("text"))));
            } else {
                mPageColorWidget->setUseDefaultColor(true);
            }
            if(element.hasAttribute(QLatin1String("background"))) {
                //mPageColorWidget->setBackgroundImageUrl();
            }
        }
    }
}



#include "pagecolorbackgrounddialog.moc"
