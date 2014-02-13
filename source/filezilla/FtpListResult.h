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

// FtpListResult.h: Schnittstelle f�r die Klasse CFtpListResult.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FTPLISTRESULT_H__A175CA67_9A91_4BC0_AAD3_238DE1A6D9AE__INCLUDED_)
#define AFX_FTPLISTRESULT_H__A175CA67_9A91_4BC0_AAD3_238DE1A6D9AE__INCLUDED_

#include "FzApiStructures.h"	// Hinzugef�gt von der Klassenansicht
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*This class parses the directory listing returned from the server. These formats are supported:
-rw-r--r--   1 root     other        531 Jan 29 03:26 README\r\n
dr-xr-xr-x   2 root     other        512 Apr  8  1994 etc\r\n
dr-xr-xr-x   2 root     512 Apr  8  1994 etc\r\n
lrwxrwxrwx   1 root     other          7 Jan 25 00:17 bin -> usr/bin\r\n
----------   1 owner    group         1803128 Jul 10 10:18 ls-lR.Z\r\n
d---------   1 owner    group               0 May  9 19:45 Softlib\r\n
-rwxrwxrwx   1 noone    nogroup      322 Aug 19  1996 message.ftp\r\n
d [R----F--] supervisor            512       Jan 16 18:53    login\r\n
- [R----F--] rhesus             214059       Oct 20 15:27    cx.exe\r\n
-------r--         326  1391972  1392298 Nov 22  1995 MegaPhone.sit\r\n
drwxrwxr-x               folder        2 May 10  1996 network\r\n
00README.TXT;1      2 30-DEC-1996 17:44 [SYSTEM] (RWED,RWED,RE,RE)\r\n
CORE.DIR;1          1  8-SEP-1996 16:09 [SYSTEM] (RWE,RWE,RE,RE)\r\n
CII-MANUAL.TEX;1  213/216  29-JAN-1996 03:33:12  [ANONYMOU,ANONYMOUS]   (RWED,RWED,,)\r\n
04-27-00  09:09PM       <DIR>          licensed\r\n
07-18-00  10:16AM       <DIR>          pub\r\n
04-14-00  03:47PM                  589 readme.htm\r\n

Multiple spaces are ignored (except within filenames), a single LF character at the end is also supported as well as multiple 
CRLF pairs and other variants.
*/

#include "ApiLog.h"

class CFtpListResult : public CApiLog
{
public:
	t_server m_server;
	void SendToMessageLog(HWND hWnd, UINT nMsg);
	void AddData(char *data,int size);
	CFtpListResult(t_server server, bool *bUTF8 = 0);
	virtual ~CFtpListResult();
	t_directory::t_direntry *getList(int &num, CTime EntryTime);

private:
	typedef std::list<t_directory::t_direntry> tEntryList;
	tEntryList m_EntryList;
	
	BOOL parseLine(const char *lineToParse, const int linelen, t_directory::t_direntry &direntry, int &nFTPServerType);

	BOOL parseAsVMS(const char *line, const int linelen, t_directory::t_direntry &direntry);
	BOOL parseAsEPLF(const char *line, const int linelen, t_directory::t_direntry &direntry);
	BOOL parseAsMlsd(const char *line, const int linelen, t_directory::t_direntry &direntry);
	BOOL parseAsUnix(const char *line, const int linelen, t_directory::t_direntry &direntry);
	BOOL parseAsDos(const char *line, const int linelen, t_directory::t_direntry &direntry);
	BOOL parseAsOther(const char *line, const int linelen, t_directory::t_direntry &direntry);
	BOOL parseAsIBM(const char *line, const int linelen, t_directory::t_direntry &direntry);
	BOOL parseAsIBMMVS(const char *line, const int linelen, t_directory::t_direntry &direntry);
	BOOL parseAsIBMMVSPDS(const char *line, const int linelen, t_directory::t_direntry &direntry);
	BOOL parseAsIBMMVSPDS2(const char *line, const int linelen, t_directory::t_direntry &direntry);
	BOOL parseAsWfFtp(const char *line, const int linelen, t_directory::t_direntry &direntry);

	const char *GetNextToken(const char *line, const int linelen, int &len, int &pos, int type) const;
	
	bool ParseShortDate(const char *str, int len, t_directory::t_direntry::t_date &date) const;
	bool parseTime(const char *str, int len, t_directory::t_direntry::t_date &date) const;
	bool ParseSize(const char* str, int len, __int64 &size) const;

	bool parseMlsdDateTime(const CString value, t_directory::t_direntry &direntry) const;

	int pos;
	struct t_list
	{
		char *buffer;
		int len;
		t_list *next;
	} *listhead, *curpos, *m_curlistaddpos;

	typedef std::list<int> tTempData;
	tTempData m_TempData;

	//Month names map
	std::map<CString, int> m_MonthNamesMap;
	
protected:
	bool *m_bUTF8;
	void copyStr(CString &target, int pos, const char *source, int len, bool mayInvalidateUTF8 = false);
	const char * strnchr(const char *str, int len, char c) const;
	const char * strnstr(const char *str, int len, const char *c) const;
	_int64 strntoi64(const char *str, int len) const;
	void AddLine(t_directory::t_direntry &direntry);
	char * GetLine();
	bool IsNumeric(const char *str, int len) const;
	char *m_prevline;
	char *m_curline;
};

#endif // !defined(AFX_FTPLISTRESULT_H__A175CA67_9A91_4BC0_AAD3_238DE1A6D9AE__INCLUDED_)
