/***************************************************************************
            filter_kmail_maildir.h  -  Kmail maildir mail import
                             -------------------
    begin                : April 06 2005
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
/* Copyright (c) 2012 Montel Laurent <montel@kde.org>                      */

#ifndef MAILIMPORTER_FILTER_KMAIL_MAILDIR_HXX
#define MAILIMPORTER_FILTER_KMAIL_MAILDIR_HXX

#include "filters.h"
/**
 * Imports KMail mail folder with maildir format recursively, recreating the folder structure.
 * @author Danny Kukawka
 */
namespace MailImporter {
class MAILIMPORTER_EXPORT FilterKMail_maildir : public Filter
{
public:
    explicit FilterKMail_maildir();
    ~FilterKMail_maildir();

    void import();
    void importMails( const QString& maildir );

private:
    void processDirectory( const QString& path);

    void importDirContents(const QString&);
    void importFiles(const QString&);
    int mImportDirDone;
    int mTotalDir;
};
}

#endif
