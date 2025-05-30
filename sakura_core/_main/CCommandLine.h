/*!	@file
	@brief コマンドラインパーサ ヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CControlTrayより分離
	Copyright (C) 2002, genta
	Copyright (C) 2005, D.S.Koba
	Copyright (C) 2007, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CCOMMANDLINE_DF7E2E03_76E1_458C_82AC_7C485EECF677_H_
#define SAKURA_CCOMMANDLINE_DF7E2E03_76E1_458C_82AC_7C485EECF677_H_
#pragma once

#include <vector>

#include "global.h"
#include "charset/charcode.h"
#include "EditInfo.h"
#include "GrepInfo.h"
#include "util/design_template.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

/*!
 * @brief コマンドラインパーサ クラス
 */
class CCommandLine : public TInstanceHolder<CCommandLine> {
private:
	using Me = CCommandLine;

	using OptionStr = std::optional<std::wstring>;

public:
	CCommandLine() = default;
	CCommandLine(Me&& other) noexcept { operator = (std::move(other)); }
	Me& operator = (Me&& other) noexcept;
	~CCommandLine() override = default;

private:
	static int CheckCommandLine(
		LPWSTR	str,		//!< [in] 検証する文字列（先頭の-は含まない）
		WCHAR**	arg,		//!< [out] 引数がある場合はその先頭へのポインタ
		int*	arglen		//!< [out] 引数の長さ
	);

	/*!
		引用符で囲まれている数値を認識するようにする
		@date 2002.12.05 genta
	*/
	static int AtoiOptionInt(const WCHAR* arg){
		return ( arg[0] == L'"' || arg[0] == L'\'' ) ?
			_wtoi( arg + 1 ) : _wtoi( arg );
	}

// member accessor method
public:
	bool IsNoWindow() const noexcept { return m_bNoWindow; }
	bool IsGrepMode() const noexcept { return m_bGrepMode; }
	bool IsGrepDlg() const noexcept { return m_bGrepDlg; }
	bool IsDebugMode() const noexcept { return m_bDebugMode; }
	bool IsViewMode() const noexcept { return m_bViewMode; }
	bool GetEditInfo(EditInfo* fi) const noexcept { *fi = m_fi; return true; }
	const EditInfo& GetEditInfoRef() const noexcept { return m_fi; }
	bool GetGrepInfo(GrepInfo* gi) const noexcept { *gi = m_gi; return true; }
	const GrepInfo& GetGrepInfoRef() const noexcept { return m_gi; }
	int GetGroupId() const noexcept { return m_nGroup; }	// 2007.06.26 ryoji
	LPCWSTR GetMacro() const noexcept { return m_cmMacro.has_value() ? m_cmMacro.value().c_str() : nullptr; }
	LPCWSTR GetMacroType() const noexcept { return m_cmMacroType.has_value() ? m_cmMacroType.value().c_str() : nullptr; }
	LPCWSTR GetProfileName() const noexcept { return m_cmProfile.has_value() ? m_cmProfile.value().c_str() : nullptr; }
	bool    IsSetProfile() const noexcept { return m_cmProfile.has_value(); }
	void    SetProfileName(std::wstring_view s) { m_cmProfile = s; }
	bool IsProfileMgr() const noexcept { return m_bProfileMgr; }

	// 以下の取得メソッドは削除する
	const CLogicPoint& GetCaretLocation() const noexcept { return m_fi.m_ptCursor; }
	CLayoutPoint GetViewLocation() const noexcept { return { m_fi.m_nViewLeftCol,  m_fi.m_nViewTopLine }; }
	tagSIZE GetWindowSize() const noexcept { return { m_fi.m_nWindowSizeX, m_fi.m_nWindowSizeY }; }
	tagPOINT GetWindowOrigin() const noexcept { return { m_fi.m_nWindowOriginX, m_fi.m_nWindowOriginY }; }
	LPCWSTR GetOpenFile() const noexcept { return m_fi.m_szPath; }
	LPCWSTR GetDocType() const noexcept { return m_fi.m_szDocType; }
	ECodeType GetDocCode() const noexcept { return m_fi.m_nCharCode; }

	int GetFileNum(void) const noexcept { return static_cast<int>(m_vFiles.size()); }
	const WCHAR* GetFileName(int i) const noexcept { return i < GetFileNum() ? m_vFiles[i].c_str() : NULL; }
	void ClearFile(void) noexcept { m_vFiles.clear(); }

	void ParseKanjiCodeFromFileName( LPWSTR pszExeFileName, int cchExeFileName );
	void ParseCommandLine( LPCWSTR pszCmdLineSrc, bool bResponse = true );

// member valiables
private:
	bool		m_bGrepMode = false;	//! [out] TRUE: Grep Mode
	bool		m_bGrepDlg = false;		//  Grepダイアログ
	bool		m_bDebugMode = false;
	bool		m_bNoWindow = false;	//! [out] TRUE: 編集Windowを開かない
	bool		m_bProfileMgr = false;
	EditInfo	m_fi{};					//!
	GrepInfo	m_gi{};					//!
	bool		m_bViewMode = false;	//! [out] TRUE: Read Only
	int			m_nGroup = -1;			//! グループID	// 2007.06.26 ryoji
	OptionStr	m_cmMacro;				//! マクロファイル名／マクロ文
	OptionStr	m_cmMacroType;			//! マクロ種別
	OptionStr	m_cmProfile;			//! プロファイル名

	std::vector<std::wstring> m_vFiles;	//!< ファイル名(複数)
};

#endif /* SAKURA_CCOMMANDLINE_DF7E2E03_76E1_458C_82AC_7C485EECF677_H_ */
