/*=========================================================================

  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2008 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmReader.h"
#include "gdcmTrace.h"
#include "gdcmVR.h"
#include "gdcmFileMetaInformation.h"
#include "gdcmSwapper.h"

#include "gdcmDeflateStream.h"

#include "gdcmExplicitDataElement.h"
#include "gdcmImplicitDataElement.h"

#ifdef GDCM_SUPPORT_BROKEN_IMPLEMENTATION
#include "gdcmUNExplicitDataElement.h"
#include "gdcmCP246ExplicitDataElement.h"
#include "gdcmExplicitImplicitDataElement.h"
#include "gdcmUNExplicitImplicitDataElement.h"
#include "gdcmVR16ExplicitDataElement.h"
#endif


namespace gdcm
{

Reader::~Reader()
{
	if (Ifstream) {
		Ifstream->close();
		delete Ifstream;
	}
}

/// \brief tells us if "DICM" is found as position 128
///        (i.e. the file is a 'true dicom' one)
/// If not found then seek back at beginning of file (could be Mallinckrodt
/// or old ACRNEMA with no preamble)
/// \precondition we are at the beginning of file
/// \postcondition we are at the beginning of the DataSet or
/// Meta Information Header
bool Reader::ReadPreamble()
{
 return true;
}

/// \brief read the DICOM Meta Information Header
/// Find out the TransferSyntax used (default: Little Endian Explicit)
/// \precondition we are at the start of group 0x0002 (well after preamble)
/// \postcondition we are at the beginning of the DataSet
bool Reader::ReadMetaInformation()
{
  return true;
}

bool Reader::ReadDataSet()
{
  return true;
}

TransferSyntax Reader::GuessTransferSyntax()
{
  // Don't call this function if you have a meta file info
  //assert( Header->GetTransferSyntaxType() == TransferSyntax::TS_END );
  std::streampos start = Stream->tellg();
  SwapCode sc = SwapCode::Unknown;
  TransferSyntax::NegociatedType nts = TransferSyntax::Unknown;
  TransferSyntax ts (TransferSyntax::TS_END);
  Tag t;
  t.Read<SwapperNoOp>(*Stream);
  if( ! (t.GetGroup() % 2) )
    {
    switch( t.GetGroup() )
      {
    case 0x0008:
      sc = SwapCode::LittleEndian;
      break;
    case 0x0800:
      sc = SwapCode::BigEndian;
      break;
    default:
      abort();
      }
    // Purposely not Re-use ReadVR since we can read VR_END
    char vr_str[3];
    Stream->read(vr_str, 2);
    vr_str[2] = '\0';
    // Cannot use GetVRTypeFromFile since is assert ...
    VR::VRType vr = VR::GetVRType(vr_str);
    if( vr != VR::VR_END )
      {
      nts = TransferSyntax::Explicit;
      }
    else
      {
      assert( !(VR::IsSwap(vr_str)));
      Stream->seekg(-2, std::ios::cur); // Seek back
      if( t.GetElement() == 0x0000 )
        {
        VL gl; // group length
        gl.Read<SwapperNoOp>(*Stream);
        switch(gl)
          {
        case 0x00000004 :
          sc = SwapCode::LittleEndian;    // 1234
          break;
        case 0x04000000 :
          sc = SwapCode::BigEndian;       // 4321
          break;
        case 0x00040000 :
          sc = SwapCode::BadLittleEndian; // 3412
          gdcmWarningMacro( "Bad Little Endian" );
          break;
        case 0x00000400 :
          sc = SwapCode::BadBigEndian;    // 2143
          gdcmWarningMacro( "Bad Big Endian" );
          break;
        default:
          abort();
          }
        }
      nts = TransferSyntax::Implicit;
      }
    }
  else
    {
    gdcmWarningMacro( "Start with a private tag creator" );
    assert( t.GetGroup() > 0x0002 );
    switch( t.GetElement() )
      {
    case 0x0010:
      sc = SwapCode::LittleEndian;
      break;
    default:
      abort();
      }
    // Purposely not Re-use ReadVR since we can read VR_END
    char vr_str[3];
    Stream->read(vr_str, 2);
    vr_str[2] = '\0';
    // Cannot use GetVRTypeFromFile since is assert ...
    VR::VRType vr = VR::GetVRType(vr_str);
    if( vr != VR::VR_END )
      {
      nts = TransferSyntax::Explicit;
      }
    else
      {
      nts = TransferSyntax::Implicit;
      // We are reading a private creator (0x0010) so it's LO, it's
      // difficult to come up with someting to check, maybe that
      // VL < 256 ...
      gdcmWarningMacro( "Very dangerous assertion needs some work" );
      }
    }
  assert( nts != TransferSyntax::Unknown );
  assert( sc != SwapCode::Unknown );
  if( nts == TransferSyntax::Implicit )
    {
    if( sc == SwapCode::BigEndian )
      {
      ts = TransferSyntax::ImplicitVRBigEndianACRNEMA;
      }
    else if ( sc == SwapCode::LittleEndian )
      {
      ts = TransferSyntax::ImplicitVRLittleEndian;
      }
    else
      {
      abort();
      }
    }
  else
    {
    abort();
    }
  Stream->seekg( start, std::ios::beg );
  assert( ts != TransferSyntax::TS_END );
  return ts;
}

bool Reader::Read()
{
  if( !Stream )
    {
    gdcmErrorMacro( "No File" );
    return false;
    }
  bool success = true;

try
    {
std::istream &is = *Stream;

  bool haspreamble = true;
  try
    {
    F->GetHeader().GetPreamble().Read( is );
    }
  catch( std::exception &ex )
    {
    // return to beginning of file, hopefully this file is simply missing preamble
    is.seekg(0, std::ios::beg);
    haspreamble = false;
    }
  catch( ... )
    {
    abort();
    }

  bool hasmetaheader = false;
  try
    {
    if( haspreamble )
      {
      try
        {
        F->GetHeader().Read( is );
        hasmetaheader = true;
        assert( !F->GetHeader().IsEmpty() );
        }
      catch( std::exception &ex )
        {
        // Weird implicit meta header:
        is.seekg(128+4, std::ios::beg );
        try
          {
          F->GetHeader().ReadCompat(is);
          }
        catch( std::exception &ex )
          {
          // Ok I get it now... there is absolutely no meta header, giving up
          //hasmetaheader = false;
          }
        }
      }
    else
      {
      F->GetHeader().ReadCompat(is);
      }
    }
  catch( std::exception &ex )
    {
    // Same player play again:
    is.seekg(0, std::ios::beg );
    hasmetaheader = false;
    }
  catch( ... )
    {
    // Ooops..
    abort();
    }
  if( F->GetHeader().IsEmpty() )
    {
    hasmetaheader = false;
    gdcmWarningMacro( "no file meta info found" );
    }

  const TransferSyntax &ts = F->GetHeader().GetDataSetTransferSyntax();
  if( !ts.IsValid() )
    {
    throw Exception( "Meta Header issue" );
    }

  //std::cerr << ts.GetNegociatedType() << std::endl;
  //std::cerr << TransferSyntax::GetTSString(ts) << std::endl;
  // Special case where the dataset was compressed using the deflate
  // algorithm
  if( ts == TransferSyntax::DeflatedExplicitVRLittleEndian )
    {
#if 0
  std::ofstream out( "/tmp/deflate.raw");
  out << is.rdbuf();
  out.close();
#endif
    zlib_stream::zip_istream gzis( is );
    // FIXME: we also know in this case that we are dealing with Explicit:
    assert( ts.GetNegociatedType() == TransferSyntax::Explicit );
    F->GetDataSet().Read<ExplicitDataElement,SwapperNoOp>(gzis);
    // I need the following hack to read: srwithgraphdeflated.dcm
    //is.clear();
    // well not anymore, see special handling of trailing \0 in:
    // basic_unzip_streambuf<charT, traits>::fill_input_buffer(void)
    return is;
    }

  try
    {
    if( ts.GetSwapCode() == SwapCode::BigEndian )
      {
      //US-RGB-8-epicard.dcm is big endian
      if( ts.GetNegociatedType() == TransferSyntax::Implicit )
        {
        // There is no such thing as Implicit Big Endian... oh well
        // LIBIDO-16-ACR_NEMA-Volume.dcm
        F->GetDataSet().Read<ImplicitDataElement,SwapperDoOp>(is);
        }
      else
        {
        F->GetDataSet().Read<ExplicitDataElement,SwapperDoOp>(is);
        }
      }
    else // LittleEndian
      {
      if( ts.GetNegociatedType() == TransferSyntax::Implicit )
        {
        if( hasmetaheader && haspreamble )
          {
          F->GetDataSet().Read<ImplicitDataElement,SwapperNoOp>(is);
          }
        else
          {
          std::streampos start = is.tellg();
          is.seekg( 0, std::ios::end);
          std::streampos end = is.tellg();
          VL l = (VL)(end - start);
          is.seekg( start, std::ios::beg );
          F->GetDataSet().ReadWithLength<ImplicitDataElement,SwapperNoOp>(is, l);
          is.peek();
          }
        }
      else
        {
        F->GetDataSet().Read<ExplicitDataElement,SwapperNoOp>(is);
        }
      }
    }
  // Only catch parse exception at this point
  catch( ParseException &ex )
    {
#ifdef GDCM_SUPPORT_BROKEN_IMPLEMENTATION
    if( ex.GetLastElement().GetVR() == VR::UN && ex.GetLastElement().IsUndefinedLength() )
      {
      // non CP 246
      // P.Read( is );
      is.clear();
      if( haspreamble )
        {
        is.seekg(128+4, std::ios::beg);
        }
      else
        {
        is.seekg(0, std::ios::beg);
        }
      if( hasmetaheader )
        {
        // FIXME: we are reading twice the same meta-header, we succedeed the first time...
        // We should be able to seek to proper place instead of re-reading
        FileMetaInformation header;
        header.Read(is);
        }

      // GDCM 1.X
      gdcmWarningMacro( "Attempt to read non CP 246" );
      F->GetDataSet().Clear(); // remove garbage from 1st attempt...
      F->GetDataSet().Read<CP246ExplicitDataElement,SwapperNoOp>(is);
      }
    else if( ex.GetLastElement().GetVR() == VR::UN )
      {
      // P.Read( is );
      is.clear();
      if( haspreamble )
        {
        is.seekg(128+4, std::ios::beg);
        }
      else
        {
        is.seekg(0, std::ios::beg);
        }
      if( hasmetaheader )
        {
        // FIXME: we are reading twice the same meta-header, we succedeed the first time...
        // We should be able to seek to proper place instead of re-reading
        FileMetaInformation header;
        header.Read(is);
        }

      // GDCM 1.X
      gdcmWarningMacro( "Attempt to read GDCM 1.X wrongly encoded");
      F->GetDataSet().Clear(); // remove garbage from 1st attempt...
      F->GetDataSet().Read<UNExplicitDataElement,SwapperNoOp>(is);
      // This file can only be rewritten as implicit...
      }
    else if ( ex.GetLastElement().GetTag() == Tag(0xfeff,0x00e0) )
      {
      // Famous philips where some private sequence were byteswapped !
      // eg. PHILIPS_Intera-16-MONO2-Uncompress.dcm
      // P.Read( is );
      is.clear();
      if( haspreamble )
        {
        is.seekg(128+4, std::ios::beg);
        }
      else
        {
        is.seekg(0, std::ios::beg);
        }
      if( hasmetaheader )
        {
        // FIXME: we are reading twice the same meta-header, we succedeed the first time...
        // We should be able to seek to proper place instead of re-reading
        FileMetaInformation header;
        header.Read(is);
        }

      //
      gdcmWarningMacro( "Attempt to read Philips with ByteSwap private sequence wrongly encoded");
      F->GetDataSet().Clear(); // remove garbage from 1st attempt...
      abort();  // TODO FIXME
      }
    else if( ex.GetLastElement().GetVR() == VR::INVALID )
      {
      if( ts.GetNegociatedType() == TransferSyntax::Explicit )
        {
        try
          {
          gdcmWarningMacro( "Attempt to read file with VR16bits" );
          // We could not read the VR in an explicit dataset
          // seek back tag + vr:
          is.seekg( -6, std::ios::cur );
          VR16ExplicitDataElement ide;
          ide.Read<SwapperNoOp>( is );
          // If we are here it means we succeeded in reading the implicit data element:
          F->GetDataSet().Insert( ide );
          F->GetDataSet().Read<VR16ExplicitDataElement,SwapperNoOp>(is);
          // This file can only be rewritten as implicit...
          }
        catch ( Exception &ex )
          {
          try
            {
            // Ouch ! the file is neither:
            // 1. An Explicit encoded
            // 2. I could not reread it using the VR16Explicit reader, last option is
            // that the file is explicit/implicit
            is.clear();
            if( haspreamble )
              {
              is.seekg(128+4, std::ios::beg);
              }
            else
              {
              is.seekg(0, std::ios::beg);
              }
            if( hasmetaheader )
              {
              // FIXME: we are reading twice the same meta-header, we succedeed the first time...
              // We should be able to seek to proper place instead of re-reading
              FileMetaInformation header;
              header.Read(is);
              }

            // Explicit/Implicit
            gdcmWarningMacro( "Attempt to read file with explicit/implicit" );
            F->GetDataSet().Clear(); // remove garbage from 1st attempt...
            F->GetDataSet().Read<ExplicitImplicitDataElement,SwapperNoOp>(is);
            }
          catch ( Exception &ex )
            {
            is.clear();
            if( haspreamble )
              {
              is.seekg(128+4, std::ios::beg);
              }
            else
              {
              is.seekg(0, std::ios::beg);
              }
            if( hasmetaheader )
              {
              // FIXME: we are reading twice the same meta-header, we succedeed the first time...
              // We should be able to seek to proper place instead of re-reading
              FileMetaInformation header;
              header.Read(is);
              }

            // Explicit/Implicit
            gdcmWarningMacro( "Attempt to read file with explicit/implicit" );
            F->GetDataSet().Clear(); // remove garbage from 1st attempt...
            F->GetDataSet().Read<UNExplicitImplicitDataElement,SwapperNoOp>(is);
            }
          }
        }
      }
    else
      {
        gdcmWarningMacro( "Attempt to read the file as mixture of explicit/implicit");
      // Let's try again with an ExplicitImplicitDataElement:
      if( ts.GetSwapCode() == SwapCode::LittleEndian &&
        ts.GetNegociatedType() == TransferSyntax::Explicit )
        {
        // P.Read( is );
        if( haspreamble )
          {
          is.seekg(128+4, std::ios::beg);
          }
        else
          {
          is.seekg(0, std::ios::beg);
          }
        if( hasmetaheader )
          {
          // FIXME: we are reading twice the same meta-header, we succedeed the first time...
          // We should be able to seek to proper place instead of re-reading
          FileMetaInformation header;
          header.ReadCompat(is);
          }

        // Philips
        F->GetDataSet().Clear(); // remove garbage from 1st attempt...
        F->GetDataSet().Read<ExplicitImplicitDataElement,SwapperNoOp>(is);
        // This file can only be rewritten as implicit...
        }
      }
#else
    gdcmDebugMacro( ex.what() );
    success = false;
#endif /* GDCM_SUPPORT_BROKEN_IMPLEMENTATION */
    }
  catch( Exception &ex )
    {
    gdcmDebugMacro( ex.what() );
    success = false;
    }
  catch( ... )
    {
    gdcmWarningMacro( "Unknown exception" );
    success = false;
    }

    if( success ) assert( Stream->eof() );
    }
  catch( Exception &ex )
    {
    gdcmDebugMacro( ex.what() );
    success = false;
    }
  catch( ... )
    {
    gdcmWarningMacro( "Unknown exception" );
    success = false;
    }
//  if( !success )
//    {
//    F->GetHeader().Clear();
//    F->GetDataSet().Clear();
//    }

  // FIXME : call this function twice...
  if (Ifstream && Ifstream->is_open())
    {
    Ifstream->close();
    delete Ifstream;
    Ifstream = NULL;
    Stream = NULL;
    }

  return success;
}

bool Reader::ReadUpToTag(const Tag & tag, std::set<Tag> const & skiptags)
{
  if( !Stream )
    {
    gdcmErrorMacro( "No File" );
    return false;
    }
  bool success = true;

  try
    {
std::istream &is = *Stream;

  bool haspreamble = true;
  try
    {
    F->GetHeader().GetPreamble().Read( is );
    }
  catch( std::exception &ex )
    {
    // return to beginning of file, hopefully this file is simply missing preamble
    is.seekg(0, std::ios::beg);
    haspreamble = false;
    }
  catch( ... )
    {
    abort();
    }

  bool hasmetaheader = true;
  try
    {
    if( haspreamble )
      {
      try
        {
        F->GetHeader().Read( is );
        }
      catch( std::exception &ex )
        {
        // Weird implicit meta header:
        is.seekg(128+4, std::ios::beg );
        try
          {
          F->GetHeader().ReadCompat(is);
          }
        catch( std::exception &ex )
          {
          // Ok I get it now... there is absolutely no meta header, giving up
          hasmetaheader = false;
          }
        }
      }
    else
      F->GetHeader().ReadCompat(is);
    }
  catch( std::exception &ex )
    {
    // Same player play again:
    is.seekg(0, std::ios::beg );
    hasmetaheader = false;
    }
  catch( ... )
    {
    // Ooops..
    abort();
    }

  const TransferSyntax &ts = F->GetHeader().GetDataSetTransferSyntax();
  if( !ts.IsValid() )
    {
    throw Exception( "Meta Header issue" );
    }

  //std::cerr << ts.GetNegociatedType() << std::endl;
  //std::cerr << TransferSyntax::GetTSString(ts) << std::endl;
  // Special case where the dataset was compressed using the deflate
  // algorithm
  if( ts == TransferSyntax::DeflatedExplicitVRLittleEndian )
    {
    zlib_stream::zip_istream gzis( is );
    // FIXME: we also know in this case that we are dealing with Explicit:
    assert( ts.GetNegociatedType() == TransferSyntax::Explicit );
    F->GetDataSet().ReadUpToTag<ExplicitDataElement,SwapperNoOp>(gzis,tag, skiptags);

    return is;
    }

  try
    {
    if( ts.GetSwapCode() == SwapCode::BigEndian )
      {
      //US-RGB-8-epicard.dcm is big endian
      if( ts.GetNegociatedType() == TransferSyntax::Implicit )
        {
        // There is no such thing as Implicit Big Endian... oh well
        // LIBIDO-16-ACR_NEMA-Volume.dcm
        F->GetDataSet().ReadUpToTag<ImplicitDataElement,SwapperDoOp>(is,tag, skiptags);
        }
      else
        {
        F->GetDataSet().ReadUpToTag<ExplicitDataElement,SwapperDoOp>(is,tag, skiptags);
        }
      }
    else // LittleEndian
      {
      if( ts.GetNegociatedType() == TransferSyntax::Implicit )
        {
        if( hasmetaheader && haspreamble )
          {
          F->GetDataSet().ReadUpToTag<ImplicitDataElement,SwapperNoOp>(is,tag, skiptags);
          }
        else
          {
          std::streampos start = is.tellg();
          is.seekg( 0, std::ios::end);
          std::streampos end = is.tellg();
          VL l = (VL)(end - start);
          is.seekg( start, std::ios::beg );
          F->GetDataSet().ReadUpToTagWithLength<ImplicitDataElement,SwapperNoOp>(is, tag, l);
          is.peek();
          }
        }
      else
        {
        F->GetDataSet().ReadUpToTag<ExplicitDataElement,SwapperNoOp>(is,tag, skiptags);
        }
      }
    }
  // Only catch parse exception at this point
  catch( ParseException &ex )
    {
#ifdef GDCM_SUPPORT_BROKEN_IMPLEMENTATION
    if( ex.GetLastElement().GetVR() == VR::UN && ex.GetLastElement().IsUndefinedLength() )
      {
      // non CP 246
      // P.Read( is );
      is.clear();
      if( haspreamble )
        {
        is.seekg(128+4, std::ios::beg);
        }
      else
        {
        is.seekg(0, std::ios::beg);
        }
      if( hasmetaheader )
        {
        // FIXME: we are reading twice the same meta-header, we succedeed the first time...
        // We should be able to seek to proper place instead of re-reading
        FileMetaInformation header;
        header.Read(is);
        }

      // GDCM 1.X
      gdcmWarningMacro( "Attempt to read non CP 246" );
      F->GetDataSet().Clear(); // remove garbage from 1st attempt...
      F->GetDataSet().ReadUpToTag<CP246ExplicitDataElement,SwapperNoOp>(is,tag, skiptags);
      }
    else if( ex.GetLastElement().GetVR() == VR::UN )
      {
      // P.Read( is );
      is.clear();
      if( haspreamble )
        {
        is.seekg(128+4, std::ios::beg);
        }
      else
        {
        is.seekg(0, std::ios::beg);
        }
      if( hasmetaheader )
        {
        // FIXME: we are reading twice the same meta-header, we succedeed the first time...
        // We should be able to seek to proper place instead of re-reading
        FileMetaInformation header;
        header.Read(is);
        }

      // GDCM 1.X
      gdcmWarningMacro( "Attempt to read GDCM 1.X wrongly encoded");
      F->GetDataSet().Clear(); // remove garbage from 1st attempt...
      F->GetDataSet().ReadUpToTag<UNExplicitDataElement,SwapperNoOp>(is,tag, skiptags);
      // This file can only be rewritten as implicit...
      }
    else if ( ex.GetLastElement().GetTag() == Tag(0xfeff,0x00e0) )
      {
      // Famous philips where some private sequence were byteswapped !
      // eg. PHILIPS_Intera-16-MONO2-Uncompress.dcm
      // P.Read( is );
      is.clear();
      if( haspreamble )
        {
        is.seekg(128+4, std::ios::beg);
        }
      else
        {
        is.seekg(0, std::ios::beg);
        }
      if( hasmetaheader )
        {
        // FIXME: we are reading twice the same meta-header, we succedeed the first time...
        // We should be able to seek to proper place instead of re-reading
        FileMetaInformation header;
        header.Read(is);
        }

      //
      gdcmWarningMacro( "Attempt to read Philips with ByteSwap private sequence wrongly encoded");
      F->GetDataSet().Clear(); // remove garbage from 1st attempt...
      abort();  // TODO FIXME
      }
    else
      {
      // Let's try again with an ExplicitImplicitDataElement:
      if( ts.GetSwapCode() == SwapCode::LittleEndian &&
        ts.GetNegociatedType() == TransferSyntax::Explicit )
        {
        // P.Read( is );
        if( haspreamble )
          {
          is.seekg(128+4, std::ios::beg);
          }
        else
          {
          is.seekg(0, std::ios::beg);
          }
        if( hasmetaheader )
          {
          // FIXME: we are reading twice the same meta-header, we succedeed the first time...
          // We should be able to seek to proper place instead of re-reading
          FileMetaInformation header;
          header.ReadCompat(is);
          }

        // Philips
        gdcmWarningMacro( "Attempt to read the file as mixture of explicit/implicit");
        F->GetDataSet().Clear(); // remove garbage from 1st attempt...
        F->GetDataSet().ReadUpToTag<ExplicitImplicitDataElement,SwapperNoOp>(is,tag, skiptags);
        // This file can only be rewritten as implicit...
        }
      }
#else
    std::cerr << ex.what() << std::endl;
    success = false;
#endif /* GDCM_SUPPORT_BROKEN_IMPLEMENTATION */
    }
  catch( Exception &ex )
    {
    gdcmDebugMacro( ex.what() );
    success = false;
    }
  catch( ... )
    {
    gdcmWarningMacro( "Unknown exception" );
    success = false;
    }

    //assert( Stream->eof() );
    }
  catch( Exception &ex )
    {
    gdcmDebugMacro( ex.what() );
    success = false;
    }
  catch( ... )
    {
    gdcmWarningMacro( "Unknown exception" );
    success = false;
    }

  // FIXME : call this function twice...
  //Stream->close();

  return success;
}

} // end namespace gdcm