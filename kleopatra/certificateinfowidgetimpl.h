/* -*- Mode: C -*-

  $Id$

  Copyright (C) 2001 by Klarälvdalens Datakonsult AB

  GPGMEPLUG is free software; you can redistribute it and/or modify
  it under the terms of GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  GPGMEPLUG is distributed in the hope that it will be useful,
  it under the terms of GNU General Public License as published by
  the Free Software Foundation; version 2 of the License
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

#ifndef CERTIFICATEINFOWIDGETIMPL_H
#define CERTIFICATEINFOWIDGETIMPL_H

#include "certificateinfowidget.h"
#include <cryptplugwrapper.h>

class QListViewItem;
class CertManager;

class CertificateInfoWidgetImpl : public CertificateInfoWidget {
  Q_OBJECT
public:
  CertificateInfoWidgetImpl( CertManager* manager, bool external,
			     QWidget* parent = 0, const char* name = 0);

  void setCert( const CryptPlugWrapper::CertificateInfo& info );
protected slots:
  void slotShowInfo( QListViewItem* );
  void slotShowCertPathDetails( QListViewItem* ); 
  void slotImportCertificate();
private:
  CertManager* _manager;
  CryptPlugWrapper::CertificateInfo _info;
};

#endif // CERTIFICATEINFOWIDGETIMPL_H
