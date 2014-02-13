unit PIDL;
{
  Description
  ===========
    Some methods to pidls. Purpose of methods are described in the code.


  Disclaimer
  ==========
    The author disclaims all warranties, expressed or implied, including,
    without limitation, the warranties of merchantability and of fitness
    for any purpose. The author assumes no liability for damages, direct or
    consequential, which may result from the use of this unit.


  Restrictions on Using the Unit
  ==============================
    This unit is copyright 1998 by Dieter Steinwedel. ALL RIGHTS
    ARE RESERVED BY DIETER STEINWEDEL. You are allowed to use it freely
    subject to the following restrictions:

    � You are not allowed delete or alter the author's name and
      copyright in any manner

    � You are not allowed to publish a copy, modified version or
      compilation neither for payment in any kind nor freely

    � You are allowed to create a link to the download in the WWW

    � These restrictions and terms apply to you as long as until
      I alter them. Changes can found on my homepage


  Contact
  =======
    homepage: http://godard.oec.uni-osnabrueck.de/student_home/dsteinwe/delphi/DietersDelphiSite.htm
}
interface

uses ShlObj, Windows, ActiveX;

function PIDL_GetNextItem(PIDL: PItemIDList):PItemIDList;
function PIDL_GetSize(pidl: PITEMIDLIST): integer;
function PIDL_Create(Size: UINT): PItemIDList;
function PIDL_Concatenate(pidl1, pidl2: PItemIDList): PItemIDList;
function PIDL_Copy(pidlSource: PItemIDList): PItemIDList;
function PIDL_GetDisplayName(piFolder: IShellFolder; pidl: PItemIDList;
   dwFlags: DWORD; pszName: PChar; cchMax: UINT): boolean;
function Pidl_GetFullyQualified(const PiParentFolder: IShellFolder;
   pidl: PItemIDList): PItemIDList;
procedure PIDL_GetRelative(var pidlFQ, ppidlRoot, ppidlItem: PItemIDList);
function PIDL_GetFromPath(pszFile: PChar): PItemIDList;
function PIDL_GetFileFolder(pidl: PItemIDList; var piFolder: IShellFolder):boolean;
function PIDL_GetFromParentFolder(pParentFolder: IShellFolder; pszFile: PChar): PItemIDList;
procedure PIDL_Free(PIDL:PItemIDList);
function PIDL_Equal(PIDL1,PIDL2:PItemIDList):boolean;

var ShellMalloc: IMalloc;

    CF_FILECONTENTS:UInt; // don't modify value
    CF_FILEDESCRIPTOR:UInt; // don't modify value
    CF_FILENAME:UInt; // don't modify value
    CF_FILENAMEMAP:UInt; // don't modify value
    CF_FILENAMEMAPW:UInt; // don't modify value
    CF_INDRAGLOOP:UInt; // don't modify value
    CF_NETRESOURCES:UInt; // don't modify value
    CF_PASTESUCCEEDED:UInt; // don't modify value
    CF_PERFORMEDDROPEFFECT:UInt; // don't modify value
    CF_PREFERREDDROPEFFECT:UInt; // don't modify value
    CF_PRINTERGROUP:UInt; // don't modify value
    CF_SHELLIDLIST:UInt; // don't modify value
    CF_SHELLIDLISTOFFSET:UInt; // don't modify value
    CF_SHELLURL:UInt; // don't modify value

implementation

const NullTerm=2;

function PIDL_GetNextItem(PIDL: PItemIDList):PItemIDList;
//  PURPOSE:    Returns a pointer to the next item in the ITEMIDLIST.
//  PARAMETERS:
//      pidl - Pointer to an ITEMIDLIST to walk through
begin
     if PIDL<>nil then Result:=PItemIDList(PAnsiChar(PIDL)+PIDL^.mkid.cb)
     else Result:=nil
end;

function PIDL_GetSize(pidl: PITEMIDLIST): integer;
//  PURPOSE:    Returns the total number of bytes in an ITEMIDLIST.
//  PARAMETERS:
//      pidl - Pointer to the ITEMIDLIST that you want the size of.
begin
     Result:=0;
     if pidl<>nil then
     begin
          Inc(Result, SizeOf(pidl^.mkid.cb));
          while pidl^.mkid.cb <> 0 do
          begin
               Inc(Result, pidl^.mkid.cb);
               Inc(longint(pidl), pidl^.mkid.cb);
          end;
     end;
end;

function PIDL_Create(Size: UINT): PItemIDList;
//  PURPOSE:    Creates a new ITEMIDLIST of the specified size.
//  PARAMETERS:
//      piMalloc - Pointer to the allocator interface that should allocate memory.
//  cbSize   - Size of the ITEMIDLIST to create.
//  RETURN VALUE:
//      Returns a pointer to the new ITEMIDLIST, or NULL if a problem occured.
begin
     Result:=ShellMalloc.Alloc(Size);
     if Result<>nil then
        FillChar(Result^, Size, #0);
end;

function PIDL_Concatenate(pidl1, pidl2: PItemIDList): PItemIDList;
//  PURPOSE:    Creates a new ITEMIDLIST with pidl2 appended to pidl1.
//  PARAMETERS:
//  piMalloc - Pointer to the allocator interface that should create the new ITEMIDLIST.
//      pidl1- Pointer to an ITEMIDLIST that contains the root.
//  pidl2    - Pointer to an ITEMIDLIST that contains what should be appended to the root.
//  RETURN VALUE:
//      Returns a new ITEMIDLIST if successful, NULL otherwise.
var cb1, cb2: UINT;
begin
     if (pidl1<>nil) then cb1:=PIDL_GetSize(pidl1)-NullTerm else cb1:=0;
     cb2:=PIDL_GetSize(pidl2);
     Result:=PIDL_Create(cb1 + cb2);
     if Result<>nil then
     begin
          if pidl1<>nil then CopyMemory(Result,pidl1,cb1);
          CopyMemory(PAnsiChar(Result)+cb1,pidl2,cb2);
     end;
end;

function PIDL_Copy(pidlSource: PItemIDList): PItemIDList;
//  PURPOSE:    Creates a new copy of an ITEMIDLIST.
//  PARAMETERS:
//      piMalloc - Pointer to the allocator interfaced to be used to allocate the new ITEMIDLIST.
//  RETURN VALUE:
//      Returns a pointer to the new ITEMIDLIST, or NULL if an error occurs.
var cbSource:UINT;
begin
     Result:=nil;
     if pidlSource=nil then exit;
     cbSource:=PIDL_GetSize(pidlSource);
     Result:=PIDL_Create(cbSource);
     if Result=nil then exit;
     CopyMemory(Result,pidlSource,cbSource);
end;

function PIDL_GetDisplayName(piFolder: IShellFolder; pidl: PItemIDList;
   dwFlags: DWORD; pszName: PChar; cchMax: UINT): boolean;
//  PURPOSE:    Returns the display name for the item pointed to by pidl.  The
//              function assumes the pidl is relative to piFolder.  If piFolder
//              is NULL, the function assumes the item is fully qualified.
//  PARAMETERS:
//  piFolder - Pointer to the IShellFolder for the folder containing the item.
//  pidl     - Pointer to an ITEMIDLIST relative to piFolder that we want
//             the display name for.
//  dwFlags  - Flags to pass to ISF::GetDisplayNameOf().
//  pszName  - Pointer to the string where the display name is returned.
//  cchMax   - Maximum number of characters in pszName.
//  RETURN VALUE:
//      Returns TRUE if successful, FALSE otherwise.
var Str: TStrRet;
begin
     if (piFolder=nil) and (Failed(SHGetDesktopFolder(piFolder))) then
     begin
          Result:=false;
          exit;
     end;
     Result:=TRUE;
     if piFolder.GetDisplayNameOf(pidl, dwFlags, Str) = NOERROR then
     begin
          case Str.uType of
               STRRET_WSTR:
                  lstrcpyn(pszName, str.pOleStr, cchMax);
               STRRET_OFFSET:
                  MultiByteToWideChar(CP_ACP, 0, PAnsiChar(pidl)+str.uOffset, -1, pszName, cchMax);
               STRRET_CSTR:
                  MultiByteToWideChar(CP_ACP, 0, str.cStr, -1, pszName, cchMax);
               else Result := FALSE;
          end;
     end
     else Result:=FALSE;
     // piFolder._Release; -> automaticly done by D4
end;

function Pidl_GetFullyQualified(const PiParentFolder: IShellFolder;
   pidl: PItemIDList): PItemIDList;
//  PURPOSE:    Takes a relative PIDL and it's parent IShellFolder, and returns
//      a fully qualified ITEMIDLIST.
//  PARAMETERS:
//      piParentFolder - Pointer to the IShellFolder of the parent folder.
//  pidl           - ITEMIDLIST relative to piParentFolder
//  RETURN VALUE:
//      Returns a fully qualified ITEMIDLIST or NULL if there is a problem.
var piDesktopFolder:IShellFolder;
    szBuffer: array[1..Max_Path] of char;
    szOleChar: array[1..Max_Path] of TOLECHAR;
    ulEaten, ulAttribs:ULong;
begin
     Result:=nil;
     if Failed(SHGetDesktopFolder(piDesktopFolder)) then exit;
     if PIDL_GetDisplayName(piParentFolder, pidl, SHGDN_FORPARSING, @szBuffer, sizeof(szBuffer))=false then
        exit;
     MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, @szBuffer, -1, @szOleChar, sizeof(szOleChar));
     if Failed(piDesktopFolder.ParseDisplayName(0, nil, @szOleChar, ulEaten,
        Result, ulAttribs)) then Result:=nil;
     // piDesktopFolder._Release; automaticly done by D4
end;

procedure PIDL_GetRelative(var pidlFQ, ppidlRoot, ppidlItem: PItemIDList);
//  PURPOSE:    Takes a fully qualified pidl and returns the the relative pidl
//  and the root part of that pidl.
//  PARAMETERS:
//  pidlFQ   - Pointer to the fully qualified ITEMIDLIST that needs to be parsed.
//  pidlRoot - Points to the pidl that will contain the root after parsing.
//  pidlItem - Points to the item relative to pidlRoot after parsing.
var pidlTemp, pidlNext: PItemIDList;
begin
     if pidlFQ=nil then
     begin
          ppidlRoot:=nil;
          ppidlItem:=nil;
          exit;
     end;
     ppidlItem:=nil;
     ppidlRoot:=PIDL_Copy(pidlFQ);
     pidlTemp:=ppidlRoot;
     while pidlTemp^.mkid.cb>0 do
     begin
          pidlNext:=PIDL_GetNextItem(pidlTemp);
          if pidlNext^.mkid.cb=0 then
          begin
               ppidlItem:=PIDL_Copy(pidlTemp);
               pidlTemp^.mkid.cb:=0;
               pidlTemp^.mkid.abID[0]:=0;
          end;
          pidlTemp:=pidlNext;
     end;
end;



function PIDL_GetFromPath(pszFile: PChar): PItemIDList;
//  PURPOSE:    This routine takes a full path to a file and converts that
//  to a fully qualified ITEMIDLIST.
//  PARAMETERS:
//      pszFile  - Full path to the file.
//  RETURN VALUE:
//      Returns a fully qualified ITEMIDLIST, or NULL if an error occurs.
var piDesktop: IShellFolder;
    ulEaten, ulAttribs:ULong;
begin
     Result:=nil;
     if Failed(SHGetDesktopFolder(piDesktop)) then exit;
     piDesktop._AddRef;
     if Failed(piDesktop.ParseDisplayName(0, nil, pszFile, ulEaten,
        Result, ulAttribs)) then Result:=nil;
     // piDesktop._Release; -> automaticly done by D4
end;

function PIDL_GetFileFolder(pidl: PItemIDList; var piFolder: IShellFolder):boolean;
//  PURPOSE:    This routine takes a fully qualified pidl for a folder and returns
//  the IShellFolder pointer for that pidl
//  PARAMETERS:
//  pidl     - Pointer to a fully qualified ITEMIDLIST for the folder
//      piParentFolder - Pointer to the IShellFolder of the folder (Return value).
//  RETURN VALUE:
//      Returns TRUE if successful, FALSE otherwise.
var piDesktopFolder: IShellFolder;
begin
     Result:=false;
     if Failed(SHGetDesktopFolder(piDesktopFolder)) then exit;
     if assigned(PiFolder)=false then
        if Failed(SHGetDesktopFolder(PiFolder)) then exit;
     if Failed(piDesktopFolder.BindToObject(pidl, nil, IID_IShellFolder,
        pointer(PiFolder)))=false then Result:=true;
     //piDesktopFolder._Release; -> automaticly done by D4
end;

function PIDL_GetFromParentFolder(pParentFolder: IShellFolder; pszFile: PChar): PItemIDList;
//  PURPOSE:    This routine takes a Shell folder for the parent and the FileName in the folder
//  and converts that to a relative ITEMIDLIST.
//  PARAMETERS:
//      pParentFolder - Pointer to the IShellFolder for the folder containing the
//                  fileName.
//      pszFile       - file name in the folder.
//  RETURN VALUE:
//      Returns a relative ITEMIDLIST, or NULL if an error occurs.
var chEaten, dwAttributes: ULONG;
    NotResult: Boolean;
    ErrorMode: Word;
begin
     ErrorMode := SetErrorMode(SEM_FAILCRITICALERRORS or SEM_NOOPENFILEERRORBOX);
     try
       NotResult := Failed(pParentFolder.ParseDisplayName(0, nil, pszFile, chEaten, Result,
          dwAttributes));
     finally
       SetErrorMode(ErrorMode);
     end;
     if NotResult then Result:=nil;
end;

procedure PIDL_Free(PIDL:PItemIDList);
begin
     if PIDL<>nil then
        ShellMalloc.Free(PIDL);
end;

function PIDL_Equal(PIDL1,PIDL2:PItemIDList):boolean;
var i,size:integer;
    p1,p2:pchar;
begin
     Result:=false;
     if (PIDL1=nil) or (PIDL2=nil) then exit;
     size:=PIDL_GetSize(PIDL1);
     if size<>PIDL_GetSize(PIDL2) then exit;
     i:=0;
     p1:=PChar(PIDL1);
     p2:=PChar(PIDL2);
     while i<size do
           if p1[i]<>p2[i] then exit else inc(i);
     Result:=true;
end;

initialization

  SHGetMalloc(ShellMalloc);
  CF_FILECONTENTS:=RegisterClipboardFormat('FileContents');
  CF_FILEDESCRIPTOR:=RegisterClipboardFormat('FileGroupDescriptor');
  CF_FILENAME:=RegisterClipboardFormat('FileName');
  CF_FILENAMEMAP:=RegisterClipboardFormat('FileNameMap');
  CF_FILENAMEMAPW:=RegisterClipboardFormat('FileNameMapW');
  CF_INDRAGLOOP:=RegisterClipboardFormat('InShellDragLoop');
  CF_NETRESOURCES:=RegisterClipboardFormat('Net Resource');
  CF_PASTESUCCEEDED:=RegisterClipboardFormat('Paste Succeeded');
  CF_PERFORMEDDROPEFFECT:=RegisterClipboardFormat('Performed DropEffect');
  CF_PREFERREDDROPEFFECT:=RegisterClipboardFormat('Preferred DropEffect');
  CF_PRINTERGROUP:=RegisterClipboardFormat('PrinterFriendlyName');
  CF_SHELLIDLIST:=RegisterClipboardFormat('Shell IDList Array');
  CF_SHELLIDLISTOFFSET:=RegisterClipboardFormat('Shell Object Offsets');
  CF_SHELLURL:=RegisterClipboardFormat('UniformResourceLocator');

finalization

  // ShellMalloc._Release; -> automaticly done by D4

end.
