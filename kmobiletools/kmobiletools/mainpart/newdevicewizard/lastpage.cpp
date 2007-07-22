/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "lastpage.h"

#include <libkmobiletools/engineslist.h>
#include <libkmobiletools/engine.h>
#include <libkmobiletools/devicesconfig.h>

#include <kiconloader.h>
#include <kdebug.h>

class LastPagePrivate {
    public:
        LastPagePrivate() {};
};

LastPage::LastPage(QWidget *parent)
    : QWizardPage(parent)
{
    d=new LastPagePrivate;
    setupUi(this);
}

void LastPage::initializePage()
{
    summaryTXT->setHtml(getHTMLTemplate() );
}

bool LastPage::validatePage()
{
    KMobileTools::EnginesList::instance()->setWizardEngine();
    return QWizardPage::validatePage();
}

QString LastPage::getHTMLTemplate() const
{
    QString ret="<div style=\"padding-left : 2em;\"><h2><img src=\"%1\" style=\"vertical-align : middle;\"> %5</h2></div><br> \
            <p><b>%2</b><br>%6<br>%7</p> \
            <p><b>%3</b><br>%8</p> \
            <p><b>%4</b><br>%9</p> \
            ";
    ret=ret.arg( KIconLoader::global()->iconPath("kmobiletools", -K3Icon::SizeHuge),
                i18nc("Wizard summary - Technical Information", "Technical Information"),
                i18nc("Wizard summary - Basic Phone Information", "Basic Phone Information"),
                i18nc("Wizard summary - Advanced Phone Information", "Advanced Phone Information")
               );
    ret=ret.arg( field("phonename").toString(),
                i18n("Engine: %1", KMobileTools::EnginesList::instance()->wizardEngine()->pluginInfo().name() )
               );
    return KMobileTools::EnginesList::instance()->wizardEngine()->parseWizardSummary(ret, wizard()->objectName() );
}

#include "lastpage.moc"
