/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma implementation "EmpathComposeWindow.h"
#endif

// Qt includes
#include <qmessagebox.h>

// KDE includes
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kapp.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathUIUtils.h"
#include "EmpathComposeWindow.h"
#include "EmpathComposeWidget.h"
#include "EmpathMessageWidget.h"
#include "EmpathFolderChooserDialog.h"
#include "EmpathMailSender.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include <RMM_Message.h>

/*
EmpathComposeWindow::EmpathComposeWindow()
    : KTMainWindow()
{
    composeWidget_    =
        new EmpathComposeWidget(this, "composeWidget");
    
    _init();
}
*/

EmpathComposeWindow::EmpathComposeWindow(const EmpathComposer::Form & f)
    :    KTMainWindow()
{
    composeWidget_    =
        new EmpathComposeWidget(f, this, "composeWidget");
    
    _init();
}

    void
EmpathComposeWindow::_init()
{
    QObject::connect(
        this,            SIGNAL(cut()),
        composeWidget_,  SLOT(s_cut()));

    QObject::connect(
        this,            SIGNAL(copy()),
        composeWidget_,  SLOT(s_copy()));

    QObject::connect(
        this,            SIGNAL(paste()),
        composeWidget_,  SLOT(s_paste()));

    QObject::connect(
        this,            SIGNAL(selectAll()),
        composeWidget_,  SLOT(s_selectAll()));
 
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setView(composeWidget_, false);
    setCaption(i18n("Compose Message"));
    updateRects();
}

EmpathComposeWindow::~EmpathComposeWindow()
{
    // Empty.
}
    
    void
EmpathComposeWindow::setupToolBar() 
{
    QPixmap p = empathIcon("toolbar-compose");
    int i = QMAX(p.width(), p.height());

    KToolBar * tb = new KToolBar(this, "tooly", i + 4);
    
    tb->setFullWidth(false);

    this->addToolBar(tb, 0);

    tb->insertButton(empathIcon("toolbar-send"), 0, SIGNAL(clicked()),
            this, SLOT(s_fileSendMessage()), true, i18n("Send"));
    
    tb->insertButton(empathIcon("toolbar-sendlater"), 0, SIGNAL(clicked()),
            this, SLOT(s_fileSendLater()), true, i18n("Send Later"));
    
    tb->insertButton(empathIcon("toolbar-save"), 0, SIGNAL(clicked()),
            this, SLOT(s_fileSaveAs()), true, i18n("Save"));
    
    KToolBar * tb2 = new KToolBar(this, "tooly2", i + 4 );

    tb2->setFullWidth(false);
    
    this->addToolBar(tb2, 1);
        
    id_confirmDelivery_     = 8;
    id_confirmReading_      = 9;
    id_addSignature_        = 10;
    id_digitallySign_       = 11;
    id_encrypt_             = 12;
    
    tb2->insertButton(
        empathIcon("toolbar-confirm-delivery"),
        id_confirmDelivery_, SIGNAL(toggled(bool)),
        this, SLOT(s_confirmDelivery(bool)), true, i18n("Confirm Delivery"));
    
    tb2->insertButton(
        empathIcon("toolbar-confirm-reading"),
        id_confirmReading_, SIGNAL(toggled(bool)),
        this, SLOT(s_confirmReading(bool)), true, i18n("Confirm Reading"));
    
    tb2->insertButton(
        empathIcon("toolbar-encrypt"),
        id_encrypt_, SIGNAL(toggled(bool)),
        this, SLOT(s_encrypt(bool)), true, i18n("Encrypt"));
    
    tb2->insertButton(
        empathIcon("toolbar-digsign"),
        id_digitallySign_, SIGNAL(toggled(bool)),
        this, SLOT(s_digitallySign(bool)), true, i18n("Digitally Sign"));

    tb2->insertButton(
        empathIcon("toolbar-sign"),
        id_addSignature_, SIGNAL(toggled(bool)),
        this, SLOT(s_addSignature(bool)), true, i18n("Add Signature"));
    
    tb2->setToggle(id_confirmDelivery_);
    tb2->setToggle(id_confirmReading_);
    tb2->setToggle(id_digitallySign_);
    tb2->setToggle(id_encrypt_);
    tb2->setToggle(id_addSignature_);
    
    KConfig * c(KGlobal::config());

    using namespace EmpathConfig;

    c->setGroup(GROUP_COMPOSE);
    
    tb2->setButton(id_confirmDelivery_,
        c->readBoolEntry(S_CONFIRM_DELIVER, false));
    tb2->setButton(id_confirmReading_,
        c->readBoolEntry(S_CONFIRM_READ, false));
    tb2->setButton(id_digitallySign_,
        c->readBoolEntry(C_ADD_DIG_SIG, false));
    tb2->setButton(id_addSignature_,
        c->readBoolEntry(C_ADD_SIG, false));
    tb2->setButton(id_encrypt_,
        c->readBoolEntry(S_ENCRYPT, false));
}

    void
EmpathComposeWindow::setupStatusBar()
{
    statusBar()->message(i18n("Empath Compose Window"));
}

// File menu slots

    void
EmpathComposeWindow::s_fileSendMessage()
{
    /*
    if (!composeWidget_->haveTo()) {
        _askForRecipient();
        return;
    }
    
    if (!composeWidget_->haveSubject()) {
        _askForSubject();
        return;
    }
    
    RMM::RMessage outMessage(composeWidget_->message());

    hide();
    empath->send(outMessage);
    delete this;
    */
}

    void
EmpathComposeWindow::s_fileSendLater()
{   
    /*
    if (!composeWidget_->haveTo()) {
        _askForRecipient();
        return;
    }
    
    if (!composeWidget_->haveSubject()) {
        _askForSubject();
        return;
    }
    
    RMM::RMessage outMessage(composeWidget_->message());

//    if (composeWidget_->messageHasAttachments())
//        outMessage.addAttachmentList(composeWidget_->messageAttachments());

    empath->queue(outMessage);
    */
}

    void
EmpathComposeWindow::s_fileSaveAs()
{
    // STUB
    // FIXME: We need a URL to the message !
    empath->saveMessage(EmpathURL(), this);
}

    void
EmpathComposeWindow::s_filePrint()
{
    // STUB
}

    void
EmpathComposeWindow::s_fileClose()
{
    // FIXME: Check if the user wants to save changes
    delete this;
}

// Edit menu slots
    
    void
EmpathComposeWindow::s_editUndo()
{
    // STUB
}

    void
EmpathComposeWindow::s_editRedo()
{
    // STUB
}

    void
EmpathComposeWindow::s_editCut()
{
    emit(cut());
}

    void
EmpathComposeWindow::s_editCopy()
{
    emit(copy());
}

    void
EmpathComposeWindow::s_editPaste()
{
    emit(paste());
}

    void
EmpathComposeWindow::s_editDelete()
{
    // STUB
}

    void
EmpathComposeWindow::s_editSelectAll()
{
    emit(selectAll());
}

    void
EmpathComposeWindow::s_editFindInMessage()
{
    // STUB
}

    void
EmpathComposeWindow::s_editFind()
{
    // STUB
}

    void
EmpathComposeWindow::s_editFindAgain()
{
    // STUB
}

// Message menu slots

    void
EmpathComposeWindow::s_messageSaveAs()
{
    /*
    RMM::RMessage message(composeWidget_->message());
    
    QString saveFilePath =
        KFileDialog::getSaveFileName(
            QString::null, QString::null, this,
            i18n("Save Message").latin1());

    if (saveFilePath.isEmpty())
        return;
    
    QFile f(saveFilePath);
    if (!f.open(IO_WriteOnly)) {
        QMessageBox::information(this, "Empath",
            i18n("Sorry I can't write to that file. "
                "Please try another filename."),
            i18n("OK"));

        return;
    }

    QDataStream d(&f);
    
    d << message.asString();

    f.close();
    */
}

    void
EmpathComposeWindow::s_messageCopyTo()
{
    /*
    RMM::RMessage message(composeWidget_->message());
    
    EmpathFolderChooserDialog fcd(this);

    if (fcd.exec() != QDialog::Accepted)
        return;

    empath->write(fcd.selected(), message);
    */
}

    void
EmpathComposeWindow::s_help()
{
    // STUB
}

    void
EmpathComposeWindow::s_aboutEmpath()
{
    empath->s_about(this);
}

    void
EmpathComposeWindow::s_aboutQt()
{
    QMessageBox::aboutQt(this, "aboutQt");
}

    void
EmpathComposeWindow::s_confirmDelivery(bool)
{
    // STUB
}
    
    void
EmpathComposeWindow::s_confirmReading(bool)
{
    // STUB
}
    
    void
EmpathComposeWindow::s_addSignature(bool)
{
    // STUB
}
    
    void
EmpathComposeWindow::s_digitallySign(bool)
{
    // STUB
}

    void
EmpathComposeWindow::s_encrypt(bool)
{
    // STUB
}

    void
EmpathComposeWindow::_askForRecipient()
{
    QMessageBox::information(this, "Empath",
        i18n("Please specify at least one recipient"),
        i18n("OK"));
}

    void
EmpathComposeWindow::_askForSubject()
{
    QMessageBox::information(this, "Empath",
        i18n("Please specify a subject"),
        i18n("OK"));
}

#include "EmpathComposeWindowMenus.cpp"

// vim:ts=4:sw=4:tw=78
