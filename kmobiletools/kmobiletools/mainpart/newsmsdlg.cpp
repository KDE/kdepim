/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

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
#include "newsmsdlg.h"
#include <klocale.h>
#include <qlayout.h>
#include <ktextedit.h>
#include <kstatusbar.h>
#include <kpushbutton.h>
#include <k3listview.h>
#include <klineedit.h>
#include <kguiitem.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <libkmobiletools/kmobiletoolshelper.h>
#include <pickphonenumberdialog.h>
#include <libkmobiletools/encodingshelper.h>
#include <libkmobiletools/engineslist.h>
#include <libkmobiletools/sms.h>
#include <libkmobiletools/engine.h>

newSMSDlg::newSMSDlg(QWidget *parent, const QString &name)
    : KDialog(parent)
{
    setObjectName(name);
    setCaption(i18n("New SMS"));
    setButtons(Cancel | User1 | User2);
    kDebug() << "newSMSDlg with name " << name << endl;
    p_sms=0;
    ui.setupUi(mainWidget());
//     ui.smsText->setText(name);
    QVBoxLayout *layoutStatusBar=new QVBoxLayout(ui.statusBarFrame);
    layoutStatusBar->setMargin(0);
    statusBar=new KStatusBar(ui.statusBarFrame);
    layoutStatusBar->addWidget(statusBar);
    connect(ui.smsText, SIGNAL(textChanged()), this, SLOT(smsTextChanged()) );
    connect(ui.pickNumber, SIGNAL(clicked() ), this, SLOT(pickPhoneNumber() ));
    connect(ui.editNumber, SIGNAL(textChanged(const QString &) ), this, SLOT(textNumberChanged( const QString & ) ));
    connect(ui.lv_numbers, SIGNAL(clicked(Q3ListViewItem*)), this, SLOT(NumberClicked( Q3ListViewItem* )) );
    connect(ui.buttonAdd, SIGNAL(clicked() ), this, SLOT(addClicked() ) );
    connect(ui.buttonRemove, SIGNAL(clicked() ), this, SLOT(remClicked() ) );
    connect(this, SIGNAL(user1Clicked()), SLOT(slotUser1()));
    connect(this, SIGNAL(user2Clicked()), SLOT(slotUser2()));
    setButtonGuiItem(User1, KGuiItem(i18nc("Send SMS directly", "Send"), "mail_send", i18n("Send SMS directly") ));
    setButtonGuiItem(User2, KGuiItem(i18nc("Save SMS to mobile phone memory", "Save"), "filesave", i18n("Save SMS to mobile phone memory") ));
    resize(567,390);
}


newSMSDlg::~newSMSDlg()
{
}


#include "newsmsdlg.moc"


/*!
    \fn newSMSDlg::smsTextChanged()
 */
void newSMSDlg::smsTextChanged()
{
    int smslength=ui.smsText->toPlainText().length();
    QString statusTexT=i18n("SMS Text Length: %1 characters. Total SMS Count: %2. Encoding: %3", smslength,
            KMobileTools::SMS::getMultiTextCount(smslength),
            KMobileTools::EncodingsHelper::encodingNameString( KMobileTools::EncodingsHelper::hasEncoding(ui.smsText->toPlainText(),
                KMobileTools::EnginesList::instance()->find(objectName(), false)->pdu()
                ) ) );
    statusBar->showMessage(statusTexT);
}


/*!
    \fn newSMSDlg::pickPhoneNumber()
 */
void newSMSDlg::pickPhoneNumber()
{
    PickPhoneNumberDialog dialog(this);
    if(dialog.exec() != QDialog::Accepted ) return;
    if(!dialog.selectedNumbers().count() ) return;
    QStringList::iterator it;
    QStringList phonenumbers=dialog.selectedNumbers();
    for(it=phonenumbers.begin(); it!=phonenumbers.end(); ++it)
        addNumber(*it);
}

void newSMSDlg::addNumber( const QString &number)
{
    ui.lv_numbers->clear();
    ui.buttonAdd->setEnabled(false);
    sl_numbers+=number;
    QStringList::Iterator it;
    for(it=sl_numbers.begin(); it!= sl_numbers.end(); ++it)
        new K3ListViewItem(ui.lv_numbers, *it, KMobileTools::KMobiletoolsHelper::translateNumber( *it ) );
}

const QString newSMSDlg::text()
{
    return ui.smsText->text();
}


/*!
    \fn newSMSDlg::textNumberChanged()
 */
void newSMSDlg::textNumberChanged(const QString &text)
{
    if(text.length() ) ui.buttonAdd->setEnabled(true);
    else ui.buttonAdd->setEnabled(false);
}


/*!
    \fn newSMSDlg::NumberClicked(QListViewItem *)
 */
void newSMSDlg::NumberClicked(Q3ListViewItem *item)
{
    if(item) ui.buttonRemove->setEnabled(true);
    else ui.buttonRemove->setEnabled(false);
}


/*!
    \fn newSMSDlg::remClicked()
 */
void newSMSDlg::remClicked()
{
    if(! ui.lv_numbers->currentItem() ) return;
    sl_numbers.removeAll( ui.lv_numbers->currentItem()->text(0) );
    delete ui.lv_numbers->currentItem();
}


/*!
    \fn newSMSDlg::addClicked()
 */
void newSMSDlg::addClicked()
{
    addNumber(ui.editNumber->text() );
}


/*!
    \fn newSMSDlg::slotUser1()
 */
void newSMSDlg::slotUser1()
{
    // User clicked "Send"
    createSMSItem();
    i_action=Send;
    done(Accepted);
}


/*!
    \fn newSMSDlg::slotUser2()
 */
void newSMSDlg::slotUser2()
{
    // User clicked "Store"
    createSMSItem();
    i_action=Store;
    done(QDialog::Accepted);
}

void newSMSDlg::createSMSItem()
{
//     kDebug() << "newSMSDlg; numbers: " << sl_numbers << endl;
    p_sms=new KMobileTools::SMS(sl_numbers, ui.smsText->text() );
    p_sms->setType( KMobileTools::SMS::Unsent );
}
