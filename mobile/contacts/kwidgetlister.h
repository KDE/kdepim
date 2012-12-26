
#ifndef KWIDGETLISTER_H
#define KWIDGETLISTER_H

#include <QWidget>

class KWidgetLister : public QWidget
{
  Q_OBJECT

  public:
    /**
     * Creates a new widget lister.
     *
     * @param parent The parent widget.
     */
    explicit KWidgetLister( QWidget *parent = 0 );

    /**
     * Destroys the widget lister.
     */
    ~KWidgetLister();

    /**
     * Sets the @p orientation of the listed widgets.
     */
    void setOrientation( Qt::Orientation orientation );

    /**
     * Returns the orientation of the listed widgets.
     */
    Qt::Orientation orientation() const;

    /**
     * Adds a new @p widget to the end of the lister.
     *
     * @note The lister takes ownership of the widget.
     */
    void addWidget( QWidget *widget );

    /**
     * Removes all widgets from the lister and deletes them.
     */
    void clear();

    /**
     * Returns the number of widgets the lister contains.
     */
    int count() const;

    /**
     * Returns the widget at the given @p index or @c 0 if the index
     * is out of range.
     */
    QWidget* widget( int index ) const;

  Q_SIGNALS:
    /**
     * This signal is emitted before the given @p widget is removed
     * from the lister.
     */
    void aboutToBeRemoved( QWidget *widget );

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void removeWidget( QWidget* ) )
    //@endcond
};

#endif
