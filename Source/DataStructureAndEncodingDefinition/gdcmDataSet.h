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

#ifndef __gdcmDataSet_h
#define __gdcmDataSet_h

#include "gdcmDataElement.h"
#include "gdcmTS.h"
#include "gdcmValue.h"

namespace gdcm
{
/**
 * \brief Class to represent a Data Set (which contains Data Elements)
 * A Data Set represents an instance of a real world Information Object
 * \note
 * DATA SET: 
 * Exchanged information consisting of a structured set of Attribute values
 * directly or indirectly related to Information Objects. The value of each
 * Attribute in a Data Set is expressed as a Data Element.
 * A collection of Data Elements ordered by increasing Data Element Tag 
 * number that is an encoding of the values of Attributes of a real world 
 * object.
 * \note
 * Implementation note. If one do:
 * DataSet ds;
 * ds.SetLength(0);
 * ds.Read(is);
 * setting length to 0 actually means try to read is as if it was a root
 * DataSet. Other value are undefined (nested dataset with undefined length)
 * or defined length (different from 0) means nested dataset with defined
 * length.
 */
class StructuredSetBase;
class GDCM_EXPORT DataSet : public Value
{
public:
  DataSet(TS::NegociatedType const &type = TS::Explicit);
  ~DataSet();

  // Clear
  void Clear();

  // Insert
  void InsertDataElement(const DataElement& de);

  // Get
  bool FindDataElement(const Tag &t) const;
  const DataElement& GetDataElement(const Tag &t) const;

  //bool IsEmpty() { return DataElements.empty(); }

  // Read
  IStream &Read(IStream &is);

  // Write
  OStream const &Write(OStream &os) const;

  TS::NegociatedType GetNegociatedType() const {
    return NegociatedTS;
  }

  const VL & GetLength() const;
  void SetLength(VL const & l) { Length = l; }

  DataSet &operator = (DataSet const &r);
  DataSet(DataSet const &ds);

  void Print(std::ostream &os) const;

  const StructuredSetBase *GetInternal() const { return Internal; }

private:
  TS::NegociatedType NegociatedTS;
  StructuredSetBase *Internal;
  
  // Really only usefull in the nested dataset case
  VL Length;

};
//-----------------------------------------------------------------------------

} // end namespace gdcm

#endif //__gdcmDataSet_h

