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
#ifndef __gdcmAttribute_h
#define __gdcmAttribute_h

#include "gdcmTypes.h"
#include "gdcmVR.h"
//#include "gdcmTagToType.h"
#include "gdcmVM.h"

#include <string>
#include <vector>
#include <sstream>

namespace gdcm
{

template<uint16_t Group, uint16_t Element> class TagToVR;
template<uint16_t Group, uint16_t Element> class TagToVM;

// Declaration, also serve as forward declaration
template<int T> class EncodingImplementation;

struct void_;
template<uint16_t Group, uint16_t Element, 
	 int TVR = TagToVR<Group, Element>::VRType, 
	 int TVM = TagToVM<Group, Element>::VMType,
	 typename SQAttribute = void_ >
class Attribute
{
public:
  typename VRToType<TVR>::Type Internal[VMToLength<TVM>::Length];

  unsigned long GetLength() const {
    return VMToLength<TVM>::Length;
  }
  // Implementation of Print is common to all Mode (ASCII/Binary)
  // TODO: Can we print a \ when in ASCII...well I don't think so
  // it would mean we used a bad VM then, right ?
  void Print(std::ostream &_os) const {
    _os << Tag(Group, Element) << " ";
    //_os << VR((VR::VRType)TVR) << " ";
    _os << Internal[0]; // VM is at least garantee to be one
    for(int i=1; i<VMToLength<TVM>::Length; ++i)
      _os << "," << Internal[i];
    }

  typename VRToType<TVR>::Type GetValue(int idx = 0) {
    assert( idx < VMToLength<TVM>::Length );
    return Internal[idx];
  }
  void SetValue(typename VRToType<TVR>::Type v, unsigned int idx = 0) {
    assert( idx < VMToLength<TVM>::Length );
    Internal[idx] = v;
  }
  void Read(IStream &_is) {
    uint16_t cref[] = { Group, Element };
    uint16_t c[2];
    _is.read((char*)&c, 4);
    const uint32_t lref = GetLength() * sizeof( typename VRToType<TVR>::Type );
    uint32_t l;
    _is.read((char*)&l, 4);
    l /= sizeof( typename VRToType<TVR>::Type );
     return EncodingImplementation<VRToEncoding<TVR>::Mode>::Read(Internal, 
      l,_is);
    }
  void Write(OStream &_os) const {
    uint16_t c[] = { Group, Element };
    _os.write((char*)&c, 4);
    uint32_t l = GetLength() * sizeof( typename VRToType<TVR>::Type );
    _os.write((char*)&l, 4);
    return EncodingImplementation<VRToEncoding<TVR>::Mode>::Write(Internal, 
      GetLength(),_os);
    }
};


// Implementation to perform formatted read and write
template<> class EncodingImplementation<VR::ASCII> {
public:
  template<typename T> // FIXME this should be VRToType<TVR>::Type
  static inline void Read(T* data, unsigned long length,
                          IStream &_is) {
    assert( data );
    assert( length ); // != 0
    assert( _is );
    _is >> data[0];
    char sep;
    //std::cout << "GetLength: " << af->GetLength() << std::endl;
    for(unsigned long i=1; i<length;++i) {
      assert( _is );
      // Get the separator in between the values
      //_is.get(sep);
      //assert( sep == '\\' ); // FIXME: Bad use of assert
      _is >> data[i];
      }
    }

  template<typename T>
  static inline void Write(const T* data, unsigned long length,
                           OStream &_os)  {
    assert( data );
    assert( length );
    assert( _os );
    _os << data[0];
    for(unsigned long i=1; i<length; ++i) {
      assert( _os );
      //_os << "\\" << data[i];
      _os << data[i];
      //abort();
      }
    }
};

// Implementation to perform binary read and write
// TODO rewrite operation so that either:
// #1. dummy implementation use a pointer to Internal and do ++p (faster)
// #2. Actually do some meta programming to unroll the loop 
// (no notion of order in VM ...)
template<> class EncodingImplementation<VR::BINARY> {
public:
  template<typename T>
  static inline void Read(T* data, unsigned long length,
    IStream &_is) {
    const unsigned int type_size = sizeof(T);
    assert( data ); // Can we read from pointer ?
    assert( length );
    assert( _is ); // Is stream valid ?
    _is.read( reinterpret_cast<char*>(data+0), type_size);
    for(unsigned long i=1; i<length; ++i) {
      assert( _is );
      _is.read( reinterpret_cast<char*>(data+i), type_size );
    }
    //ByteSwap<T>::SwapRangeFromSwapCodeIntoSystem(data,
    //  _is.GetSwapCode(), length);
  }
  template<typename T>
  static inline void Write(const T* data, unsigned long length,
    OStream &_os) { 
    const unsigned int type_size = sizeof(T);
    assert( data ); // Can we write into pointer ?
    assert( length );
    assert( _os ); // Is stream valid ?
    //ByteSwap<T>::SwapRangeFromSwapCodeIntoSystem((T*)data,
    //  _os.GetSwapCode(), length);
    _os.write( reinterpret_cast<const char*>(&(data[0])), type_size);
    for(unsigned long i=1; i<length;++i) {
      assert( _os );
      _os.write( reinterpret_cast<const char*>(&(data[i])), type_size );
    }
    //ByteSwap<T>::SwapRangeFromSwapCodeIntoSystem((T*)data,
    //  _os.GetSwapCode(), length);
  }
};

// For particular case for ASCII string
// WARNING: This template explicitely instanciates a particular 
// EncodingImplementation THEREFORE it is required to be declared after the
// EncodingImplementation is needs (doh!)
#if 0
template<int TVM>
class Attribute<TVM>
{
public:
  Attribute(const char array[])
    {
    unsigned int i = 0;
    const char sep = '\\';
    std::string sarray = array;
    std::string::size_type pos1 = 0;
    std::string::size_type pos2 = sarray.find(sep, pos1+1);
    while(pos2 != std::string::npos)
      {
      Internal[i++] = sarray.substr(pos1, pos2-pos1);
      pos1 = pos2+1;
      pos2 = sarray.find(sep, pos1+1);
      } 
    Internal[i] = sarray.substr(pos1, pos2-pos1);
    // Shouldn't we do the contrary, since we know how many separators
    // (and default behavior is to discard anything after the VM declared
    assert( GetLength()-1 == i );
    }

  unsigned long GetLength() const {
    return VMToLength<TVM>::Length;
  }
  // Implementation of Print is common to all Mode (ASCII/Binary)
  void Print(std::ostream &_os) const {
    _os << Internal[0]; // VM is at least garantee to be one
    for(int i=1; i<VMToLength<TVM>::Length; ++i)
      _os << "," << Internal[i];
    }

  void Read(IStream &_is) {
    EncodingImplementation<VR::ASCII>::Read(Internal, GetLength(),_is);
    }
  void Write(std::ostream &_os) const {
    EncodingImplementation<VR::ASCII>::Write(Internal, GetLength(),_os);
    }
private:
  typename String Internal[VMToLength<TVM>::Length];
};

template< int TVM>
class Attribute<VR::PN, TVM> : public StringAttribute<TVM>
{
};
#endif

#if 0

// Implementation for the undefined length (dynamically allocated array) 
template<int TVR>
class Attribute<TVR, VM::VM1_n>
{
public:
  // This the way to prevent default initialization
  explicit Attribute() { Internal=0; Length=0; }
  ~Attribute() {
    delete[] Internal;
    Internal = 0;
  }

  // Length manipulation
  // SetLength should really be protected anyway...all operation
  // should go through SetArray
  unsigned long GetLength() const { return Length; }
  typedef typename VRToType<TVR>::Type ArrayType;
  void SetLength(unsigned long len) {
    const unsigned int size = sizeof(ArrayType);
    if( len ) {
      if( len > Length ) {
        // perform realloc
        assert( (len / size) * size == len );
        ArrayType *internal = new ArrayType[len / size];
        memcpy(internal, Internal, Length * size);
        delete[] Internal;
        Internal = internal;
        }
      }
    Length = len / size;
  }

  // If save is set to zero user should not delete the pointer
  //void SetArray(const typename VRToType<TVR>::Type *array, int len, bool save = false) 
  void SetArray(const ArrayType *array, unsigned long len,
    bool save = false) {
    if( save ) {
      SetLength(len); // realloc
      memcpy(Internal, array, len/*/sizeof(ArrayType)*/);
      }
    else {
      // TODO rewrite this stupid code:
      Length = len;
      //Internal = array;
      abort();
      }
  }
  // Implementation of Print is common to all Mode (ASCII/Binary)
  void Print(std::ostream &_os) const {
    assert( Length );
    assert( Internal );
    _os << Internal[0]; // VM is at least garantee to be one
    const unsigned long length = GetLength() < 25 ? GetLength() : 25;
    for(unsigned long i=1; i<length; ++i)
      _os << "," << Internal[i];
    }
  void Read(IStream &_is) {
    EncodingImplementation<VRToEncoding<TVR>::Mode>::Read(Internal, 
      GetLength(),_is);
    }
  void Write(std::ostream &_os) const {
    EncodingImplementation<VRToEncoding<TVR>::Mode>::Write(Internal, 
      GetLength(),_os);
    }

  Attribute(const Attribute&_val) {
    if( this != &_val) {
      *this = _val;
      }
    }

  Attribute &operator=(const Attribute &_val) {
    Length = 0; // SYITF
    Internal = 0;
    SetArray(_val.Internal, _val.Length, true);
    return *this;
    }

private:
  typename VRToType<TVR>::Type *Internal;
  unsigned long Length; // unsigned int ??
};

//template <int TVM = VM::VM1_n>
//class Attribute<VR::OB, TVM > : public Attribute<VR::OB, VM::VM1_n> {};

// Partial specialization for derivatives of 1-n : 2-n, 3-n ...
template<int TVR>
class Attribute<TVR, VM::VM2_n> : public Attribute<TVR, VM::VM1_n>
{
public:
  typedef Attribute<TVR, VM::VM1_n> Parent;
  void SetLength(int len) {
    if( len <= 1 ) return;
    Parent::SetLength(len);
  }
};
template<int TVR>
class Attribute<TVR, VM::VM2_2n> : public Attribute<TVR, VM::VM2_n>
{
public:
  typedef Attribute<TVR, VM::VM2_n> Parent;
  void SetLength(int len) {
    if( len % 2 ) return;
    Parent::SetLength(len);
  }
};
template<int TVR>
class Attribute<TVR, VM::VM3_n> : public Attribute<TVR, VM::VM1_n>
{
public:
  typedef Attribute<TVR, VM::VM1_n> Parent;
  void SetLength(int len) {
    if( len <= 2 ) return;
    Parent::SetLength(len);
  }
};
template<int TVR>
class Attribute<TVR, VM::VM3_3n> : public Attribute<TVR, VM::VM3_n>
{
public:
  typedef Attribute<TVR, VM::VM3_n> Parent;
  void SetLength(int len) {
    if( len % 3 ) return;
    Parent::SetLength(len);
  }
};


//template<int T> struct VRToLength;
//template <> struct VRToLength<VR::AS>
//{ enum { Length  = VM::VM1 }; }
//template<>
//class Attribute<VR::AS> : public Attribute<VR::AS, VRToLength<VR::AS>::Length >

// only 0010 1010 AS 1 Patient’s Age
template<>
class Attribute<VR::AS, VM::VM5>
{
public:
  char Internal[VMToLength<VM::VM5>::Length];
  void Print(std::ostream &_os) const {
    _os << Internal;
    }
};

template <>
class Attribute<VR::OB, VM::VM1> : public Attribute<VR::OB, VM::VM1_n> {};
// Make it impossible to compile any other cases:
template <int TVM> class Attribute<VR::OB, TVM>;

// Same for OW:
template <>
class Attribute<VR::OW, VM::VM1> : public Attribute<VR::OW, VM::VM1_n> {};
// Make it impossible to compile any other cases:
template <int TVM> class Attribute<VR::OW, TVM>;
#endif

template<>
class Attribute<0x7fe0,0x0010, VR::OW, VM::VM1>
{
public:
  char *Internal;
  unsigned long Length; // unsigned int ??

  void Print(std::ostream &_os) const {
    _os << Internal[0];
    }
  void SetBytes(char *bytes, unsigned long length) {
    Internal = bytes;
    Length = length;
  }
  void Read(IStream &_is) {
     uint16_t c[2];
    _is.read((char*)&c, 4);
    uint32_t l;
    _is.read((char*)&l, 4);
    Length = l;
    _is.read( Internal, Length );
    }
  void Write(OStream &_os) const {
     uint16_t c[] = {0x7fe0, 0x0010};
    _os.write((char*)&c, 4);
    _os.write((char*)&Length, 4);
    _os.write( Internal, Length );
    }
};

template<uint16_t Group, uint16_t Element, typename SQA>
class Attribute<Group,Element, VR::SQ, VM::VM1, SQA>
{
public:
  SQA sqa;
  void Print(std::ostream &_os) const {
    _os << Tag(Group,Element);
    sqa.Print(_os << std::endl << '\t');
    }
 void Write(OStream &_os) const {
    uint16_t c[] = {Group, Element};
    _os.write((char*)&c, 4);
    uint32_t undef = 0xffffffff;
    _os.write((char*)&undef, 4);
    uint16_t item_beg[] = {0xfffe,0xe000};
    _os.write((char*)&item_beg, 4);
    _os.write((char*)&undef, 4);
    sqa.Write(_os);
    uint16_t item_end[] = {0xfffe,0xe00d};
    _os.write((char*)&item_end, 4);
    uint32_t zero = 0x0;
    _os.write((char*)&zero, 4);
    uint16_t seq_end[] = {0xfffe, 0xe0dd};
    _os.write((char*)&seq_end, 4);
    _os.write((char*)&zero, 4);
    }
};


} // namespace gdcm

#endif //__gdcmAttribute_h