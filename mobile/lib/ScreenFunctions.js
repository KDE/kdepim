/*
    Copyright (C) 2011 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

/**
 * Contains the minimum size necessary for usable finger interaction elements
 */
var fingerSize = KDE.mm2px( 12 );

/**
 * Returns size of a screen partition.
 * This is supposed to be used when dividing a bit of screen space into equaly sized sub-spaces
 * without having unused/partial areas left.
 * @param totalSize Total amount of available pixel space
 * @param minPartitions Minimum number of partitions requried
 * @returns Pixel-size of a partition.
 */
function partition( totalSize, minPartitions )
{
  // at least 15mm for touch interaction
  var optimalPartitionCount = Math.floor(totalSize / fingerSize);
  return totalSize / Math.max(minPartitions, optimalPartitionCount);
}
