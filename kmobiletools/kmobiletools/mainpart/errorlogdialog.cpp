/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "errorlogdialog.h"

#include <QVBoxLayout>
#include <QDateTime>

#include <KLocale>
#include <KIconLoader>

using namespace KMobileTools;

ErrorLogDialog::ErrorLogDialog( QWidget* parent )
 : KDialog( parent )
{
    setCaption( i18n( "Error log" ) );
    setButtons( KDialog::Ok );

    // create widgets and layout
    m_errorLogListWidget = new QListWidget( this );
    m_errorDetailsTextEdit = new KTextEdit( this );
    m_errorDetailsTextEdit->setReadOnly( true );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_errorLogListWidget );
    layout->addWidget( m_errorDetailsTextEdit );

    QWidget* dummyWidget = new QWidget( this );
    dummyWidget->setLayout( layout );

    // set initial size
    setInitialSize( QSize( 500, 400 ) );

    setMainWidget( dummyWidget );

    connect( m_errorLogListWidget, SIGNAL(itemActivated(QListWidgetItem *)),
             this, SLOT(showErrorDetails(QListWidgetItem *)) );
    connect( m_errorLogListWidget, SIGNAL(currentItemChanged ( QListWidgetItem*, QListWidgetItem* )),
             this, SLOT(showErrorDetails(QListWidgetItem *)) );


}


ErrorLogDialog::~ErrorLogDialog()
{
}

void ErrorLogDialog::show() {
    updateErrorStack();
    updateErrorView();
    KDialog::show();
}

void ErrorLogDialog::updateErrorStack() {
    QStack<const BaseError*> newErrorStack = ErrorHandler::instance()->errorStack();

    if( newErrorStack.isEmpty() )
        return;

    m_errorStack = newErrorStack;
    return;
}

void ErrorLogDialog::updateErrorView() {
    if( m_errorStack.isEmpty() ) {
        m_errorLogListWidget->clear();
        return;
    }

    m_errorLogListWidget->clear();

    ErrorLogItem* currentItem;
    for( int i=0; i<m_errorStack.size(); i++ )
        currentItem = new ErrorLogItem( m_errorStack.at(i), m_errorLogListWidget );

    m_errorLogListWidget->setCurrentItem( currentItem );
}

void ErrorLogDialog::showErrorDetails( QListWidgetItem* widgetItem ) {
    ErrorLogItem* errorItem = dynamic_cast<ErrorLogItem*>( widgetItem );
    if( errorItem == 0 )
        return;

    const BaseError* errorObject = errorItem->errorObject();

    QString htmlDescription;
    htmlDescription += "<strong>";
    htmlDescription += i18n("Error description:");
    htmlDescription += "</strong><br>";
    htmlDescription += errorObject->description();
    htmlDescription += "<br><br>";


    htmlDescription += "<strong>";
    htmlDescription += i18n("Error occurred on:");
    htmlDescription += "</strong><br>";
    htmlDescription += errorObject->dateTime().toString( Qt::LocaleDate );
    htmlDescription += "<br><br>";


    htmlDescription += "<strong>";
    htmlDescription += i18n("Error priority:");
    htmlDescription += "</strong><br>";
    switch( errorObject->priority() ) {
        case BaseError::Low:
            htmlDescription += i18n("Low");
            break;

        case BaseError::Medium:
            htmlDescription += i18n("Medium");
            break;

        case BaseError::High:
            htmlDescription += i18n("High");
            break;
    }
    htmlDescription += "<br><br>";


    htmlDescription += "<strong>";
    htmlDescription += i18n("The following information is relevant for developers only:");
    htmlDescription += "</strong><br><br>";


    htmlDescription += "<strong>";
    htmlDescription += i18n("Error occurred in:");
    htmlDescription += "</strong><br>";
    htmlDescription += QString( "%1:%2").arg( errorObject->fileName() )
                                        .arg( QString::number( errorObject->lineNumber() ) );
    htmlDescription += "<br><br>";


    htmlDescription += "<strong>";
    htmlDescription += i18n("Method name:");
    htmlDescription += "</strong><br>";
    htmlDescription += errorObject->methodName();
    htmlDescription += "<br><br>";


    htmlDescription += "<strong>";
    htmlDescription += i18n("Additional debug information:");
    htmlDescription += "</strong><br>";
    QHashIterator<QString, QVariant> i( errorObject->customDebugInformation() );
    while( i.hasNext() ) {
        i.next();
        htmlDescription += QString( "%1: %2" ).arg( i.key() )
                                              .arg( i.value().toString() );
    }
    htmlDescription += "<br><br>";

    m_errorDetailsTextEdit->setHtml( htmlDescription );
}

//-------------------------------------------------------------------------------------

ErrorLogItem::ErrorLogItem( const KMobileTools::BaseError* errorObject, QListWidget* parent )
: QListWidgetItem( parent, QListWidgetItem::UserType ) {
    m_errorObject = errorObject;

    // set item text
    QString errorText;
    errorText += errorObject->dateTime().toString( Qt::LocaleDate );
    errorText += ": ";
    errorText += errorObject->description();

    setText( errorText );

    // set icon according to error priority
    QIcon icon;
    switch( errorObject->priority() ) {
        case BaseError::Low:
            break;

        case BaseError::Medium:
            icon.addPixmap( KIconLoader::global()->loadIcon("dialog-warning",
                                                            K3Icon::NoGroup,
                                                            K3Icon::SizeMedium ) );
            break;

        case BaseError::High:
            icon.addPixmap( KIconLoader::global()->loadIcon("dialog-error",
                                                            K3Icon::NoGroup,
                                                            K3Icon::SizeMedium ) );
            break;
    }
    setIcon( icon );
}

const BaseError* ErrorLogItem::errorObject() const {
    return m_errorObject;
};

#include "errorlogdialog.moc"
