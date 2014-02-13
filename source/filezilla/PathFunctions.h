// FileZilla - a Windows ftp client

// Copyright (C) 2002-2004 - Tim Kosse <tim.kosse@gmx.de>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

//Replacement of the Path functions defined in shlwapi.h / shlwapi.dll
//shlwapi requires IE4, FileZilla should not!

#pragma once

CString PathAppend(CString path, LPCTSTR sub);
void PathRemoveArgs(CString &path);
void PathUnquoteSpaces(CString &path);
CString PathFindExtension(CString path);
void PathRemoveFileSpec(CString &path);