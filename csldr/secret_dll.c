#include "pch.h"

/* support for loading encrypted client dlls */

#if defined(_WIN32)

#include <windows.h>

#define SECRET_MAGIC 0x12345678
#define SECRET_KEY 0x57

typedef BOOL (WINAPI *dllmain_t)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);
typedef void(__cdecl *func_t)(void *pv);

typedef struct
{
	struct
	{
		byte gap[64];
		uint32 magic;
	} mainHeader;

	uint32 unknown;
	uint32 numSections;
	uint32 func;
	uint32 imageBase;
	uint32 dllMain;
	uint32 importDirectory;
} secrethdr_t;

typedef struct
{
	uint32 virtualAddress;
	uint32 virtualSize;
	uint32 fileSize;
	uint32 fileAddress;
	uint32 unknown;
} secretsec_t;

static bool Secret_LoadFromBuffer(byte *buf, void *pv, size_t len)
{
	size_t i;
	byte key;
	secrethdr_t *hdr;
	secretsec_t *sec;
	IMAGE_IMPORT_DESCRIPTOR *iid;
	HMODULE module;
	IMAGE_THUNK_DATA *thunk, *ogThunk;
	DWORD proc;
	IMAGE_IMPORT_BY_NAME *byName;
	dllmain_t dllMain;
	func_t func;

	key = SECRET_KEY;

	hdr = (secrethdr_t *)buf;

	for (i = sizeof(((secrethdr_t*)0)->mainHeader); i < len; i++)
	{
		buf[i] ^= key;
		key += buf[i] + SECRET_KEY;
	}

	hdr->func ^= 0x7A32BC85;
	hdr->imageBase ^= 0x49C042D1;
	hdr->dllMain -= 0xC;
	hdr->importDirectory ^= 0x872C3D47;

	sec = (secretsec_t *)(buf + sizeof(secrethdr_t));

	for (i = 0; i <= hdr->numSections; i++, sec++)
	{
		if (sec->virtualSize > sec->fileSize)
			memset((void *)(sec->virtualAddress + sec->fileSize), 0, sec->virtualSize - sec->fileSize);

		memcpy((void *)sec->virtualAddress, buf + sec->fileAddress, sec->fileSize);
	}

	iid = (IMAGE_IMPORT_DESCRIPTOR *)hdr->importDirectory;

	while (iid->Characteristics)
	{
		ogThunk = (PIMAGE_THUNK_DATA)(hdr->imageBase + iid->OriginalFirstThunk);
		thunk = (PIMAGE_THUNK_DATA)(hdr->imageBase + iid->FirstThunk);

		module = LoadLibraryA((LPCSTR)hdr->imageBase + iid->Name);

		if (!module)
			return FALSE;

		while (ogThunk->u1.AddressOfData)
		{
			if (ogThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				proc = (DWORD)GetProcAddress(module, (LPCSTR)(ogThunk->u1.Ordinal & 0xFFFF));

				if (!proc)
					return FALSE;

				*(DWORD *)(&thunk->u1.Function) = proc;
			}
			else
			{
				byName = (IMAGE_IMPORT_BY_NAME *)(hdr->imageBase + ogThunk->u1.AddressOfData);

				proc = (DWORD)GetProcAddress(module, (LPCSTR)byName->Name);

				if (!proc)
					return FALSE;

				*(DWORD *)(&thunk->u1.Function) = proc;
			}

			ogThunk++;
			thunk++;
		}

		iid++;
	}

	dllMain = (dllmain_t)hdr->dllMain;
	func = (func_t)hdr->func;

	dllMain(NULL, DLL_PROCESS_ATTACH, NULL);
	func(pv);

	return true;
}

static bool Secret_LoadLibrary(const char *fileName, void *pv)
{
	HANDLE file;
	DWORD len;
	byte *buf;
	bool result;
	DWORD numRead;
	uint32 magic;

	result = false;

	file = CreateFileA(fileName, 
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(file, 64, NULL, FILE_BEGIN);
		ReadFile(file, &magic, sizeof(magic), &numRead, NULL);

		if (magic == SECRET_MAGIC)
		{
			/* dumb */
			SetFilePointer(file, 0, NULL, FILE_BEGIN);

			len = GetFileSize(file, NULL);

			if (len > sizeof(secrethdr_t))
			{
				buf = (byte *)HeapAlloc(GetProcessHeap(), 0, len);
				ReadFile(file, buf, len, &numRead, NULL);
				result = Secret_LoadFromBuffer(buf, pv, len);
				HeapFree(GetProcessHeap(), 0, buf);
			}
		}

		CloseHandle(file);
	}

	return result;
}

/*
-------------------------------------------------
 client specific stuff
-------------------------------------------------
*/

typedef void (*dummyfunc_t)(void);

static void Dummy(void)
{
}

dummyfunc_t clDstAddrs[42] =
{
	Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy,
	Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy,
	Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy,
	Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy,
	Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy,
	Dummy, Dummy
};

dummyfunc_t modFuncs[29] =
{
	Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy,
	Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy,
	Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy, Dummy,
	Dummy, Dummy, Dummy, Dummy, Dummy
};


bool Secret_LoadClient(const char *fileName)
{
	void *pmodFuncs;
	void *pclDstAddrs;

	pmodFuncs = (void *)modFuncs;
	pclDstAddrs = (void *)clDstAddrs;

	cl_funcs.pInitFunc = *((int(**)(cl_enginefunc_t *, int))(&pmodFuncs));
	cl_funcs.pHudVidInitFunc = *((int(**)(void))(&pclDstAddrs));
	return Secret_LoadLibrary(fileName, &cl_funcs);
}

#elif defined(__linux__)

bool Secret_LoadClient(const char *fileName)
{
	(void)fileName;
	return false;
}

#endif
