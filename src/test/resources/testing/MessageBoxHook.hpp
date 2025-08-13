/*! @file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

namespace testing {

/*!
 * メッセージボックスのAPIフック。
 */
class MessageBoxHook {
private:
	using PFN_TARGET = decltype(&MessageBoxExW);

	// 排他ロックに使うミューテックス
	static inline std::mutex gm_mtx{};

	static inline std::vector<int> gm_returnValues{};

	// テストで検証したい値を記録する構造体
	struct SLog {
		HWND hWnd = nullptr;
		std::wstring text;
		std::wstring caption;
		UINT uType = 0;
	};

	static inline std::vector<SLog> gm_logs;

	static int WINAPI MessageBoxExWStub(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId);

	public:
	explicit MessageBoxHook(int returnValue = IDOK);
	~MessageBoxHook() noexcept;

	const auto& back() const { return gm_logs.back(); };
	void push(int returnValue = IDOK);
};

} // namespace testing

#define EXPECT_MSGBOX2(_action, _text, _caption) \
	::testing::MessageBoxHook hook; \
	_action; \
	EXPECT_THAT(hook.back().caption, StrEq(_caption)); \
	EXPECT_THAT(hook.back().text, StrEq(_text))

#define EXPECT_MSGBOX(_action, _text) \
	EXPECT_MSGBOX2(_action, _text, GSTR_APPNAME)

