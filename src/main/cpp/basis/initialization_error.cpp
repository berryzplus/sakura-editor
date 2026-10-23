/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "basis/initialization_error.hpp"

namespace basis {

initialization_error::initialization_error(
	std::string_view what,
	std::wstring_view message,
	std::optional<int> optExitCode
)
	: Base{ std::data(what) }
	, m_Message{ message }
	, m_ExitCode{ optExitCode.has_value() ? std::make_optional(EAppExitCode(*optExitCode)) : std::nullopt }
{
}

}	// namespace basis
