/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "tagwidget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <KDialog>
#include <KColorCombo>
#include <KFontRequester>
#include <KIconButton>
#include <KKeySequenceWidget>
#include <KActionCollection>
#include <KLineEdit>
#include <KLocale>
#include <KIcon>
using namespace MailCommon;

TagWidget::TagWidget(const QList<KActionCollection*>& actionCollections, QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *settings = new QGridLayout;
    settings->setMargin( KDialog::marginHint() );
    settings->setSpacing( KDialog::spacingHint() );
    setLayout(settings);

    //Stretcher layout for adding some space after the label
    QVBoxLayout *spacer = new QVBoxLayout();
    settings->addLayout( spacer, 0, 0, 1, 2 );
    spacer->addSpacing( 2 * KDialog::spacingHint() );

    //First row for renaming
    mTagNameLineEdit = new KLineEdit( this );
    mTagNameLineEdit->setClearButtonShown(true);
    mTagNameLineEdit->setTrapReturnKey( true );
    settings->addWidget( mTagNameLineEdit, 1, 1 );

    QLabel *namelabel = new QLabel( i18nc("@label:listbox Name of the tag", "Name:")
                                    , this );
    namelabel->setBuddy( mTagNameLineEdit );
    settings->addWidget( namelabel, 1, 0 );

    connect( mTagNameLineEdit, SIGNAL(textChanged(QString)),
             this, SLOT(slotEmitChangeCheck()) );

    //Second row for text color
    mTextColorCheck = new QCheckBox( i18n("Change te&xt color:"),
                                     this );
    settings->addWidget( mTextColorCheck, 2, 0 );

    mTextColorCombo = new KColorCombo( this );
    settings->addWidget( mTextColorCombo, 2, 1 );
    mTextColorCombo->setEnabled(false);

    connect( mTextColorCheck, SIGNAL(toggled(bool)),
             mTextColorCombo, SLOT(setEnabled(bool)) );
    connect( mTextColorCheck, SIGNAL(stateChanged(int)),
             this, SLOT(slotEmitChangeCheck()) );
    connect( mTextColorCombo, SIGNAL(activated(int)),
             this, SLOT(slotEmitChangeCheck()) );

    //Third row for text background color
    mBackgroundColorCheck = new QCheckBox( i18n("Change &background color:"),
                                           this );
    settings->addWidget( mBackgroundColorCheck, 3, 0 );

    mBackgroundColorCombo = new KColorCombo( this );
    settings->addWidget( mBackgroundColorCombo, 3, 1 );
    mBackgroundColorCombo->setEnabled(false);

    connect( mBackgroundColorCheck, SIGNAL(toggled(bool)),
             mBackgroundColorCombo, SLOT(setEnabled(bool)) );
    connect( mBackgroundColorCheck, SIGNAL(stateChanged(int)),
             this, SLOT(slotEmitChangeCheck()) );
    connect( mBackgroundColorCombo, SIGNAL(activated(int)),
             this, SLOT(slotEmitChangeCheck()) );

    //Fourth for font selection
    mTextFontCheck = new QCheckBox( i18n("Change fo&nt:"), this );
    settings->addWidget( mTextFontCheck, 4, 0 );

    mFontRequester = new KFontRequester( this );
    settings->addWidget( mFontRequester, 4, 1 );
    mFontRequester->setEnabled(false);

    connect( mTextFontCheck, SIGNAL(toggled(bool)),
             mFontRequester, SLOT(setEnabled(bool)) );
    connect( mTextFontCheck, SIGNAL(stateChanged(int)),
             this, SLOT(slotEmitChangeCheck()) );
    connect( mFontRequester, SIGNAL(fontSelected(QFont)),
             this, SLOT(slotEmitChangeCheck()) );

    //Fifth for toolbar icon
    mIconButton = new KIconButton( this );
    mIconButton->setIconSize( 16 );
    mIconButton->setIconType( KIconLoader::NoGroup, KIconLoader::Action );
    mIconButton->setIcon(KIcon(QLatin1String("mail-tagged")));
    settings->addWidget( mIconButton, 5, 1 );
    connect( mIconButton, SIGNAL(iconChanged(QString)),
             SIGNAL(iconNameChanged(QString)) );

    QLabel *iconlabel = new QLabel( i18n("Message tag &icon:"),
                                    this );
    iconlabel->setBuddy( mIconButton );
    settings->addWidget( iconlabel, 5, 0 );

    //We do not connect the checkbox to icon selector since icons are used in the
    //menus as well
    connect( mIconButton, SIGNAL(iconChanged(QString)),
             this, SLOT(slotEmitChangeCheck()) );

    //Sixth for shortcut
    mKeySequenceWidget = new KKeySequenceWidget( this );
    settings->addWidget( mKeySequenceWidget, 6, 1 );
    QLabel *sclabel = new QLabel( i18n("Shortc&ut:") , this );
    sclabel->setBuddy( mKeySequenceWidget );
    settings->addWidget( sclabel, 6, 0 );
    if ( !actionCollections.isEmpty() ) {
        mKeySequenceWidget->setCheckActionCollections( actionCollections );
        connect( mKeySequenceWidget, SIGNAL(keySequenceChanged(QKeySequence)),
                 this, SLOT(slotEmitChangeCheck()) );
    }
    else
        mKeySequenceWidget->setEnabled(false);


    //Seventh for Toolbar checkbox
    mInToolbarCheck = new QCheckBox( i18n("Enable &toolbar button"),
                                     this );
    settings->addWidget( mInToolbarCheck, 7, 0 );
    connect( mInToolbarCheck, SIGNAL(stateChanged(int)),
             this, SLOT(slotEmitChangeCheck()) );

}

TagWidget::~TagWidget()
{

}

void TagWidget::slotEmitChangeCheck()
{
    Q_EMIT changed();
}

void TagWidget::setTagTextColor(const QColor& color)
{
    mTextColorCheck->setEnabled( true );
    if ( color.isValid() ) {
        mTextColorCheck->setChecked( true );
        mTextColorCombo->setColor( color );
    } else {
        mTextColorCheck->setChecked( false );
        mTextColorCombo->setColor( Qt::white );
    }
    mTextColorCombo->setEnabled(mTextColorCheck->isChecked());
}

void TagWidget::setTagBackgroundColor(const QColor& color)
{
    mBackgroundColorCheck->setEnabled( true );
    if ( color.isValid() ) {
        mBackgroundColorCheck->setChecked( true );
        mBackgroundColorCombo->setColor( color );
    } else {
        mBackgroundColorCheck->setChecked( false );
        mBackgroundColorCombo->setColor( Qt::white );
    }
    mBackgroundColorCombo->setEnabled( mBackgroundColorCheck->isChecked() );
}

void TagWidget::setTagTextFont(const QFont& font)
{
    mTextFontCheck->setEnabled( true );
    mTextFontCheck->setChecked( ( font != QFont() ) );
    mFontRequester->setFont( font );
    mFontRequester->setEnabled( mTextFontCheck->isChecked() );
}

MailCommon::Tag::SaveFlags TagWidget::saveFlags() const
{
    MailCommon::Tag::SaveFlags saveFlags = 0;
    if ( mTextColorCheck->isChecked() )
        saveFlags |= MailCommon::Tag::TextColor;
    if ( mBackgroundColorCheck->isChecked() )
        saveFlags |= MailCommon::Tag::BackgroundColor;
    if ( mTextFontCheck->isChecked() )
        saveFlags |= MailCommon::Tag::Font;

    return saveFlags;
}

void TagWidget::recordTagSettings( MailCommon::Tag::Ptr tag)
{
    tag->textColor = mTextColorCheck->isChecked() ? mTextColorCombo->color() : QColor();

    tag->backgroundColor = mBackgroundColorCheck->isChecked() ? mBackgroundColorCombo->color() : QColor();

    tag->textFont = mTextFontCheck->isChecked() ? mFontRequester->font() : QFont();

    tag->iconName = iconButton()->icon();
    if (mKeySequenceWidget->isEnabled()) {
        mKeySequenceWidget->applyStealShortcut();
        tag->shortcut = KShortcut( mKeySequenceWidget->keySequence() );
    }

    tag->inToolbar = mInToolbarCheck->isChecked();
}

