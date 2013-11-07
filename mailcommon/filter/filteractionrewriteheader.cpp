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

#include "filteractionrewriteheader.h"

#include "widgets/regexplineedit.h"
#include <pimcommon/widgets/minimumcombobox.h>

#include <KDE/KLineEdit>
#include <KDE/KLocale>

#include <QHBoxLayout>
#include <QLabel>
#include <QTextDocument>

using namespace MailCommon;

FilterAction* FilterActionRewriteHeader::newAction()
{
    return new FilterActionRewriteHeader;
}

FilterActionRewriteHeader::FilterActionRewriteHeader( QObject *parent )
    : FilterActionWithStringList( QLatin1String("rewrite header"), i18n( "Rewrite Header" ), parent )
{
    mParameterList << QLatin1String("")
                   << QLatin1String("Subject")
                   << QLatin1String("Reply-To")
                   << QLatin1String("Delivered-To")
                   << QLatin1String("X-KDE-PR-Message")
                   << QLatin1String("X-KDE-PR-Package")
                   << QLatin1String("X-KDE-PR-Keywords");

    mParameter = mParameterList.at( 0 );
}

FilterAction::ReturnCode FilterActionRewriteHeader::process(ItemContext &context , bool) const
{
    if ( mParameter.isEmpty() || !mRegExp.isValid() )
        return ErrorButGoOn;

    const KMime::Message::Ptr msg = context.item().payload<KMime::Message::Ptr>();

    const QByteArray param(mParameter.toLatin1());
    KMime::Headers::Base *header = msg->headerByType(param);
    if ( !header ) {
        return GoOn; //TODO: Maybe create a new header by type?
    }

    QString value = header->asUnicodeString();

    const QString newValue = value.replace( mRegExp, mReplacementString );

    msg->removeHeader( param );

    KMime::Headers::Base *newheader = KMime::Headers::createHeader(param);
    if ( !newheader ) {
        newheader = new KMime::Headers::Generic(param, msg.get(), newValue, "utf-8" );
    } else {
        newheader->fromUnicodeString( newValue, "utf-8" );
    }
    msg->setHeader( newheader );
    msg->assemble();

    context.setNeedsPayloadStore();

    return GoOn;
}

SearchRule::RequiredPart FilterActionRewriteHeader::requiredPart() const
{
    return SearchRule::CompleteMessage;
}


QWidget* FilterActionRewriteHeader::createParamWidget( QWidget *parent ) const
{
    QWidget *widget = new QWidget( parent );
    QHBoxLayout *layout = new QHBoxLayout( widget );
    layout->setSpacing( 4 );
    layout->setMargin( 0 );

    PimCommon::MinimumComboBox *comboBox = new PimCommon::MinimumComboBox( widget );
    comboBox->setEditable( true );
    comboBox->setObjectName( QLatin1String("combo") );
    comboBox->setInsertPolicy( QComboBox::InsertAtBottom );
    layout->addWidget( comboBox, 0 /* stretch */ );

    KCompletion *comp = comboBox->completionObject();
    comp->setIgnoreCase(true);
    comp->insertItems(mParameterList);
    comp->setCompletionMode(KGlobalSettings::CompletionPopupAuto);

    QLabel *label = new QLabel( i18n( "Replace:" ), widget );
    label->setFixedWidth( label->sizeHint().width() );
    layout->addWidget( label, 0 );

    RegExpLineEdit *regExpLineEdit = new RegExpLineEdit( widget );
    regExpLineEdit->setObjectName( QLatin1String("search") );
    layout->addWidget( regExpLineEdit, 1 );

    label = new QLabel( i18n( "With:" ), widget );
    label->setFixedWidth( label->sizeHint().width() );
    layout->addWidget( label, 0 );

    KLineEdit *lineEdit = new KLineEdit( widget );
    lineEdit->setObjectName( QLatin1String("replace") );
    lineEdit->setClearButtonShown( true );
    lineEdit->setTrapReturnKey(true);
    layout->addWidget( lineEdit, 1 );

    setParamWidgetValue( widget );

    connect( comboBox, SIGNAL(currentIndexChanged(int)),
             this, SIGNAL(filterActionModified()) );
    connect( comboBox->lineEdit(), SIGNAL(textChanged(QString)),
             this, SIGNAL(filterActionModified()) );
    connect( regExpLineEdit, SIGNAL(textChanged(QString)),
             this, SIGNAL(filterActionModified()) );
    connect( lineEdit, SIGNAL(textChanged(QString)),
             this, SIGNAL(filterActionModified()) );

    return widget;
}

void FilterActionRewriteHeader::setParamWidgetValue( QWidget *paramWidget ) const
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

    RegExpLineEdit *regExpLineEdit = paramWidget->findChild<RegExpLineEdit*>( QLatin1String("search") );
    Q_ASSERT( regExpLineEdit );
    regExpLineEdit->setText( mRegExp.pattern() );

    KLineEdit *lineEdit = paramWidget->findChild<KLineEdit*>( QLatin1String("replace") );
    Q_ASSERT( lineEdit );
    lineEdit->setText( mReplacementString );
}

void FilterActionRewriteHeader::applyParamWidgetValue( QWidget *paramWidget )
{
    const PimCommon::MinimumComboBox *comboBox = paramWidget->findChild<PimCommon::MinimumComboBox*>( QLatin1String("combo") );
    Q_ASSERT( comboBox );
    mParameter = comboBox->currentText();

    const RegExpLineEdit *regExpLineEdit = paramWidget->findChild<RegExpLineEdit*>( QLatin1String("search") );
    Q_ASSERT( regExpLineEdit );
    mRegExp.setPattern( regExpLineEdit->text() );

    const KLineEdit *lineEdit = paramWidget->findChild<KLineEdit*>( QLatin1String("replace") );
    Q_ASSERT( lineEdit );
    mReplacementString = lineEdit->text();
}

void FilterActionRewriteHeader::clearParamWidget( QWidget *paramWidget ) const
{
    PimCommon::MinimumComboBox *comboBox = paramWidget->findChild<PimCommon::MinimumComboBox*>( QLatin1String("combo") );
    Q_ASSERT( comboBox );
    comboBox->setCurrentIndex( 0 );

    RegExpLineEdit *regExpLineEdit = paramWidget->findChild<RegExpLineEdit*>( QLatin1String("search") );
    Q_ASSERT( regExpLineEdit );
    regExpLineEdit->clear();

    KLineEdit *lineEdit = paramWidget->findChild<KLineEdit*>( QLatin1String("replace") );
    Q_ASSERT( lineEdit );
    lineEdit->clear();
}

QString FilterActionRewriteHeader::argsAsString() const
{
    QString result = mParameter;
    result += QLatin1Char( '\t' );
    result += mRegExp.pattern();
    result += QLatin1Char( '\t' );
    result += mReplacementString;

    return result;
}

QString FilterActionRewriteHeader::displayString() const
{
    return label() + QLatin1String( " \"" ) + Qt::escape( argsAsString() ) + QLatin1String( "\"" );
}

void FilterActionRewriteHeader::argsFromString( const QString &argsStr )
{
    const QStringList list = argsStr.split( QLatin1Char( '\t' ) );
    QString result;

    result = list[ 0 ];
    mRegExp.setPattern( list[ 1 ] );
    mReplacementString = list[ 2 ];

    int index = mParameterList.indexOf( result );
    if ( index < 0 ) {
        mParameterList.append( result );
        index = mParameterList.count() - 1;
    }

    mParameter = mParameterList.at( index );
}



