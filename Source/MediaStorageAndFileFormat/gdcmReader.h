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

#ifndef __gdcmReader_h
#define __gdcmReader_h

#include "gdcmIFStream.h"
#include "gdcmDataSet.h"

namespace gdcm
{
/**
 * \brief Reader ala DOM (Document Object Model)
 *
 * Detailled description here
 *
 * A DataSet DOES NOT contains group 0x0002
 *
 * This is really a DataSet reader. This will not make sure the dataset
 * conform to any IOD at all. This is a completely different step. The
 * reasoning was that user could control the IOD there lib would handle
 * and thus we would not be able to read a DataSet if the IOD was not found
 * Instead we separate the reading from the validation.
 *
 * From GDCM1.x. Users will realize that one feature is missing
 * from this DOM implementation. In GDCM 1.x user used to be able to
 * control the size of the Value to be read. By default it was 0xfff.
 * The main author of GDCM2 thought this was too dangerous and harmful and
 * therefore this feaure did not make it into GDCM2
 *
 * WARNING:
 * GDCM will not produce warning for unorder (non-alphabetical order). 
 * See gdcm::Writer for more info
 */
class FileMetaInformation;
class GDCM_EXPORT Reader
{
public:
  Reader():Stream(),Preamble(0),DS(0),Header(0) {}
  virtual ~Reader();

  virtual bool Read(); // Execute()
  void SetFileName(const char *filename) {
    Stream.Open(filename);
  }

  const FileMetaInformation &GetHeader() const {
    return *Header;
  }
  const DataSet &GetDataSet() const {
    return *DS;
  }

  // Some fine notes
  // ACUSON-24-YBR_FULL-RLE-b.dcm cannot be completely rewritten
  // indeed they use the 'extension' of DICOM where you can write almost
  // anything in the preamble for instance they write something like: C.mdat
  // which of course is difficult to reproduce
  const char *GetPreamble() const { return Preamble; }

protected:
  bool ReadPreamble();
  bool ReadMetaInformation();
  bool ReadDataSet();
  TS GuessTransferSyntax();

  IFStream Stream;
  char *Preamble;

private:
  DataSet *DS;
  FileMetaInformation *Header;
};

} // end namespace gdcm

#endif //__gdcmReader_h