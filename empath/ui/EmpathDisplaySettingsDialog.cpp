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
# pragma implementation "EmpathDisplaySettingsDialog.h"
#endif

// Qt includes
#include <qimage.h>
#include <qdir.h>
#include <qlayout.h>
#include <qwhatsthis.h>

// KDE includes
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kapp.h>
#include <kfontdialog.h>
#include <kstddirs.h>

// Local includes
#include "EmpathSeparatorWidget.h"
#include "EmpathUIUtils.h"
#include "EmpathDisplaySettingsDialog.h"
#include "EmpathConfig.h"
#include "Empath.h"

EmpathDisplaySettingsDialog::EmpathDisplaySettingsDialog(QWidget * parent)
    :   QDialog(parent, "DisplaySettings", true),
        applied_(false)
{
    setCaption(i18n("Display Settings"));
    
////////////////////////////////////////////////////////////////////////    
// Message view
// 
    QLabel * l_displayHeaders =
        new QLabel(i18n("Display headers"), this, "l_displayHeaders");
    
    le_displayHeaders_ = new QLineEdit(this, "le_displayHeaders");
    
    QLabel * l_fixedFont =
        new QLabel(i18n("Message font"), this, "l_fixedFont");
    
    pb_chooseFixedFont_ =
        new QPushButton(
            KGlobal::fixedFont().family(), this, "pb_chooseFixedFont");
    
    cb_underlineLinks_    =
        new QCheckBox(i18n("&Underline Links"), this, "cb_underlineLinks");
    
    QLabel * l_quoteColourOne =
        new QLabel(i18n("Quote colour one"), this, "l_quoteColourOne");
    
    kcb_quoteColourOne_ = new KColorButton(this, "kcb_quoteColourOne");

    QLabel * l_quoteColourTwo =
        new QLabel(i18n("Quote colour two"), this, "l_quoteColourTwo");
    
    kcb_quoteColourTwo_ = new KColorButton(this, "kcb_quoteColourTwo");

    QLabel * l_linkColour =
        new QLabel(i18n("Link Colour"), this, "l_linkColour");
    
    kcb_linkColour_ = new KColorButton(this, "kcb_linkColour");
            
    QLabel * l_visitedColour =
        new QLabel(i18n("Visited Link Colour"), this, "l_visitedColour");
    
    kcb_visitedLinkColour_ = new KColorButton(this, "kcb_visitedColour");
        
    cb_threadMessages_ =
        new QCheckBox(i18n("Thread messages"), this, "cb_threadMessages");
   
    cb_timer_ =
        new QCheckBox(i18n("Mark messages as read after"), this, "cb_timer");
   
    sb_timer_ = new QSpinBox(0, 60, 1, this, "sb_timer");
    
    sb_timer_->setSuffix(" s");
    
#include "EmpathDialogButtonBox.cpp"

    // Layouts
    
    QVBoxLayout * layout = new QVBoxLayout(this, dialogSpace);

    QHBoxLayout * layout2 = new QHBoxLayout(layout);
    layout2->addWidget(cb_timer_);
    layout2->addWidget(sb_timer_);

    layout->addWidget(cb_threadMessages_);

    layout->addWidget(new EmpathSeparatorWidget(this));
 
    QHBoxLayout * layout3 = new QHBoxLayout(layout);
    layout3->addWidget(l_displayHeaders);
    layout3->addWidget(le_displayHeaders_);

    QHBoxLayout * layout4 = new QHBoxLayout(layout);
    layout4->addWidget(l_fixedFont);
    layout4->addWidget(pb_chooseFixedFont_);

    QGridLayout * layout5 = new QGridLayout(layout);

    layout5->addWidget(l_quoteColourOne,    0, 0);
    layout5->addWidget(l_quoteColourTwo,    1, 0);
    layout5->addWidget(l_linkColour,        2, 0);
    layout5->addWidget(l_visitedColour,     3, 0);

    layout5->addWidget(kcb_quoteColourOne_,     0, 1);
    layout5->addWidget(kcb_quoteColourTwo_,     1, 1);
    layout5->addWidget(kcb_linkColour_,         2, 1);
    layout5->addWidget(kcb_visitedLinkColour_,  3, 1);

    int w = kcb_quoteColourOne_->sizeHint().width();
    
    kcb_quoteColourOne_     ->setFixedWidth(w);
    kcb_quoteColourTwo_     ->setFixedWidth(w);
    kcb_linkColour_         ->setFixedWidth(w);
    kcb_visitedLinkColour_  ->setFixedWidth(w);

    layout->addWidget(cb_underlineLinks_);

    layout->addStretch(10);

    layout->addWidget(new EmpathSeparatorWidget(this));
 
    layout->addWidget(buttonBox_);
    
    QWhatsThis::add(le_displayHeaders_, i18n(
        "Here you may enter the headers that you\n"
        "want to appear in the block above the message\n"
        "you are reading. The default is:\n"
        "From,Date,Subject\n\n"
        "You must separate the header names by commas (,).\n"
        "This is not case-sensitive, i.e. you can write\n"
        "DATE and 'Date', 'date', 'DaTe' etc will all work\n\n"
        "Note that headers that you specify that do not appear\n"
        "in the message envelope will not be shown at all.\n"
        "This is to save space."));

   
    QWhatsThis::add(pb_chooseFixedFont_, i18n(
        "Here you may set the font to use for displaying\n"
        "messages. It's best to use a fixed-width font\n"
        "as the rest of the world expects that. Doing\n"
        "this allows people to line up text properly."));

    QWhatsThis::add(cb_timer_, i18n(
        "If you check this, messages will be marked\n"
        "as read after you've been looking at them for\n"
        "the time specified"));

    QWhatsThis::add(sb_timer_, i18n(
        "Messages will be marked as read after you've\n"
        "been looking at them for the time specified"));

     QWhatsThis::add(cb_underlineLinks_, i18n(
        "Choose whether to have links underlined.\n"
        "Links are email addresses, http:// type\n"
        "addresses, etc. If you're colour blind,\n"
        "this is a smart move."));

     QWhatsThis::add(kcb_quoteColourTwo_, i18n(
        "Choose the secondary colour for quoted text.\n"
        "Text can be quoted to multiple depths.\n"
        "Text that's quoted to an even number, e.g.\n"
        "where the line begins with '&gt; &gt; ' or '&gt; &gt; &gt; &gt; '\n"
        "will be shown in this colour."));
    
     QWhatsThis::add(kcb_linkColour_, i18n(
        "Choose the colour that links in messages\n"
        "are shown in. Links means URLs, including\n"
        "mailto: URLs."));

     QWhatsThis::add(kcb_visitedLinkColour_, i18n(
        "Choose the colour that visited links in messages\n"
        "are shown in. Links means URLs, including\n"
        "mailto: URLs."));

     QWhatsThis::add(cb_threadMessages_, i18n(
        "If you select this, messages will be 'threaded'\n"
        "this means that when one message is a reply to\n"
        "another, it will be placed in a tree, where it\n"
        "is a branch from the previous message."));
     
    QObject::connect(
        pb_chooseFixedFont_,    SIGNAL(clicked()),
        this,                   SLOT(s_chooseFixedFont()));   
};

EmpathDisplaySettingsDialog::~EmpathDisplaySettingsDialog()
{
    // Empty.
}

    void
EmpathDisplaySettingsDialog::s_chooseFixedFont()
{
    QFont fnt = pb_chooseFixedFont_->font();
    KFontDialog d(this);
    d.setFont(fnt);
    if (d.getFont(fnt) == QDialog::Accepted) {
        pb_chooseFixedFont_->setFont(fnt);
        pb_chooseFixedFont_->setText(fnt.family());
    }
}

    void
EmpathDisplaySettingsDialog::saveData()
{
    KConfig * c(KGlobal::config());
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_DISPLAY);

    c->writeEntry(UI_FIXED_FONT,      pb_chooseFixedFont_->font());
    c->writeEntry(UI_UNDERLINE_LINKS, cb_underlineLinks_->isChecked());
    c->writeEntry(UI_QUOTE_ONE,       kcb_quoteColourOne_->color());
    c->writeEntry(UI_QUOTE_TWO,       kcb_quoteColourTwo_->color());
    c->writeEntry(UI_LINK,            kcb_linkColour_->color());
    c->writeEntry(UI_VLINK,           kcb_visitedLinkColour_->color());
    
    c->writeEntry(UI_THREAD,          cb_threadMessages_->isChecked());
    c->writeEntry(UI_SHOW_HEADERS,    le_displayHeaders_->text());
    c->writeEntry(UI_MARK_READ,       cb_timer_->isChecked());
    c->writeEntry(UI_MARK_TIME,       sb_timer_->value());
}

    void
EmpathDisplaySettingsDialog::loadData()
{
    KConfig * c = KGlobal::config();
    
    using namespace EmpathConfig;
    
    c->setGroup(GROUP_DISPLAY);
    
    QFont f = KGlobal::fixedFont();
    QFont savedFont = c->readFontEntry(UI_FIXED_FONT, &f);

    pb_chooseFixedFont_->setFont(savedFont);
    pb_chooseFixedFont_->setText(savedFont.family());
    cb_underlineLinks_->setChecked
        (c->readBoolEntry(UI_UNDERLINE_LINKS, DFLT_UNDER_LINKS));
    kcb_quoteColourOne_->setColor(c->readColorEntry(UI_QUOTE_ONE, &DFLT_Q_1));
    kcb_quoteColourTwo_->setColor(c->readColorEntry(UI_QUOTE_TWO, &DFLT_Q_2));
    kcb_linkColour_->setColor(c->readColorEntry(UI_LINK, &DFLT_LINK));
    kcb_visitedLinkColour_->setColor(c->readColorEntry(UI_VLINK, &DFLT_VLINK));
    cb_threadMessages_->setChecked(c->readBoolEntry(UI_THREAD, DFLT_THREAD));
    le_displayHeaders_->setText(c->readEntry(UI_SHOW_HEADERS, DFLT_HEADERS));
    cb_timer_->setChecked(c->readBoolEntry(UI_MARK_READ, DFLT_MARK));
    sb_timer_->setValue(c->readNumEntry(UI_MARK_TIME, DFLT_MARK_TIMER));
}

    void
EmpathDisplaySettingsDialog::s_OK()
{
    hide();
    if (!applied_)
        s_apply();
    KGlobal::config()->sync();
    accept();
}

    void
EmpathDisplaySettingsDialog::s_help()
{
    // STUB
}

    void
EmpathDisplaySettingsDialog::s_apply()
{
    if (applied_) {
        pb_apply_->setText(i18n("&Apply"));
        KGlobal::config()->rollback(true);
        KGlobal::config()->reparseConfiguration();
        loadData();
        applied_ = false;
    } else {
        pb_apply_->setText(i18n("&Revert"));
        pb_cancel_->setText(i18n("&Close"));
        applied_ = true;
    }
    saveData();
}

    void
EmpathDisplaySettingsDialog::s_default()
{
    using namespace EmpathConfig;

    pb_chooseFixedFont_->setFont(KGlobal::fixedFont());
    pb_chooseFixedFont_->setText(KGlobal::fixedFont().family());
    cb_underlineLinks_->setChecked(DFLT_UNDER_LINKS);
    kcb_quoteColourOne_->setColor(DFLT_Q_1);
    kcb_quoteColourTwo_->setColor(DFLT_Q_2);
    kcb_linkColour_->setColor(DFLT_LINK);
    kcb_visitedLinkColour_->setColor(DFLT_VLINK);
    cb_threadMessages_->setChecked(DFLT_THREAD);
    le_displayHeaders_->setText(DFLT_HEADERS);
    cb_timer_->setChecked(DFLT_MARK);
    sb_timer_->setValue(DFLT_MARK_TIMER);
    saveData();
}
    
    void
EmpathDisplaySettingsDialog::s_cancel()
{
    if (!applied_)
        KGlobal::config()->rollback(true);
    reject();
}

// vim:ts=4:sw=4:tw=78
