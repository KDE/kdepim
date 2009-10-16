/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "linkcommandeditor.h"

#include "createvirtualfeedjob.h"
#include "krss/virtualfeedpropertiesattribute.h"
#include "krss/virtualfeedcollection.h"

#include <krss/feed.h>
#include <akonadi/filter/componentfactory.h>
#include <akonadi/filter/ui/editorfactory.h>

#include <akonadi/filter/action.h>
#include <Akonadi/SearchCreateJob>
#include <Akonadi/CollectionFetchJob>
#include <KLineEdit>
#include <KLocale>
#include <KDebug>
#include <KInputDialog>

#include <QGridLayout>

using namespace Akonadi::Filter;
using namespace Akonadi::Filter::UI;
using Akonadi::Collection;
using Akonadi::CollectionFetchJob;
using namespace KRss;

LinkCommandEditor::LinkCommandEditor( QWidget* parent, const CommandDescriptor * commandDescriptor,
                                      ComponentFactory* componentFactory,
                                      EditorFactory* editorComponentFactory )
    : CommandEditor( parent, commandDescriptor, componentFactory, editorComponentFactory )
{
    m_ui.setupUi( this );
    connect( m_ui.newVirtualFeedButton, SIGNAL( clicked() ), this, SLOT( slotNewVirtualFeed() ) );

    CollectionFetchJob* const fjob = new CollectionFetchJob( Collection( 1 ),
                                                             CollectionFetchJob::FirstLevel, this );
    if ( !fjob->exec() ) {
        kWarning() << fjob->errorString();
        return;
    }

    const QList<Collection> cols = fjob->collections();
    QList<KRss::VirtualFeedCollection> virtualFeeds;
    Q_FOREACH( const Collection& col, cols ) {
        if ( col.hasAttribute<VirtualFeedPropertiesAttribute>() ) {
            kDebug() << "Found a filtering feed at id:" << col.id();
            virtualFeeds.append( col );
        }
    }

    m_virtualFeedListModel = new VirtualFeedListModel( virtualFeeds, this );
    m_ui.virtualFeedCombo->setModel( m_virtualFeedListModel );
}

void LinkCommandEditor::slotNewVirtualFeed()
{
    bool ok;
    const QString name = KInputDialog::getText( i18n( "Creating new virtual feed" ),
                                                i18n( "Name of the virtual feed" ), QString(), &ok, this );

    if ( !ok )
        return;

    CreateVirtualFeedJob* const job = new CreateVirtualFeedJob( name, QString(), this );
    job->start();
}

void LinkCommandEditor::fillFromAction( Action::Base* action )
{
    Q_ASSERT( action );
    Q_ASSERT( action->actionType() == Action::ActionTypeCommand );

    const Action::Command* const command = dynamic_cast<Action::Command*>( action );
    Q_ASSERT( command );
    Q_ASSERT( command->parameters()->count() == 1 );

    const CommandDescriptor* const descriptor = command->commandDescriptor();
    Q_ASSERT( descriptor );
    Q_ASSERT( descriptor->parameters()->count() == 1 );

    const CommandDescriptor::ParameterDescriptor* const formalParameter = descriptor->parameters()->first();
    Q_ASSERT( formalParameter );
    Q_ASSERT( formalParameter->dataType() == DataTypeInteger );

    const QVariant parameter = command->parameters()->first();
    bool ok;
    const qlonglong id = parameter.toLongLong( &ok );
    if( !ok ) {
        kWarning() << "The target collection parameter '" << parameter << "' of the action is"
                      " not convertible to a Collection::Id";
        return;
    }

    const int row = m_virtualFeedListModel->rowForFeed( id );
    if ( row < 0 ) {
        kWarning() << "No feed with id:" << id;
        return;
    }

    m_ui.virtualFeedCombo->setCurrentIndex( row );
}

Action::Base* LinkCommandEditor::commitState( Component* parent )
{
    const Akonadi::Filter::CommandDescriptor* command = mComponentFactory->findCommand(
                                                                            QLatin1String( "link" ) );
    Q_ASSERT( command );

    const VirtualFeedListModel* const model = qobject_cast<VirtualFeedListModel*>(
                                                                m_ui.virtualFeedCombo->model() );
    Q_ASSERT( model );
    const int row = m_ui.virtualFeedCombo->currentIndex();
    const QVariant param = model->data( model->index( row ), VirtualFeedListModel::FeedIdRole );
    return mComponentFactory->createCommand( parent, command, QList<QVariant>() << param );
}

#include "linkcommandeditor.moc"
