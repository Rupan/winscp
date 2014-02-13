//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Common.h"
#include "FileBuffer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
char * __fastcall EOLToStr(TEOLType EOLType)
{
  switch (EOLType) {
    case eolLF: return "\n";
    case eolCRLF: return "\r\n";
    case eolCR: return "\r";
    default: assert(false); return "";
  }
}
//---------------------------------------------------------------------------
__fastcall TFileBuffer::TFileBuffer()
{
  FMemory = new TMemoryStream();
  FSize = 0;
}
//---------------------------------------------------------------------------
__fastcall TFileBuffer::~TFileBuffer()
{
  delete FMemory;
}
//---------------------------------------------------------------------------
void __fastcall TFileBuffer::SetSize(int value)
{
  if (FSize != value)
  {
    FMemory->Size = value;
    FSize = value;
  }
}
//---------------------------------------------------------------------------
void __fastcall TFileBuffer::SetPosition(int value)
{
  FMemory->Position = value;
}
//---------------------------------------------------------------------------
int __fastcall TFileBuffer::GetPosition() const
{
  return (int)FMemory->Position;
}
//---------------------------------------------------------------------------
void __fastcall TFileBuffer::SetMemory(TMemoryStream * value)
{
  if (FMemory != value)
  {
    if (FMemory) delete FMemory;
    FMemory = value;
  }
}
//---------------------------------------------------------------------------
DWORD __fastcall TFileBuffer::ReadStream(TStream * Stream, const DWORD Len, bool ForceLen)
{
  DWORD Result;
  try
  {
    Size = Position + Len;
    // C++5
    // FMemory->SetSize(FMemory->Position + Len);
    if (ForceLen)
    {
      Stream->ReadBuffer(Data + Position, Len);
      Result = Len;
    }
    else
    {
      Result = Stream->Read(Data + Position, Len);
    }
    if (Result != Len)
    {
      Size = Size - Len + Result;
    }
    FMemory->Seek(Len, soFromCurrent);
  }
  catch(EReadError &)
  {
    RaiseLastOSError();
  }
  return Result;
}
//---------------------------------------------------------------------------
DWORD __fastcall TFileBuffer::LoadStream(TStream * Stream, const DWORD Len, bool ForceLen)
{
  FMemory->Seek(0, soFromBeginning);
  return ReadStream(Stream, Len, ForceLen);
}
//---------------------------------------------------------------------------
void __fastcall TFileBuffer::Convert(char * Source, char * Dest, int Params,
  bool & Token)
{
  assert(strlen(Source) <= 2);
  assert(strlen(Dest) <= 2);

  if (FLAGSET(Params, cpRemoveBOM) && (Size >= 3) &&
      (memcmp(Data, Bom, sizeof(Bom)) == 0))
  {
    Delete(0, 3);
  }

  if (FLAGSET(Params, cpRemoveCtrlZ) && (Size > 0) && ((*(Data + Size - 1)) == '\x1A'))
  {
    Delete(Size-1, 1);
  }

  if (strcmp(Source, Dest) == 0)
  {
    return;
  }

  char * Ptr = Data;

  // one character source EOL
  if (!Source[1])
  {
    bool PrevToken = Token;
    Token = false;

    for (int Index = 0; Index < Size; Index++)
    {
      // EOL already in destination format, make sure to pass it unmodified
      if ((Index < Size - 1) && (*Ptr == Dest[0]) && (*(Ptr+1) == Dest[1]))
      {
        Index++;
        Ptr++;
      }
      // last buffer ended with the first char of destination 2-char EOL format,
      // which got expanded to full destination format.
      // now we got the second char, so get rid of it.
      else if ((Index == 0) && PrevToken && (*Ptr == Dest[1]))
      {
        Delete(Index, 1);
      }
      // we are ending with the first char of destination 2-char EOL format,
      // append the second char and make sure we strip it from the next buffer, if any
      else if ((*Ptr == Dest[0]) && (Index == Size - 1) && Dest[1])
      {
        Token = true;
        Insert(Index+1, Dest+1, 1);
        Index++;
        Ptr = Data + Index;
      }
      else if (*Ptr == Source[0])
      {
        *Ptr = Dest[0];
        if (Dest[1])
        {
          Insert(Index+1, Dest+1, 1);
          Index++;
          Ptr = Data + Index;
        }
      }
      Ptr++;
    }
  }
  // two character source EOL
  else
  {
    int Index;
    for (Index = 0; Index < Size - 1; Index++)
    {
      if ((*Ptr == Source[0]) && (*(Ptr+1) == Source[1]))
      {
        *Ptr = Dest[0];
        if (Dest[1])
        {
          *(Ptr+1) = Dest[1];
          Index++; Ptr++;
        }
        else
        {
          Delete(Index+1, 1);
          Ptr = Data + Index;
        }
      }
      Ptr++;
    }
    if ((Index < Size) && (*Ptr == Source[0]))
    {
      Delete(Index, 1);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TFileBuffer::Convert(TEOLType Source, TEOLType Dest, int Params,
  bool & Token)
{
  Convert(EOLToStr(Source), EOLToStr(Dest), Params, Token);
}
//---------------------------------------------------------------------------
void __fastcall TFileBuffer::Convert(char * Source, TEOLType Dest, int Params,
  bool & Token)
{
  Convert(Source, EOLToStr(Dest), Params, Token);
}
//---------------------------------------------------------------------------
void __fastcall TFileBuffer::Convert(TEOLType Source, char * Dest, int Params,
  bool & Token)
{
  Convert(EOLToStr(Source), Dest, Params, Token);
}
//---------------------------------------------------------------------------
void __fastcall TFileBuffer::Insert(int Index, const char * Buf, int Len)
{
  Size += Len;
  memmove(Data + Index + Len, Data + Index, Size - Index - Len);
  memmove(Data + Index, Buf, Len);
}
//---------------------------------------------------------------------------
void __fastcall TFileBuffer::Delete(int Index, int Len)
{
  memmove(Data + Index, Data + Index + Len, Size - Index - Len);
  Size -= Len;
}
//---------------------------------------------------------------------------
void __fastcall TFileBuffer::WriteToStream(TStream * Stream, const DWORD Len)
{
  try
  {
    Stream->WriteBuffer(Data + Position, Len);
    FMemory->Seek(Len, soFromCurrent);
  }
  catch(EWriteError &)
  {
    RaiseLastOSError();
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TSafeHandleStream::TSafeHandleStream(int AHandle) :
  THandleStream(AHandle)
{
}
//---------------------------------------------------------------------------
int __fastcall TSafeHandleStream::Read(void * Buffer, int Count)
{
  int Result = FileRead(FHandle, Buffer, Count);
  if (Result == -1)
  {
    RaiseLastOSError();
  }
  return Result;
}
//---------------------------------------------------------------------------
int __fastcall TSafeHandleStream::Write(const void * Buffer, int Count)
{
  int Result = FileWrite(FHandle, Buffer, Count);
  if (Result == -1)
  {
    RaiseLastOSError();
  }
  return Result;
}
