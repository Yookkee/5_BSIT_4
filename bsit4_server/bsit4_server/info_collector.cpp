#pragma warning(disable : 4996)

#include <windows.h>
#include <time.h>
#include "info_collector.h"
#include <iostream>
#include <Aclapi.h>
#include <Sddl.h>

std::string do_get_os_version()
{
	OSVERSIONINFO osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

	int ver = osvi.dwMajorVersion * 10 + osvi.dwMinorVersion;
	//if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0))
	switch (ver)
	{
	case 50:
		return "Windows 2000";
	case 51:
		return "Windows XP";
	case 52:
		return "Windows XP 64-Bit Edition";
	case 60:
		return "Windows Vista";
	case 61:
		return "Windows 7";
	case 62:
		return "Windows 8 or higher";
	case 63:
		return "Windows 8.1";
	case 100:
		return "Windows 10";
	default:
		return "Unknown";
	}

}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
std::string do_get_current_time() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	return buf;
}

std::string do_get_ticks()
{
	int msec = GetTickCount();
	int sec = ((msec + 500) / 1000);
	int minutes = (int)(sec / 60);
	sec %= 60;
	int hours = (int)(minutes / 60);
	minutes %= 60;

	std::string msg = std::to_string(hours);
	msg += minutes < 10 ? ":0" : ":";
	msg += std::to_string(minutes);
	msg += sec < 10 ? ":0" : ":";
	msg += std::to_string(sec);

	return msg;

	return std::to_string(GetTickCount());
}

std::string do_get_memory_info()
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx(&statex);
	std::string msg = "";

	msg += "Percents of memory in use : ";
	msg += std::to_string(statex.dwMemoryLoad);
	msg += "\n";

	msg += "Total MB of physical memory : ";
	msg += std::to_string(statex.ullTotalPhys / 1024 / 1024);
	msg += "\n";

	msg += "Free MB of physical memory : ";
	msg += std::to_string(statex.ullAvailPhys / 1024 / 1024);
	msg += "\n";

	msg += "Total MB of paging file : ";
	msg += std::to_string(statex.ullTotalPageFile >> 20);
	msg += "\n";

	msg += "Free MB of paging file : ";
	msg += std::to_string(statex.ullAvailPageFile >> 20);
	msg += "\n";

	msg += "Free MB of virtual file : ";
	msg += std::to_string(statex.ullTotalVirtual >> 20);
	msg += "\n";

	msg += "Free MB of virtual file : ";
	msg += std::to_string(statex.ullAvailVirtual >> 20);

	return msg;
}

std::string do_get_disks_info()
{
	std::string msg = "";

	int n;
	char dd[4];
	DWORD dr = GetLogicalDrives();

	for (int i = 0; i < 26; i++)
	{
		n = ((dr >> i) & 0x00000001);
		if (n == 1)
		{
			dd[0] = char(65 + i); dd[1] = ':'; dd[2] = '\\'; dd[3] = 0;

			long long int FreeBytesAvailable = 0;
			long long int TotalNumberOfBytes = 0;
			long long int TotalNumberOfFreeBytes = 0;

			BOOL status = GetDiskFreeSpaceExA(
				dd, // directory name
				NULL, // bytes available to caller
				(PULARGE_INTEGER)&TotalNumberOfBytes, // bytes on disk
				(PULARGE_INTEGER)&TotalNumberOfFreeBytes // free bytes on disk
				);

			if (status)
			{
				msg += dd[0];
				msg += dd[1];
				msg += " Free/Total : ";
				msg += std::to_string(TotalNumberOfFreeBytes >> 20);
				msg += "/";
				msg += std::to_string(TotalNumberOfBytes >> 20);

				status = GetDriveTypeA(dd);
				
				switch (status)
				{
				case 0:
					msg += " (DRIVE_UNKNOWN)";
					break;
				case 1:
					msg += " (DRIVE_NO_ROOT_DIR)";
					break;
				case 2:
					msg += " (DRIVE_REMOVABLE)";
					break;
				case 3:
					msg += " (DRIVE_FIXED)";
					break;
				case 4:
					msg += " (DRIVE_REMOTE)";
					break;
				case 5:
					msg += " (DRIVE_CDROM)";
					break;
				case 6:
					msg += " (DRIVE_RAMDISK)";
					break;
				}

				msg += '\n';
			}
		}
	}
	msg.pop_back();
	return msg;
}

//**************************************************************
std::string do_get_owner_file(const char * str)
{
	DWORD dwRtnCode = 0;
	PSID pSidOwner = NULL;
	BOOL bRtnBool = TRUE;
	char * AcctName = NULL;
	char * DomainName = NULL;
	DWORD dwAcctName = 1, dwDomainName = 1;
	SID_NAME_USE eUse = SidTypeUnknown;
	HANDLE hFile;
	PSECURITY_DESCRIPTOR pSD = NULL;

	//wchar_t * _file_name = file_path;
	// Get the handle of the file object.
	hFile = CreateFileA(
		str,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	// Check GetLastError for CreateFile error code.
	if (hFile == INVALID_HANDLE_VALUE) {
		DWORD dwErrorCode = 0;
		CloseHandle(hFile);
		std::cout << "do_get_owner_file : CreateFileA : " << GetLastError();
		return "Unknown";
	}


	// Get the owner SID of the file.
	dwRtnCode = GetSecurityInfo(
		hFile,
		SE_FILE_OBJECT,
		OWNER_SECURITY_INFORMATION,
		&pSidOwner,
		NULL,
		NULL,
		NULL,
		&pSD);
	CloseHandle(hFile);

	// Check GetLastError for GetSecurityInfo error condition.
	if (dwRtnCode != ERROR_SUCCESS) {
		CloseHandle(hFile);
		std::cout << "do_get_owner_file : GetSecurityInfo : " << GetLastError();
		return "Unknown";
	}

	// First call to LookupAccountSid to get the buffer sizes.
	bRtnBool = LookupAccountSidA(
		NULL,           // local computer
		pSidOwner,
		AcctName,
		(LPDWORD)&dwAcctName,
		DomainName,
		(LPDWORD)&dwDomainName,
		&eUse);

	// Reallocate memory for the buffers.
	AcctName = (char *)GlobalAlloc(
		GMEM_FIXED,
		dwAcctName);

	// Check GetLastError for GlobalAlloc error condition.
	if (AcctName == NULL) {
		std::cout << "do_get_owner_file : GetSecurityInfo : " << GetLastError();
		return "Unknown";
	}

	DomainName = (char *)GlobalAlloc(
		GMEM_FIXED,
		dwDomainName);

	// Check GetLastError for GlobalAlloc error condition.
	if (DomainName == NULL) {
		std::cout << "do_get_owner_file : GetSecurityInfo : " << GetLastError();
		return "Unknown";

	}

	// Second call to LookupAccountSid to get the account name.
	bRtnBool = LookupAccountSidA(
		NULL,                   // name of local or remote computer
		pSidOwner,              // security identifier
		AcctName,               // account name buffer
		(LPDWORD)&dwAcctName,   // size of account name buffer 
		DomainName,             // domain name
		(LPDWORD)&dwDomainName, // size of domain name buffer
		&eUse);                 // SID type

	// Check GetLastError for LookupAccountSid error condition.
	if (bRtnBool == FALSE) {
		std::cout << "do_get_owner_file : LookupAccountSid : " << GetLastError();
		return "Unknown";

	}

	std::string res = AcctName;
	char *strsidd;
	if (ConvertSidToStringSidA(pSidOwner, &strsidd))
	{
		res += " : ";
		res += strsidd;
	}
	return res;
}

std::string do_get_owner_registry(const char * str)
{
	int status, bRtnBool;

	PSID pSidOwner = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	status = GetNamedSecurityInfoA(str, SE_REGISTRY_KEY, OWNER_SECURITY_INFORMATION, &pSidOwner, NULL, NULL, NULL, &pSD);

	if (status != ERROR_SUCCESS)
	{
		std::cout << "do_get_owner_registry : GetNamedSecurityInfoA : " << GetLastError() << std::endl;
		return "Unknown";
	}

	char * AcctName = NULL;
	char * DomainName = NULL;
	SID_NAME_USE eUse = SidTypeUnknown;

	DWORD dwAcctName = 1, dwDomainName = 1;
	// First call to LookupAccountSid to get the buffer sizes.
	bRtnBool = LookupAccountSidA(
		NULL,           // local computer
		pSidOwner,
		AcctName,
		(LPDWORD)&dwAcctName,
		DomainName,
		(LPDWORD)&dwDomainName,
		&eUse);

	// Reallocate memory for the buffers.
	AcctName = (char *)GlobalAlloc(
		GMEM_FIXED,
		dwAcctName);

	// Check GetLastError for GlobalAlloc error condition.
	if (AcctName == NULL) {
		std::cout << "do_get_owner_registry : GlobalAlloc : " << GetLastError();
		return "Unknown";
	}

	DomainName = (char *)GlobalAlloc(
		GMEM_FIXED,
		dwDomainName);

	// Check GetLastError for GlobalAlloc error condition.
	if (DomainName == NULL) {
		std::cout << "do_get_owner_registry : GlobalAlloc : " << GetLastError();
		return "Unknown";

	}

	// Second call to LookupAccountSid to get the account name.
	bRtnBool = LookupAccountSidA(
		NULL,                   // name of local or remote computer
		pSidOwner,              // security identifier
		AcctName,               // account name buffer
		(LPDWORD)&dwAcctName,   // size of account name buffer 
		DomainName,             // domain name
		(LPDWORD)&dwDomainName, // size of domain name buffer
		&eUse);                 // SID type

	// Check GetLastError for LookupAccountSid error condition.
	if (bRtnBool == FALSE) {
		std::cout << "do_get_owner_registry : LookupAccountSid : " << GetLastError();
		return "Unknown";

	}

	std::string res = AcctName;
	char *strsidd;
	if (ConvertSidToStringSidA(pSidOwner, &strsidd))
	{
		res += " : ";
		res += strsidd;
	}
	return res;
}

std::string do_get_owner(const char * str)
{
	if (str[1] == ':' && str[2] == '\\')
		return do_get_owner_file(str);
	else
		return do_get_owner_registry(str);
}

//**************************************************************
std::string do_get_acl_sup(const char * str, SE_OBJECT_TYPE type)
{
	std::string msg = "";

	PACL curDacl;
	ACCESS_ALLOWED_ACE *pAce = NULL;
	PSECURITY_DESCRIPTOR pSecDescriptor;

	/*Ïîëó÷àåì óêàçàòåëü íà DACL è äåñêðèïòîð áåçîïàñíîñòè*/
	DWORD result = GetNamedSecurityInfoA(str, type, DACL_SECURITY_INFORMATION, NULL, NULL, &curDacl, NULL, &pSecDescriptor);
	if (result != ERROR_SUCCESS){
		std::cout << "do_get_acl_file : GetNamedSecurityInfoA : " << GetLastError();
		return "Unknown";
	}

	/*Âûâîäèì ïðàâà äîñòóïà*/
	for (int i = 0; i < curDacl->AceCount; i++) {
		if (GetAce(curDacl, i, (LPVOID *)&pAce) == 0)
		{
			std::cout << "do_get_acl_file : GetAce : " << GetLastError();
			return "Unknown";
		}
		if (msg.length() > 0) msg += '\n';
		//std::cout << pAce->Mask << std::endl;

		PSID sid = &pAce->SidStart;
		SID_NAME_USE typeAccount;
		DWORD sizeNameUser = 0, sizeNameDomain = 0;

		LookupAccountSidA(NULL, sid, 0, &sizeNameUser, 0, &sizeNameDomain, &typeAccount);
		char * nameDomain = (char *)malloc(sizeNameDomain*sizeof(char));
		char * nameUser = (char *)malloc(sizeNameUser*sizeof(char));

		/*Ïîëó÷àåì èìåíà*/
		if (LookupAccountSidA(NULL, sid, nameUser, &sizeNameUser, nameDomain, &sizeNameDomain, &typeAccount) == 0)
		{
			free(nameDomain);
			free(nameUser);
			std::cout << "do_get_acl_file : LookupAccountSidA : " << GetLastError();
			return "Unknown";
		}

		msg += nameUser;
		char *str;
		if (ConvertSidToStringSidA(sid, &str))
		{
			msg += " : ";
			msg += str;
		}
		msg += '\n';

		msg += ACCESS_ALLOWED_ACE_TYPE == pAce->Header.AceType ? "Allow" : "Deny";
		msg += '\n';

		int dac = WRITE_DAC;
		int res = pAce->Mask & WRITE_DAC;

		msg += (pAce->Mask & WRITE_DAC) ? "1" : "0";
		msg += " - WRITE_DAC\n";
		msg += (pAce->Mask & WRITE_OWNER) ? "1" : "0";
		msg += " - WRITE_OWNER\n";
		msg += (pAce->Mask & DELETE) ? "1" : "0";
		msg += " - DELETE\n";
		msg += (pAce->Mask & FILE_GENERIC_READ) ? "1" : "0";
		msg += " - FILE_GENERIC_READ\n";
		msg += (pAce->Mask & FILE_GENERIC_WRITE) ? "1" : "0";
		msg += " - FILE_GENERIC_WRITE\n";
		msg += (pAce->Mask & FILE_GENERIC_EXECUTE) ? "1" : "0";
		msg += " - FILE_GENERIC_EXECUTE\n";
	}

	msg.pop_back();
	return msg;
}

std::string do_get_acl(const char * str)
{
	SE_OBJECT_TYPE type = SE_REGISTRY_KEY;
	if (str[1] == ':' && str[2] == '\\')
		type = SE_OBJECT_TYPE::SE_FILE_OBJECT;

	return do_get_acl_sup(str, type);
}