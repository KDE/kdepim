/*
    This file is part of libkdepim.

    Copyright (C) 2004 Antonio Larrosa <larrosa@kde.org

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kpixmapregionselectordialog.h"
#include <kdialogbase.h>
#include <tqdialog.h>
#include <tqdesktopwidget.h>
#include <klocale.h>
#include <kdialog.h>

using namespace KPIM;

KPixmapRegionSelectorDialog::KPixmapRegionSelectorDialog(TQWidget *parent,
     const char *name, bool modal ) : KDialogBase(parent, name, modal, i18n("Select Region of Image"), Help|Ok|Cancel, Ok, true )
{
  TQVBox *vbox=new TQVBox(this);
  new TQLabel(i18n("Please click and drag on the image to select the region of interest:"), vbox);
  m_pixmapSelectorWidget= new KPixmapRegionSelectorWidget(vbox);

  vbox->setSpacing( KDialog::spacingHint() );

  setMainWidget(vbox);
}

KPixmapRegionSelectorDialog::~KPixmapRegionSelectorDialog()
{
}

TQRect KPixmapRegionSelectorDialog::getSelectedRegion(const TQPixmap &pixmap, TQWidget *parent )
{
  KPixmapRegionSelectorDialog dialog(parent);

  dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);

  TQDesktopWidget desktopWidget;
  TQRect screen=desktopWidget.availableGeometry();
  dialog.pixmapRegionSelectorWidget()->setMaximumWidgetSize(
        (int)(screen.width()*4.0/5), (int)(screen.height()*4.0/5));

  int result = dialog.exec();

  TQRect rect;

  if ( result == TQDialog::Accepted )
    rect = dialog.pixmapRegionSelectorWidget()->unzoomedSelectedRegion();

  return rect;
}

TQRect KPixmapRegionSelectorDialog::getSelectedRegion(const TQPixmap &pixmap, int aspectRatioWidth, int aspectRatioHeight, TQWidget *parent )
{
  KPixmapRegionSelectorDialog dialog(parent);

  dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);
  dialog.pixmapRegionSelectorWidget()->setSelectionAspectRatio(aspectRatioWidth,aspectRatioHeight);

  TQDesktopWidget desktopWidget;
  TQRect screen=desktopWidget.availableGeometry();
  dialog.pixmapRegionSelectorWidget()->setMaximumWidgetSize(
        (int)(screen.width()*4.0/5), (int)(screen.height()*4.0/5));

  int result = dialog.exec();

  TQRect rect;

  if ( result == TQDialog::Accepted )
    rect = dialog.pixmapRegionSelectorWidget()->unzoomedSelectedRegion();

  return rect;
}

TQImage KPixmapRegionSelectorDialog::getSelectedImage(const TQPixmap &pixmap, TQWidget *parent )
{
  KPixmapRegionSelectorDialog dialog(parent);

  dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);

  TQDesktopWidget desktopWidget;
  TQRect screen=desktopWidget.availableGeometry();
  dialog.pixmapRegionSelectorWidget()->setMaximumWidgetSize(
        (int)(screen.width()*4.0/5), (int)(screen.height()*4.0/5));
  int result = dialog.exec();

  TQImage image;

  if ( result == TQDialog::Accepted )
    image = dialog.pixmapRegionSelectorWidget()->selectedImage();

  return image;
}

TQImage KPixmapRegionSelectorDialog::getSelectedImage(const TQPixmap &pixmap, int aspectRatioWidth, int aspectRatioHeight, TQWidget *parent )
{
  KPixmapRegionSelectorDialog dialog(parent);

  dialog.pixmapRegionSelectorWidget()->setPixmap(pixmap);
  dialog.pixmapRegionSelectorWidget()->setSelectionAspectRatio(aspectRatioWidth,aspectRatioHeight);

  TQDesktopWidget desktopWidget;
  TQRect screen=desktopWidget.availableGeometry();
  dialog.pixmapRegionSelectorWidget()->setMaximumWidgetSize(
        (int)(screen.width()*4.0/5), (int)(screen.height()*4.0/5));

  int result = dialog.exec();

  TQImage image;

  if ( result == TQDialog::Accepted )
    image = dialog.pixmapRegionSelectorWidget()->selectedImage();

  return image;
}

