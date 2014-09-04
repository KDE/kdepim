
#include "chiasmuskeyselector.h"

#include <KLineEdit>
#include <QListWidget>
#include <KLocalizedString>

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <KConfigGroup>
#include <QDialogButtonBox>

using namespace MessageViewer;

ChiasmusKeySelector::ChiasmusKeySelector( QWidget* parent, const QString& caption,
                                          const QStringList& keys, const QString& currentKey,
                                          const QString& lastOptions )
    : QDialog( parent )
{
    setWindowTitle( caption );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ChiasmusKeySelector::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ChiasmusKeySelector::reject);
    QWidget *page = new QWidget( this );
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);


    QVBoxLayout *layout = new QVBoxLayout(page);
//TODO PORT QT5     layout->setSpacing(QDialog::spacingHint());

    mLabel = new QLabel( i18n( "Please select the Chiasmus key file to use:" ), page );
    mainLayout->addWidget(mLabel);
    layout->addWidget( mLabel );

    mListBox = new QListWidget( page );
    mainLayout->addWidget(mListBox);
    mListBox->addItems( keys );
    const int current = keys.indexOf( currentKey );
    mListBox->setCurrentRow( qMax( 0, current ) );
    mListBox->scrollToItem( mListBox->item( qMax( 0, current ) ) );
    layout->addWidget( mListBox, 1 );

    QLabel* optionLabel = new QLabel( i18n( "Additional arguments for chiasmus:" ), page );
    mainLayout->addWidget(optionLabel);
    layout->addWidget( optionLabel );

    mOptions = new KLineEdit( lastOptions, page );
    mainLayout->addWidget(mOptions);
    optionLabel->setBuddy( mOptions );
    layout->addWidget( mOptions );

    layout->addStretch();

    connect(mListBox, &QListWidget::itemDoubleClicked, this, &ChiasmusKeySelector::accept);
    connect(mListBox, &QListWidget::itemSelectionChanged, this, &ChiasmusKeySelector::slotItemSelectionChanged);

    slotItemSelectionChanged();
    mListBox->setFocus();
}

void ChiasmusKeySelector::slotItemSelectionChanged()
{
    mOkButton->setEnabled( !mListBox->selectedItems().isEmpty() );
}

QString ChiasmusKeySelector::key() const
{
    if (mListBox->selectedItems().isEmpty()) {
        return QString();
    } else {
        return mListBox->currentItem()->text();
    }
}

QString ChiasmusKeySelector::options() const
{
    return mOptions->text();
}
