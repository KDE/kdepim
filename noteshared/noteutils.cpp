/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "noteutils.h"
#include "network/notesnetworksender.h"
#include "network/notehostdialog.h"
#include "noteshared/attributes/notedisplayattribute.h"
#include "notesharedglobalconfig.h"
#include <KProcess>
#include <ksocketfactory.h>
#include <KMessageBox>
#include <KLocalizedString>

#include <KMime/KMimeMessage>

#include <QPointer>
#include <QTextDocument>
#include <QApplication>

bool NoteShared::NoteUtils::sendToMail(QWidget *parent, const QString &title, const QString &message)
{
    // get the mail action command
    const QStringList cmd_list = NoteShared::NoteSharedGlobalConfig::mailAction().split( QLatin1Char(' '), QString::SkipEmptyParts );
    if (cmd_list.isEmpty()) {
        KMessageBox::sorry( parent, i18n( "Please configure send mail action." ) );
        return false;
    }
    KProcess mail;
    Q_FOREACH ( const QString &cmd, cmd_list ) {
        if ( cmd == QLatin1String("%f") ) {
            mail << message;
        } else if ( cmd == QLatin1String("%t") ) {
            mail << i18n("Note: \"%1\"",title);
        } else {
            mail << cmd;
        }
    }

    if ( !mail.startDetached() ) {
        KMessageBox::sorry( parent, i18n( "Unable to start the mail process." ) );
        return false;
    }
    return true;
}

void NoteShared::NoteUtils::sendToNetwork(QWidget *parent, const QString &title, const QString &message)
{
    // pop up dialog to get the IP
    QPointer<NoteShared::NoteHostDialog> hostDlg = new NoteShared::NoteHostDialog( i18n( "Send \"%1\"", title ), parent );
    if ( hostDlg->exec() ) {

        const QString host = hostDlg->host();
        quint16 port = hostDlg->port();

        if ( !port ) { // not specified, use default
            port = NoteShared::NoteSharedGlobalConfig::port();
        }

        if ( host.isEmpty() ) {
            KMessageBox::sorry( parent, i18n( "The host cannot be empty." ) );
            delete hostDlg;
            return;
        }

        // Send the note

        //TODO verify connectToHost
        NoteShared::NotesNetworkSender *sender = new NoteShared::NotesNetworkSender(
                    KSocketFactory::connectToHost( QLatin1String("notes"), host, port ) );
        sender->setSenderId( NoteShared::NoteSharedGlobalConfig::senderID() );
        sender->setNote( title, message ); // FIXME: plainText ??
    }
    delete hostDlg;
}


QString NoteShared::NoteUtils::createToolTip(const Akonadi::Item &item)
{
    const KMime::Message::Ptr noteMessage = item.payload<KMime::Message::Ptr>();
    const QString description = QString::fromUtf8(noteMessage->mainBodyPart()->decodedContent());
    const QString realName = noteMessage->subject(false)->asUnicodeString();
    const bool isRichText = noteMessage->contentType()->isHTMLText();

    QString tip;
    if (item.hasAttribute<NoteDisplayAttribute>()) {
        NoteDisplayAttribute *attr = item.attribute<NoteDisplayAttribute>();
        if (attr) {
            const QString bckColorName = attr->backgroundColor().name();
            const QString txtColorName = attr->foregroundColor().name();;
            const bool textIsLeftToRight = ( QApplication::layoutDirection() == Qt::LeftToRight );
            const QString textDirection =  textIsLeftToRight ? QLatin1String( "left" ) : QLatin1String( "right" );

            tip = QString::fromLatin1(
                        "<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"0\">"
                        );
            tip += QString::fromLatin1(
                        "<tr>" \
                        "<td bgcolor=\"%1\" align=\"%4\" valign=\"middle\">" \
                        "<div style=\"color: %2; font-weight: bold;\">" \
                        "%3" \
                        "</div>" \
                        "</td>" \
                        "</tr>"
                        ).arg( bckColorName ).arg( txtColorName ).arg( Qt::escape( realName ) ).arg( textDirection );
            const QString htmlCodeForStandardRow = QString::fromLatin1(
                        "<tr>" \
                        "<td bgcolor=\"%1\" align=\"left\" valign=\"top\">" \
                        "<div style=\"color: %2;\">" \
                        "%3" \
                        "</div>" \
                        "</td>" \
                        "</tr>" );

            QString content = description;
            if ( !content.trimmed().isEmpty() ) {
                if ( textIsLeftToRight ) {
                    tip += htmlCodeForStandardRow.arg(bckColorName).arg( txtColorName ).arg( isRichText ? content : content.replace( QLatin1Char( '\n' ), QLatin1String( "<br>" ) ) );
                } else {
                    tip += htmlCodeForStandardRow.arg(bckColorName).arg( txtColorName ).arg( isRichText ? content : content.replace( QLatin1Char( '\n' ), QLatin1String( "<br>" ) ) );
                }
            }

            tip += QString::fromLatin1(
                        "</table" \
                        "</td>" \
                        "</tr>"
                        );
        }
    }
    return tip;
}

