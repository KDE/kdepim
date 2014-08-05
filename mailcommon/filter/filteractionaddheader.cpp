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

#include "filteractionaddheader.h"

#include <pimcommon/widgets/minimumcombobox.h>

#include <QLineEdit>
#include <KLocale>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextDocument>

using namespace MailCommon;

FilterActionAddHeader::FilterActionAddHeader( QObject *parent )
    : FilterActionWithStringList( QLatin1String("add header"), i18n( "Add Header" ), parent )
{
    mParameterList << QLatin1String("")
                   << QLatin1String("Reply-To")
                   << QLatin1String("Delivered-To")
                   << QLatin1String("X-KDE-PR-Message")
                   << QLatin1String("X-KDE-PR-Package")
                   << QLatin1String("X-KDE-PR-Keywords");

    mParameter = mParameterList.at( 0 );
}

FilterAction::ReturnCode FilterActionAddHeader::process(ItemContext &context , bool) const
{
    if ( mParameter.isEmpty() )
        return ErrorButGoOn;

    KMime::Message::Ptr msg = context.item().payload<KMime::Message::Ptr>();

    KMime::Headers::Base *header = KMime::Headers::createHeader( mParameter.toLatin1() );
    if ( !header ) {
        header = new KMime::Headers::Generic( mParameter.toLatin1(), msg.get(), mValue, "utf-8" );
    } else {
        header->fromUnicodeString( mValue, "utf-8" );
    }

    msg->setHeader( header );
    msg->assemble();

    context.setNeedsPayloadStore();

    return GoOn;
}

QWidget* FilterActionAddHeader::createParamWidget( QWidget *parent ) const
{
    QWidget *widget = new QWidget( parent );
    QHBoxLayout *layout = new QHBoxLayout( widget );
    layout->setSpacing( 4 );
    layout->setMargin( 0 );

    PimCommon::MinimumComboBox *comboBox = new PimCommon::MinimumComboBox( widget );
    comboBox->setObjectName( QLatin1String("combo") );
    comboBox->setEditable( true );
    comboBox->setInsertPolicy( QComboBox::InsertAtBottom );

    KCompletion *comp = comboBox->completionObject();
    comp->setIgnoreCase(true);
    comp->insertItems(mParameterList);
    comp->setCompletionMode(KCompletion::CompletionPopupAuto);


    layout->addWidget( comboBox, 0 /* stretch */ );

    QLabel *label = new QLabel( i18n( "With value:" ), widget );
    label->setFixedWidth( label->sizeHint().width() );
    layout->addWidget( label, 0 );

    QLineEdit *lineEdit = new QLineEdit( widget );
    lineEdit->setObjectName( QLatin1String("ledit") );
    //QT5 lineEdit->setTrapReturnKey(true);
    lineEdit->setClearButtonEnabled( true );
    layout->addWidget( lineEdit, 1 );

    setParamWidgetValue( widget );

    connect( comboBox, SIGNAL(currentIndexChanged(int)),
             this, SIGNAL(filterActionModified()) );
    connect( comboBox->lineEdit(), SIGNAL(textChanged(QString)),
             this, SIGNAL(filterActionModified()) );
    connect( lineEdit, SIGNAL(textChanged(QString)),
             this, SIGNAL(filterActionModified()) );

    return widget;
}

void FilterActionAddHeader::setParamWidgetValue( QWidget *paramWidget ) const
{
    const int index = mParameterList.indexOf( mParameter );

    PimCommon::MinimumComboBox *comboBox = paramWidget->findChild<PimCommon::MinimumComboBox*>( QLatin1String("combo") );
    Q_ASSERT( comboBox );
    comboBox->clear();
    comboBox->addItems( mParameterList );

    if ( index < 0 ) {
        comboBox->addItem( mParameter );
        comboBox->setCurrentIndex( comboBox->count() - 1 );
    } else {
        comboBox->setCurrentIndex( index );
    }

    QLineEdit *lineEdit = paramWidget->findChild<QLineEdit*>( QLatin1String("ledit") );
    Q_ASSERT( lineEdit );

    lineEdit->setText( mValue );
}

void FilterActionAddHeader::applyParamWidgetValue( QWidget *paramWidget )
{
    const PimCommon::MinimumComboBox *comboBox = paramWidget->findChild<PimCommon::MinimumComboBox*>( QLatin1String("combo") );
    Q_ASSERT( comboBox );
    mParameter = comboBox->currentText();

    const QLineEdit *lineEdit = paramWidget->findChild<QLineEdit*>( QLatin1String("ledit") );
    Q_ASSERT( lineEdit );
    mValue = lineEdit->text();
}

void FilterActionAddHeader::clearParamWidget( QWidget *paramWidget ) const
{
    PimCommon::MinimumComboBox *comboBox = paramWidget->findChild<PimCommon::MinimumComboBox*>( QLatin1String("combo") );
    Q_ASSERT( comboBox );
    comboBox->setCurrentIndex( 0 );

    QLineEdit *lineEdit = paramWidget->findChild<QLineEdit*>( QLatin1String("ledit") );
    Q_ASSERT( lineEdit );
    lineEdit->clear();
}

SearchRule::RequiredPart FilterActionAddHeader::requiredPart() const
{
    return SearchRule::CompleteMessage;
}


QString FilterActionAddHeader::argsAsString() const
{
    QString result = mParameter;
    result += QLatin1Char( '\t' );
    result += mValue;

    return result;
}

QString FilterActionAddHeader::displayString() const
{
    return label() + QLatin1String( " \"" ) + argsAsString().toHtmlEscaped() + QLatin1String( "\"" );
}

void FilterActionAddHeader::argsFromString( const QString &argsStr )
{
    const QStringList list = argsStr.split( QLatin1Char( '\t' ) );
    QString result;
    if ( list.count() < 2 ) {
        result = list[ 0 ];
        mValue.clear();
    } else {
        result = list[ 0 ];
        mValue = list[ 1 ];
    }

    int index = mParameterList.indexOf( result );
    if ( index < 0 ) {
        mParameterList.append( result );
        index = mParameterList.count() - 1;
    }

    mParameter = mParameterList.at( index );
}

FilterAction* FilterActionAddHeader::newAction()
{
    return new FilterActionAddHeader;
}

QStringList FilterActionAddHeader::sieveRequires() const
{
    return QStringList() <<QLatin1String("editheader");
}

QString FilterActionAddHeader::sieveCode() const
{
    return QString::fromLatin1("addheader \"%1\" \"%2\";").arg(mParameter).arg(mValue);
}


