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
#include <array>
#include <wrl.h>
#include <Shlwapi.h>
#include <ShObjIdl.h>
#include "charset/CCodePage.h"
#include "dlg/CDlgOpenFile.h"
#include "env/CShareData.h"
#include "env/CDocTypeManager.h"
#include "doc/CDocListener.h"
#include "util/shell.h"
#include "util/file.h"
#include "util/os.h"
#include "util/module.h"
#include "CFileExt.h"
#include "env/DLLSHAREDATA.h"
#include "String_define.h"

#include "basis/TComImpl.hpp"
#include "CDataProfile.h"

// COMスマートポインタの定義（追加するときは昇順で。）
DEFINE_COM_SMARTPTR(IFileDialog);
DEFINE_COM_SMARTPTR(IFileDialogCustomize);
DEFINE_COM_SMARTPTR(IFileOpenDialog);
DEFINE_COM_SMARTPTR(IFileSaveDialog);
DEFINE_COM_SMARTPTR(IShellItem);
DEFINE_COM_SMARTPTR(IShellItemArray);

struct CDlgOpenFile_CommonItemDialog final
	: public IDlgOpenFile
	, public TComImpl<IFileDialogControlEvents>
{
	CDlgOpenFile_CommonItemDialog() = default;

	void Create(
		HINSTANCE					hInstance,
		HWND						hwndParent,
		const WCHAR*				pszUserWildCard,
		const WCHAR*				pszDefaultPath,
		const std::vector<LPCWSTR>& vMRU,
		const std::vector<LPCWSTR>& vOPENFOLDER
	) override;

	bool DoModal_GetOpenFileName( WCHAR* pszPath, EFilter eAddFileter ) override;
	bool DoModal_GetSaveFileName( WCHAR* pszPath ) override;

	bool DoModalOpenDlg( SLoadInfo* pLoadInfo,
						 std::vector<std::wstring>* pFileNames,
						 bool bOptions ) override;
	bool DoModalSaveDlg( SSaveInfo*	pSaveInfo,
						 bool bSimpleMode ) override;

	bool DoModalOpenDlgImpl0(
		std::vector<std::wstring>& fileNames,
		std::wstring_view fileName,
		const std::vector<COMDLG_FILTERSPEC>& specs,
		bool bAllowMultiSelect
	);

	void DoModalOpenDlgImpl1(
		_In_ IFileOpenDialog* pFileDialog,
		std::vector<std::wstring>& fileNames,
		std::wstring_view fileName,
		const std::vector<COMDLG_FILTERSPEC>& specs,
		bool bAllowMultiSelect
	);

	bool DoModalSaveDlgImpl0(std::filesystem::path& path);

	void DoModalSaveDlgImpl1(
		_In_ IFileSaveDialog* pFileDialog,
		std::filesystem::path& path
	);

	struct CustomizeSetting {
		bool bCustomize;
		bool bSkipAutoDetect;	// 文字コードコンボボックスのアイテムの自動選択を飛ばす
		bool bShowReadOnly;		// 読み取り専用チェックボックスを表示する
		bool bUseEol;
		bool bUseBom;			// BOMの有無を選択する機能を利用するかどうか
		bool bUseCharCode;
	};

	HWND					m_hwndParent = nullptr;			/* オーナーウィンドウのハンドル */
	std::wstring			m_strDefaultWildCard{ L"*.*" };	/* 「開く」での最初のワイルドカード（保存時の拡張子補完でも使用される） */
	SFilePath				m_szInitialDir;					/* 「開く」での初期ディレクトリ */
	std::vector<LPCWSTR>	m_vMRU;
	std::vector<LPCWSTR>	m_vOPENFOLDER;

	CustomizeSetting m_customizeSetting{};

	bool			m_bViewMode = false;		//!< ビューモードか
	ECodeType		m_nCharCode = CODE_UTF8;	//!< 文字コード
	CEol			m_cEol;
	bool			m_bBom      = true;			//!< BOMを付けるかどうか

	int AddComboCodePages(IFileDialogCustomize *pfdc, int nSelCode) const;

	void Customize(IFileDialogCustomize *pfdc) const;
	void UseCharCode(IFileDialogCustomize* pfdc) const;

	// IFileDialogControlEvents

	IFACEMETHODIMP OnItemSelected(
		/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ DWORD dwIDItem) override;
		
	IFACEMETHODIMP OnButtonClicked(
		/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
		/* [in] */ DWORD dwIDCtl) override {
		return E_NOTIMPL;
	}
	
	IFACEMETHODIMP OnCheckButtonToggled(
		/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
		/* [in] */ DWORD dwIDCtl,
		/* [in] */ BOOL bChecked) override;
	
	IFACEMETHODIMP OnControlActivating(
		/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
		/* [in] */ DWORD dwIDCtl) override {
		return E_NOTIMPL;
	}

	static HRESULT SetFileName(IFileDialog* pFileDialog, const std::filesystem::path& fileName) {
		return pFileDialog->SetFileName(fileName.c_str());
	}

	static HRESULT SetFolder(IFileDialog* pFileDialog, const std::filesystem::path& folderName) {
		IShellItemPtr psiFolder;
		if (const auto hr = SHCreateItemFromParsingName(folderName.c_str(), nullptr, IID_PPV_ARGS(&psiFolder)); FAILED(hr)) {
			return hr;
		}
		return pFileDialog->SetFolder(psiFolder);
	}
};

enum CtrlId {
	CHECK_READONLY = 2000,
	LABEL_CODE,
	COMBO_CODE,
	CHECK_BOM,
	CHECK_CP,
	LABEL_EOL,
	COMBO_EOL,
	LABEL_MRU,
	COMBO_MRU,
	LABEL_OPENFOLDER,
	COMBO_OPENFOLDER,
};

/* 初期化 */
void CDlgOpenFile_CommonItemDialog::Create(
	HINSTANCE					/* hInstance */,
	HWND						hwndParent,
	const WCHAR*				pszUserWildCard,
	const WCHAR*				pszDefaultPath,
	const std::vector<LPCWSTR>& vMRU,
	const std::vector<LPCWSTR>& vOPENFOLDER)
{
	m_hwndParent = hwndParent;

	/* ユーザー定義ワイルドカード（保存時の拡張子補完でも使用される） */
	if (pszUserWildCard) {
		m_strDefaultWildCard = pszUserWildCard;
	}

	/* 「開く」での初期フォルダー */
	if (pszDefaultPath && *pszDefaultPath) {
		SFilePath szPath = pszDefaultPath;
		SFilePath szFolderPath = std::filesystem::path(szPath).remove_filename();
		if (!GetLongFileName(szFolderPath, m_szInitialDir)) {
			m_szInitialDir = szFolderPath;
		}
	} else {
		m_szInitialDir = GetExeFileName().remove_filename();
	}

	m_vMRU = vMRU;
	m_vOPENFOLDER = vOPENFOLDER;
}

bool CDlgOpenFile_CommonItemDialog::DoModal_GetOpenFileName( WCHAR* pszPath, EFilter eAddFilter )
{
	StringBufferW szPath(pszPath, _MAX_PATH);

	std::vector<std::wstring> strs = {
		LS(STR_DLGOPNFL_EXTNAME1)	// L"ユーザー指定"
	};
	std::vector<COMDLG_FILTERSPEC> specs = {{
		{ strs.back().c_str(), m_strDefaultWildCard.c_str() }
	}};

	switch( eAddFilter ){
	case EFITER_TEXT:
		strs.emplace_back(LS(STR_DLGOPNFL_EXTNAME2));	// L"テキストファイル"
		specs.emplace_back(COMDLG_FILTERSPEC{ strs.back().c_str(), L"*.txt" });
		break;

	case EFITER_MACRO:
		specs.insert(specs.end(), {
			COMDLG_FILTERSPEC{ L"Macros",    L"*.js;*.vbs;*.ppa;*.mac" },
			COMDLG_FILTERSPEC{ L"JScript",   L"*.js" },
			COMDLG_FILTERSPEC{ L"VBScript",  L"*.vbs" },
			COMDLG_FILTERSPEC{ L"Pascal",    L"*.ppa" },
			COMDLG_FILTERSPEC{ L"Key Macro", L"*.mac" }
		});
		break;

	default:
		break;
	}

	if (m_strDefaultWildCard != L"*.*") {
		strs.emplace_back(LS(STR_DLGOPNFL_EXTNAME3));	// L"すべてのファイル"
		specs.emplace_back(COMDLG_FILTERSPEC{strs.back().c_str(), L"*.*"});
	}

	m_customizeSetting.bCustomize = false;
	std::vector<std::wstring> fileNames;
	const auto ret = DoModalOpenDlgImpl0(fileNames, L"", specs, false);
	if (ret) {
		szPath = fileNames[0].c_str();
	}
	return ret;
}

/*! 保存ダイアログ モーダルダイアログの表示
	@param pszPath [i/o] 初期ファイル名．選択されたファイル名の格納場所
 */
bool CDlgOpenFile_CommonItemDialog::DoModal_GetSaveFileName( WCHAR* pszPath )
{
	StringBufferW szPath(pszPath, _MAX_PATH);

	// カレントディレクトリを移動するのでパス解決する
	if (szPath.length()) {
		SFilePath szFullPath;
		if (GetLongFileName(szPath, szFullPath)) {
			szPath = szFullPath;
		}
	}

	m_customizeSetting.bCustomize = false;

	std::filesystem::path path(szPath);

	const auto ret = DoModalSaveDlgImpl0(path);
	if (ret) {
		szPath = path;
	}

	return ret;
}

void CDlgOpenFile_CommonItemDialog::Customize(
	IFileDialogCustomize* pfdc
) const
{
	if (m_customizeSetting.bShowReadOnly) {
		_com_util::CheckError(pfdc->AddCheckButton(CtrlId::CHECK_READONLY, LS(STR_FILEDIALOG_READONLY), m_bViewMode));
	}

	if (m_customizeSetting.bUseCharCode) {
		UseCharCode(pfdc);
	}

	if (m_customizeSetting.bUseEol) {
		_com_util::CheckError(pfdc->StartVisualGroup(CtrlId::LABEL_EOL, LS(STR_FILEDIALOG_EOL)));
		_com_util::CheckError(pfdc->AddComboBox(CtrlId::COMBO_EOL));
		_com_util::CheckError(pfdc->AddControlItem(CtrlId::COMBO_EOL, static_cast<DWORD>(EEolType::none), LS(STR_DLGOPNFL1)));
		_com_util::CheckError(pfdc->AddControlItem(CtrlId::COMBO_EOL, static_cast<DWORD>(EEolType::cr_and_lf), L"CR+LF"));
		_com_util::CheckError(pfdc->AddControlItem(CtrlId::COMBO_EOL, static_cast<DWORD>(EEolType::line_feed), L"LF (UNIX)"));
		_com_util::CheckError(pfdc->AddControlItem(CtrlId::COMBO_EOL, static_cast<DWORD>(EEolType::carriage_return), L"CR (Mac)"));
		_com_util::CheckError(pfdc->SetSelectedControlItem(CtrlId::COMBO_EOL, 0));
		_com_util::CheckError(pfdc->EndVisualGroup());
	}

	_com_util::CheckError(pfdc->StartVisualGroup(CtrlId::LABEL_MRU, LS(STR_FILEDIALOG_MRU)));
	_com_util::CheckError(pfdc->AddComboBox(CtrlId::COMBO_MRU));
	for (int i = 0; i < int(m_vMRU.size()); ++i) {
		_com_util::CheckError(pfdc->AddControlItem(CtrlId::COMBO_MRU, i + 1, m_vMRU[i]));
	}
	_com_util::CheckError(pfdc->EndVisualGroup());

	_com_util::CheckError(pfdc->StartVisualGroup(CtrlId::LABEL_OPENFOLDER, LS(STR_FILEDIALOG_OPENFOLDER)));
	_com_util::CheckError(pfdc->AddComboBox(CtrlId::COMBO_OPENFOLDER));
	for (int i = 0; i < int(m_vOPENFOLDER.size()); ++i) {
		_com_util::CheckError(pfdc->AddControlItem(CtrlId::COMBO_OPENFOLDER, i + 1, m_vOPENFOLDER[i]));
	}
	_com_util::CheckError(pfdc->EndVisualGroup());
}

void CDlgOpenFile_CommonItemDialog::UseCharCode(
	IFileDialogCustomize* pfdc
) const
{
	_com_util::CheckError(pfdc->StartVisualGroup(CtrlId::LABEL_CODE, LS(STR_FILEDIALOG_CODE)));
	_com_util::CheckError(pfdc->AddComboBox(CtrlId::COMBO_CODE));
	CCodeTypesForCombobox cCodeTypes;
	bool bCodeSel = false;
	ECodeType eCodeSel = CODE_NONE;
	for( int i = (m_customizeSetting.bSkipAutoDetect ? 1 : 0) /* 保存の場合は自動選択飛ばし */; i < cCodeTypes.GetCount(); ++i ){
		auto code = cCodeTypes.GetCode(i);
		_com_util::CheckError(pfdc->AddControlItem(CtrlId::COMBO_CODE, (DWORD)code, cCodeTypes.GetName(i)));
		if( code == m_nCharCode ){
			bCodeSel = true;
			eCodeSel = code;
		}
	}
	if (bCodeSel) {
		_com_util::CheckError(pfdc->SetSelectedControlItem(CtrlId::COMBO_CODE, (DWORD)eCodeSel));
		_com_util::CheckError(pfdc->AddCheckButton(CtrlId::CHECK_CP, L"C&P", FALSE));
	}
	else {
		if( -1 == AddComboCodePages( pfdc, m_nCharCode ) ){
			_com_util::CheckError(pfdc->SetSelectedControlItem(CtrlId::COMBO_CODE, CODE_SJIS));
		}
		_com_util::CheckError(pfdc->AddCheckButton(CtrlId::CHECK_CP, L"C&P", TRUE));
		_com_util::CheckError(pfdc->SetControlState(CtrlId::CHECK_CP, CDCS_VISIBLE));
	}
	if (m_customizeSetting.bUseBom) {
		const auto isUnicode = CCodeTypeName(m_nCharCode).UseBom();
		_com_util::CheckError(pfdc->AddCheckButton(CtrlId::CHECK_BOM, L"&BOM", FALSE));
		if (isUnicode) {
			_com_util::CheckError(pfdc->SetCheckButtonState(CtrlId::CHECK_BOM, m_bBom));
		}
		else {
			_com_util::CheckError(pfdc->SetControlState(CtrlId::CHECK_BOM, CDCS_VISIBLE));
		}
	}
	_com_util::CheckError(pfdc->EndVisualGroup());
}

void CDlgOpenFile_CommonItemDialog::DoModalOpenDlgImpl1(
	_In_ IFileOpenDialog* pFileDialog,
	std::vector<std::wstring>& fileNames,
	std::wstring_view fileName,
	const std::vector<COMDLG_FILTERSPEC>& specs,
	bool bAllowMultiSelect
)
{
	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	// 2010.08.28 Moca DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	m_customizeSetting.bUseEol = false;	//	Feb. 9, 2001 genta
	m_customizeSetting.bUseBom = false;	//	Jul. 26, 2003 ryoji

	FILEOPENDIALOGOPTIONS options;
	_com_util::CheckError(pFileDialog->GetOptions(&options));
	options |= FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST;
	if (bAllowMultiSelect) {
		options |= FOS_ALLOWMULTISELECT;
	}

	_com_util::CheckError(pFileDialog->SetOptions(options));
	_com_util::CheckError(pFileDialog->SetFileTypes(UINT(specs.size()), specs.data()));
	_com_util::CheckError(pFileDialog->SetFileName(fileName.data()));

	_com_util::CheckError(SetFolder(pFileDialog, std::filesystem::path(m_szInitialDir)));

	// カスタマイズ
	if (m_customizeSetting.bCustomize) {
		auto pFileDialogCustomize = IFileDialogCustomizePtr(pFileDialog);
		Customize(pFileDialogCustomize);
	}
	_com_util::CheckError(pFileDialog->Show(m_hwndParent));

	IShellItemArrayPtr pShellItems;
	_com_util::CheckError(pFileDialog->GetResults(&pShellItems));

	DWORD numItems;
	_com_util::CheckError(pShellItems->GetCount(&numItems));
	fileNames.reserve(numItems);

	for (DWORD i = 0; i < numItems; ++i) {
		IShellItemPtr pShellItem;
		_com_util::CheckError(pShellItems->GetItemAt(i, &pShellItem));
		PWSTR pszFilePath;
		_com_util::CheckError(pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath));
		fileNames.emplace_back(pszFilePath);
		CoTaskMemFree(pszFilePath);
	}
}

bool CDlgOpenFile_CommonItemDialog::DoModalOpenDlgImpl0(
	std::vector<std::wstring>& fileNames,
	std::wstring_view fileName,
	const std::vector<COMDLG_FILTERSPEC>& specs,
	bool bAllowMultiSelect
)
{
	try {
		IFileOpenDialogPtr pFileDialog;
		_com_util::CheckError(pFileDialog.CreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL));

		DoModalOpenDlgImpl1(pFileDialog, fileNames, fileName, specs, bAllowMultiSelect);
	}
	catch (const _com_error&) {
		return false;
	}

	return true;
}

/*! 「開く」ダイアログ モーダルダイアログの表示
*/
bool CDlgOpenFile_CommonItemDialog::DoModalOpenDlg(
	SLoadInfo* pLoadInfo,
	std::vector<std::wstring>* pFileNames,
	bool bOptions )
{
	std::vector<std::wstring> strs = {
		LS(STR_DLGOPNFL_EXTNAME3),	// L"すべてのファイル"
		LS(STR_DLGOPNFL_EXTNAME2)	// L"テキストファイル"
	};
	std::vector<COMDLG_FILTERSPEC> specs = {{
		COMDLG_FILTERSPEC{ strs[0].c_str(), L"*.*" },
		COMDLG_FILTERSPEC{ strs[1].c_str(), L"*.txt" }
	}};

	CDocTypeManager docTypeMgr;
	for (int i = 0; i < GetDllShareData().m_nTypesCount; ++i) {
		// タイプ名と拡張子リストを取得
		const STypeConfigMini* type = nullptr;
		if (!docTypeMgr.GetTypeConfigMini(CTypeConfig(i), &type)) {
			continue;
		}
		// 拡張子リストをダイアログ表示用に変換
		LPCWSTR pszSpec = L"";
		if (const auto dlgExt = CDocTypeManager::ConvertTypesExtToDlgExt(type->m_szTypeExts, nullptr); dlgExt.length()) {
			strs.push_back(dlgExt);
			pszSpec = strs.back().c_str();
		}
		specs.emplace_back(COMDLG_FILTERSPEC{ type->m_szTypeName, pszSpec });
	}
	m_bViewMode = pLoadInfo->bViewMode;
	m_nCharCode = pLoadInfo->eCharCode;	/* 文字コード自動判別 */
	m_customizeSetting.bCustomize = true;
	m_customizeSetting.bShowReadOnly = true;
	m_customizeSetting.bSkipAutoDetect = false;
	m_customizeSetting.bUseCharCode = bOptions;
	const auto ret = DoModalOpenDlgImpl0(*pFileNames, std::wstring_view(pLoadInfo->cFilePath), specs, true);
	if (ret) {
		pLoadInfo->eCharCode = m_nCharCode;
		pLoadInfo->bViewMode = m_bViewMode;
	}
	return ret;
}

void CDlgOpenFile_CommonItemDialog::DoModalSaveDlgImpl1(
	_In_ IFileSaveDialog* m_pFileDialog,
	std::filesystem::path& path
)
{
	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;

	// 2010.08.28 Moca DLLが読み込まれるので移動
	ChangeCurrentDirectoryToExeDir();

	std::array<std::wstring, 3> strs = {
		LS(STR_DLGOPNFL_EXTNAME1),	// L"ユーザー指定"
		LS(STR_DLGOPNFL_EXTNAME2),	// L"テキストファイル"
		LS(STR_DLGOPNFL_EXTNAME3),	// L"すべてのファイル"
	};

	std::array<COMDLG_FILTERSPEC, 3> specs = {{
		{ strs[0].c_str(), m_strDefaultWildCard.c_str() },
		{ strs[1].c_str(), L"*.txt" },
		{ strs[2].c_str(), L"*.*" },
	}};

	_com_util::CheckError(m_pFileDialog->SetDefaultExtension(L"txt"));
	_com_util::CheckError(m_pFileDialog->SetFileTypes(UINT(specs.size()), specs.data()));

	_com_util::CheckError(SetFolder(m_pFileDialog, std::filesystem::path(m_szInitialDir)));
	_com_util::CheckError(SetFileName(m_pFileDialog, path.filename()));

	if (m_customizeSetting.bCustomize) {
		auto pFileDialogCustomize = IFileDialogCustomizePtr(m_pFileDialog);
		Customize(pFileDialogCustomize);
	}

	_com_util::CheckError(m_pFileDialog->Show(m_hwndParent));

	IShellItemPtr pShellItem;
	_com_util::CheckError(m_pFileDialog->GetResult(&pShellItem));
	PWSTR pszFilePath;
	_com_util::CheckError(pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath));
	path = pszFilePath;
	CoTaskMemFree(pszFilePath);
}

bool CDlgOpenFile_CommonItemDialog::DoModalSaveDlgImpl0(
	std::filesystem::path& path
)
{
	try {
		IFileSaveDialogPtr pFileDialog;
		_com_util::CheckError(pFileDialog.CreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL));

		DoModalSaveDlgImpl1(pFileDialog, path);
	}
	catch (const _com_error&) {
		return false;
	}

	return true;
}

/*! 保存ダイアログ モーダルダイアログの表示
 */
bool CDlgOpenFile_CommonItemDialog::DoModalSaveDlg( SSaveInfo* pSaveInfo, bool bSimpleMode )
{
	if (bSimpleMode) {
		m_customizeSetting.bCustomize = false;
	}
	else {
		m_nCharCode = pSaveInfo->eCharCode;
		m_bBom = pSaveInfo->bBomExist;
		m_cEol = pSaveInfo->cEol;
		m_customizeSetting.bCustomize = true;
		m_customizeSetting.bUseCharCode = true;
		m_customizeSetting.bUseEol = true;
		m_customizeSetting.bUseBom = true;
		m_customizeSetting.bSkipAutoDetect = true;
		m_customizeSetting.bShowReadOnly = false;
	}

	std::filesystem::path path(pSaveInfo->cFilePath);

	const auto ret = DoModalSaveDlgImpl0(path);
	if (ret) {
		pSaveInfo->cFilePath = path.c_str();
	} else {
		pSaveInfo->cFilePath = nullptr;
	}

	if (ret && !bSimpleMode) {
		pSaveInfo->eCharCode = m_nCharCode;
		//	Feb. 9, 2001 genta
		if( m_customizeSetting.bUseEol ){
			pSaveInfo->cEol = m_cEol;
		}
		//	Jul. 26, 2003 ryoji BOM設定
		if( m_customizeSetting.bUseBom ){
			pSaveInfo->bBomExist = m_bBom;
		}
	}
	return ret;
}

IFACEMETHODIMP CDlgOpenFile_CommonItemDialog::OnItemSelected(
	/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
	/* [in] */ DWORD dwIDCtl,
	/* [in] */ DWORD dwIDItem)
{
	switch (dwIDCtl) {
	case CtrlId::COMBO_CODE:
		{
			CCodeTypeName cCodeTypeName( (int)dwIDItem );
			CDCONTROLSTATEF state;
			bool bChecked;
			if (cCodeTypeName.UseBom()) {
				state = CDCS_ENABLEDVISIBLE;
				bChecked = (dwIDItem == m_nCharCode) ? m_bBom : cCodeTypeName.IsBomDefOn();
			}
			else {
				state = CDCS_VISIBLE;
				bChecked = false;
			}
			pfdc->SetControlState(CtrlId::CHECK_BOM, state);
			pfdc->SetCheckButtonState(CtrlId::CHECK_BOM, bChecked ? TRUE : FALSE);
			m_nCharCode = static_cast<ECodeType>(dwIDItem);
		}
		break;

	case CtrlId::COMBO_EOL:
		m_cEol = static_cast<EEolType>(dwIDItem);
		break;

	case CtrlId::COMBO_MRU:
		if (dwIDItem != 0) {
			IFileDialogPtr pFileDialog(pfdc);
			SetFileName(pFileDialog, m_vMRU[dwIDItem - 1]);
		}
		break;

	case CtrlId::COMBO_OPENFOLDER:
		if (dwIDItem != 0) {
			IFileDialogPtr pFileDialog(pfdc);
			return SetFolder(pFileDialog, m_vOPENFOLDER[dwIDItem - 1]);
		}
		break;

	default:
		return E_NOTIMPL;
	}

	return S_OK;
}

IFACEMETHODIMP CDlgOpenFile_CommonItemDialog::OnCheckButtonToggled(
	/* [in] */ __RPC__in_opt IFileDialogCustomize *pfdc,
	/* [in] */ DWORD dwIDCtl,
	/* [in] */ BOOL bChecked)
{
	switch (dwIDCtl) {
	case CtrlId::CHECK_READONLY:
		m_bViewMode = bChecked ? true : false;
		break;

	case CtrlId::CHECK_CP:
		pfdc->SetControlState(CtrlId::CHECK_CP, CDCS_VISIBLE);
		AddComboCodePages(pfdc, m_nCharCode);
		break;

	case CtrlId::CHECK_BOM:
		m_bBom = bChecked ? true : false;
		break;

	default:
		return E_NOTIMPL;
	}

	return S_OK;
}

int CDlgOpenFile_CommonItemDialog::AddComboCodePages(
	IFileDialogCustomize* pfdc,
	int nSelCode
) const
{
	int nSel = -1;
	_com_util::CheckError(pfdc->AddControlItem(CtrlId::COMBO_CODE, (DWORD)CODE_CPACP, L"CP_ACP"));
	if( nSelCode == CODE_CPACP ){
		_com_util::CheckError(pfdc->SetSelectedControlItem(CtrlId::COMBO_CODE, (DWORD)CODE_CPACP));
		nSel = nSelCode;
	}
	_com_util::CheckError(pfdc->AddControlItem(CtrlId::COMBO_CODE, (DWORD)CODE_CPOEM, L"CP_OEM"));
	if( nSelCode == CODE_CPOEM ){
		_com_util::CheckError(pfdc->SetSelectedControlItem(CtrlId::COMBO_CODE, (DWORD)CODE_CPOEM));
		nSel = nSelCode;
	}
	const auto& cpList = CCodePage::GetCodePageList();
	for( auto it = cpList.cbegin(); it != cpList.cend(); ++it ){
		_com_util::CheckError(pfdc->AddControlItem(CtrlId::COMBO_CODE, (DWORD)it->first, it->second.c_str()));
		if( nSelCode == it->first ){
			_com_util::CheckError(pfdc->SetSelectedControlItem(CtrlId::COMBO_CODE, (DWORD)it->first));
			nSel = nSelCode;
		}
	}
	return nSel;
}

std::shared_ptr<IDlgOpenFile> New_CDlgOpenFile_CommonItemDialog()
{
	return std::make_shared<CDlgOpenFile_CommonItemDialog>();
}
