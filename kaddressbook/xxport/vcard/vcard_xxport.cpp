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
#include "vcardexportselectionwidget.h"

#include "pimcommon/widgets/renamefiledialog.h"

#include <kaddressbookgrantlee/widget/grantleecontactviewer.h>

#ifdef QGPGME_FOUND
#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/key.h>
#include <qgpgme/dataprovider.h>
#endif // QGPGME_FOUND

#include <KContacts/VCardConverter>

#include "kaddressbook_debug.h"
#include <QDialog>
#include <QFileDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <QTemporaryFile>
#include <QUrl>
#include <KStandardGuiItem>
#include <KIO/NetAccess>
#include <KSharedConfig>

#include <QFile>
#include <QPointer>

VCardXXPort::VCardXXPort(QWidget *parent)
    : XXPort(parent)
{
}

bool VCardXXPort::exportContacts(const ContactList &contacts, VCardExportSelectionWidget::ExportFields exportFields) const
{
    KContacts::VCardConverter converter;
    QUrl url;

    const KContacts::Addressee::List list = filterContacts(contacts.addressList(), exportFields);
    if (list.isEmpty()) {   // no contact selected
        return true;
    }

    bool ok = true;
    if (list.count() == 1) {
        url = QFileDialog::getSaveFileUrl(parentWidget(), QString(), QUrl::fromLocalFile(
                                              QString(list[ 0 ].givenName() +
                                                      QLatin1Char(QLatin1Char('_')) +
                                                      list[ 0 ].familyName() +
                                                      QLatin1String(".vcf"))));
        if (url.isEmpty()) {   // user canceled export
            return true;
        }

        if (option(QLatin1String("version")) == QLatin1String("v21")) {
            ok = doExport(url, converter.exportVCards(list, KContacts::VCardConverter::v2_1));
        } else if (option(QLatin1String("version")) == QLatin1String("v30")) {
            ok = doExport(url, converter.exportVCards(list, KContacts::VCardConverter::v3_0));
        } else {
            ok = doExport(url, converter.exportVCards(list, KContacts::VCardConverter::v4_0));
        }
    } else {
        const int answer =
            KMessageBox::questionYesNoCancel(
                parentWidget(),
                i18nc("@info",
                      "You have selected a list of contacts, "
                      "shall they be exported to several files?"),
                QString(),
                KGuiItem(i18nc("@action:button", "Export to One File")),
                KGuiItem(i18nc("@action:button", "Export to Several Files")));

        switch (answer) {
        case KMessageBox::No: {
            const QUrl baseUrl = QFileDialog::getExistingDirectoryUrl();
            if (baseUrl.isEmpty()) {
                return true; // user canceled export
            }

            for (int i = 0; i < list.count(); ++i) {
                const KContacts::Addressee contact = list.at(i);

                url = QUrl::fromLocalFile(baseUrl.path() + QLatin1Char('/') + contactFileName(contact) + QLatin1String(".vcf"));

                bool tmpOk = false;

                if (option(QLatin1String("version")) == QLatin1String("v21")) {
                    tmpOk = doExport(url, converter.exportVCard(contact, KContacts::VCardConverter::v2_1));
                } else if (option(QLatin1String("version")) == QLatin1String("v30")) {
                    tmpOk = doExport(url, converter.exportVCard(contact, KContacts::VCardConverter::v3_0));
                } else {
                    tmpOk = doExport(url, converter.exportVCard(contact, KContacts::VCardConverter::v4_0));
                }

                ok = ok && tmpOk;
            }
            break;
        }
        case KMessageBox::Yes: {
            url = QFileDialog::getSaveFileUrl(parentWidget(), QString(), QUrl::fromLocalFile(QStringLiteral("addressbook.vcf")));
            if (url.isEmpty()) {
                return true; // user canceled export
            }

            if (option(QLatin1String("version")) == QLatin1String("v21")) {
                ok = doExport(url, converter.exportVCards(list, KContacts::VCardConverter::v2_1));
            } else if (option(QLatin1String("version")) == QLatin1String("v30")) {
                ok = doExport(url, converter.exportVCards(list, KContacts::VCardConverter::v3_0));
            } else {
                ok = doExport(url, converter.exportVCards(list, KContacts::VCardConverter::v4_0));
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

ContactList VCardXXPort::importContacts() const
{
    QString fileName;
    ContactList contactList;
    KContacts::Addressee::List addrList;
    QList<QUrl> urls;

    if (!option(QStringLiteral("importData")).isEmpty()) {
        addrList = parseVCard(option(QStringLiteral("importData")).toUtf8());
    } else {
        if (!option(QStringLiteral("importUrl")).isEmpty()) {
            urls.append(QUrl::fromLocalFile(option(QStringLiteral("importUrl"))));
        } else {
            const QString filter = i18n("*.vcf|vCard (*.vcf)\n*|all files (*)");
            urls =
                QFileDialog::getOpenFileUrls(parentWidget(), i18nc("@title:window", "Select vCard to Import"),
                                             QUrl(),
                                             filter);
        }

        if (urls.isEmpty()) {
            return contactList;
        }

        const QString caption(i18nc("@title:window", "vCard Import Failed"));
        bool anyFailures = false;

        const int numberOfUrl(urls.count());
        for (int i = 0; i < numberOfUrl; ++i) {
            const QUrl url = urls.at(i);

            if (KIO::NetAccess::download(url, fileName, parentWidget())) {

                QFile file(fileName);

                if (file.open(QIODevice::ReadOnly)) {
                    const QByteArray data = file.readAll();
                    file.close();
                    if (!data.isEmpty()) {
                        addrList += parseVCard(data);
                    }

                    KIO::NetAccess::removeTempFile(fileName);
                } else {
                    const QString msg = xi18nc(
                                            "@info",
                                            "<para>When trying to read the vCard, "
                                            "there was an error opening the file <filename>%1</filename>:</para>"
                                            "<para>%2</para>",
                                            url.toDisplayString(),
                                            i18nc("QFile", file.errorString().toLatin1()));
                    KMessageBox::error(parentWidget(), msg, caption);
                    anyFailures = true;
                }
            } else {
                const QString msg = xi18nc(
                                        "@info",
                                        "<para>Unable to access vCard:</para><para>%1</para>",
                                        KIO::NetAccess::lastErrorString());
                KMessageBox::error(parentWidget(), msg, caption);
                anyFailures = true;
            }
        }

        if (!option(QLatin1String("importUrl")).isEmpty()) {     // a vcard was passed via cmd
            if (addrList.isEmpty()) {
                if (anyFailures && urls.count() > 1) {
                    KMessageBox::information(
                        parentWidget(),
                        i18nc("@info", "No contacts were imported, due to errors with the vCards."));
                } else if (!anyFailures) {
                    KMessageBox::information(
                        parentWidget(),
                        i18nc("@info", "The vCard does not contain any contacts."));
                }
            } else {
                QPointer<VCardViewerDialog> dlg = new VCardViewerDialog(addrList, parentWidget());
                if (dlg->exec() && dlg) {
                    addrList = dlg->contacts();
                } else {
                    addrList.clear();
                }
                delete dlg;
            }
        }
    }
    contactList.setAddressList(addrList);
    return contactList;
}

KContacts::Addressee::List VCardXXPort::parseVCard(const QByteArray &data) const
{
    KContacts::VCardConverter converter;

    return converter.parseVCards(data);
}

bool VCardXXPort::doExport(const QUrl &url, const QByteArray &data) const
{
    QUrl newUrl(url);
    if (newUrl.isLocalFile() && QFileInfo(newUrl.toLocalFile()).exists()) {
        PimCommon::RenameFileDialog *dialog = new PimCommon::RenameFileDialog(newUrl, false, parentWidget());
        PimCommon::RenameFileDialog::RenameFileDialogResult result = static_cast<PimCommon::RenameFileDialog::RenameFileDialogResult>(dialog->exec());
        if (result == PimCommon::RenameFileDialog::RENAMEFILE_RENAME) {
            newUrl = dialog->newName();
        } else if (result == PimCommon::RenameFileDialog::RENAMEFILE_IGNORE) {
            delete dialog;
            return true;
        }
        delete dialog;
    }

    QTemporaryFile tmpFile;
    tmpFile.open();

    tmpFile.write(data);
    tmpFile.flush();

    return KIO::NetAccess::upload(tmpFile.fileName(), newUrl, parentWidget());
}

KContacts::Addressee::List VCardXXPort::filterContacts(const KContacts::Addressee::List &addrList, VCardExportSelectionWidget::ExportFields exportFieldType) const
{
    KContacts::Addressee::List list;

    if (addrList.isEmpty()) {
        return addrList;
    }

    KContacts::Addressee::List::ConstIterator it;
    KContacts::Addressee::List::ConstIterator end(addrList.end());
    for (it = addrList.begin(); it != end; ++it) {
        KContacts::Addressee addr;

        addr.setUid((*it).uid());
        addr.setFormattedName((*it).formattedName());

        bool addrDone = false;
        if (exportFieldType & VCardExportSelectionWidget::DiplayName) {                  // output display name as N field
            QString fmtName = (*it).formattedName();
            QStringList splitNames = fmtName.split(QLatin1Char(' '), QString::SkipEmptyParts);
            if (splitNames.count() >= 2) {
                addr.setPrefix(QString());
                addr.setGivenName(splitNames.takeFirst());
                addr.setFamilyName(splitNames.takeLast());
                addr.setAdditionalName(splitNames.join(QStringLiteral(" ")));
                addr.setSuffix(QString());
                addrDone = true;
            }
        }

        if (!addrDone) {                                  // not wanted, or could not be split
            addr.setPrefix((*it).prefix());
            addr.setGivenName((*it).givenName());
            addr.setAdditionalName((*it).additionalName());
            addr.setFamilyName((*it).familyName());
            addr.setSuffix((*it).suffix());
        }

        addr.setNickName((*it).nickName());
        addr.setMailer((*it).mailer());
        addr.setTimeZone((*it).timeZone());
        addr.setGeo((*it).geo());
        addr.setProductId((*it).productId());
        addr.setSortString((*it).sortString());
        addr.setUrl((*it).url());
        addr.setExtraUrlList((*it).extraUrlList());
        addr.setSecrecy((*it).secrecy());
        addr.setSound((*it).sound());
        addr.setEmailList((*it).emailList());
        addr.setCategories((*it).categories());
        addr.setExtraSoundList((*it).extraSoundList());
        addr.setGender((*it).gender());
        addr.setLangs((*it).langs());
        addr.setKind((*it).kind());
        addr.setMembers((*it).members());
        addr.setRelationShips((*it).relationShips());
        addr.setSourcesUrlList((*it).sourcesUrlList());
        addr.setImppList((*it).imppList());

        if (exportFieldType & VCardExportSelectionWidget::Private) {
            addr.setBirthday((*it).birthday());
            addr.setNote((*it).note());
        }

        if (exportFieldType & VCardExportSelectionWidget::Picture) {
            if (exportFieldType & VCardExportSelectionWidget::Private) {
                addr.setPhoto((*it).photo());
                addr.setExtraPhotoList((*it).extraPhotoList());
            }

            if (exportFieldType & VCardExportSelectionWidget::Business) {
                addr.setLogo((*it).logo());
                addr.setExtraLogoList((*it).extraLogoList());
            }
        }

        if (exportFieldType & VCardExportSelectionWidget::Business) {
            addr.setTitle((*it).title());
            addr.setRole((*it).role());
            addr.setOrganization((*it).organization());
            addr.setDepartment((*it).department());

            KContacts::PhoneNumber::List phones = (*it).phoneNumbers(KContacts::PhoneNumber::Work);
            KContacts::PhoneNumber::List::Iterator phoneIt;
            for (phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt) {
                addr.insertPhoneNumber(*phoneIt);
            }

            KContacts::Address::List addresses = (*it).addresses(KContacts::Address::Work);
            KContacts::Address::List::Iterator addrIt;
            for (addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt) {
                addr.insertAddress(*addrIt);
            }
        }

        KContacts::PhoneNumber::List phones = (*it).phoneNumbers();
        KContacts::PhoneNumber::List::Iterator phoneIt;
        for (phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt) {
            int phoneType = (*phoneIt).type();

            if ((phoneType & KContacts::PhoneNumber::Home) && (exportFieldType & VCardExportSelectionWidget::Private)) {
                addr.insertPhoneNumber(*phoneIt);
            } else if ((phoneType & KContacts::PhoneNumber::Work) && (exportFieldType & VCardExportSelectionWidget::Business)) {
                addr.insertPhoneNumber(*phoneIt);
            } else if ((exportFieldType & VCardExportSelectionWidget::Other)) {
                addr.insertPhoneNumber(*phoneIt);
            }
        }

        KContacts::Address::List addresses = (*it).addresses();
        KContacts::Address::List::Iterator addrIt;
        for (addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt) {
            int addressType = (*addrIt).type();

            if ((addressType & KContacts::Address::Home) && exportFieldType & VCardExportSelectionWidget::Private) {
                addr.insertAddress(*addrIt);
            } else if ((addressType & KContacts::Address::Work) && (exportFieldType & VCardExportSelectionWidget::Business)) {
                addr.insertAddress(*addrIt);
            } else if (exportFieldType & VCardExportSelectionWidget::Other) {
                addr.insertAddress(*addrIt);
            }
        }

        if (exportFieldType & VCardExportSelectionWidget::Other) {
            addr.setCustoms((*it).customs());
        }

        if (exportFieldType & VCardExportSelectionWidget::Encryption) {
            addKey(addr, KContacts::Key::PGP);
            addKey(addr, KContacts::Key::X509);
        }

        list.append(addr);
    }

    return list;
}

void VCardXXPort::addKey(KContacts::Addressee &addr, KContacts::Key::Type type) const
{
#ifdef QGPGME_FOUND
    const QString fingerprint = addr.custom(QLatin1String("KADDRESSBOOK"),
                                            (type == KContacts::Key::PGP ? QLatin1String("OPENPGPFP") : QLatin1String("SMIMEFP")));
    if (fingerprint.isEmpty()) {
        return;
    }

    GpgME::Context *context = GpgME::Context::createForProtocol(GpgME::OpenPGP);
    if (!context) {
        qCritical() << "No context available";
        return;
    }

    context->setArmor(false);
    context->setTextMode(false);

    QGpgME::QByteArrayDataProvider dataProvider;
    GpgME::Data dataObj(&dataProvider);
    GpgME::Error error = context->exportPublicKeys(fingerprint.toLatin1(), dataObj);
    delete context;

    if (error) {
        qCritical() << error.asString();
        return;
    }

    KContacts::Key key;
    key.setType(type);
    key.setBinaryData(dataProvider.data());

    addr.insertKey(key);
#else
    return;
#endif
}

