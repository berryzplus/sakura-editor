/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "basis/initialization_error.hpp"

#include <optional>
#include <string_view>

namespace basis {

initialization_error make_initialization_error(
	std::wstring_view message,
	std::optional<int> optExitCode = std::nullopt
);

}	// namespace basis
