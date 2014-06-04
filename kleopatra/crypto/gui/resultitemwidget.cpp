/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/resultitemwidget.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include <config-kleopatra.h>

#include "resultitemwidget.h"

#include <utils/auditlog.h>

#include <ui/messagebox.h>

#include <KDebug>
#include <KLocalizedString>
#include <KPushButton>
#include <KStandardGuiItem>
#include <KUrl>

#include <QHBoxLayout>
#include <QLabel>
#include <QStringList>
#include <QUrl>
#include <QVBoxLayout>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;
using namespace boost;

namespace {
    //### TODO move outta here, make colors configurable
    static QColor colorForVisualCode( Task::Result::VisualCode code ) {
        switch ( code ) {
            case Task::Result::AllGood:
                return QColor(0x3E, 0xAC, 0x31); //green
            case Task::Result::NeutralError:
            case Task::Result::Warning:
                return QColor(0xE1, 0xB6, 0x13); //yellow
            case Task::Result::Danger:
                return QColor(0xD4, 0x23, 0x23); //red
            case Task::Result::NeutralSuccess:
            default:
                return Qt::blue;
        }
    }
}

class ResultItemWidget::Private {
    ResultItemWidget* const q;
public:
    explicit Private( const shared_ptr<const Task::Result> result, ResultItemWidget* qq ) : q( qq ), m_result( result ), m_detailsLabel( 0 ), m_showDetailsLabel( 0 ), m_closeButton( 0 ) { assert( m_result ); }

    void slotLinkActivated( const QString & );
    void updateShowDetailsLabel();

    const shared_ptr<const Task::Result> m_result;
    QLabel * m_detailsLabel;
    QLabel * m_showDetailsLabel;
    KPushButton * m_closeButton;
};

static KUrl auditlog_url_template() {
    KUrl url;
    url.setScheme( QLatin1String("kleoresultitem") );
    url.setHost( QLatin1String("showauditlog") );
    return url;
}

void ResultItemWidget::Private::updateShowDetailsLabel()
{
    if ( !m_showDetailsLabel || !m_detailsLabel )
        return;

    const bool detailsVisible = m_detailsLabel->isVisible();
    const QString auditLogLink = m_result->auditLog().formatLink( auditlog_url_template() );
    m_showDetailsLabel->setText( QString::fromLatin1( "<a href=\"kleoresultitem://toggledetails/\">%1</a><br/>%2" ).arg( detailsVisible ? i18n( "Hide Details" ) : i18n( "Show Details" ), auditLogLink ) );
}

ResultItemWidget::ResultItemWidget( const shared_ptr<const Task::Result> & result, QWidget * parent, Qt::WindowFlags flags ) : QWidget( parent, flags ), d( new Private( result, this ) )
{
    const QColor color = colorForVisualCode( d->m_result->code() );
    setStyleSheet( QString::fromLatin1( "* { background-color: %1; margin: 0px; } QFrame#resultFrame{ border-color: %2; border-style: solid; border-radius: 3px; border-width: 2px } QLabel { padding: 5px; border-radius: 3px }" ).arg( color.lighter( 150 ).name(), color.name() ) );
    QVBoxLayout* topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    QFrame* frame = new QFrame;
    frame->setObjectName( QLatin1String("resultFrame") );
    topLayout->addWidget( frame );
    QVBoxLayout* layout = new QVBoxLayout( frame );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    QWidget* hbox = new QWidget;
    QHBoxLayout* hlay = new QHBoxLayout( hbox );
    hlay->setMargin( 0 );
    hlay->setSpacing( 0 );
    QLabel* overview = new QLabel;
    overview->setWordWrap( true );
    overview->setTextFormat( Qt::RichText );
    overview->setText( d->m_result->overview() );
    connect( overview, SIGNAL(linkActivated(QString)), this, SLOT(slotLinkActivated(QString)) );

    hlay->addWidget( overview, 1, Qt::AlignTop );
    layout->addWidget( hbox );

    const QString details = d->m_result->details();

    d->m_showDetailsLabel = new QLabel;
    connect( d->m_showDetailsLabel, SIGNAL(linkActivated(QString)), this, SLOT(slotLinkActivated(QString)) );
    hlay->addWidget( d->m_showDetailsLabel );
    d->m_showDetailsLabel->setVisible( !details.isEmpty() );

    d->m_detailsLabel = new QLabel;
    d->m_detailsLabel->setWordWrap( true );
    d->m_detailsLabel->setTextFormat( Qt::RichText );
    d->m_detailsLabel->setText( details );
    connect( d->m_detailsLabel, SIGNAL(linkActivated(QString)), this, SLOT(slotLinkActivated(QString)) );
    layout->addWidget( d->m_detailsLabel );

    d->m_detailsLabel->setVisible( false );

    d->m_closeButton = new KPushButton;
    d->m_closeButton->setGuiItem( KStandardGuiItem::close() );
    d->m_closeButton->setFixedSize( d->m_closeButton->sizeHint() );
    connect( d->m_closeButton, SIGNAL(clicked()), this, SIGNAL(closeButtonClicked()) );
    layout->addWidget( d->m_closeButton, 0, Qt::AlignRight );
    d->m_closeButton->setVisible( false );

    d->updateShowDetailsLabel();
}

ResultItemWidget::~ResultItemWidget()
{
}

void ResultItemWidget::showCloseButton( bool show )
{
    d->m_closeButton->setVisible( show );
}

bool ResultItemWidget::detailsVisible() const
{
    return d->m_detailsLabel && d->m_detailsLabel->isVisible();
}

bool ResultItemWidget::hasErrorResult() const
{
    return d->m_result->hasError();
}

void ResultItemWidget::Private::slotLinkActivated( const QString & link )
{
    assert( m_result );
    if ( link.startsWith( QLatin1String( "key:" ) ) ) {
        const QStringList split = link.split( QLatin1Char(':') );
        if ( split.size() == 3 || m_result->nonce() != split.value( 1 ) )
            emit q->linkActivated( QLatin1String("key://") + split.value( 2 ) );
        else
            qWarning() << "key link invalid, or nonce not matching! link=" << link << " nonce" << m_result->nonce();
        return;
    }

    const QUrl url( link );
    if ( url.host() == QLatin1String("toggledetails") ) {
        q->showDetails( !q->detailsVisible() );
        return;
    }

    if ( url.host() == QLatin1String("showauditlog") ) {
        q->showAuditLog();
        return;
    }
    qWarning() << "Unexpected link scheme: " << link;
}

void ResultItemWidget::showAuditLog() {
    MessageBox::auditLog( parentWidget(), d->m_result->auditLog().text() );
}

void ResultItemWidget::showDetails( bool show )
{
    if ( show == d->m_detailsLabel->isVisible() )
        return;
    d->m_detailsLabel->setVisible( show );
    d->updateShowDetailsLabel();
    emit detailsToggled( show );
}

#include "moc_resultitemwidget.cpp"
