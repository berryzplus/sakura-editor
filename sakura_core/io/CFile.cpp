/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "io/CFile.h"

#include "window/CEditWnd.h" // 変更予定

#include <sys/types.h>
#include <sys/stat.h>	// _fstat で必要。先に sys/types.h をincludeする必要がある。

#include <fcntl.h>
#include <io.h>

#include "CSelectLang.h"

namespace cxx {

/* static */ NamedFileHolder FileHolder::CreateTempFile(
	const std::filesystem::path& path
)
{
	HANDLE hFile = ::CreateFileW(
		path.c_str(),
		GENERIC_READ | GENERIC_WRITE | DELETE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		CREATE_NEW,
		FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
		nullptr
	);

	return OpenFromHandle(hFile, path);
}

/* static */ NamedFileHolder FileHolder::OpenFilePath(
	const std::filesystem::path& path,
	std::wstring_view mode
)
{
	// パスは必須。既存コードが空パスで呼ぶので、エラーにできない。
	if (path.empty()) return NamedFileHolder{};

	// モードも必須。省略すると fopen で落ちる。
	if (mode.empty()) throw std::invalid_argument("missing mode");

	const std::wstring strPath{ path };
	const std::wstring strMode{ mode };

	const auto pszPath = strPath.c_str();
	const auto pszMode = strMode.c_str();

	if (FILE* fp = nullptr; 0 == ::_wfopen_s(&fp, pszPath, pszMode)) {
		// 通常ファイルはここで開けるはず
		return NamedFileHolder{ FileHolder{ fp }, path };
	}

	DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

	// OSのファイルハンドルを開く
	HANDLE hFile = ::CreateFileW(
		pszPath,
		dwDesiredAccess | DELETE,
		dwShareMode | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		0L,				// 属性は指定しない
		nullptr
	);

	if (INVALID_HANDLE_VALUE == hFile) {
		// 削除権限を要求せずリトライする
		hFile = ::CreateFileW(
			pszPath,
			dwDesiredAccess,
			dwShareMode,
			nullptr,
			OPEN_EXISTING,
			0L,
			nullptr
		);
	}

	return OpenFromHandle(hFile, path);
}

/* static */ NamedFileHolder FileHolder::OpenFromHandle(HANDLE hFile, const std::filesystem::path& path)
{
	// OSのファイルハンドルからファイル記述子を開く
	const auto fd = ::_open_osfhandle(
		intptr_t(hFile),
		_O_RDWR | _O_BINARY
	);

	// 失敗した場合、-1 になる
	if (-1 == fd) {
		// 成功時と挙動を合わせるため、自分で閉じておく
		::CloseHandle(hFile);

		return NamedFileHolder{};	// 開けなかった
	}

	// hFileの所有権はCRTに移っている

	return NamedFileHolder{ FileHolder{ fd, L"w+b" }, path};
}

FileHolder::FileHolder(int fd, std::wstring_view mode)
{
	assert(fd != -1);

	// fdからCストリームを開く
	if (FILE* fp = ::_wfdopen(fd, std::data(mode))) {
		reset(fp);
	}
	// 開けなかった場合、有効なfdなら_closeしておく
	else {
		::_close(fd); // 元のhFileも閉じられる
	}
}

} // namespace cxx

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CFile::CFile(LPCWSTR pszPath)
: m_hLockedFile( INVALID_HANDLE_VALUE )
, m_nFileShareModeOld( SHAREMODE_NOT_EXCLUSIVE )
{
	if(pszPath){
		SetFilePath(pszPath);
	}
}

CFile::~CFile()
{
	FileUnlock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         各種判定                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFile::IsFileExist() const
{
	return fexist(GetFilePath());
}

bool CFile::HasWritablePermission() const
{
	return -1 != _waccess( GetFilePath(), 2 );
}

bool CFile::IsFileWritable() const
{
	//書き込めるか検査
	// Note. 他のプロセスが明示的に書き込み禁止しているかどうか
	//       ⇒ GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE でチェックする
	//          実際のファイル保存もこれと等価な _wfopen の L"wb" を使用している
	HANDLE hFile = CreateFile(
		this->GetFilePath(),			//ファイル名
		GENERIC_WRITE,					//書きモード
		FILE_SHARE_READ | FILE_SHARE_WRITE,	//読み書き共有
		nullptr,							//既定のセキュリティ記述子
		OPEN_EXISTING,					//ファイルが存在しなければ失敗
		FILE_ATTRIBUTE_NORMAL,			//特に属性は指定しない
		nullptr							//テンプレート無し
	);
	if(hFile==INVALID_HANDLE_VALUE){
		return false;
	}
	CloseHandle(hFile);
	return true;
}

bool CFile::IsFileReadable() const
{
	HANDLE hTest = CreateFile(
		this->GetFilePath(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		nullptr
	);
	if(hTest==INVALID_HANDLE_VALUE){
		// 読み込みアクセス権がない
		return false;
	}
	CloseHandle( hTest );
	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ロック                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ファイルの排他ロック解除
void CFile::FileUnlock()
{
	//クローズ
	if( m_hLockedFile != INVALID_HANDLE_VALUE ){
		::CloseHandle( m_hLockedFile );
		m_hLockedFile = INVALID_HANDLE_VALUE;
	}
}

//! ファイルの排他ロック
bool CFile::FileLock( EShareMode eShareMode, bool bMsg )
{
	// ロック解除
	FileUnlock();

	// ファイルの存在チェック
	if( !this->IsFileExist() ){
		return false;
	}

	// モード設定
	if(eShareMode==SHAREMODE_NOT_EXCLUSIVE)return true;
	
	//フラグ
	DWORD dwShareMode=0;
	switch(eShareMode){
	case SHAREMODE_NOT_EXCLUSIVE:	return true;										break; //排他制御無し
	case SHAREMODE_DENY_READWRITE:	dwShareMode = 0;									break; //読み書き禁止→共有無し
	case SHAREMODE_DENY_WRITE:		dwShareMode = FILE_SHARE_READ;						break; //書き込み禁止→読み込みのみ認める
	default:						dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;	break; //禁止事項なし→読み書き共に認める
	}

	//オープン
	m_hLockedFile = CreateFile(
		this->GetFilePath(),			//ファイル名
		GENERIC_READ,					//読み書きタイプ
		dwShareMode,					//共有モード
		nullptr,							//既定のセキュリティ記述子
		OPEN_EXISTING,					//ファイルが存在しなければ失敗
		FILE_ATTRIBUTE_NORMAL,			//特に属性は指定しない
		nullptr							//テンプレート無し
	);

	//結果
	if( INVALID_HANDLE_VALUE == m_hLockedFile && bMsg ){
		const WCHAR*	pszMode;
		switch( eShareMode ){
		case SHAREMODE_DENY_READWRITE:	pszMode = LS(STR_EXCLU_DENY_READWRITE); break;
		case SHAREMODE_DENY_WRITE:		pszMode = LS(STR_EXCLU_DENY_WRITE); break;
		default:						pszMode = LS(STR_EXCLU_UNDEFINED); break;
		}
		TopWarningMessage(
			CEditWnd::getInstance()->GetHwnd(),
			LS(STR_FILE_LOCK_ERR),
			GetFilePathClass().IsValidPath() ? GetFilePath() : LS(STR_NO_TITLE1),
			pszMode
		);
		return false;
	}

	return true;
}
