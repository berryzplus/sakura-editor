﻿/*! @file */
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
#include "apiwrap/apiwrap.hpp"

namespace apiwrap {

struct cstring
{
	std::wstring_view _View;
	std::wstring      _Buffer;

	explicit cstring(_In_opt_ LPCWSTR pszValue)
	{
		if (pszValue && *pszValue)
		{
			_View = pszValue;
		}
	}

	explicit cstring(std::wstring_view value)
		: _View(value)
	{
		if (0 < _View.length() && *(_View.data() + _View.length()))
		{
			_Buffer = _View;
			_View   = _Buffer;
		}
	}

	virtual ~cstring() = default;

	explicit virtual operator std::wstring_view() const noexcept
	{
		return _View;
	}
};

LPCWSTR get_psz_or_null(const cstring& str) noexcept
{
	if (const auto text = static_cast<std::wstring_view>(str);
		0 < text.length())
	{
		return text.data();
	}

	return nullptr;
}

bool SetWindowTextW(_In_ HWND hWnd, std::wstring_view text, std::shared_ptr<User32Dll> _User32Dll)
{
	return _User32Dll->SetWindowTextW(hWnd, get_psz_or_null(cstring(text)));
}

bool SetWindowTextW(_In_ HWND hWnd, _In_opt_ LPCWSTR psz, std::shared_ptr<User32Dll> _User32Dll)
{
	return SetWindowTextW(hWnd, static_cast<std::wstring_view>(cstring(psz)), std::move(_User32Dll));
}

std::wstring GetWindowTextW(_In_ HWND hWnd, std::wstring&& buffer, std::shared_ptr<User32Dll> _User32Dll)
{
	// バッファをクリアしておく
	buffer.clear();

	// バッファが小さかったら拡張する
	if (const auto cchRequired = _User32Dll->GetWindowTextLengthW(hWnd);
		static_cast<int>(buffer.capacity()) < cchRequired)
	{
		buffer.resize(cchRequired);
	}

	// 書き込まれたデータが小さければ縮小する
	if (const auto actualCopied = _User32Dll->GetWindowTextW(hWnd, buffer.data(), static_cast<int>(buffer.capacity()));
		actualCopied < static_cast<int>(buffer.length()))
	{
		buffer.resize(actualCopied);
	}

	return std::move(buffer);
}

std::wstring GetWindowTextW(_In_ HWND hWnd, std::shared_ptr<User32Dll> _User32Dll)
{
	return GetWindowTextW(hWnd, std::wstring(), std::move(_User32Dll));
}

bool SetDlgItemTextW(_In_ HWND hDlg, int nIDDlgItem, std::wstring_view text, std::shared_ptr<User32Dll> _User32Dll)
{
	bool ret = false;
	if (const auto hWnd = _User32Dll->GetDlgItem(hDlg, nIDDlgItem))
	{
		ret = SetWindowTextW(hWnd, text, std::move(_User32Dll));
	}

	return ret;
}

bool SetDlgItemTextW(_In_ HWND hDlg, int nIDDlgItem, _In_opt_ LPCWSTR psz, std::shared_ptr<User32Dll> _User32Dll)
{
	bool ret = false;
	if (const auto hWnd = _User32Dll->GetDlgItem(hDlg, nIDDlgItem))
	{
		ret = SetWindowTextW(hWnd, psz, std::move(_User32Dll));
	}

	return ret;
}

std::wstring GetDlgItemTextW(_In_ HWND hDlg, int nIDDlgItem, std::wstring&& buffer, std::shared_ptr<User32Dll> _User32Dll)
{
	const auto hWnd = _User32Dll->GetDlgItem(hDlg, nIDDlgItem);
	if (!hWnd)
	{
		buffer.clear();
		return std::move(buffer);
	}

	return GetWindowTextW(hWnd, std::move(buffer), std::move(_User32Dll));
}

std::wstring GetDlgItemTextW(_In_ HWND hDlg, int nIDDlgItem, std::shared_ptr<User32Dll> _User32Dll)
{
	return GetDlgItemTextW(hDlg, nIDDlgItem, std::wstring(), std::move(_User32Dll));
}

void SendEmLimitTextW(_In_ HWND hEdit, size_t cchLimit, std::shared_ptr<User32Dll> _User32Dll)
{
	_User32Dll->SendMessageW(hEdit, EM_LIMITTEXT, static_cast<int>(cchLimit), 0L);
}

void SendEmLimitTextW(_In_ HWND hDlg, int nIDDlgItem, size_t cchLimit, std::shared_ptr<User32Dll> _User32Dll)
{
	if (const auto hWnd = _User32Dll->GetDlgItem(hDlg, nIDDlgItem))
	{
		SendEmLimitTextW(hWnd, cchLimit, std::move(_User32Dll));
	}
}

} //end of namespace apiwrap