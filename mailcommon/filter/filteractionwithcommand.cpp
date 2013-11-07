/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionwithcommand.h"

#include <KDE/KPIMUtils/KFileIO>
#include <KDE/KProcess>
#include <KDE/KShell>
#include <KDE/KTemporaryFile>
#include <KDE/KUrl>

using namespace MailCommon;

FilterActionWithCommand::FilterActionWithCommand( const QString &name, const QString &label, QObject *parent )
    : FilterActionWithUrl( name, label, parent )
{
}

QWidget* FilterActionWithCommand::createParamWidget( QWidget *parent ) const
{
    return FilterActionWithUrl::createParamWidget( parent );
}

void FilterActionWithCommand::applyParamWidgetValue( QWidget *paramWidget )
{
    FilterActionWithUrl::applyParamWidgetValue( paramWidget );
}

void FilterActionWithCommand::setParamWidgetValue( QWidget *paramWidget ) const
{
    FilterActionWithUrl::setParamWidgetValue( paramWidget );
}

void FilterActionWithCommand::clearParamWidget( QWidget *paramWidget ) const
{
    FilterActionWithUrl::clearParamWidget( paramWidget );
}

static KMime::Content* findMimeNodeForIndex( KMime::Content* node, int &index )
{
    if ( index <= 0 )
        return node;

    foreach ( KMime::Content* child, node->contents() ) {
        KMime::Content *result = findMimeNodeForIndex( child, --index );
        if ( result )
            return result;
    }

    return 0;
}

QString FilterActionWithCommand::substituteCommandLineArgsFor( const KMime::Message::Ptr &aMsg, QList<KTemporaryFile*> &aTempFileList ) const
{
    QString result = mParameter;
    QList<int> argList;
    QRegExp r( QLatin1String("%[0-9-]+") );

    // search for '%n'
    int start = -1;
    while ( ( start = r.indexIn( result, start + 1 ) ) > 0 ) {
        const int len = r.matchedLength();

        // and save the encountered 'n' in a list.
        bool ok = false;
        const int n = result.mid( start + 1, len - 1 ).toInt( &ok );
        if ( ok )
            argList.append( n );
    }

    // sort the list of n's
    qSort( argList );

    // and use QString::arg to substitute filenames for the %n's.
    int lastSeen = -2;
    QString tempFileName;
    QList<int>::ConstIterator end( argList.constEnd() );
    for ( QList<int>::ConstIterator it = argList.constBegin() ; it != end ; ++it ) {
        // setup temp files with check for duplicate %n's
        if ( (*it) != lastSeen ) {
            KTemporaryFile *tempFile = new KTemporaryFile();
            if ( !tempFile->open() ) {
                delete tempFile;
                kDebug() << "FilterActionWithCommand: Could not create temp file!";
                return QString();
            }

            aTempFileList.append( tempFile );
            tempFileName = tempFile->fileName();

            if ( (*it) == -1 )
                KPIMUtils::kByteArrayToFile( aMsg->encodedContent(), tempFileName, //###
                                             false, false, false );
            else if (aMsg->contents().size() == 0)
                KPIMUtils::kByteArrayToFile( aMsg->decodedContent(), tempFileName,
                                             false, false, false );
            else {
                int index = *it; // we pass by reference below, so this is not const
                KMime::Content *content = findMimeNodeForIndex( aMsg.get(), index );
                if ( content ) {
                    KPIMUtils::kByteArrayToFile( content->decodedContent(), tempFileName,
                                                 false, false, false );
                }
            }
            tempFile->close();
        }

        // QString( "%0 and %1 and %1" ).arg( 0 ).arg( 1 )
        // returns "0 and 1 and %1", so we must call .arg as
        // many times as there are %n's, regardless of their multiplicity.
        if ( (*it) == -1 )
            result.replace( QLatin1String("%-1"), tempFileName );
        else
            result = result.arg( tempFileName );
    }

    return result;
}


namespace {

/**
 * Substitutes placeholders in the command line with the
 * content of the correspoding header in the message.
 * %{From} -> Joe Author <joe@acme.com>
 */
void substituteMessageHeaders( const KMime::Message::Ptr &aMsg, QString &result )
{
    // Replace the %{foo} with the content of the foo header field.
    // If the header doesn't exist, remove the placeholder.
    QRegExp header_rx( QLatin1String("%\\{([a-z0-9-]+)\\}"), Qt::CaseInsensitive );
    int idx = 0;
    while ( ( idx = header_rx.indexIn( result, idx ) ) != -1 ) {
        const KMime::Headers::Base* header = aMsg->headerByType( header_rx.cap(1).toLatin1() );
        QString replacement;
        if ( header )
            replacement = KShell::quoteArg( QString::fromLatin1(header->as7BitString()) );
        result.replace( idx, header_rx.matchedLength(), replacement );
        idx += replacement.length();
    }
}

/**
 * Substitutes placeholders in the command line with the
 * corresponding information from the item. Currently supported
 * are %{itemid} and %{itemurl}.
 */
void substituteCommandLineArgsForItem( const Akonadi::Item &item, QString &commandLine )
{
    commandLine.replace( QLatin1String( "%{itemurl}" ), item.url( Akonadi::Item::UrlWithMimeType ).url() );
    commandLine.replace( QLatin1String( "%{itemid}" ), QString::number( item.id() ) );
}

}

FilterAction::ReturnCode FilterActionWithCommand::genericProcess( ItemContext &context, bool withOutput ) const
{
    const KMime::Message::Ptr aMsg = context.item().payload<KMime::Message::Ptr>();
    Q_ASSERT( aMsg );

    if ( mParameter.isEmpty() )
        return ErrorButGoOn;

    // KProcess doesn't support a QProcess::launch() equivalent, so
    // we must use a temp file :-(
    KTemporaryFile * inFile = new KTemporaryFile;
    if ( !inFile->open() ) {
        delete inFile;
        return ErrorButGoOn;
    }

    QList<KTemporaryFile*> atmList;
    atmList.append( inFile );

    QString commandLine = substituteCommandLineArgsFor( aMsg, atmList );
    substituteCommandLineArgsForItem( context.item(), commandLine );
    substituteMessageHeaders( aMsg, commandLine );

    if ( commandLine.isEmpty() ) {
        qDeleteAll( atmList );
        atmList.clear();
        return ErrorButGoOn;
    }
    // The parentheses force the creation of a subshell
    // in which the user-specified command is executed.
    // This is to really catch all output of the command as well
    // as to avoid clashes of our redirection with the ones
    // the user may have specified. In the long run, we
    // shouldn't be using tempfiles at all for this class, due
    // to security aspects. (mmutz)
    commandLine =  QLatin1Char( '(' ) + commandLine + QLatin1String( ") <" ) + inFile->fileName();

    // write message to file
    QString tempFileName = inFile->fileName();
    if ( !KPIMUtils::kByteArrayToFile( aMsg->encodedContent(), tempFileName, //###
                                       false, false, false ) ) {
        qDeleteAll( atmList );
        atmList.clear();
        return CriticalError;
    }

    inFile->close();

    KProcess shProc;
    shProc.setOutputChannelMode( KProcess::SeparateChannels );
    shProc.setShellCommand( commandLine );
    int result = shProc.execute();

    if ( result != 0 ) {
        qDeleteAll( atmList );
        atmList.clear();
        return ErrorButGoOn;
    }

    if ( withOutput ) {
        // read altered message:
        const QByteArray msgText = shProc.readAllStandardOutput();

        if ( !msgText.isEmpty() ) {
            /* If the pipe through alters the message, it could very well
       happen that it no longer has a X-UID header afterwards. That is
       unfortunate, as we need to removed the original from the folder
       using that, and look it up in the message. When the (new) message
       is uploaded, the header is stripped anyhow. */
            const QString uid = aMsg->headerByType( "X-UID" ) ? aMsg->headerByType( "X-UID" )->asUnicodeString() : QString();
            aMsg->setContent( KMime::CRLFtoLF( msgText ) );
            aMsg->parse();

            KMime::Headers::Generic *header = new KMime::Headers::Generic( "X-UID", aMsg.get(), uid, "utf-8" );
            aMsg->setHeader( header );

            context.setNeedsPayloadStore();
        } else {
            qDeleteAll( atmList );
            atmList.clear();
            return ErrorButGoOn;
        }
    }

    qDeleteAll( atmList );
    atmList.clear();

    return GoOn;
}

