/*
* typedlg.cpp -- Implementation of class TypeDialog.
* Author:	Sirtaj Singh Kang
* Generated:	Sun May 10 09:01:23 EST 1998
*/


#include <qlistbox.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <qgroupbox.h>
#include <qlayout.h>

#include <klocale.h>

#include "typedlg.h"

//#include "typolayout.h"


TypeDialog::TypeDialog( const QStringList& types, QWidget *parent,
		     const char *name, bool modal )
	: QDialog( parent, name, modal )
{
    setCaption( i18n( "Korn: Select Mailbox Type" ) );

    QBoxLayout *l = new QVBoxLayout (this, 10);

    QGroupBox *aGroup = new QGroupBox ( i18n( "Mailbox Type" ), this );
    l->addWidget( aGroup );

    QGridLayout *layout = new QGridLayout( aGroup , 5, 2, 10 );
    layout->addRowSpacing(0,10);
    layout->addRowSpacing(4,30);
    layout->setRowStretch(1, 0);
    layout->setRowStretch(2, 0);
    layout->setRowStretch(3, 0);
    layout->setRowStretch(4, 1);


    _list = new QListBox( aGroup );
    layout->addMultiCellWidget( _list, 1, 4, 0, 0);
    _list->insertStringList( types );
    _list->setMultiSelection( false );

    connect( _list, SIGNAL(selected(const QString&)),
             this, SLOT(select(const QString&)) );
    connect( _list, SIGNAL(highlighted(const QString&)),
             this, SLOT(setType(const QString&)) );

    btOk = new KPushButton( KStdGuiItem::ok(),  aGroup );
    layout->addWidget( btOk, 1, 1);

    connect( btOk, SIGNAL(clicked()), this, SLOT(accept()) );

    KPushButton *btCancel = new KPushButton( KStdGuiItem::cancel(), aGroup );
    layout->addWidget( btCancel, 3, 1);

    connect(btCancel, SIGNAL(clicked()), this, SLOT(reject()) );
    btOk->setEnabled(_list->currentItem()!=-1);

}

void TypeDialog::setType( const QString& text )
{
    _type = text;
    btOk->setEnabled(_list->currentItem()!=-1);
}

void TypeDialog::select( const QString& text )
{
    _type = text;
    accept();

}
#include "typedlg.moc"
