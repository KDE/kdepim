/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#include "pref.h"

#include <klocale.h>

#include <tqlayout.h>
#include <tqlabel.h>

KMobilePreferences::KMobilePreferences()
    : KDialogBase(TreeList, i18n("Preferences"),
                  Help|Default|Ok|Apply|Cancel, Ok)
{
    // this is the base class for your preferences dialog.  it is now
    // a Treelist dialog.. but there are a number of other
    // possibilities (including Tab, Swallow, and just Plain)
    TQFrame *frame;
    frame = addPage(i18n("First Page"), i18n("Page One Options"));
    m_pageOne = new KMobilePrefPageOne(frame);

    frame = addPage(i18n("Second Page"), i18n("Page Two Options"));
    m_pageTwo = new KMobilePrefPageTwo(frame);
}

KMobilePrefPageOne::KMobilePrefPageOne(TQWidget *parent)
    : TQFrame(parent)
{
    TQHBoxLayout *layout = new TQHBoxLayout(this);
    layout->setAutoAdd(true);

    new TQLabel(i18n("Add something here"), this);
}

KMobilePrefPageTwo::KMobilePrefPageTwo(TQWidget *parent)
    : TQFrame(parent)
{
    TQHBoxLayout *layout = new TQHBoxLayout(this);
    layout->setAutoAdd(true);

    new TQLabel(i18n("Add something here"), this);
}
#include "pref.moc"
