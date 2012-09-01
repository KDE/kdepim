/***************************************************************************
            filter_sylpheed.h  -  Sylpheed maildir mail import
                             -------------------
    begin                : April 07 2005
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

#ifndef MAILIMPORTER_FILTER_SYLPHEED_HXX
#define MAILIMPORTER_FILTER_SYLPHEED_HXX

#include <QHash>

#include "filters.h"
/**
 * Imports Sylpheed mail folder with maildir format recursively, recreating the folder structure.
 * @author Danny Kukawka
 */
namespace MailImporter {
class MAILIMPORTER_EXPORT FilterSylpheed : public Filter
{

public:
  explicit FilterSylpheed();
  ~FilterSylpheed();

  static QString defaultSettingsPath();

  void import();
  void importMails( const QString& maildir );

  /* return local mail dir from folderlist.xml*/
  QString localMailDirPath();
  
private:
  bool excludeFile(const QString& file);
  void importDirContents(const QString&);
  void importFiles(const QString&);
  void processDirectory( const QString& path);
  
  void readMarkFile( const QString&, QHash<QString,unsigned long>&);
  Akonadi::MessageStatus msgFlagsToString(unsigned long flags);
  int mImportDirDone;
  int mTotalDir;

};
}

#endif
