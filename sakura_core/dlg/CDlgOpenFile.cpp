/*!	@file
	@brief ファイルオープンダイアログボックス

	@author Norio Nakatani
	@date	1998/08/10 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, Stonee, genta
	Copyright (C) 2002, MIK, YAZAKI, genta
	Copyright (C) 2003, MIK, KEITA, Moca, ryoji
	Copyright (C) 2004, genta
	Copyright (C) 2005, novice, ryoji
	Copyright (C) 2006, ryoji, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "dlg/CDlgOpenFile.h"

#include "env/DLLSHAREDATA.h"

extern std::shared_ptr<IDlgOpenFile> New_CDlgOpenFile_CommonFileDialog();
extern std::shared_ptr<IDlgOpenFile> New_CDlgOpenFile_CommonItemDialog();

CDlgOpenFile::CDlgOpenFile() = default;

void CDlgOpenFile::Create(
	HINSTANCE					hInstance,
	HWND						hwndParent,
	const WCHAR*				pszUserWildCard,
	const WCHAR*				pszDefaultPath,
	const std::vector<LPCWSTR>& vMRU,
	const std::vector<LPCWSTR>& vOPENFOLDER
) {
	if( GetDllShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog ){
		m_pImpl = New_CDlgOpenFile_CommonItemDialog();
	}
	else {
		m_pImpl = New_CDlgOpenFile_CommonFileDialog();
	}
	m_pImpl->Create(hInstance, hwndParent, pszUserWildCard, pszDefaultPath, vMRU, vOPENFOLDER);
}

inline bool CDlgOpenFile::DoModal_GetOpenFileName(
	WCHAR* pszPath,
	EFilter eAddFileter)
{
	return m_pImpl->DoModal_GetOpenFileName(pszPath, eAddFileter);
}

inline bool CDlgOpenFile::DoModal_GetSaveFileName( WCHAR* pszPath )
{
	return m_pImpl->DoModal_GetSaveFileName(pszPath);
}

inline bool CDlgOpenFile::DoModalOpenDlg(
	SLoadInfo* pLoadInfo,
	std::vector<std::wstring>* pFilenames,
	bool bOptions)
{
	return m_pImpl->DoModalOpenDlg(pLoadInfo, pFilenames, bOptions);
}

inline bool CDlgOpenFile::DoModalSaveDlg(
	SSaveInfo* pSaveInfo,
	bool bSimpleMode)
{
	return m_pImpl->DoModalSaveDlg(pSaveInfo, bSimpleMode);
}


/*! ファイル選択
	@note 実行ファイルのパスor設定ファイルのパスが含まれる場合は相対パスに変換
 */
/* static */ BOOL CDlgOpenFile::SelectFile(
	HWND parent,
	HWND hwndCtl,
	const WCHAR* filter,
	bool resolvePath,
	EFilter eAddFilter)
{
	SFilePath szFilePath;
	GetWindowTextW(hwndCtl, szFilePath, int(std::size(szFilePath)) - 1);

	SFilePath szPath;
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	if( resolvePath && _IS_REL_PATH( szFilePath ) ){
		GetInidirOrExedir(szPath, szFilePath);
	}else{
		szPath = szFilePath;
	}

	/* ファイルオープンダイアログの初期化 */
	CDlgOpenFile cDlgOpenFile;
	cDlgOpenFile.Create(
		GetModuleHandleW(nullptr),
		parent,
		filter,
		szPath
	);

	const auto ret = cDlgOpenFile.DoModal_GetOpenFileName(szPath, eAddFilter);
	if (ret) {
		const WCHAR* fileName;
		if( resolvePath ){
			fileName = GetRelPath( szPath );
		}else{
			fileName = szPath;
		}
		SetWindowTextW(hwndCtl, fileName);
	}
	return ret;
}
