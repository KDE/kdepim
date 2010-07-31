// This must be first
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "klistboxdialog.h"

#include <tqlabel.h>
#include <tqlayout.h>

KListBoxDialog::KListBoxDialog( TQString& _selectedString,
                                const TQString& caption,
                                const TQString& labelText,
                                TQWidget* parent,
                                const char* name,
                                bool modal )
    : KDialogBase( parent, name, modal, caption, Ok|Cancel, Ok, true ),
      selectedString( _selectedString )

{
    if ( !name )
      setName( "KListBoxDialog" );
    resize( 400, 180 );

    TQFrame *page = makeMainWidget();
    TQVBoxLayout *topLayout = new TQVBoxLayout( page, 0, spacingHint() );
    labelAboveLA = new TQLabel( page, "labelAboveLA" );
    labelAboveLA->setText( labelText );

    topLayout->addWidget( labelAboveLA );

    entriesLB = new TQListBox( page, "entriesLB" );

    topLayout->addWidget( entriesLB );

    commentBelowLA = new TQLabel( page, "commentBelowLA" );
    commentBelowLA->setText( "" );
    topLayout->addWidget( commentBelowLA );
    commentBelowLA->hide();

    // signals and slots connections
    connect( entriesLB, TQT_SIGNAL( highlighted( const TQString& ) ),
             this,      TQT_SLOT(   highlighted( const TQString& ) ) );
    connect( entriesLB, TQT_SIGNAL( selected(int) ),
                        TQT_SLOT(   slotOk() ) );
    // buddies
    labelAboveLA->setBuddy( entriesLB );
}

/*
 *  Destroys the object and frees any allocated resources
 */
KListBoxDialog::~KListBoxDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void KListBoxDialog::setLabelAbove(const TQString& label)
{
    labelAboveLA->setText( label );
    if( label.isEmpty() )
        labelAboveLA->hide();
    else
        labelAboveLA->show();
}

void KListBoxDialog::setCommentBelow(const TQString& comment)
{
    commentBelowLA->setText( comment );
    if( comment.isEmpty() )
        commentBelowLA->hide();
    else
        commentBelowLA->show();
}



void KListBoxDialog::highlighted( const TQString& txt )
{
    selectedString = txt;
}

#include "klistboxdialog.moc"
