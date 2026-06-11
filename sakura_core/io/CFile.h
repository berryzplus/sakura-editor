/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFILE_53DA3C63_95C0_49D0_9ED1_1C0131493912_H_
#define SAKURA_CFILE_53DA3C63_95C0_49D0_9ED1_1C0131493912_H_
#pragma once

#include "basis/CMyString.h" //CFilePath
#include "cxx/ResourceHolder.hpp"
#include "util/file.h"

namespace cxx {

class NamedFileHolder;

/*!
 * @brief Cストリーム型のスマートポインター
 */
class FileHolder : public cxx::ResourceHolder<&::fclose>
{
private:
	using Base = cxx::ResourceHolder<&::fclose>;
	using Me = FileHolder;

public:
	static NamedFileHolder CreateTempFile(const std::filesystem::path& path);

	static NamedFileHolder OpenFilePath(const std::filesystem::path& path, std::wstring_view mode);

	static NamedFileHolder OpenFromHandle(HANDLE hFile, const std::filesystem::path& path);

	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;

	explicit FileHolder(int fd, std::wstring_view mode);

	FileHolder(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	FileHolder(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;
};

/*!
 * @brief Cストリーム型のスマートポインター
 */
class NamedFileHolder : public cxx::FileHolder
{
private:
	using Base = cxx::FileHolder;
	using Me = NamedFileHolder;

public:
	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;

	explicit NamedFileHolder(FileHolder&& file, const std::filesystem::path& path)
		: Base(std::move(file))
		, m_Path{ path }
	{
	}

	NamedFileHolder(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	NamedFileHolder(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

	virtual ~NamedFileHolder() = default;

	std::filesystem::path	GetPath() const noexcept { return m_Path; }

private:
	std::filesystem::path m_Path;
};

} // namespace cxx

//!ファイルの排他制御モード  2007.10.11 kobake 作成
enum EShareMode{
	SHAREMODE_NOT_EXCLUSIVE,	//!< 排他制御しない
	SHAREMODE_DENY_WRITE,		//!< 他プロセスからの上書きを禁止
	SHAREMODE_DENY_READWRITE,	//!< 他プロセスからの読み書きを禁止
};

class CFile{
	using Me = CFile;

public:
	//コンストラクタ・デストラクタ
	CFile(LPCWSTR pszPath = nullptr);
	CFile(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CFile(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CFile();
	//パス
	const CFilePath& GetFilePathClass() const { return m_szFilePath; }
	LPCWSTR GetFilePath() const { return m_szFilePath; }
	//設定
	void SetFilePath(LPCWSTR pszPath){ m_szFilePath.Assign(pszPath); }
	//各種判定
	bool IsFileExist() const;
	bool HasWritablePermission() const;
	bool IsFileWritable() const;
	bool IsFileReadable() const;
	//ロック
	bool FileLock(EShareMode eShareMode, bool bMsg);	//!< ファイルの排他ロック
	void FileUnlock();						//!< ファイルの排他ロック解除
	bool IsFileLocking() const{ return m_hLockedFile!=INVALID_HANDLE_VALUE; }
	EShareMode GetShareMode() const{ return m_nFileShareModeOld; }
	void SetShareMode(EShareMode eShareMode) { m_nFileShareModeOld = eShareMode; }
private:
	CFilePath	m_szFilePath;				//!< ファイルパス
	HANDLE		m_hLockedFile;				//!< ロックしているファイルのハンドル
	EShareMode	m_nFileShareModeOld;		//!< ファイルの排他制御モード
};

//!一時ファイル
class CTmpFile{
	using Me = CTmpFile;

public:
	CTmpFile(){ m_fp = tmpfile(); }
	CTmpFile(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CTmpFile(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CTmpFile(){ fclose(m_fp); }
	FILE* GetFilePointer() const{ return m_fp; }
private:
	FILE* m_fp;
};
#endif /* SAKURA_CFILE_53DA3C63_95C0_49D0_9ED1_1C0131493912_H_ */
