/* Copyright 2009 James Bendig <james@imptalk.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "messagelist/utils/aggregationconfigbutton.h"

#include "messagelist/utils/aggregationcombobox.h"
#include "messagelist/utils/aggregationcombobox_p.h"
#include "messagelist/utils/configureaggregationsdialog.h"
#include "messagelist/core/manager.h"

#include <KLocalizedString>

using namespace MessageList::Core;
using namespace MessageList::Utils;

class MessageList::Utils::AggregationConfigButtonPrivate
{
public:
    AggregationConfigButtonPrivate( AggregationConfigButton *owner )
        : q( owner ), mAggregationComboBox( 0 )  { }

    AggregationConfigButton * const q;

    const AggregationComboBox * mAggregationComboBox;

    void slotConfigureAggregations();
};

AggregationConfigButton::AggregationConfigButton( QWidget * parent, const AggregationComboBox * aggregationComboBox )
    : QPushButton( i18n( "Configure..." ), parent ), d( new AggregationConfigButtonPrivate( this ) )
{
    d->mAggregationComboBox = aggregationComboBox;
    connect( this, SIGNAL(pressed()),
             this, SLOT(slotConfigureAggregations()) );

    // Keep aggregation combo up-to-date with any changes made in the configure dialog.
    if ( d->mAggregationComboBox != 0 )
        connect( this, SIGNAL(configureDialogCompleted()),
                 d->mAggregationComboBox, SLOT(slotLoadAggregations()) );
    setEnabled(Manager::instance());
}

AggregationConfigButton::~AggregationConfigButton()
{
    delete d;
}

void AggregationConfigButtonPrivate::slotConfigureAggregations()
{
    QString currentAggregationID;
    if ( mAggregationComboBox ) {
        currentAggregationID = mAggregationComboBox->currentAggregation();
    }

    ConfigureAggregationsDialog *dialog = new ConfigureAggregationsDialog( q->window() );
    dialog->selectAggregation( currentAggregationID );

    QObject::connect( dialog, SIGNAL(okClicked()), q, SIGNAL(configureDialogCompleted()) );

    dialog->show();
}

#include "moc_aggregationconfigbutton.cpp"
