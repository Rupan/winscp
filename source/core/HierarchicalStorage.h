//---------------------------------------------------------------------------
#ifndef HierarchicalStorageH
#define HierarchicalStorageH

#include <registry.hpp>
//---------------------------------------------------------------------------
enum TStorage { stDetect, stRegistry, stIniFile, stNul };
enum TStorageAccessMode { smRead, smReadWrite };
//---------------------------------------------------------------------------
class THierarchicalStorage
{
public:
  __fastcall THierarchicalStorage(const UnicodeString AStorage);
  virtual __fastcall ~THierarchicalStorage();
  bool __fastcall OpenRootKey(bool CanCreate);
  bool __fastcall OpenSubKey(UnicodeString SubKey, bool CanCreate, bool Path = false);
  virtual void __fastcall CloseSubKey();
  virtual bool __fastcall DeleteSubKey(const UnicodeString SubKey) = 0;
  virtual void __fastcall GetSubKeyNames(Classes::TStrings* Strings) = 0;
  virtual void __fastcall GetValueNames(Classes::TStrings* Strings) = 0;
  bool __fastcall HasSubKeys();
  bool __fastcall HasSubKey(const UnicodeString SubKey);
  bool __fastcall KeyExists(const UnicodeString SubKey);
  virtual bool __fastcall ValueExists(const UnicodeString Value) = 0;
  virtual void __fastcall RecursiveDeleteSubKey(const UnicodeString Key);
  virtual void __fastcall ClearSubKeys();
  virtual void __fastcall ReadValues(Classes::TStrings* Strings, bool MaintainKeys = false);
  virtual void __fastcall WriteValues(Classes::TStrings* Strings, bool MaintainKeys = false);
  virtual void __fastcall ClearValues();
  virtual bool __fastcall DeleteValue(const UnicodeString Name) = 0;

  virtual size_t __fastcall BinaryDataSize(const UnicodeString Name) = 0;

  virtual bool __fastcall ReadBool(const UnicodeString Name, bool Default) = 0;
  virtual int __fastcall ReadInteger(const UnicodeString Name, int Default) = 0;
  virtual __int64 __fastcall ReadInt64(const UnicodeString Name, __int64 Default) = 0;
  virtual TDateTime __fastcall ReadDateTime(const UnicodeString Name, TDateTime Default) = 0;
  virtual double __fastcall ReadFloat(const UnicodeString Name, double Default) = 0;
  virtual UnicodeString __fastcall ReadStringRaw(const UnicodeString Name, const UnicodeString Default) = 0;
  virtual size_t __fastcall ReadBinaryData(const UnicodeString Name, void * Buffer, size_t Size) = 0;

  virtual UnicodeString __fastcall ReadString(UnicodeString Name, UnicodeString Default);
  RawByteString __fastcall ReadBinaryData(const UnicodeString Name);
  RawByteString __fastcall ReadStringAsBinaryData(const UnicodeString Name, const RawByteString Default);

  virtual void __fastcall WriteBool(const UnicodeString Name, bool Value) = 0;
  virtual void __fastcall WriteStringRaw(const UnicodeString Name, const UnicodeString Value) = 0;
  virtual void __fastcall WriteInteger(const UnicodeString Name, int Value) = 0;
  virtual void __fastcall WriteInt64(const UnicodeString Name, __int64 Value) = 0;
  virtual void __fastcall WriteDateTime(const UnicodeString Name, TDateTime Value) = 0;
  virtual void __fastcall WriteFloat(const UnicodeString Name, double Value) = 0;
  virtual void __fastcall WriteBinaryData(const UnicodeString Name, const void * Buffer, int Size) = 0;

  virtual void __fastcall WriteString(const UnicodeString Name, const UnicodeString Value);
  void __fastcall WriteBinaryData(const UnicodeString Name, const RawByteString Value);
  void __fastcall WriteBinaryDataAsString(const UnicodeString Name, const RawByteString Value);

  virtual void __fastcall Flush();

  __property UnicodeString Storage  = { read=FStorage };
  __property UnicodeString CurrentSubKey  = { read=GetCurrentSubKey };
  __property TStorageAccessMode AccessMode  = { read=FAccessMode, write=SetAccessMode };
  __property bool Explicit = { read = FExplicit, write = FExplicit };
  __property bool ForceAnsi = { read = FForceAnsi, write = FForceAnsi };
  __property bool MungeStringValues = { read = FMungeStringValues, write = FMungeStringValues };
  __property UnicodeString Source = { read = GetSource };

protected:
  UnicodeString FStorage;
  TStrings * FKeyHistory;
  TStorageAccessMode FAccessMode;
  bool FExplicit;
  bool FMungeStringValues;
  bool FForceAnsi;

  UnicodeString __fastcall GetCurrentSubKey();
  UnicodeString __fastcall GetCurrentSubKeyMunged();
  virtual void __fastcall SetAccessMode(TStorageAccessMode value);
  virtual bool __fastcall DoKeyExists(const UnicodeString SubKey, bool ForceAnsi) = 0;
  static UnicodeString __fastcall IncludeTrailingBackslash(const UnicodeString & S);
  static UnicodeString __fastcall ExcludeTrailingBackslash(const UnicodeString & S);
  virtual bool __fastcall DoOpenSubKey(const UnicodeString SubKey, bool CanCreate) = 0;
  UnicodeString __fastcall MungeKeyName(UnicodeString Key);
  virtual UnicodeString __fastcall GetSource() = 0;
};
//---------------------------------------------------------------------------
class TRegistryStorage : public THierarchicalStorage
{
public:
  __fastcall TRegistryStorage(const UnicodeString AStorage, HKEY ARootKey);
  __fastcall TRegistryStorage(const UnicodeString AStorage);
  virtual __fastcall ~TRegistryStorage();

  bool __fastcall Copy(TRegistryStorage * Storage);

  virtual void __fastcall CloseSubKey();
  virtual bool __fastcall DeleteSubKey(const UnicodeString SubKey);
  virtual bool __fastcall DeleteValue(const UnicodeString Name);
  virtual void __fastcall GetSubKeyNames(Classes::TStrings* Strings);
  virtual bool __fastcall ValueExists(const UnicodeString Value);

  virtual size_t __fastcall BinaryDataSize(const UnicodeString Name);

  virtual bool __fastcall ReadBool(const UnicodeString Name, bool Default);
  virtual int __fastcall ReadInteger(const UnicodeString Name, int Default);
  virtual __int64 __fastcall ReadInt64(const UnicodeString Name, __int64 Default);
  virtual TDateTime __fastcall ReadDateTime(const UnicodeString Name, TDateTime Default);
  virtual double __fastcall ReadFloat(const UnicodeString Name, double Default);
  virtual UnicodeString __fastcall ReadStringRaw(const UnicodeString Name, const UnicodeString Default);
  virtual size_t __fastcall ReadBinaryData(const UnicodeString Name, void * Buffer, size_t Size);

  virtual void __fastcall WriteBool(const UnicodeString Name, bool Value);
  virtual void __fastcall WriteInteger(const UnicodeString Name, int Value);
  virtual void __fastcall WriteInt64(const UnicodeString Name, __int64 Value);
  virtual void __fastcall WriteDateTime(const UnicodeString Name, TDateTime Value);
  virtual void __fastcall WriteFloat(const UnicodeString Name, double Value);
  virtual void __fastcall WriteStringRaw(const UnicodeString Name, const UnicodeString Value);
  virtual void __fastcall WriteBinaryData(const UnicodeString Name, const void * Buffer, int Size);

  virtual void __fastcall GetValueNames(Classes::TStrings* Strings);

protected:
  int __fastcall GetFailed();
  virtual void __fastcall SetAccessMode(TStorageAccessMode value);
  virtual bool __fastcall DoKeyExists(const UnicodeString SubKey, bool ForceAnsi);
  virtual bool __fastcall DoOpenSubKey(const UnicodeString SubKey, bool CanCreate);
  virtual UnicodeString __fastcall GetSource();

  __property int Failed  = { read=GetFailed, write=FFailed };

private:
  TRegistry * FRegistry;
  int FFailed;

  void __fastcall Init();
};
//---------------------------------------------------------------------------
class TCustomIniFileStorage : public THierarchicalStorage
{
public:
  __fastcall TCustomIniFileStorage(const UnicodeString Storage, TCustomIniFile * IniFile);
  virtual __fastcall ~TCustomIniFileStorage();

  virtual bool __fastcall DeleteSubKey(const UnicodeString SubKey);
  virtual bool __fastcall DeleteValue(const UnicodeString Name);
  virtual void __fastcall GetSubKeyNames(Classes::TStrings* Strings);
  virtual bool __fastcall ValueExists(const UnicodeString Value);

  virtual size_t __fastcall BinaryDataSize(const UnicodeString Name);

  virtual bool __fastcall ReadBool(const UnicodeString Name, bool Default);
  virtual int __fastcall ReadInteger(const UnicodeString Name, int Default);
  virtual __int64 __fastcall ReadInt64(const UnicodeString Name, __int64 Default);
  virtual TDateTime __fastcall ReadDateTime(const UnicodeString Name, TDateTime Default);
  virtual double __fastcall ReadFloat(const UnicodeString Name, double Default);
  virtual UnicodeString __fastcall ReadStringRaw(const UnicodeString Name, const UnicodeString Default);
  virtual size_t __fastcall ReadBinaryData(const UnicodeString Name, void * Buffer, size_t Size);

  virtual void __fastcall WriteBool(const UnicodeString Name, bool Value);
  virtual void __fastcall WriteInteger(const UnicodeString Name, int Value);
  virtual void __fastcall WriteInt64(const UnicodeString Name, __int64 Value);
  virtual void __fastcall WriteDateTime(const UnicodeString Name, TDateTime Value);
  virtual void __fastcall WriteFloat(const UnicodeString Name, double Value);
  virtual void __fastcall WriteStringRaw(const UnicodeString Name, const UnicodeString Value);
  virtual void __fastcall WriteBinaryData(const UnicodeString Name, const void * Buffer, int Size);

  virtual void __fastcall GetValueNames(Classes::TStrings* Strings);

private:
  UnicodeString __fastcall GetCurrentSection();
protected:
  TCustomIniFile * FIniFile;

  __property UnicodeString CurrentSection  = { read=GetCurrentSection };
  virtual bool __fastcall DoKeyExists(const UnicodeString SubKey, bool ForceAnsi);
  virtual bool __fastcall DoOpenSubKey(const UnicodeString SubKey, bool CanCreate);
  virtual UnicodeString __fastcall GetSource();
};
//---------------------------------------------------------------------------
class TIniFileStorage : public TCustomIniFileStorage
{
public:
  __fastcall TIniFileStorage(const UnicodeString FileName);
  virtual __fastcall ~TIniFileStorage();

  virtual void __fastcall Flush();

private:
  TStrings * FOriginal;
  void __fastcall ApplyOverrides();
};
//---------------------------------------------------------------------------
class TOptionsStorage : public TCustomIniFileStorage
{
public:
  __fastcall TOptionsStorage(TStrings * Options);
};
//---------------------------------------------------------------------------
UnicodeString __fastcall PuttyMungeStr(const UnicodeString Str);
//---------------------------------------------------------------------------
#endif
