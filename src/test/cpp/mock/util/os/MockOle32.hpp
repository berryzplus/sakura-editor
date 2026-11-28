/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/os/Ole32.hpp"

#include <gmock/gmock.h>

struct MockOle32 final : public Ole32
{
	MOCK_CONST_METHOD4(DoDragDrop, HRESULT(
		IN LPDATAOBJECT pDataObj,
		IN LPDROPSOURCE pDropSource,
		IN DWORD dwOKEffects,
		OUT LPDWORD pdwEffect
	));

	MOCK_CONST_METHOD1(OleGetClipboard, HRESULT(
		_Outptr_ LPDATAOBJECT FAR* ppDataObj
	));

	static HRESULT _DoDragDrop(
		IN LPDATAOBJECT pDataObj,
		IN LPDROPSOURCE pDropSource,
		IN DWORD dwOKEffects,
		OUT LPDWORD pdwEffect
	);

	static HRESULT _OleGetClipboard(
		_Outptr_ LPDATAOBJECT FAR* ppDataObj
	);

	MockOle32();
};
