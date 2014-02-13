//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Common.h"
#include "Security.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#define PWALG_SIMPLE_INTERNAL 0x00
#define PWALG_SIMPLE_EXTERNAL 0x01
//---------------------------------------------------------------------------
RawByteString SimpleEncryptChar(unsigned char Ch)
{
  Ch = (unsigned char)((~Ch) ^ PWALG_SIMPLE_MAGIC);
  return
    PWALG_SIMPLE_STRING.SubString(((Ch & 0xF0) >> 4) + 1, 1) +
    PWALG_SIMPLE_STRING.SubString(((Ch & 0x0F) >> 0) + 1, 1);
}
//---------------------------------------------------------------------------
unsigned char SimpleDecryptNextChar(RawByteString &Str)
{
  if (Str.Length() > 0)
  {
    unsigned char Result = (unsigned char)
      ~((((PWALG_SIMPLE_STRING.Pos(Str.c_str()[0])-1) << 4) +
         ((PWALG_SIMPLE_STRING.Pos(Str.c_str()[1])-1) << 0)) ^ PWALG_SIMPLE_MAGIC);
    Str.Delete(1, 2);
    return Result;
  }
  else return 0x00;
}
//---------------------------------------------------------------------------
RawByteString EncryptPassword(UnicodeString UnicodePassword, UnicodeString UnicodeKey, Integer /* Algorithm */)
{
  UTF8String Password = UnicodePassword;
  UTF8String Key = UnicodeKey;

  RawByteString Result("");
  int Shift, Index;

  if (!RandSeed) Randomize();
  Password = Key + Password;
  Shift = (Password.Length() < PWALG_SIMPLE_MAXLEN) ?
    (unsigned char)random(PWALG_SIMPLE_MAXLEN - Password.Length()) : 0;
  Result += SimpleEncryptChar((unsigned char)PWALG_SIMPLE_FLAG); // Flag
  Result += SimpleEncryptChar((unsigned char)PWALG_SIMPLE_INTERNAL); // Dummy
  Result += SimpleEncryptChar((unsigned char)Password.Length());
  Result += SimpleEncryptChar((unsigned char)Shift);
  for (Index = 0; Index < Shift; Index++)
    Result += SimpleEncryptChar((unsigned char)random(256));
  for (Index = 0; Index < Password.Length(); Index++)
    Result += SimpleEncryptChar(Password.c_str()[Index]);
  while (Result.Length() < PWALG_SIMPLE_MAXLEN * 2)
    Result += SimpleEncryptChar((unsigned char)random(256));
  return Result;
}
//---------------------------------------------------------------------------
UnicodeString DecryptPassword(RawByteString Password, UnicodeString UnicodeKey, Integer /* Algorithm */)
{
  UTF8String Key = UnicodeKey;
  UTF8String Result("");
  Integer Index;
  unsigned char Length, Flag;

  Flag = SimpleDecryptNextChar(Password);
  if (Flag == PWALG_SIMPLE_FLAG)
  {
    /* Dummy = */ SimpleDecryptNextChar(Password);
    Length = SimpleDecryptNextChar(Password);
  }
    else Length = Flag;
  Password.Delete(1, ((Integer)SimpleDecryptNextChar(Password))*2);
  for (Index = 0; Index < Length; Index++)
    Result += (char)SimpleDecryptNextChar(Password);
  if (Flag == PWALG_SIMPLE_FLAG)
  {
    if (Result.SubString(1, Key.Length()) != Key) Result = "";
      else Result.Delete(1, Key.Length());
  }
  return UnicodeString(Result);
}
//---------------------------------------------------------------------------
RawByteString SetExternalEncryptedPassword(RawByteString Password)
{
  RawByteString Result;
  Result += SimpleEncryptChar((unsigned char)PWALG_SIMPLE_FLAG);
  Result += SimpleEncryptChar((unsigned char)PWALG_SIMPLE_EXTERNAL);
  Result += UTF8String(BytesToHex(reinterpret_cast<const unsigned char *>(Password.c_str()), Password.Length()));
  return Result;
}
//---------------------------------------------------------------------------
bool GetExternalEncryptedPassword(RawByteString Encrypted, RawByteString & Password)
{
  bool Result =
    (SimpleDecryptNextChar(Encrypted) == PWALG_SIMPLE_FLAG) &&
    (SimpleDecryptNextChar(Encrypted) == PWALG_SIMPLE_EXTERNAL);
  if (Result)
  {
    Password = HexToBytes(UnicodeString(UTF8String(Encrypted.c_str(), Encrypted.Length())));
  }
  return Result;
}
//---------------------------------------------------------------------------
