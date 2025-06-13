/*!	@file
	@brief 各国語メッセージリソース対応

	@author nasukoji
	@date 2011.04.10	新規作成
*/
/*
	Copyright (C) 2011, nasukoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CSelectLang.h"

#include "_main/CProcess.h"
#include "util/os.h"
#include "util/module.h"
#include "debug/Debug2.h"
#include "String_define.h"

/* static */ void CSelectLang::UninitializeLanguageEnvironment()
{
	gm_LangDllList.clear();
}

/*!
	@brief メッセージリソースDLLのインスタンスハンドルを返す

	@retval メッセージリソースDLLのインスタンスハンドル

	@note メッセージリソースDLLをロードしていない場合exeのインスタンスハンドルが返る

	@date 2011.04.10 nasukoji	新規作成
*/
HINSTANCE CSelectLang::getLangRsrcInstance( void )
{
	return gm_LangDllList.size() ? HINSTANCE(gm_LangDll->hInstance) : G_AppInstance();
}

/*!
	@brief メッセージリソースDLL未読み込み時のデフォルト言語の文字列を返す

	@retval デフォルト言語の文字列（"(Japanese)" または "(English(United States))"）

	@note アプリケーションリソースより読み込んだ "(Japanese)" または "(English(United States))"

	@date 2011.04.10 nasukoji	新規作成
 */
LPCWSTR CSelectLang::getDefaultLangString( void )
{
	return gm_LangDllList.front().langName.c_str();
}

// 言語IDを返す
WORD CSelectLang::getDefaultLangId(void)
{
	if (gm_LangDllList.empty()) {
		return GetUserDefaultLangID();
	}
	return gm_LangDllList.front().wLangId;
}

/*!
	@brief 言語環境を初期化する
	
	@retval メッセージリソースDLLのインスタンスハンドル

	@note メッセージリソースDLLが未指定、または読み込みエラー発生の時はexeのインスタンスハンドルが返る
	@note （LoadString()の引数としてそのまま使用するため）
	@note デフォルト言語の文字列の読み込みも行う
	@note プロセス毎にProcessFactoryの最初に1回だけ呼ばれる

	@date 2011.04.10 nasukoji	新規作成
*/
HINSTANCE CSelectLang::InitializeLanguageEnvironment( void )
{
	if (gm_LangDllList.empty()) {
		const auto hInstance = G_AppInstance();

		// デフォルト情報を作成する
		auto& langDll = gm_LangDllList.emplace_back();

		langDll.hInstance = hInstance;
		
		// 言語情報ダイアログで "System default" に表示する文字列を作成する
		SString<MAX_SELLANG_NAME_STR + 1> szSelLangName;
		LoadStringW(hInstance, STR_SELLANG_NAME, szSelLangName, int(std::size(szSelLangName)));

		// 言語IDを取得
		SString<7> szBuf;		// "0x" + 4桁 + 番兵
		LoadStringW(hInstance, STR_SELLANG_LANGID, szBuf, int(std::size(szBuf)));

		langDll.wLangId = WORD(wcstoul(szBuf, nullptr, 16));		// 言語IDを数値化
	}

	else if (1 < gm_LangDllList.size()) {
		// 読み込み済みのDLLを解放する
		gm_LangDllList.erase(gm_LangDllList.begin() + 1, gm_LangDllList.end());
	}

	//カレントディレクトリを保存。関数から抜けるときに自動でカレントディレクトリは復元される。
	CCurrentDirectoryBackupPoint cCurDirBackup;
	ChangeCurrentDirectoryToExeDir();
// ★iniまたはexeフォルダーとなるように改造が必要

	//! HANDLE型のスマートポインタ
	using ModuleHolder = cxx_util::ResourceHolder<HMODULE, &FreeLibrary>;

	//! HANDLE型のスマートポインタ
	using FindHolder = cxx_util::ResourceHolder<HANDLE, &FindClose>;

	WIN32_FIND_DATA w32fd{};
	FindHolder handle = FindFirstFileW(L"sakura_lang_*.dll", &w32fd);
	bool result = INVALID_HANDLE_VALUE != handle;

	while (result) {
		if (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}

		std::filesystem::path dllPath(w32fd.cFileName);
		ModuleHolder hInstance = LoadLibraryExedir(dllPath.c_str());

		SString<MAX_SELLANG_NAME_STR + 1> szSelLangName;
		LoadStringW(hInstance, STR_SELLANG_NAME, szSelLangName, int(std::size(szSelLangName)));

		// 言語IDを取得
		SString<7> szBuf;		// "0x" + 4桁 + 番兵
		LoadStringW(hInstance, STR_SELLANG_LANGID, szBuf, int( std::size(szBuf)));
		const auto wLangId = WORD(wcstoul(szBuf, nullptr, 16));		// 言語IDを数値化

		if (szSelLangName.empty() || !wLangId) {
			continue;
		}

		// バッファに登録する。
		gm_LangDllList.emplace_back(dllPath, szSelLangName, wLangId);

		result = FindNextFileW(handle, &w32fd);
	}

	gm_LangDll = gm_LangDllList.begin();	// 最初の言語DLLを選択する

	return getLangRsrcInstance();
}

void CSelectLang::ChangeLang(const std::filesystem::path& dllPath)
{
	/* 言語を選択する */
	if (const auto found = std::find_if(gm_LangDllList.cbegin(), gm_LangDllList.cend(),
		[&dllPath](const SLangDll& langDll) { return langDll.dllPath == dllPath; }); found != gm_LangDllList.cend())
	{
		ChangeLang(UINT(std::distance(gm_LangDllList.cbegin(), found)));
	}
}

HINSTANCE CSelectLang::ChangeLang( UINT nIndex )
{
	if (gm_LangDllList.size() <= nIndex) {
		return getLangRsrcInstance();
	}

	const auto oldIndex = UINT(std::distance(gm_LangDllList.begin(), gm_LangDll));
	if (oldIndex == nIndex) {
		return getLangRsrcInstance();
	}

	if (nIndex) {
		gm_LangDllList[nIndex].hInstance = LoadLibraryExedir(gm_LangDllList[nIndex].dllPath.c_str());
	}

	if (oldIndex) {
		gm_LangDllList[oldIndex].hInstance = nullptr;
	}

	gm_LangDll = gm_LangDllList.begin() + nIndex;	// 選択言語を変更

	// ロケールを設定
	SetThreadUILanguage(gm_LangDll->wLangId);

	return gm_LangDll->hInstance;
}

/*!
	@brief 静的バッファに文字列リソースを読み込む（各国語メッセージリソース対応）

	@param[in] uid リソースID

	@retval 読み込んだ文字列（文字列無しの時 "" が返る）

	@note 静的バッファ（m_acLoadStrBufferTemp[?]）に文字列リソースを読み込む。
	@note バッファは複数準備しているが、呼び出す毎に更新するのでバッファ個数を
	@note 超えて呼び出すと順次内容が失われていく。
	@note 呼び出し直後での使用や関数の引数などでの使用を想定しており、前回値を
	@note 取り出すことはできない。
	@note 使用例）::SetWindowText( m_hWnd, CLoadString::LoadStringSt(STR_ERR_DLGSMCLR1) );
	@note アプリケーション内の関数への引数とする場合、その関数が本関数を使用
	@note しているか意識する必要がある（上限を超えれば内容が更新されるため）
	@note 内容を保持したい場合は CLoadString::LoadString() を使用する。

	@date 2011.06.01 nasukoji	新規作成
*/
LPCWSTR CLoadString::LoadStringSt( UINT uid )
{
	// 使用するバッファの現在位置を進める
	m_nDataTempArrayIndex = (m_nDataTempArrayIndex + 1) % std::size(m_acLoadStrBufferTemp);

	m_acLoadStrBufferTemp[m_nDataTempArrayIndex].LoadString( uid );

	return /* CLoadString:: */ m_acLoadStrBufferTemp[m_nDataTempArrayIndex].GetStringPtr();
}

/*!
	@brief 文字列リソースを読み込む（各国語メッセージリソース対応）

	@param[in] uid リソースID

	@retval 読み込んだ文字列（文字列無しの時 "" が返る）

	@note メンバ変数内に記憶されるため  CLoadString::LoadStringSt() の様に
	@note 不用意に破壊されることはない。
	@note ただし、変数を準備する必要があるのが不便。
	@note 使用例）
	@note   CLoadString cStr[2];
	@note   cDlgInput1.DoModal( m_hInstance, m_hWnd,
	@note       cStr[0].LoadString(STR_ERR_DLGPRNST1),
	@note       cStr[1].LoadString(STR_ERR_DLGPRNST2),
	@note       sizeof( m_PrintSettingArr[m_nCurrentPrintSetting].m_szPrintSettingName ) - 1, szWork ) )

	@date 2011.06.01 nasukoji	新規作成
*/
LPCWSTR CLoadString::LoadString( UINT uid )
{
	m_cLoadStrBuffer.LoadString( uid );

	return /* this-> */ m_cLoadStrBuffer.GetStringPtr();
}

/*!
	@brief 文字列リソースを読み込む（読み込み実行部）

	@param[in] uid  リソースID

	@retval 読み込んだ文字数（WCHAR単位）

	@note メッセージリソースより文字列を読み込む。メッセージリソースDLLに指定の
	@note リソースが存在しない、またはメッセージリソースDLL自体が読み込まれて
	@note いない場合、内部リソースより文字列を読み込む。
	@note 最初は静的バッファに読み込むがバッファ不足となったらバッファを拡張
	@note して読み直す。
	@note 取得したバッファはデストラクタで解放する。
	@note ANSI版は2バイト文字の都合により（バッファ - 2）バイトまでしか読まない
	@note 場合があるので1バイト少ない値でバッファ拡張を判定する。

	@date 2011.06.01 nasukoji	新規作成
*/
int CLoadString::CLoadStrBuffer::LoadString( UINT uid )
{
	if( !m_pszString ){
		// バッファポインタが設定されていない場合初期化する（普通はあり得ない）
		m_pszString = m_szString;					// 変数内に準備したバッファを接続
		m_nBufferSize = _countof(m_szString);		// 配列個数
		m_szString[m_nBufferSize - 1] = 0;
		m_nLength = wcslen(m_szString);			// 文字数
	}

	HINSTANCE hRsrc = CSelectLang::getLangRsrcInstance();		// メッセージリソースDLLのインスタンスハンドル

	if( !hRsrc ){
		// メッセージリソースDLL読込処理前は内部リソースを使う
		hRsrc = G_AppInstance();
	}

	int nRet = 0;

	while(1){
		nRet = ::LoadString( hRsrc, uid, m_pszString, m_nBufferSize );

		// リソースが無い
		if( nRet == 0 ){
			if( hRsrc != G_AppInstance()){
				hRsrc = G_AppInstance();	// 内部リソースを使う
			}else{
				// 内部リソースからも読めなかったら諦める（普通はあり得ない）
				m_pszString[0] = L'\0';
				break;
			}
		}else if( nRet >= m_nBufferSize - 1 ){
			// 読みきれなかった場合、バッファを拡張して読み直す
			int nTemp = m_nBufferSize + LOADSTR_ADD_SIZE;		// 拡張したサイズ
			LPWSTR pTemp;

			try{
				pTemp = new WCHAR[nTemp];
			}
			catch(const std::bad_alloc&){
				// メモリ割り当て例外（例外の発生する環境の場合でも旧来の処理にする）
				pTemp = nullptr;
			}

			if( pTemp ){
				if( m_pszString != m_szString ){
					delete[] m_pszString;
				}

				m_pszString = pTemp;
				m_nBufferSize = nTemp;
			}else{
				// メモリ取得に失敗した場合は直前の内容で諦める
				nRet = wcslen( m_pszString );
				break;
			}
		}else{
			break;		// 文字列リソースが正常に取得できた
		}
	}

	m_nLength = nRet;	// 読み込んだ文字数

	return nRet;
}
