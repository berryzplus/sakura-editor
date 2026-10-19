/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "basis/make_initialization_error.hpp"

#include "util/tchar_convert.h"

namespace basis {

initialization_error make_initialization_error(
	std::wstring_view message,
	std::optional<int> optExitCode
)
{
	return initialization_error(
		cxx::to_string(message, CP_UTF8),
		message,
		optExitCode
	);
}

}	// namespace basis
