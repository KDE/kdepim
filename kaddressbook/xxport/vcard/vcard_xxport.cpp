/*
  This file is part of KAddressBook.
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "vcard_xxport.h"

#include "vcardviewerdialog.h"
#include "vcardexportselectiondialog.h"

#include "pimcommon/widgets/renamefiledialog.h"

#include <kaddressbookgrantlee/widget/grantleecontactviewer.h>

#ifdef QGPGME_FOUND
#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/key.h>
#include <qgpgme/dataprovider.h>
#endif // QGPGME_FOUND

#include <KABC/VCardConverter>

#include <KDebug>
#include <KDialog>
#include <KFileDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPushButton>
#include <KTemporaryFile>
#include <KUrl>
#include <KStandardGuiItem>
#include <KIO/NetAccess>
#include <KSharedConfig>

#include <QtCore/QFile>
#include <QtCore/QPointer>


VCardXXPort::VCardXXPort( QWidget *parent )
    : XXPort( parent )
{
}

bool VCardXXPort::exportContacts( const KABC::Addressee::List &contacts ) const
{
    KABC::VCardConverter converter;
    KUrl url;

    const KABC::Addressee::List list = filterContacts( contacts );
    if ( list.isEmpty() ) { // no contact selected
        return true;
    }

    bool ok = true;
    if ( list.count() == 1 ) {
        url = KFileDialog::getSaveUrl(
                    QString( list[ 0 ].givenName() +
                    QLatin1Char( QLatin1Char('_') ) +
                    list[ 0 ].familyName() +
                QLatin1String( ".vcf" ) ) );
        if ( url.isEmpty() ) { // user canceled export
            return true;
        }

        if ( option( QLatin1String("version") ) == QLatin1String("v21") ) {
            ok = doExport( url, converter.exportVCards( list, KABC::VCardConverter::v2_1 ) );
        } else if ( option( QLatin1String("version") ) == QLatin1String("v30") ) {
            ok = doExport( url, converter.exportVCards( list, KABC::VCardConverter::v3_0 ) );
        } else {
            ok = doExport( url, converter.exportVCards( list, KABC::VCardConverter::v4_0 ) );
        }
    } else {
        const int answer =
                KMessageBox::questionYesNoCancel(
                    parentWidget(),
                    i18nc( "@info",
                           "You have selected a list of contacts, "
                           "shall they be exported to several files?" ),
                    QString(),
                    KGuiItem( i18nc( "@action:button", "Export to One File" ) ),
                    KGuiItem( i18nc( "@action:button", "Export to Several Files" ) ) );

        switch( answer ) {
        case KMessageBox::No:
        {
            const KUrl baseUrl = KFileDialog::getExistingDirectoryUrl();
            if ( baseUrl.isEmpty() ) {
                return true; // user canceled export
            }

            for ( int i = 0; i < list.count(); ++i ) {
                const KABC::Addressee contact = list.at( i );

                url = baseUrl.url() + QLatin1Char('/') + contactFileName( contact ) + QLatin1String(".vcf");

                bool tmpOk = false;

                if ( option( QLatin1String("version") ) == QLatin1String("v21") ) {
                    tmpOk = doExport( url, converter.exportVCard( contact, KABC::VCardConverter::v2_1 ) );
                } else if ( option( QLatin1String("version") ) == QLatin1String("v30") ) {
                    tmpOk = doExport( url, converter.exportVCard( contact, KABC::VCardConverter::v3_0 ) );
                } else {
                    tmpOk = doExport( url, converter.exportVCard( contact, KABC::VCardConverter::v4_0 ) );
                }

                ok = ok && tmpOk;
            }
            break;
        }
        case KMessageBox::Yes:
        {
            url = KFileDialog::getSaveUrl( KUrl( QLatin1String("addressbook.vcf") ) );
            if ( url.isEmpty() ) {
                return true; // user canceled export
            }

            if ( option( QLatin1String("version") ) == QLatin1String("v21") ) {
                ok = doExport( url, converter.exportVCards( list, KABC::VCardConverter::v2_1 ) );
            } else if ( option( QLatin1String("version") ) == QLatin1String("v30") ) {
                ok = doExport( url, converter.exportVCards( list, KABC::VCardConverter::v3_0 ) );
            } else {
                ok = doExport( url, converter.exportVCards( list, KABC::VCardConverter::v4_0 ) );
            }
            break;
        }
        case KMessageBox::Cancel:
        default:
            return true; // user canceled export
        }
    }

    return ok;
}

KABC::Addressee::List VCardXXPort::importContacts() const
{
    QString fileName;
    KABC::Addressee::List addrList;
    KUrl::List urls;

    if ( !option( QLatin1String("importData") ).isEmpty() ) {
        addrList = parseVCard( option( QLatin1String("importData") ).toUtf8() );
    } else {
        if ( !option( QLatin1String("importUrl") ).isEmpty() ) {
            urls.append( KUrl( option( QLatin1String("importUrl") ) ) );
        } else {
            const QString filter = i18n( "*.vcf|vCard (*.vcf)\n*|all files (*)" );
            urls =
                    KFileDialog::getOpenUrls(
                        KUrl(),
                        filter,
                        parentWidget(),
                        i18nc( "@title:window", "Select vCard to Import" ) );
        }

        if ( urls.isEmpty() ) {
            return addrList;
        }

        const QString caption( i18nc( "@title:window", "vCard Import Failed" ) );
        bool anyFailures = false;

        const int numberOfUrl( urls.count() );
        for ( int i = 0; i < numberOfUrl; ++i ) {
            const KUrl url = urls.at( i );

            if ( KIO::NetAccess::download( url, fileName, parentWidget() ) ) {

                QFile file( fileName );

                if ( file.open( QIODevice::ReadOnly ) ) {
                    const QByteArray data = file.readAll();
                    file.close();
                    if ( !data.isEmpty() ) {
                        addrList += parseVCard( data );
                    }

                    KIO::NetAccess::removeTempFile( fileName );
                } else {
                    const QString msg = i18nc(
                                "@info",
                                "<para>When trying to read the vCard, "
                                "there was an error opening the file <filename>%1</filename>:</para>"
                                "<para>%2</para>",
                                url.pathOrUrl(),
                                i18nc( "QFile", file.errorString().toLatin1() ) );
                    KMessageBox::error( parentWidget(), msg, caption );
                    anyFailures = true;
                }
            } else {
                const QString msg = i18nc(
                            "@info",
                            "<para>Unable to access vCard:</para><para>%1</para>",
                            KIO::NetAccess::lastErrorString() );
                KMessageBox::error( parentWidget(), msg, caption );
                anyFailures = true;
            }
        }

        if ( !option( QLatin1String("importUrl") ).isEmpty() ) { // a vcard was passed via cmd
            if ( addrList.isEmpty() ) {
                if ( anyFailures && urls.count() > 1 ) {
                    KMessageBox::information(
                                parentWidget(),
                                i18nc( "@info", "No contacts were imported, due to errors with the vCards." ) );
                } else if ( !anyFailures ) {
                    KMessageBox::information(
                                parentWidget(),
                                i18nc( "@info", "The vCard does not contain any contacts." ) );
                }
            } else {
                QPointer<VCardViewerDialog> dlg = new VCardViewerDialog( addrList, parentWidget() );
                if ( dlg->exec() && dlg ) {
                    addrList = dlg->contacts();
                } else {
                    addrList.clear();
                }
                delete dlg;
            }
        }
    }
    return addrList;
}

KABC::Addressee::List VCardXXPort::parseVCard( const QByteArray &data ) const
{
    KABC::VCardConverter converter;

    return converter.parseVCards( data );
}

bool VCardXXPort::doExport( const KUrl &url, const QByteArray &data ) const
{   
    KUrl newUrl(url);
    if ( newUrl.isLocalFile() && QFileInfo( newUrl.toLocalFile() ).exists() ) {
        PimCommon::RenameFileDialog *dialog = new PimCommon::RenameFileDialog(newUrl, false, parentWidget());
        PimCommon::RenameFileDialog::RenameFileDialogResult result = static_cast<PimCommon::RenameFileDialog::RenameFileDialogResult>(dialog->exec());
        if ( result == PimCommon::RenameFileDialog::RENAMEFILE_RENAME ) {
            newUrl = dialog->newName();
        } else if (result == PimCommon::RenameFileDialog::RENAMEFILE_IGNORE) {
            delete dialog;
            return true;
        }
        delete dialog;
    }

    KTemporaryFile tmpFile;
    tmpFile.open();

    tmpFile.write( data );
    tmpFile.flush();

    return KIO::NetAccess::upload( tmpFile.fileName(), newUrl, parentWidget() );
}

KABC::Addressee::List VCardXXPort::filterContacts( const KABC::Addressee::List &addrList ) const
{
    KABC::Addressee::List list;

    if ( addrList.isEmpty() ) {
        return addrList;
    }

    QPointer<VCardExportSelectionDialog> dlg = new VCardExportSelectionDialog( parentWidget() );
    if ( !dlg->exec() || !dlg ) {
        delete dlg;
        return list;
    }
    VCardExportSelectionDialog::ExportFields exportFieldType = dlg->exportType();
    KABC::Addressee::List::ConstIterator it;
    KABC::Addressee::List::ConstIterator end( addrList.end() );
    for ( it = addrList.begin(); it != end; ++it ) {
        KABC::Addressee addr;

        addr.setUid( (*it).uid() );
        addr.setFormattedName( (*it).formattedName() );

        bool addrDone = false;
        if ( exportFieldType & VCardExportSelectionDialog::DiplayName ) {                // output display name as N field
            QString fmtName = (*it).formattedName();
            QStringList splitNames = fmtName.split( QLatin1Char(' '), QString::SkipEmptyParts );
            if ( splitNames.count() >= 2 ) {
                addr.setPrefix( QString() );
                addr.setGivenName( splitNames.takeFirst() );
                addr.setFamilyName( splitNames.takeLast() );
                addr.setAdditionalName( splitNames.join( QLatin1String(" ") ) );
                addr.setSuffix( QString() );
                addrDone = true;
            }
        }

        if ( !addrDone ) {                                // not wanted, or could not be split
            addr.setPrefix( (*it).prefix() );
            addr.setGivenName( (*it).givenName() );
            addr.setAdditionalName( (*it).additionalName() );
            addr.setFamilyName( (*it).familyName() );
            addr.setSuffix( (*it).suffix() );
        }

        addr.setNickName( (*it).nickName() );
        addr.setMailer( (*it).mailer() );
        addr.setTimeZone( (*it).timeZone() );
        addr.setGeo( (*it).geo() );
        addr.setProductId( (*it).productId() );
        addr.setSortString( (*it).sortString() );
        addr.setUrl( (*it).url() );
        addr.setExtraUrlList( (*it).extraUrlList() );
        addr.setSecrecy( (*it).secrecy() );
        addr.setSound( (*it).sound() );
        addr.setEmailList( (*it).emailList() );
        addr.setCategories( (*it).categories() );
        addr.setExtraSoundList( (*it).extraSoundList() );
        addr.setGender( (*it).gender() );
        addr.setLangs( (*it).langs() );
        addr.setKind( (*it).kind() );
        addr.setMembers( (*it).members() );
        addr.setRelationShips( (*it).relationShips() );

        if ( exportFieldType & VCardExportSelectionDialog::Private ) {
            addr.setBirthday( (*it).birthday() );
            addr.setNote( (*it).note() );
        }

        if ( exportFieldType & VCardExportSelectionDialog::Picture ) {
            if ( exportFieldType & VCardExportSelectionDialog::Private ) {
                addr.setPhoto( (*it).photo() );
                addr.setExtraPhotoList( (*it).extraPhotoList() );
            }

            if ( exportFieldType & VCardExportSelectionDialog::Business ) {
                addr.setLogo( (*it).logo() );
                addr.setExtraLogoList( (*it).extraLogoList() );
            }
        }

        if ( exportFieldType & VCardExportSelectionDialog::Business ) {
            addr.setTitle( (*it).title() );
            addr.setRole( (*it).role() );
            addr.setOrganization( (*it).organization() );
            addr.setDepartment( (*it).department() );

            KABC::PhoneNumber::List phones = (*it).phoneNumbers( KABC::PhoneNumber::Work );
            KABC::PhoneNumber::List::Iterator phoneIt;
            for ( phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt ) {
                addr.insertPhoneNumber( *phoneIt );
            }

            KABC::Address::List addresses = (*it).addresses( KABC::Address::Work );
            KABC::Address::List::Iterator addrIt;
            for ( addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt ) {
                addr.insertAddress( *addrIt );
            }
        }

        KABC::PhoneNumber::List phones = (*it).phoneNumbers();
        KABC::PhoneNumber::List::Iterator phoneIt;
        for ( phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt ) {
            int phoneType = (*phoneIt).type();

            if ( (phoneType & KABC::PhoneNumber::Home) && (exportFieldType & VCardExportSelectionDialog::Private) ) {
                addr.insertPhoneNumber( *phoneIt );
            } else if ( (phoneType & KABC::PhoneNumber::Work) && (exportFieldType & VCardExportSelectionDialog::Business) ) {
                addr.insertPhoneNumber( *phoneIt );
            } else if ( (exportFieldType & VCardExportSelectionDialog::Other) ) {
                addr.insertPhoneNumber( *phoneIt );
            }
        }

        KABC::Address::List addresses = (*it).addresses();
        KABC::Address::List::Iterator addrIt;
        for ( addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt ) {
            int addressType = (*addrIt).type();

            if ( (addressType & KABC::Address::Home) && exportFieldType & VCardExportSelectionDialog::Private ) {
                addr.insertAddress( *addrIt );
            } else if ( (addressType & KABC::Address::Work) && (exportFieldType & VCardExportSelectionDialog::Business) ) {
                addr.insertAddress( *addrIt );
            } else if ( exportFieldType & VCardExportSelectionDialog::Other ) {
                addr.insertAddress( *addrIt );
            }
        }

        if ( exportFieldType & VCardExportSelectionDialog::Other ) {
            addr.setCustoms( (*it).customs() );
        }

        if ( exportFieldType & VCardExportSelectionDialog::Encryption ) {
            addKey( addr, KABC::Key::PGP );
            addKey( addr, KABC::Key::X509 );
        }

        list.append( addr );
    }

    delete dlg;

    return list;
}

void VCardXXPort::addKey( KABC::Addressee &addr, KABC::Key::Type type ) const
{
#ifdef QGPGME_FOUND
    const QString fingerprint = addr.custom( QLatin1String("KADDRESSBOOK"),
                                             ( type == KABC::Key::PGP ? QLatin1String("OPENPGPFP") : QLatin1String("SMIMEFP") ) );
    if ( fingerprint.isEmpty() ) {
        return;
    }

    GpgME::Context *context = GpgME::Context::createForProtocol( GpgME::OpenPGP );
    if ( !context ) {
        kError() << "No context available";
        return;
    }

    context->setArmor( false );
    context->setTextMode( false );

    QGpgME::QByteArrayDataProvider dataProvider;
    GpgME::Data dataObj( &dataProvider );
    GpgME::Error error = context->exportPublicKeys( fingerprint.toLatin1(), dataObj );
    delete context;

    if ( error ) {
        kError() << error.asString();
        return;
    }

    KABC::Key key;
    key.setType( type );
    key.setBinaryData( dataProvider.data() );

    addr.insertKey( key );
#else
    return;
#endif
}
