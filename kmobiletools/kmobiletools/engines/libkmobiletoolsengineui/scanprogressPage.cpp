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
#include "scanprogressPage.h"
#include "ui_scanprogressPage.h"
#include <QProgressBar>

class ScanProgressPagePrivate {
public:
    ScanProgressPagePrivate() {}
    Ui::scanProgressPage *wizPage;
};

ScanProgressPage::ScanProgressPage( QWidget * parent )
    : QWizardPage(parent), d(new ScanProgressPagePrivate)
{
    d->wizPage=new Ui::scanProgressPage;
    d->wizPage->setupUi(this);
    setProgress(0);
}

QString ScanProgressPage::statusString() const {
    return d->wizPage->statuslabel->text();
}

void ScanProgressPage::setStatusString(const QString & text) {
    d->wizPage->statuslabel->setText(text);
}

QProgressBar *ScanProgressPage::progressBar() {
    return d->wizPage->progressBar;
}

void ScanProgressPage::setProgress(int progress) {
    progressBar()->setValue(progress);
}

int ScanProgressPage::progress() {
    return progressBar()->value();
}

void ScanProgressPage::cleanupPage() {
    kDebug() << "void ScanProgressPage::cleanupPage()\n";
    setProgress(0);
}

#include "scanprogressPage.moc"
