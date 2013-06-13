/***************************************************************************
            filter_thunderbird.h  -  Thunderbird mail import
                             -------------------
    begin                : Januar 26 2005
    copyright            : (C) 2005 by Danny Kukawka
    email                : danny.kukawka@web.de
 ***************************************************************************/
/* Copyright (c) 2012 Montel Laurent <montel@kde.org>                      */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAILIMPORTER_FILTER_THUNDERBIRD_HXX
#define MAILIMPORTER_FILTER_THUNDERBIRD_HXX

#include "filters.h"

/**
 * Imports Thinderbird mail folder recursively, keeping the folder structure.
 * @author Danny Kukawka
 */
namespace MailImporter {
class MAILIMPORTER_EXPORT FilterThunderbird : public Filter
{
public:
    explicit FilterThunderbird();
    ~FilterThunderbird();

    void import();
    void importMails( const QString & maildir );

    static QString defaultSettingsPath();
    static QString defaultProfile(QWidget *parent = 0);
    static QMap<QString, QString> listProfile(QString&currentProfile);

private:
    void importDirContents(const QString&, const QString&, const QString&);
    void importMBox(const QString&, const QString&, const QString&);
    bool excludeFiles( const QString & file );
};
}

#endif
