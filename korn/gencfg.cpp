/*
* gencfg.cpp -- Implementation of class KGeneralCfg.
* Author:  Sirtaj Singh Kang
* Version:  $Id$
* Generated:  Wed Jul 29 03:47:49 EST 1998
*/

#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#include <kcolorbutton.h>
#include <kicondialog.h>
#include <klocale.h>
#include <kglobal.h>

#include "gencfg.h"
#include "maildrop.h"
#include <kdebug.h>

  QWidget *
KGeneralCfg::makeWidget(QWidget * parent)
{
  QWidget * w = new QWidget(parent);

  int marginHint = KDialog::marginHint();
  int spacingHint = KDialog::spacingHint();

  QVBoxLayout * topLayout       = new QVBoxLayout(w, marginHint, spacingHint);
  QVBoxLayout * viewLayout      = new QVBoxLayout(topLayout);
  QHBoxLayout * captionLayout   = new QHBoxLayout(viewLayout);
  QHBoxLayout * styleLayout     = new QHBoxLayout(viewLayout);
  QGridLayout * displayLayout   = new QGridLayout(viewLayout, 3, 3);

  topLayout->addStretch(1);

  _displayStyleGroup = new QButtonGroup(w);

  connect(
    _displayStyleGroup, SIGNAL(clicked(int)),
    this, SLOT(slotDisplayStyleChanged(int))
  );

  _caption    = new QLineEdit(w);

  _bg         = new KColorButton(w);
  _bgNew      = new KColorButton(w);
  _fg         = new KColorButton(w);
  _fgNew      = new KColorButton(w);
  _icon       = new KIconButton(KGlobal::iconLoader(), w);
  _iconNew    = new KIconButton(KGlobal::iconLoader(), w);

  _displayStyleGroup->hide();

  QRadioButton * rb_plain   = new QRadioButton(i18n("Plain"), w);
  QRadioButton * rb_colour  = new QRadioButton(i18n("Use color"), w);
  QRadioButton * rb_icon    = new QRadioButton(i18n("Use icon"), w);

  _displayStyleGroup->insert(rb_plain,  KMailDrop::Plain);
  _displayStyleGroup->insert(rb_colour, KMailDrop::Colour);
  _displayStyleGroup->insert(rb_icon,   KMailDrop::Icon);

  // Layout
  captionLayout->addWidget(new QLabel(i18n("Caption:"), w));
  captionLayout->addWidget(_caption);
  styleLayout->addWidget(rb_plain);
  styleLayout->addWidget(rb_colour);
  styleLayout->addWidget(rb_icon);

  displayLayout->addWidget(new QLabel(i18n("Normal:"), w),  0, 1);
  displayLayout->addWidget(new QLabel(i18n("New mail:"), w),     0, 2);

  displayLayout->addWidget(new QLabel(i18n("Text:"), w),         2, 0);
  displayLayout->addWidget(new QLabel(i18n("Icon:"), w),         3, 0);

  QLabel * l;

  l = new QLabel(i18n("&Background:"), w);
  l->setBuddy(_bg);
  displayLayout->addWidget(l, 1, 0);

  l = new QLabel(i18n("&Text:"), w);
  l->setBuddy(_fg);
  displayLayout->addWidget(l, 2, 0);

  l = new QLabel(i18n("&Icon:"), w);
  l->setBuddy(_icon);
  displayLayout->addWidget(l, 3, 0);

  displayLayout->addWidget(_bg,       1, 1);
  displayLayout->addWidget(_fg,       2, 1);
  displayLayout->addWidget(_icon,     3, 1);

  displayLayout->addWidget(_bgNew,    1, 2);
  displayLayout->addWidget(_fgNew,    2, 2);
  displayLayout->addWidget(_iconNew,  3, 2);

  readConfig();

  return w;
}

QString KGeneralCfg::name() const
{
  return i18n("&View");
}

void KGeneralCfg::readConfig()
{
  KMailDrop *d = drop();

  _caption->setText(d->caption());
  int style=d->displayStyle();
  _displayStyleGroup->setButton(style);
  slotDisplayStyleChanged(style);

  _bg->setColor(d->bgColour());
  _fg->setColor(d->fgColour());
  _bgNew->setColor(d->newBgColour());
  _fgNew->setColor(d->newFgColour());
  _icon->setIcon(d->icon());
  _iconNew->setIcon(d->newIcon());
  slotDisplayStyleChanged(d->displayStyle());
}

void KGeneralCfg::updateConfig()
{
  kdDebug() << "KGeneralCfg::updateConfig()" << endl;
  KMailDrop *d = drop();

  d->setCaption     (_caption->text());

  int i = _displayStyleGroup->id(_displayStyleGroup->selected());
  d->setDisplayStyle(KMailDrop::Style(i));

  d->setBgColour    (_bg->color());
  d->setFgColour    (_fg->color());
  d->setNewBgColour (_bgNew->color());
  d->setNewFgColour (_fgNew->color());
  d->setIcon        (_icon->icon());
  d->setNewIcon     (_iconNew->icon());
}

void KGeneralCfg::slotDisplayStyleChanged(int i)
{
    switch (KMailDrop::Style(i)) {

        case KMailDrop::Colour:
            _bg->setEnabled(true);
            _fg->setEnabled(true);
            _bgNew->setEnabled(true);
            _fgNew->setEnabled(true);
            _icon->setEnabled(false);
            _iconNew->setEnabled(false);
            break;

        case KMailDrop::Icon:
            _bg->setEnabled(false);
            _fg->setEnabled(false);
            _bgNew->setEnabled(false);
            _fgNew->setEnabled(false);
            _icon->setEnabled(true);
            _iconNew->setEnabled(true);
            break;
        case KMailDrop::Plain:
            _bg->setEnabled(false);
            _fg->setEnabled(false);
            _bgNew->setEnabled(false);
            _fgNew->setEnabled(false);
            _icon->setEnabled(false);
            _iconNew->setEnabled(false);
            break;
    }
}

#include "gencfg.moc"
