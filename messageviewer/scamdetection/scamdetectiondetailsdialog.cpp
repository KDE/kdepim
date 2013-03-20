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

#include "scamdetectiondetailsdialog.h"

#include <KLocale>

#include <QTextEdit>

using namespace MessageViewer;

ScamDetectionDetailsDialog::ScamDetectionDetailsDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Details") );
    setAttribute( Qt::WA_DeleteOnClose );
    setButtons( Close );
    setModal( false );
    mDetails = new QTextEdit;
    mDetails->setReadOnly(true);
    mDetails->setAcceptRichText(false);
    setMainWidget(mDetails);
    resize(600,400);
}

ScamDetectionDetailsDialog::~ScamDetectionDetailsDialog()
{
}

void ScamDetectionDetailsDialog::setDetails(const QString &details)
{
    mDetails->setPlainText(details);
}

#include "scamdetectiondetailsdialog.moc"
