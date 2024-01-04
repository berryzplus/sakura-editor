/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
 */
#include "StdAfx.h"
#include "apiwrap/CClipboardApi.hpp"

namespace apiwrap {

//static
UINT CClipboardApi::GetClipboardFormatW(std::wstring_view name, const User32Dll& _User32Dll)
{
	if (name.empty())
	{
		return CF_INVALID;
	}

#pragma push_macro("HandlePreDefinedClipFormat")
#define HandlePreDefinedClipFormat(format) if (0 == ::_wcsnicmp(name.data(), L ## #format, _countof(L ## #format))) return format
	HandlePreDefinedClipFormat(CF_TEXT);
	HandlePreDefinedClipFormat(CF_BITMAP);
	HandlePreDefinedClipFormat(CF_METAFILEPICT);
	HandlePreDefinedClipFormat(CF_SYLK);
	HandlePreDefinedClipFormat(CF_DIF);
	HandlePreDefinedClipFormat(CF_TIFF);
	HandlePreDefinedClipFormat(CF_OEMTEXT);
	HandlePreDefinedClipFormat(CF_DIB);
	HandlePreDefinedClipFormat(CF_PALETTE);
	HandlePreDefinedClipFormat(CF_PENDATA);
	HandlePreDefinedClipFormat(CF_RIFF);
	HandlePreDefinedClipFormat(CF_WAVE);
	HandlePreDefinedClipFormat(CF_UNICODETEXT);
	HandlePreDefinedClipFormat(CF_ENHMETAFILE);
	HandlePreDefinedClipFormat(CF_HDROP);
	HandlePreDefinedClipFormat(CF_LOCALE);
	HandlePreDefinedClipFormat(CF_DIBV5);
#pragma pop_macro("HandlePreDefinedClipFormat")

	return _User32Dll.RegisterClipboardFormatW(name.data());
}

CClipboardApi::CClipboardApi(const User32Dll& User32Dll_, const Kernel32Dll& Kernel32Dll_, const Shell32Dll& Shell32Dll_) noexcept
	: User32DllClient(User32Dll_)
	, Kernel32DllClient(Kernel32Dll_)
	, Shell32DllClient(Shell32Dll_)
{
}

bool CClipboardApi::OpenClipboard(_In_opt_ HWND hWnd, int retryCount) const
{
	for (int i = 0; i <= retryCount; ++i)
	{
		if (GetUser32Dll().OpenClipboard(hWnd))
		{
			return true;
		}

		GetKernel32Dll().Sleep(0);
	}

	return false;
}

} //end of namespace apiwrap
