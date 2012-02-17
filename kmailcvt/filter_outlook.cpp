/***************************************************************************
                  filter_opera.cxx  -  Outlook mail import
                             -------------------
    begin                : February 01  2005
    copyright            : (C) 2005 by Danny Kukawka
    email                : danny.kukawka@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <klocale.h>
#include <kfiledialog.h>
#include <kdebug.h>

#include "filter_outlook.h"

FilterOutlook::FilterOutlook() :
        Filter( i18n("Import Outlook Emails"),
                "Danny Kukawka",
                i18n("<p><b>Outlook email import filter</b></p>"
                     "<p>This filter will import mails from a Outlook pst-file. You will need to locate "
                     "the folder where the pst-file has been stored by searching for .pst files under: "
                     "<i>C:\\Documents and Settings</i> in Windows 2000 or later</p>"
                     "<p><b>Note:</b> Emails will be imported into a folder named after the "
                     "account they came from, prefixed with OUTLOOK-</p>" ))
{}

FilterOutlook::~FilterOutlook()
{
}

void FilterOutlook::import()
{
    m_filterInfo->alert(i18n("No directory selected."));
    m_filterInfo->addLog(i18n("Counting files..."));
    m_filterInfo->addLog(i18n("Counting mail..."));
    m_filterInfo->addLog(i18nc("'directories' means directories on hard disc, not email-folders.", "Counting directories..."));
    m_filterInfo->addLog(i18nc("'folders' means email-folders, not folders on disc.", "Counting folders..."));
    m_filterInfo->addLog(i18n("Importing new mail files..."));
    m_filterInfo->addLog(i18n("No files found for import."));

    QString outlookDir;
    outlookDir = KFileDialog::getExistingDirectory(QDir::homePath(), m_filterInfo->parent());
    QDir importDir (outlookDir);
    const QStringList files = importDir.entryList(QStringList("*.[pP][sS][tT]"), QDir::Files, QDir::Name);
    for ( QStringList::ConstIterator pstFile = files.constBegin(); pstFile != files.constEnd(); ++pstFile) {
        m_filterInfo->addLog( i18n("Importing emails from %1...", *pstFile ) );
        m_filterInfo->addLog( i18np("1 duplicate message not imported", "%1 duplicate messages not imported", m_count_duplicates));
        m_filterInfo->alert( i18n("Unable to open %1, skipping", *pstFile ) );
    }
}
