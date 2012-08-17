/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#ifndef SYLPHEEDSETTINGS_H
#define SYLPHEEDSETTINGS_H

#include "abstractsettings.h"
#include <QString>

class ImportWizard;
class KConfigGroup;
class QFile;

class SylpheedSettings : public AbstractSettings
{
public:
  explicit SylpheedSettings(const QString& filename, const QString &path, ImportWizard *parent );
  ~SylpheedSettings();
private:
  void readCustomHeader(QFile *customHeaderFile);
  void readGlobalSettings(const KConfigGroup& group);
  void readAccount(const KConfigGroup& accountConfig, bool checkMailOnStartup , int intervalCheckMail);
  void readIdentity( const KConfigGroup& accountConfig );
  QString readTransport( const KConfigGroup& accountConfig );
  void readPop3Account(const KConfigGroup& accountConfig, bool checkMailOnStartup , int intervalCheckMail);
  void readImapAccount(const KConfigGroup& accountConfig, bool checkMailOnStartup , int intervalCheckMail);
  void readSignature( const KConfigGroup& accountConfig, KPIMIdentities::Identity* identity );
  bool readConfig( const QString& key, const KConfigGroup& accountConfig, QString& value, bool remove_underscore = false );
  bool readConfig( const QString& key, const KConfigGroup& accountConfig, int& value, bool remove_underscore = false );



};

#endif /* SYLPHEEDSETTINGS_H */

