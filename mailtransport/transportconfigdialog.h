/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KPIM_TRANSPORTCONFIGDIALOG_H
#define KPIM_TRANSPORTCONFIGDIALOG_H

#include <mailtransport/mailtransport_export.h>
#include <kdialog.h>

namespace KPIM {

class Transport;

/**
  Configuration dialog for a mail transport.
*/
class MAILTRANSPORT_EXPORT TransportConfigDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
      Creates a new mail transport configuration dialog for the given
      Transport object.
      @param transport The Transport object to configure.
      @param parent The parent widget.
    */
    explicit TransportConfigDialog( Transport* transport, QWidget* parent = 0 );

    /**
      Destroys the dialog.
    */
    virtual ~TransportConfigDialog();

  private slots:
    void checkSmtpCapabilities();
    void chooseSendmail();
    void passwordsLoaded();
    void save();
    void smtpCapabilities( const QStringList &capaNormal, const QStringList &capaSSL,
                           const QString &authNone, const QString &authSSL, const QString &authTLS );
    void hostNameChanged( const QString &text );
    void encryptionChanged( int enc );

  private:
    class Private;
    Private* const d;
};

}

#endif
