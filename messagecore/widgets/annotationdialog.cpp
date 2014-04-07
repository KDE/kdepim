/*
   Copyright 2010 Thomas McGuire <mcguire@kde.org>
   Copyright 2014 Laurent Montel <montel@kde.org>

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

#include "annotationdialog.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditorwidget.h"

#include <KMessageBox>
#include <KLocalizedString>
#include <Akonadi/ItemModifyJob>
#include <KSharedConfig>
#include <akonadi/item.h>
#include <akonadi/entityannotationsattribute.h>

#include <KComboBox>

#include <QLabel>
#include <QGridLayout>

using namespace MessageCore;

class AnnotationEditDialog::Private
{
public:
    Private()
        : mTextEdit( 0 ),
          mNoteType( 0 ),
          mHasAnnotation( false )
    {
    }

    Akonadi::Item mItem;
    PimCommon::RichTextEditorWidget *mTextEdit;
    KComboBox *mNoteType;
    bool mHasAnnotation;
};

AnnotationEditDialog::AnnotationEditDialog( const Akonadi::Item &item, QWidget *parent )
    : KDialog( parent ), d( new Private )
{
    d->mItem = item;
    //check for correct key?
    d->mHasAnnotation = item.hasAttribute<Akonadi::EntityAnnotationsAttribute>();
    if ( d->mHasAnnotation ) {
        setCaption( i18n( "Edit Note" ) );
        setButtons( Ok | Cancel | User1 );
        setButtonText( User1, i18n( "Delete Note" ) );
        setButtonIcon( User1, KIcon( QLatin1String("edit-delete") ) );
    } else {
        setCaption( i18n( "Add Note" ) );
        setButtons( Ok | Cancel );
    }

    setDefaultButton( KDialog::Ok );

    QLabel *label = new QLabel( i18n( "Enter the text that should be stored as a note to the mail:" ) );
    QVBoxLayout *vbox = new QVBoxLayout( mainWidget() );
    d->mTextEdit = new PimCommon::RichTextEditorWidget( this );
    d->mTextEdit->setAcceptRichText(false);
    vbox->addWidget( label );
    vbox->addWidget( d->mTextEdit );
    d->mTextEdit->setFocus();

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addStretch();
    label = new QLabel(i18n("Note type:"));
    hbox->addWidget(label);
    d->mNoteType = new KComboBox;
    hbox->addWidget(d->mNoteType);
    d->mNoteType->addItem(i18n("Private note"), QByteArray("/private/comment"));
    d->mNoteType->addItem(i18n("Shared note"), QByteArray("/shared/comment"));

    vbox->addLayout(hbox);
    if ( d->mHasAnnotation && item.attribute<Akonadi::EntityAnnotationsAttribute>() ) {
        if (item.attribute<Akonadi::EntityAnnotationsAttribute>()->contains("/private/comment")) {
            d->mNoteType->setCurrentIndex(d->mNoteType->findData(QLatin1String("/private/comment")));
            d->mTextEdit->setPlainText( item.attribute<Akonadi::EntityAnnotationsAttribute>()->value("/private/comment") );
        } else {
            d->mNoteType->setCurrentIndex(d->mNoteType->findData(QLatin1String("/shared/comment")));
            d->mTextEdit->setPlainText( item.attribute<Akonadi::EntityAnnotationsAttribute>()->value("/shared/comment") );
        }
        //TODO activate it when fix crash
        d->mNoteType->setEnabled(false);
    }
    readConfig();
}

AnnotationEditDialog::~AnnotationEditDialog()
{
    writeConfig();
    delete d;
}

void AnnotationEditDialog::slotButtonClicked( int button )
{
    if ( button == KDialog::Ok ) {
        bool textIsEmpty = d->mTextEdit->toPlainText().isEmpty();
        if ( !textIsEmpty ) {
            Akonadi::EntityAnnotationsAttribute *annotation = d->mItem.attribute<Akonadi::EntityAnnotationsAttribute>(Akonadi::Entity::AddIfMissing);
            annotation->insert(d->mNoteType->itemData(d->mNoteType->currentIndex()).toByteArray(), d->mTextEdit->toPlainText());
            d->mItem.addAttribute(annotation);
        } else if ( d->mHasAnnotation && textIsEmpty ) {
            d->mItem.removeAttribute<Akonadi::EntityAnnotationsAttribute>();
        }
        new Akonadi::ItemModifyJob(d->mItem);

        accept();
    } else if ( button == KDialog::Cancel ) {
        reject();
    } else if ( button == KDialog::User1 ) {
        const int answer = KMessageBox::warningContinueCancel( this,
                                                               i18n( "Do you really want to delete this note?" ),
                                                               i18n( "Delete Note?" ), KGuiItem( i18n( "Delete" ), QLatin1String("edit-delete") ) );
        if ( answer == KMessageBox::Continue ) {
            d->mItem.removeAttribute<Akonadi::EntityAnnotationsAttribute>();
            new Akonadi::ItemModifyJob(d->mItem);
            accept();
        }
    }
}

void AnnotationEditDialog::readConfig()
{
    KSharedConfig::Ptr cfg = KGlobal::config();
    KConfigGroup group( cfg, "AnnotationEditDialog" );
    QSize size = group.readEntry( "Size", QSize() );
    if ( !size.isEmpty() ) {
        resize( size );
    }
}

void AnnotationEditDialog::writeConfig()
{
    KSharedConfig::Ptr cfg = KGlobal::config();
    KConfigGroup group( cfg, "AnnotationEditDialog" );
    group.writeEntry( "Size", size() );
}

