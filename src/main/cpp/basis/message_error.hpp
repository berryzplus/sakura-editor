/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

namespace basis {

/*!
 * メッセージエラー
 *
 * ワイド文字列を渡して構築できる実行時例外。
 */
class message_error : public std::runtime_error {
private:
	std::wstring _Message;

public:
	explicit message_error(std::wstring_view message);

	std::wstring_view message() const noexcept { return _Message; }
};

} // namespace basis
