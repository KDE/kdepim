/*
  This file is part of KAddressBook.
  Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef PRINTSTYLE_H
#define PRINTSTYLE_H

#include "contactfields.h"

#include <KContacts/Addressee>

#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QPixmap>
#include <QWidget>

class KPageWidgetItem;

namespace KABPrinting
{

class PrintingWizard;
class PrintProgress;

/**
 * @short The abstract interface to the PrintingWizards style objects.
 *
 * To implement a print style, derive from this class and read
 * the information in printingwizard.h to see how this two pieces
 * work together. Basically, the print style gets the contacts it
 * is supposed to print from the PrintingWizard is will not
 * change this set - neither its content nor its order.
 *
 * To register your new style in the printing wizard, you need to
 * define a PrintStyleFactory that handles how your objects are
 * created and deleted. See the existing print styles for
 * examples.
 *
 * A print style should have a preview image that gives the user
 * a basic impression on how it will look. Add this image to the
 * printing folder and edit CMakeLists.txt to have
 * it installed along with kaddressbook.
 * Load it using setPreview( const QString& ).
 *
 * Your print style is supposed to add its options as pages to
 * the printing wizard. The method wizard() gives you a pointer
 * to the wizard object.
 */
class PrintStyle : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new print style.
     *
     * @wizard The wizard the style belongs to.
     */
    explicit PrintStyle(PrintingWizard *wizard);

    /**
     * Destroys the print style.
     */
    virtual ~PrintStyle();

    /**
     * This method must be reimplemented to actually print something.
     *
     * @param contacts The filtered and sorted list of contacts.
     * @param progress The object to inform the user about the progress of printing.
     */
    virtual void print(const KContacts::Addressee::List &contacts, PrintProgress *progress) = 0;

    /**
     * This method should be reimplemented to provide a preview of what
     * the printed page will look like.
     *
     * An invalid pixmap is returned by default, which means no preview
     * is available.
     */
    const QPixmap &preview() const;

    /**
     * Hides all style specific pages in the printing wizard.
     */
    void hidePages();

    /**
     * Show all style specific pages in the printing wizard.
     */
    void showPages();

    /**
     * Returns the preferred contact field that shall be used for sorting.
     */
    ContactFields::Field preferredSortField() const;

    /**
     * Returns the preferred order that shall be used for sorting.
     */
    Qt::SortOrder preferredSortOrder() const;

protected:
    /**
     * Loads the preview image from the kaddressbook data directory.
     *
     * @param fileName The name of the preview image in the "printing" subdirectory
     *                 without any prefix.
     * @returns Whether the image was loaded successfully.
     */
    bool setPreview(const QString &fileName);

    /**
     * Sets the preview @p image.
     */
    void setPreview(const QPixmap &image);

    /**
     * Sets the preferred sort options for this printing style.
     */
    void setPreferredSortOptions(ContactFields::Field,
                                 Qt::SortOrder sortOrder = Qt::AscendingOrder);

    /**
     * Returns the printing wizard that is responsible for this style.
     */
    PrintingWizard *wizard() const;

    /**
     * Adds an additional page to the printing wizard, e.g. a configuration
     * page for the style.
     *
     * @param page The page widget.
     * @param tile The page title.
     */
    void addPage(QWidget *page, const QString &title);

private:
    PrintingWizard *mWizard;
    QPixmap mPreview;
    QList<QWidget *> mPageList;
    QHash<QWidget *, KPageWidgetItem *> mPageItems;
    QStringList mPageTitles;

    ContactFields::Field mSortField;
    Qt::SortOrder mSortOrder;
};

/**
 * The factories are used to have all object of the respective
 * print style created in one place.
 */
class PrintStyleFactory
{
public:
    explicit PrintStyleFactory(PrintingWizard *parent);
    virtual ~PrintStyleFactory();

    virtual PrintStyle *create() const = 0;

    /**
     * Overload this method to provide a one-liner description
     * for your print style.
     */
    virtual QString description() const = 0;

protected:
    PrintingWizard *mParent;
};

}

#endif
