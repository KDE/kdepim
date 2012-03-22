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

#ifndef ABSTRACTSETTINGS_H
#define ABSTRACTSETTINGS_H

#include <QObject>

class ImportWizard;
class KJob;

namespace KPIMIdentities {
  class Identity;
  class IdentityManager;
}

namespace MailTransport {
  class Transport;
}

class AbstractSettings : public QObject
{
  Q_OBJECT
public:
  explicit AbstractSettings(ImportWizard *parent);
  ~AbstractSettings();
private slots:
  void instanceCreateResult( KJob* job );

protected:
  void addFilterImportInfo( const QString& log );
  void addFilterImportError( const QString& log );

  void createResource( const QString& resources );
  KPIMIdentities::Identity* createIdentity();
  MailTransport::Transport *createTransport();
  void storeIdentity(KPIMIdentities::Identity* identity);

  ImportWizard *mImportWizard;
  KPIMIdentities::IdentityManager *mManager;
};

#endif // ABSTRACTSETTINGS_H
