/* -*- mode: c++; c-basic-offset:4 -*-
    certificatepickerwidget.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KLEOPATRA_CERTIFICATEPICKERWIDGET_H__
#define __KLEOPATRA_CERTIFICATEPICKERWIDGET_H__

#include "wizardpage.h"

#include <gpgme++/global.h>

#include <utils/pimpl_ptr.h>

#include <boost/shared_ptr.hpp>

class QStringList;

namespace GpgME {
    class Key;
}

namespace Kleo {

    class RecipientResolveWidget : public QWidget {
        Q_OBJECT
    public:
        explicit RecipientResolveWidget( QWidget* parent = 0 );

        void setIdentifier( const QString& identifier );
        void setCertificates( const std::vector<GpgME::Key>& keys );
        GpgME::Key chosenCertificate() const;
        bool rememberSelection() const;
        bool isComplete() const;

        void setProtocol( GpgME::Protocol protocol );
        GpgME::Protocol protocol() const;

    Q_SIGNALS:
        void changed();

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;

        Q_PRIVATE_SLOT( d, void selectAnotherCertificate() );
        Q_PRIVATE_SLOT( d, void currentIndexChanged( int ) );
    };

    class RecipientResolvePage : public WizardPage {
        Q_OBJECT
    public:
 
        explicit RecipientResolvePage( QWidget * parent=0 );
        ~RecipientResolvePage();
    
        bool isComplete() const;

        void setIdentifiers( const QStringList& identifiers );
        QStringList identifiers() const;


        /** 
         * The protocol selected by the user (which is chosen by 
         * the user in case none was preset)
         */
        GpgME::Protocol selectedProtocol() const;

        /**
         * the protocol set before the dialog is shown. Defaults to 
         * GpgME::UnknownProtocol */
        GpgME::Protocol presetProtocol() const;
        void setPresetProtocol( GpgME::Protocol protocol );

        unsigned int numRecipientResolveWidgets() const;
        RecipientResolveWidget * recipientResolveWidget( unsigned int idx ) const;

        void ensureIndexAvailable( unsigned int idx );

        bool isMultipleProtocolsAllowed() const;
        void setMultipleProtocolsAllowed( bool allowed );

    Q_SIGNALS:
        void selectedProtocolChanged();
 
    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
        Q_PRIVATE_SLOT( d, void protocolSelected( int ) );
    };
}


#endif // __KLEOPATRA_CERTIFICATEPICKERWIDGET_H__
