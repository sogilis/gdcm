/*=========================================================================

  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006 Mathieu Malaterre
  Copyright (c) 1993-2005 CREATIS
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __gdcmTable_h
#define __gdcmTable_h

#include "gdcmTableEntry.h"
#include "gdcmTag.h"

#include <map>

namespace gdcm
{

/* \brief Table
 */
class Table
{
public:
  typedef std::map<Tag, TableEntry> MapTableEntry;
  Table() {}
  ~Table() {}

  friend std::ostream& operator<<(std::ostream& _os, const Table &_val);

  void InsertEntry(Tag const &tag, TableEntry const &te)
    {
#ifndef NDEBUG
    MapTableEntry::size_type s = TableInternal.size();
#endif
    TableInternal.insert(
      MapTableEntry::value_type(tag, te));
    assert( s < TableInternal.size() );
    }

  const TableEntry &GetTableEntry(const Tag &tag) const
    {
    MapTableEntry::const_iterator it = 
      TableInternal.find(tag);
    if (it == TableInternal.end())
      {
      assert( 0 && "Impossible" );
      return GetTableEntry(Tag(0,0));
      }
    return it->second;
    }

private:
  Table &operator=(const Table &_val); // purposely not implemented
  Table(const Table&_val); // purposely not implemented

  MapTableEntry TableInternal;
};

} // end namespace gdcm

#endif //__gdcmTable_h