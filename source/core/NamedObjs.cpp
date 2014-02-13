//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "NamedObjs.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
int __fastcall NamedObjectSortProc(void * Item1, void * Item2)
{
  bool HasPrefix1 = ((TNamedObject *)Item1)->Hidden;
  bool HasPrefix2 = ((TNamedObject *)Item2)->Hidden;
  if (HasPrefix1 && !HasPrefix2) return -1;
    else
  if (!HasPrefix1 && HasPrefix2) return 1;
    else
  return AnsiCompareStr(((TNamedObject *)Item1)->Name, ((TNamedObject *)Item2)->Name);
}
//--- TNamedObject ----------------------------------------------------------
__fastcall TNamedObject::TNamedObject(UnicodeString AName)
{
  Name = AName;
}
//---------------------------------------------------------------------------
void __fastcall TNamedObject::SetName(UnicodeString value)
{
  FHidden = (value.SubString(1, TNamedObjectList::HiddenPrefix.Length()) == TNamedObjectList::HiddenPrefix);
  FName = value;
}
//---------------------------------------------------------------------------
Integer __fastcall TNamedObject::CompareName(UnicodeString aName,
  Boolean CaseSensitive)
{
  if (CaseSensitive)
    return Name.Compare(aName);
  else
    return Name.CompareIC(aName);
}
//---------------------------------------------------------------------------
void __fastcall TNamedObject::MakeUniqueIn(TNamedObjectList * List)
{
  // This object can't be item of list, it would create infinite loop
  if (List && (List->IndexOf(this) == -1))
    while (List->FindByName(Name))
    {
      Integer N = 0, P;
      // If name already contains number parenthesis remove it (and remember it)
      if ((Name[Name.Length()] == L')') && ((P = Name.LastDelimiter(L'(')) > 0))
        try
        {
          N = StrToInt(Name.SubString(P + 1, Name.Length() - P - 1));
          Name.Delete(P, Name.Length() - P + 1);
          Name = Name.TrimRight();
        }
        catch (Exception &E)
        {
          N = 0;
        }
      Name += L" (" + IntToStr(N+1) + L")";
    }
}
//--- TNamedObjectList ------------------------------------------------------
const UnicodeString TNamedObjectList::HiddenPrefix = L"_!_";
//---------------------------------------------------------------------------
__fastcall TNamedObjectList::TNamedObjectList():
  TObjectList()
{
  AutoSort = True;
}
//---------------------------------------------------------------------------
TNamedObject * __fastcall TNamedObjectList::AtObject(Integer Index)
{
  return (TNamedObject *)Items[Index+HiddenCount];
}
//---------------------------------------------------------------------------
void __fastcall TNamedObjectList::Recount()
{
  int i = 0;
  while ((i < TObjectList::Count) && ((TNamedObject *)Items[i])->Hidden) i++;
  FHiddenCount = i;
}
//---------------------------------------------------------------------------
void __fastcall TNamedObjectList::AlphaSort()
{
  Sort(NamedObjectSortProc);
}
//---------------------------------------------------------------------------
void __fastcall TNamedObjectList::Notify(void *Ptr, TListNotification Action)
{
  TObjectList::Notify(Ptr, Action);
  if (AutoSort && (Action == lnAdded)) AlphaSort();
  Recount();
}
//---------------------------------------------------------------------------
TNamedObject * __fastcall TNamedObjectList::FindByName(UnicodeString Name,
  Boolean CaseSensitive)
{
  for (Integer Index = 0; Index < TObjectList::Count; Index++)
    if (!((TNamedObject *)Items[Index])->CompareName(Name, CaseSensitive))
      return (TNamedObject *)Items[Index];
  return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TNamedObjectList::SetCount(int value)
{
  TObjectList::SetCount(value/*+HiddenCount*/);
}
//---------------------------------------------------------------------------
int __fastcall TNamedObjectList::GetCount()
{
  return TObjectList::Count - HiddenCount;
}
