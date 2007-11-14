/* -*- mode: c++; c-basic-offset:4 -*-
    certificatepickerwidget.cpp

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

#include "certificatepickerwidget.h"
#include "certificatepickerwidget_p.h"

#include <gpgme++/key.h>

#include <KLocale>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QHash>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>

#include <cassert>
 
using namespace Kleo;

class Kleo::CertificatePickerWidget::Private {
    friend class ::CertificatePickerWidget;
    CertificatePickerWidget * const q;
public:
    explicit Private( const boost::shared_ptr<SuggestionMaker>& suggester, CertificatePickerWidget * qq );
    ~Private();
    void addLineForIdentifier( const QString& identifier );
    void completionStateChanged( const QString& id );
    void emitCompletionStateChange();
    std::vector<GpgME::Key> makeSuggestions( const QString& id ) const;
    void clear();

    mutable uint completeCount;
    QVBoxLayout* lineLayout;
    QScrollArea* scrollArea;
    QHash<QString, CertificatePickerLine*> lines;
    boost::shared_ptr<SuggestionMaker> suggester;
};


CertificatePickerWidget::Private::Private( const boost::shared_ptr<SuggestionMaker>& suggester_, CertificatePickerWidget * qq )
    : q( qq ), completeCount( 0 ), suggester( suggester_ )
{
    assert( suggester );
}

CertificatePickerWidget::Private::~Private() {}

CertificatePickerWidget::SuggestionMaker::~SuggestionMaker() {}

std::vector<GpgME::Key> CertificatePickerWidget::DefaultSuggestionMaker::makeSuggestions( const QString& identifier ) const
{
    return std::vector<GpgME::Key>();
}


CertificatePickerWidget::CertificatePickerWidget( const boost::shared_ptr<SuggestionMaker>& suggestionMaker, QWidget * parent, Qt::WFlags f )
    : QWidget( parent, f ), d( new Private( suggestionMaker, this ) )
{
    QGridLayout* const top = new QGridLayout( this );
    top->setRowStretch( 1, 1 );
    d->scrollArea = new QScrollArea( this );
    d->scrollArea->setFrameShape( QFrame::NoFrame );
    top->addWidget( d->scrollArea, 0, 0 );
    QWidget* const container = new QWidget;
    d->lineLayout = new QVBoxLayout( container );
    d->scrollArea->setWidget( container );
    d->scrollArea->setWidgetResizable( true );
}

CertificatePickerWidget::~CertificatePickerWidget() {}

bool CertificatePickerWidget::isComplete() const
{
    return d->completeCount == d->lines.count();
}

void CertificatePickerWidget::setIdentifiers( const QStringList& ids )
{
    d->clear();
    Q_FOREACH ( const QString& i, ids )
        d->addLineForIdentifier( i );
}

void CertificatePickerWidget::Private::clear()
{
    qDeleteAll( lines );
    lines.clear();
    completeCount = 0;
}

void CertificatePickerWidget::Private::addLineForIdentifier( const QString& id )
{
    assert( !lines.contains( id ) );
    CertificatePickerLine* const line = new CertificatePickerLine( id );
    line->setSuggestions( makeSuggestions( id ) );
    lines[id] = line;
    if ( line->isComplete() )
        ++completeCount;
    lineLayout->addWidget( line );
    line->show();
}

std::vector<GpgME::Key> CertificatePickerWidget::Private::makeSuggestions( const QString& id ) const
{
    return suggester->makeSuggestions( id );
}

void CertificatePickerWidget::Private::completionStateChanged( const QString& id )
{
    const bool wasComplete = completeCount == lines.count();
    CertificatePickerLine* const line = lines[id];
    assert( line );
    if ( line->isComplete() )
    {
        ++completeCount;
        assert( completeCount <= lines.count() );
    }
    else
    {
        assert( completeCount > 0 );
        --completeCount;
    }
    if ( wasComplete != ( completeCount == lines.count() ) )
        emit q->completionStateChanged();
} 

QStringList CertificatePickerWidget::identifiers() const
{
    return d->lines.keys();
}

GpgME::Key CertificatePickerWidget::selectedKey( const QString& identifier ) const
{
    return d->lines.contains( identifier ) ? d->lines[identifier]->selectedKey() : GpgME::Key();
}

CertificatePickerLine::CertificatePickerLine( const QString& identifier, QWidget* parent ) : QWidget( parent ), m_identifier( identifier )
{
    QGridLayout* layout = new QGridLayout( this );
    layout->setColumnStretch( 0, 1 );
    layout->setColumnStretch( 1, 1 );
    m_label = new QLabel;
    m_label->setText( i18nc( "identifier (email address or name) used as hint to find encryption key", "%1:", m_identifier ) );
    layout->addWidget( m_label, 0, 0 );
    m_combo = new QComboBox;
    m_label->setBuddy( m_combo );
    layout->addWidget( m_combo, 0, 1 );
    m_selectButton = new QPushButton;
    m_selectButton->setText( i18n( "Choose..." ) );
    connect( m_selectButton, SIGNAL( clicked() ), SLOT( selectAnother() ) );
    layout->addWidget( m_selectButton, 0, 2 );
    m_rememberChoiceCO = new QCheckBox;
    m_rememberChoiceCO->setText( i18n( "Remember choice" ) );
    layout->addWidget( m_rememberChoiceCO, 1, 0, /*rowSpan=*/1, /*columnSpan=*/-1 );
}

bool CertificatePickerLine::rememberSelection() const
{
    return m_rememberChoiceCO->checkState() == Qt::Checked;
}

void CertificatePickerLine::setSuggestions( const std::vector<GpgME::Key>& keys )
{
    // TODO: fill combobox
}

GpgME::Key CertificatePickerLine::selectedKey() const
{
    // TODO
    return GpgME::Key();
}

void CertificatePickerLine::selectAnother()
{
}

bool CertificatePickerLine::isComplete() const
{
    return false;
}

#include "moc_certificatepickerwidget.cpp"
#include "moc_certificatepickerwidget_p.cpp"

