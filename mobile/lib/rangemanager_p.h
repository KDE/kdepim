
#ifndef RANGEMANAGER_P_H
#define RANGEMANAGER_P_H

#include <QtCore/QVector>

/**
 * @short A class that manages a list of ranges.
 *
 */
class RangeManager
{
  public:
    /**
     * Creates a new range manager.
     */
    RangeManager();

    /**
     * Destroys the range manager.
     */
    ~RangeManager();

    /**
     * Returns the number of ranges.
     */
    int count() const;

    /**
     * Removes all ranges from the range manager.
     */
    void clear();

    /**
     * Inserts a new range of the given @p size before @p range.
     */
    void insertRange( int range, int size );

    /**
     * Removes the given @p range.
     */
    void removeRange( int range );

    /**
     * Increases the given range by @p elements.
     */
    void increaseRange( int range, int elements );

    /**
     * Decreases the given range by @p elements.
     */
    void decreaseRange( int range, int elements );

    /**
     * Returns the absolute start position of the given @p range.
     */
    int rangeStart( int range ) const;

    /**
     * Returns the number of elements of the given @p range.
     */
    int rangeSize( int range ) const;

    /**
     * Returns the range that contains the given absolute @p position.
     */
    int rangeForPosition( int position ) const;

    /**
     * Dumps the current range layout to console. Useful for debugging.
     */
    void dump() const;

  private:
    QVector<int> mRangeSizes;
};

#endif
