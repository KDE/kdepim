#include <kapp.h>
#include "addressEditor.moc"

void AddressEditor::initLayout(void) {
	QWidget *tmp0;
	tmp0 = new QWidget ( this);

	QLabel *tmp1;
	tmp1 = new QLabel ( tmp0, "");

	QLabel *tmp2;
	tmp2 = new QLabel ( tmp0, "");

	QLabel *tmp3;
	tmp3 = new QLabel ( tmp0, "");

	QLabel *tmp4;
	tmp4 = new QLabel ( tmp0, "");

	QLabel *tmp5;
	tmp5 = new QLabel ( tmp0, "");

	QLabel *tmp6;
	tmp6 = new QLabel ( tmp0, "");

	QLabel *tmp7;
	tmp7 = new QLabel ( tmp0, "");

	QLabel *tmp8;
	tmp8 = new QLabel ( tmp0, "");

	QLabel *tmp9;
	tmp9 = new QLabel ( tmp0, "");

	QLabel *tmp10;
	tmp10 = new QLabel ( tmp0, "");

	QLabel *tmp11;
	tmp11 = new QLabel ( tmp0, "");

	QLabel *tmp12;
	tmp12 = new QLabel ( tmp0, "");

	QLabel *tmp13;
	tmp13 = new QLabel ( tmp0, "");

	QLabel *tmp14;
	tmp14 = new QLabel ( tmp0, "");

	QLabel *tmp15;
	tmp15 = new QLabel ( tmp0, "");

	QLabel *tmp16;
	tmp16 = new QLabel ( tmp0, "");

	QLabel *tmp17;
	tmp17 = new QLabel ( tmp0, "");

	QLabel *tmp18;
	tmp18 = new QLabel ( tmp0, "");

	fLastNameField = new QLineEdit ( tmp0, "QLineEdit1" );

	fFirstNameField = new QLineEdit ( tmp0, "QLineEdit2" );

	fTitleField = new QLineEdit ( tmp0, "QLineEdit3" );

	fCompanyField = new QLineEdit ( tmp0, "QLineEdit4" );

	fPhone1Field = new QLineEdit ( tmp0, "QLineEdit5" );

	fPhone2Field = new QLineEdit ( tmp0, "QLineEdit6" );

	fPhone3Field = new QLineEdit ( tmp0, "QLineEdit7" );

	fPhone4Field = new QLineEdit ( tmp0, "QLineEdit8" );

	fPhone5Field = new QLineEdit ( tmp0, "QLineEdit9" );

	fAddressField = new QLineEdit ( tmp0, "QLineEdit10" );

	fCityField = new QLineEdit ( tmp0, "QLineEdit11" );

	fStateField = new QLineEdit ( tmp0, "QLineEdit12" );

	fZipField = new QLineEdit ( tmp0, "QLineEdit13" );

	fCountryField = new QLineEdit ( tmp0, "QLineEdit14" );

	fCustom1Field = new QLineEdit ( tmp0, "QLineEdit15" );

	fCustom2Field = new QLineEdit ( tmp0, "QLineEdit16" );

	fCustom3Field = new QLineEdit ( tmp0, "QLineEdit17" );

	fCustom4Field = new QLineEdit ( tmp0, "QLineEdit18" );

	fOkButton = new QPushButton ( tmp0, "QButton1");

	fCancelButton = new QPushButton ( tmp0, "QButton2");
	setCaption(klocale->translate("Address Editor"));
	setGeometry(0, 0, 570, 360);
	tmp0->setCaption( klocale->translate("Address Editor"));
	tmp0->setGeometry( 0, 0, 570, 360 );
	tmp0->setMinimumSize( 0, 0 );
	tmp0->setMaximumSize( 32767, 32767 );
	tmp0->setMouseTracking( FALSE);
	tmp0->setSizeIncrement( 0, 0 );

	tmp1->setCaption( "" );
	tmp1->setGeometry( 15, 15, 75, 30 );
	tmp1->setMinimumSize( 0, 0 );
	tmp1->setMaximumSize( 32767, 32767 );
	tmp1->setMouseTracking( FALSE);
	tmp1->setSizeIncrement( 0, 0 );
	tmp1->setLineWidth( 1 );
	tmp1->setMidLineWidth( 0 );
	tmp1->setText( klocale->translate("Last Name:") );
	tmp1->setMargin( -1 );
	tmp1->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp1->setAutoResize( FALSE );

	tmp2->setCaption( "" );
	tmp2->setGeometry( 15, 45, 75, 30 );
	tmp2->setMinimumSize( 0, 0 );
	tmp2->setMaximumSize( 32767, 32767 );
	tmp2->setMouseTracking( FALSE);
	tmp2->setSizeIncrement( 0, 0 );
	tmp2->setLineWidth( 1 );
	tmp2->setMidLineWidth( 0 );
	tmp2->setText( klocale->translate("First Name:") );
	tmp2->setMargin( -1 );
	tmp2->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp2->setAutoResize( FALSE );

	tmp3->setCaption( "" );
	tmp3->setGeometry( 15, 75, 75, 30 );
	tmp3->setMinimumSize( 0, 0 );
	tmp3->setMaximumSize( 32767, 32767 );
	tmp3->setMouseTracking( FALSE);
	tmp3->setSizeIncrement( 0, 0 );
	tmp3->setLineWidth( 1 );
	tmp3->setMidLineWidth( 0 );
	tmp3->setText( klocale->translate("Title:") );
	tmp3->setMargin( -1 );
	tmp3->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp3->setAutoResize( FALSE );

	tmp4->setCaption( "" );
	tmp4->setGeometry( 15, 105, 75, 30 );
	tmp4->setMinimumSize( 0, 0 );
	tmp4->setMaximumSize( 32767, 32767 );
	tmp4->setMouseTracking( FALSE);
	tmp4->setSizeIncrement( 0, 0 );
	tmp4->setLineWidth( 1 );
	tmp4->setMidLineWidth( 0 );
	tmp4->setText( klocale->translate("Company:") );
	tmp4->setMargin( -1 );
	tmp4->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp4->setAutoResize( FALSE );

	tmp5->setCaption( "" );
	tmp5->setGeometry( 15, 135, 75, 30 );
	tmp5->setMinimumSize( 0, 0 );
	tmp5->setMaximumSize( 32767, 32767 );
	tmp5->setMouseTracking( FALSE);
	tmp5->setSizeIncrement( 0, 0 );
	tmp5->setLineWidth( 1 );
	tmp5->setMidLineWidth( 0 );
	tmp5->setText( klocale->translate("Phone 1:") );
	tmp5->setMargin( -1 );
	tmp5->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp5->setAutoResize( FALSE );

	tmp6->setCaption( "" );
	tmp6->setGeometry( 15, 165, 75, 30 );
	tmp6->setMinimumSize( 0, 0 );
	tmp6->setMaximumSize( 32767, 32767 );
	tmp6->setMouseTracking( FALSE);
	tmp6->setSizeIncrement( 0, 0 );
	tmp6->setLineWidth( 1 );
	tmp6->setMidLineWidth( 0 );
	tmp6->setText( klocale->translate("Phone 2:") );
	tmp6->setMargin( -1 );
	tmp6->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp6->setAutoResize( FALSE );

	tmp7->setCaption( "" );
	tmp7->setGeometry( 15, 195, 75, 30 );
	tmp7->setMinimumSize( 0, 0 );
	tmp7->setMaximumSize( 32767, 32767 );
	tmp7->setMouseTracking( FALSE);
	tmp7->setSizeIncrement( 0, 0 );
	tmp7->setLineWidth( 1 );
	tmp7->setMidLineWidth( 0 );
	tmp7->setText(klocale->translate("Phone 3:") );
	tmp7->setMargin( -1 );
	tmp7->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp7->setAutoResize( FALSE );

	tmp8->setCaption( "" );
	tmp8->setGeometry( 15, 225, 75, 30 );
	tmp8->setMinimumSize( 0, 0 );
	tmp8->setMaximumSize( 32767, 32767 );
	tmp8->setMouseTracking( FALSE);
	tmp8->setSizeIncrement( 0, 0 );
	tmp8->setLineWidth( 1 );
	tmp8->setMidLineWidth( 0 );
	tmp8->setText(klocale->translate("Phone 4:"));
	tmp8->setMargin( -1 );
	tmp8->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp8->setAutoResize( FALSE );

	tmp9->setCaption( "" );
	tmp9->setGeometry( 15, 255, 75, 30 );
	tmp9->setMinimumSize( 0, 0 );
	tmp9->setMaximumSize( 32767, 32767 );
	tmp9->setMouseTracking( FALSE);
	tmp9->setSizeIncrement( 0, 0 );
	tmp9->setLineWidth( 1 );
	tmp9->setMidLineWidth( 0 );
	tmp9->setText( klocale->translate("Phone 5:"));
	tmp9->setMargin( -1 );
	tmp9->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp9->setAutoResize( FALSE );

	tmp10->setCaption( "" );
	tmp10->setGeometry( 300, 15, 60, 30 );
	tmp10->setMinimumSize( 0, 0 );
	tmp10->setMaximumSize( 32767, 32767 );
	tmp10->setMouseTracking( FALSE);
	tmp10->setSizeIncrement( 0, 0 );
	tmp10->setLineWidth( 1 );
	tmp10->setMidLineWidth( 0 );
	tmp10->setText(klocale->translate("Address:"));
	tmp10->setMargin( -1 );
	tmp10->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp10->setAutoResize( FALSE );

	tmp11->setCaption( "" );
	tmp11->setGeometry( 300, 45, 60, 30 );
	tmp11->setMinimumSize( 0, 0 );
	tmp11->setMaximumSize( 32767, 32767 );
	tmp11->setMouseTracking( FALSE);
	tmp11->setSizeIncrement( 0, 0 );
	tmp11->setLineWidth( 1 );
	tmp11->setMidLineWidth( 0 );
	tmp11->setText(klocale->translate("City:"));
	tmp11->setMargin( -1 );
	tmp11->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp11->setAutoResize( FALSE );

	tmp12->setCaption( "" );
	tmp12->setGeometry( 300, 75, 60, 30 );
	tmp12->setMinimumSize( 0, 0 );
	tmp12->setMaximumSize( 32767, 32767 );
	tmp12->setMouseTracking( FALSE);
	tmp12->setSizeIncrement( 0, 0 );
	tmp12->setLineWidth( 1 );
	tmp12->setMidLineWidth( 0 );
	tmp12->setText(klocale->translate("State:"));
	tmp12->setMargin( -1 );
	tmp12->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp12->setAutoResize( FALSE );

	tmp13->setCaption( "" );
	tmp13->setGeometry( 300, 105, 60, 30 );
	tmp13->setMinimumSize( 0, 0 );
	tmp13->setMaximumSize( 32767, 32767 );
	tmp13->setMouseTracking( FALSE);
	tmp13->setSizeIncrement( 0, 0 );
	tmp13->setLineWidth( 1 );
	tmp13->setMidLineWidth( 0 );
	tmp13->setText(klocale->translate("Zip Code:"));
	tmp13->setMargin( -1 );
	tmp13->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp13->setAutoResize( FALSE );

	tmp14->setCaption( "" );
	tmp14->setGeometry( 300, 135, 60, 30 );
	tmp14->setMinimumSize( 0, 0 );
	tmp14->setMaximumSize( 32767, 32767 );
	tmp14->setMouseTracking( FALSE);
	tmp14->setSizeIncrement( 0, 0 );
	tmp14->setLineWidth( 1 );
	tmp14->setMidLineWidth( 0 );
	tmp14->setText(klocale->translate("Country:"));
	tmp14->setMargin( -1 );
	tmp14->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp14->setAutoResize( FALSE );

	tmp15->setCaption( "" );
	tmp15->setGeometry( 300, 165, 60, 30 );
	tmp15->setMinimumSize( 0, 0 );
	tmp15->setMaximumSize( 32767, 32767 );
	tmp15->setMouseTracking( FALSE);
	tmp15->setSizeIncrement( 0, 0 );
	tmp15->setLineWidth( 1 );
	tmp15->setMidLineWidth( 0 );
	tmp15->setText(klocale->translate("Custom 1:"));
	tmp15->setMargin( -1 );
	tmp15->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp15->setAutoResize( FALSE );

	tmp16->setCaption( "" );
	tmp16->setGeometry( 300, 195, 60, 30 );
	tmp16->setMinimumSize( 0, 0 );
	tmp16->setMaximumSize( 32767, 32767 );
	tmp16->setMouseTracking( FALSE);
	tmp16->setSizeIncrement( 0, 0 );
	tmp16->setLineWidth( 1 );
	tmp16->setMidLineWidth( 0 );
	tmp16->setText(klocale->translate("Custom 2:"));
	tmp16->setMargin( -1 );
	tmp16->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp16->setAutoResize( FALSE );

	tmp17->setCaption( "" );
	tmp17->setGeometry( 300, 225, 60, 30 );
	tmp17->setMinimumSize( 0, 0 );
	tmp17->setMaximumSize( 32767, 32767 );
	tmp17->setMouseTracking( FALSE);
	tmp17->setSizeIncrement( 0, 0 );
	tmp17->setLineWidth( 1 );
	tmp17->setMidLineWidth( 0 );
	tmp17->setText(klocale->translate("Custom 3:"));
	tmp17->setMargin( -1 );
	tmp17->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp17->setAutoResize( FALSE );

	tmp18->setCaption( "" );
	tmp18->setGeometry( 300, 255, 60, 30 );
	tmp18->setMinimumSize( 0, 0 );
	tmp18->setMaximumSize( 32767, 32767 );
	tmp18->setMouseTracking( FALSE);
	tmp18->setSizeIncrement( 0, 0 );
	tmp18->setLineWidth( 1 );
	tmp18->setMidLineWidth( 0 );
	tmp18->setText(klocale->translate("Custom 4:"));
	tmp18->setMargin( -1 );
	tmp18->setAlignment( AlignLeft | AlignVCenter | ExpandTabs );
	tmp18->setAutoResize( FALSE );

	fLastNameField->setCaption( "" );
	fLastNameField->setGeometry( 90, 15, 180, 30 );
	fLastNameField->setMinimumSize( 0, 0 );
	fLastNameField->setMaximumSize( 32767, 32767 );
	fLastNameField->setMouseTracking( FALSE);
	fLastNameField->setSizeIncrement( 0, 0 );
	fLastNameField->setText( "" );
// 	fLastNameField->setMaxLength( 30 );
	fLastNameField->setFrame( TRUE );
	fLastNameField->setEchoMode( QLineEdit::Normal );

	fFirstNameField->setCaption( "" );
	fFirstNameField->setGeometry( 90, 45, 180, 30 );
	fFirstNameField->setMinimumSize( 0, 0 );
	fFirstNameField->setMaximumSize( 32767, 32767 );
	fFirstNameField->setMouseTracking( FALSE);
	fFirstNameField->setSizeIncrement( 0, 0 );
	fFirstNameField->setText( "" );
// 	fFirstNameField->setMaxLength( 30 );
	fFirstNameField->setFrame( TRUE );
	fFirstNameField->setEchoMode( QLineEdit::Normal );

	fTitleField->setCaption( "" );
	fTitleField->setGeometry( 90, 75, 180, 30 );
	fTitleField->setMinimumSize( 0, 0 );
	fTitleField->setMaximumSize( 32767, 32767 );
	fTitleField->setMouseTracking( FALSE);
	fTitleField->setSizeIncrement( 0, 0 );
	fTitleField->setText( "" );
// 	fTitleField->setMaxLength( 30 );
	fTitleField->setFrame( TRUE );
	fTitleField->setEchoMode( QLineEdit::Normal );

	fCompanyField->setCaption( "" );
	fCompanyField->setGeometry( 90, 105, 180, 30 );
	fCompanyField->setMinimumSize( 0, 0 );
	fCompanyField->setMaximumSize( 32767, 32767 );
	fCompanyField->setMouseTracking( FALSE);
	fCompanyField->setSizeIncrement( 0, 0 );
	fCompanyField->setText( "" );
// 	fCompanyField->setMaxLength( 30 );
	fCompanyField->setFrame( TRUE );
	fCompanyField->setEchoMode( QLineEdit::Normal );

	fPhone1Field->setCaption( "" );
	fPhone1Field->setGeometry( 90, 135, 180, 30 );
	fPhone1Field->setMinimumSize( 0, 0 );
	fPhone1Field->setMaximumSize( 32767, 32767 );
	fPhone1Field->setMouseTracking( FALSE);
	fPhone1Field->setSizeIncrement( 0, 0 );
	fPhone1Field->setText( "" );
// 	fPhone1Field->setMaxLength( 30 );
	fPhone1Field->setFrame( TRUE );
	fPhone1Field->setEchoMode( QLineEdit::Normal );

	fPhone2Field->setCaption( "" );
	fPhone2Field->setGeometry( 90, 165, 180, 30 );
	fPhone2Field->setMinimumSize( 0, 0 );
	fPhone2Field->setMaximumSize( 32767, 32767 );
	fPhone2Field->setMouseTracking( FALSE);
	fPhone2Field->setSizeIncrement( 0, 0 );
	fPhone2Field->setText( "" );
// 	fPhone2Field->setMaxLength( 30 );
	fPhone2Field->setFrame( TRUE );
	fPhone2Field->setEchoMode( QLineEdit::Normal );

	fPhone3Field->setCaption( "" );
	fPhone3Field->setGeometry( 90, 195, 180, 30 );
	fPhone3Field->setMinimumSize( 0, 0 );
	fPhone3Field->setMaximumSize( 32767, 32767 );
	fPhone3Field->setMouseTracking( FALSE);
	fPhone3Field->setSizeIncrement( 0, 0 );
	fPhone3Field->setText( "" );
// 	fPhone3Field->setMaxLength( 30 );
	fPhone3Field->setFrame( TRUE );
	fPhone3Field->setEchoMode( QLineEdit::Normal );

	fPhone4Field->setCaption( "" );
	fPhone4Field->setGeometry( 90, 225, 180, 30 );
	fPhone4Field->setMinimumSize( 0, 0 );
	fPhone4Field->setMaximumSize( 32767, 32767 );
	fPhone4Field->setMouseTracking( FALSE);
	fPhone4Field->setSizeIncrement( 0, 0 );
	fPhone4Field->setText( "" );
// 	fPhone4Field->setMaxLength( 30 );
	fPhone4Field->setFrame( TRUE );
	fPhone4Field->setEchoMode( QLineEdit::Normal );

	fPhone5Field->setCaption( "" );
	fPhone5Field->setGeometry( 90, 255, 180, 30 );
	fPhone5Field->setMinimumSize( 0, 0 );
	fPhone5Field->setMaximumSize( 32767, 32767 );
	fPhone5Field->setMouseTracking( FALSE);
	fPhone5Field->setSizeIncrement( 0, 0 );
	fPhone5Field->setText( "" );
// 	fPhone5Field->setMaxLength( 30 );
	fPhone5Field->setFrame( TRUE );
	fPhone5Field->setEchoMode( QLineEdit::Normal );

	fAddressField->setCaption( "" );
	fAddressField->setGeometry( 375, 15, 180, 30 );
	fAddressField->setMinimumSize( 0, 0 );
	fAddressField->setMaximumSize( 32767, 32767 );
	fAddressField->setMouseTracking( FALSE);
	fAddressField->setSizeIncrement( 0, 0 );
	fAddressField->setText( "" );
// 	fAddressField->setMaxLength( 30 );
	fAddressField->setFrame( TRUE );
	fAddressField->setEchoMode( QLineEdit::Normal );

	fCityField->setCaption( "" );
	fCityField->setGeometry( 375, 45, 180, 30 );
	fCityField->setMinimumSize( 0, 0 );
	fCityField->setMaximumSize( 32767, 32767 );
	fCityField->setMouseTracking( FALSE);
	fCityField->setSizeIncrement( 0, 0 );
	fCityField->setText( "" );
// 	fCityField->setMaxLength( 30 );
	fCityField->setFrame( TRUE );
	fCityField->setEchoMode( QLineEdit::Normal );

	fStateField->setCaption( "" );
	fStateField->setGeometry( 375, 75, 180, 30 );
	fStateField->setMinimumSize( 0, 0 );
	fStateField->setMaximumSize( 32767, 32767 );
	fStateField->setMouseTracking( FALSE);
	fStateField->setSizeIncrement( 0, 0 );
	fStateField->setText( "" );
// 	fStateField->setMaxLength( 30 );
	fStateField->setFrame( TRUE );
	fStateField->setEchoMode( QLineEdit::Normal );

	fZipField->setCaption( "" );
	fZipField->setGeometry( 375, 105, 180, 30 );
	fZipField->setMinimumSize( 0, 0 );
	fZipField->setMaximumSize( 32767, 32767 );
	fZipField->setMouseTracking( FALSE);
	fZipField->setSizeIncrement( 0, 0 );
	fZipField->setText( "" );
// 	fZipField->setMaxLength( 30 );
	fZipField->setFrame( TRUE );
	fZipField->setEchoMode( QLineEdit::Normal );

	fCountryField->setCaption( "" );
	fCountryField->setGeometry( 375, 135, 180, 30 );
	fCountryField->setMinimumSize( 0, 0 );
	fCountryField->setMaximumSize( 32767, 32767 );
	fCountryField->setMouseTracking( FALSE);
	fCountryField->setSizeIncrement( 0, 0 );
	fCountryField->setText( "" );
// 	fCountryField->setMaxLength( 30 );
	fCountryField->setFrame( TRUE );
	fCountryField->setEchoMode( QLineEdit::Normal );

	fCustom1Field->setCaption( "" );
	fCustom1Field->setGeometry( 375, 165, 180, 30 );
	fCustom1Field->setMinimumSize( 0, 0 );
	fCustom1Field->setMaximumSize( 32767, 32767 );
	fCustom1Field->setMouseTracking( FALSE);
	fCustom1Field->setSizeIncrement( 0, 0 );
	fCustom1Field->setText( "" );
// 	fCustom1Field->setMaxLength( 30 );
	fCustom1Field->setFrame( TRUE );
	fCustom1Field->setEchoMode( QLineEdit::Normal );

	fCustom2Field->setCaption( "" );
	fCustom2Field->setGeometry( 375, 195, 180, 30 );
	fCustom2Field->setMinimumSize( 0, 0 );
	fCustom2Field->setMaximumSize( 32767, 32767 );
	fCustom2Field->setMouseTracking( FALSE);
	fCustom2Field->setSizeIncrement( 0, 0 );
	fCustom2Field->setText( "" );
// 	fCustom2Field->setMaxLength( 30 );
	fCustom2Field->setFrame( TRUE );
	fCustom2Field->setEchoMode( QLineEdit::Normal );

	fCustom3Field->setCaption( "" );
	fCustom3Field->setGeometry( 375, 225, 180, 30 );
	fCustom3Field->setMinimumSize( 0, 0 );
	fCustom3Field->setMaximumSize( 32767, 32767 );
	fCustom3Field->setMouseTracking( FALSE);
	fCustom3Field->setSizeIncrement( 0, 0 );
	fCustom3Field->setText( "" );
// 	fCustom3Field->setMaxLength( 30 );
	fCustom3Field->setFrame( TRUE );
	fCustom3Field->setEchoMode( QLineEdit::Normal );

	fCustom4Field->setCaption( "" );
	fCustom4Field->setGeometry( 375, 255, 180, 30 );
	fCustom4Field->setMinimumSize( 0, 0 );
	fCustom4Field->setMaximumSize( 32767, 32767 );
	fCustom4Field->setMouseTracking( FALSE);
	fCustom4Field->setSizeIncrement( 0, 0 );
	fCustom4Field->setText( "" );
// 	fCustom4Field->setMaxLength( 30 );
	fCustom4Field->setFrame( TRUE );
	fCustom4Field->setEchoMode( QLineEdit::Normal );

	fOkButton->setCaption( "" );
	fOkButton->setGeometry( 135, 315, 90, 30 );
	fOkButton->setMinimumSize( 0, 0 );
	fOkButton->setMaximumSize( 32767, 32767 );
	fOkButton->setMouseTracking( FALSE);
	fOkButton->setSizeIncrement( 0, 0 );
	fOkButton->setText(klocale->translate("OK"));
	fOkButton->setAutoResize( FALSE );
	fOkButton->setAutoRepeat( FALSE );
	connect(fOkButton, SIGNAL(clicked()), this, SLOT(commitChanges()));

	fCancelButton->setCaption( "" );
	fCancelButton->setGeometry( 345, 315, 90, 30 );
	fCancelButton->setMinimumSize( 0, 0 );
	fCancelButton->setMaximumSize( 32767, 32767 );
	fCancelButton->setMouseTracking( FALSE);
	fCancelButton->setSizeIncrement( 0, 0 );
	fCancelButton->setText(klocale->translate("Cancel"));
	fCancelButton->setAutoResize( FALSE );
	fCancelButton->setAutoRepeat( FALSE );
	connect(fCancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
}
