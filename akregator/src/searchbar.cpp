/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "akregatorconfig.h"
#include "articlefilter.h"
#include "article.h"
#include "searchbar.h"

#include <kcombobox.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <tqapplication.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqpixmap.h>
#include <tqstring.h>
#include <tqtimer.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>

using Akregator::Filters::ArticleMatcher;
using Akregator::Filters::Criterion;

namespace Akregator
{

class SearchBar::SearchBarPrivate
{
public:
    Akregator::Filters::ArticleMatcher textFilter;
    Akregator::Filters::ArticleMatcher statusFilter;
    TQString searchText;
    TQTimer timer;
    KLineEdit* searchLine;
    KComboBox* searchCombo;
    int delay;
};

SearchBar::SearchBar(TQWidget* parent, const char* name) : TQHBox(parent, name), d(new SearchBar::SearchBarPrivate)
{
    d->delay = 400;
    setMargin(2);
    setSpacing(5);
    setSizePolicy( TQSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Fixed ) );
    TQToolButton *clearButton = new TQToolButton(this);
    clearButton->setIconSet( SmallIconSet( TQApplication::reverseLayout() ? "clear_left" : "locationbar_erase" ) );

    clearButton->setAutoRaise(true);

    TQLabel* searchLabel = new TQLabel(this);
    searchLabel->setText( i18n("S&earch:") );

    d->searchLine = new KLineEdit(this, "searchline");
    connect(d->searchLine, TQT_SIGNAL(textChanged(const TQString &)),
                        this, TQT_SLOT(slotSearchStringChanged(const TQString &)));

    searchLabel->setBuddy(d->searchLine);

    TQLabel* statusLabel = new TQLabel(this);
    statusLabel->setText( i18n("Status:") );

    d->searchCombo = new KComboBox(this, "searchcombo");
    TQPixmap iconAll = KGlobal::iconLoader()->loadIcon("exec", KIcon::Small);
    TQPixmap iconNew(locate("data", "akregator/pics/kmmsgnew.png"));
    TQPixmap iconUnread(locate("data", "akregator/pics/kmmsgunseen.png"));
    TQPixmap iconKeep(locate("data", "akregator/pics/kmmsgflag.png"));
    
    d->searchCombo->insertItem(iconAll, i18n("All Articles"));
    d->searchCombo->insertItem(iconUnread, i18n("Unread"));
    d->searchCombo->insertItem(iconNew, i18n("New"));
    d->searchCombo->insertItem(iconKeep, i18n("Important"));
    
    TQToolTip::add( clearButton, i18n( "Clear filter" ) );
    TQToolTip::add( d->searchLine, i18n( "Enter space-separated terms to filter article list" ) );
    TQToolTip::add( d->searchCombo, i18n( "Choose what kind of articles to show in article list" ) );

    connect(clearButton, TQT_SIGNAL( clicked() ),
                    this, TQT_SLOT(slotClearSearch()) );

    connect(d->searchCombo, TQT_SIGNAL(activated(int)),
                        this, TQT_SLOT(slotSearchComboChanged(int)));

    connect(&(d->timer), TQT_SIGNAL(timeout()), this, TQT_SLOT(slotActivateSearch()));
}

SearchBar::~SearchBar()
{
    delete d;
    d = 0;
}

TQString SearchBar::text() const
{
    return d->searchText;
}

int SearchBar::status() const
{
    return d->searchCombo->currentItem();
}

void SearchBar::setDelay(int ms)
{
    d->delay = ms;
}

int SearchBar::delay() const
{
    return d->delay;
}
                
void SearchBar::slotClearSearch()
{
    if (status() != 0 || !d->searchLine->text().isEmpty())
    {
        d->searchLine->clear();
        d->searchCombo->setCurrentItem(0);
        d->timer.stop();
        slotActivateSearch();
    }
}

void SearchBar::slotSetStatus(int status)
{
     d->searchCombo->setCurrentItem(status);
     slotSearchComboChanged(status);
}

void SearchBar::slotSetText(const TQString& text)
{
     d->searchLine->setText(text);
     slotSearchStringChanged(text);
}
        
void SearchBar::slotSearchComboChanged(int /*index*/)
{
    if (d->timer.isActive())
        d->timer.stop();    
        
    d->timer.start(200, true);
}

void SearchBar::slotSearchStringChanged(const TQString& search)
{
    d->searchText = search;
    if (d->timer.isActive())
    	d->timer.stop();    

    d->timer.start(200, true);
}

void SearchBar::slotActivateSearch()
{
    TQValueList<Criterion> textCriteria;
    TQValueList<Criterion> statusCriteria;

    if (!d->searchText.isEmpty())
    {
        Criterion subjCrit( Criterion::Title, Criterion::Contains, d->searchText);
        textCriteria << subjCrit;
        Criterion crit1( Criterion::Description, Criterion::Contains, d->searchText);
        textCriteria << crit1;
        Criterion crit2( Criterion::Author, Criterion::Contains, d->searchText);
        textCriteria << crit2;
    }

    if (d->searchCombo->currentItem())
    {
        switch (d->searchCombo->currentItem())
        {
            case 1: // Unread
            {
                Criterion crit1( Criterion::Status, Criterion::Equals, Article::New);
                Criterion crit2( Criterion::Status, Criterion::Equals, Article::Unread);
                statusCriteria << crit1;
                statusCriteria << crit2;
                break;
            }
            case 2: // New
            {
                Criterion crit( Criterion::Status, Criterion::Equals, Article::New);
                statusCriteria << crit;
                break;
            }
            case 3: // Keep flag set
            {
                Criterion crit( Criterion::KeepFlag, Criterion::Equals, true);
                statusCriteria << crit;
                break;
            }
            default:
                break;
        }
    }

    d->textFilter = ArticleMatcher(textCriteria, ArticleMatcher::LogicalOr);
    d->statusFilter = ArticleMatcher(statusCriteria, ArticleMatcher::LogicalOr);
    Settings::setStatusFilter(d->searchCombo->currentItem());
    Settings::setTextFilter(d->searchText);
    emit signalSearch(d->textFilter, d->statusFilter);
}

}

#include "searchbar.moc"
