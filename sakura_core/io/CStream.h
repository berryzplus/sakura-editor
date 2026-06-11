/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSTREAM_0083EDD7_A671_4315_801D_41FED1A2E3DA_H_
#define SAKURA_CSTREAM_0083EDD7_A671_4315_801D_41FED1A2E3DA_H_
#pragma once

#include "io/CFile.h"

//! 例外：ファイルオープンに失敗
class CError_FileOpen {
public:
	enum EReason {
		UNKNOWN,
		TOO_BIG
	};
public:
	CError_FileOpen() : m_reason(UNKNOWN) {}
	CError_FileOpen(EReason reason) : m_reason(reason) {}
	EReason Reason() const { return m_reason; }
private:
	EReason m_reason;
};

class CError_FileWrite{};	//!< 例外：ファイル書き込み失敗
class CError_FileRead{};	//!< 例外：ファイル読み込み失敗

//ストリーム基底クラス
class CStream : public cxx::NamedFileHolder {
private:
	using Base = cxx::NamedFileHolder;
	using Me = CStream;

public:
	//コンストラクタ・デストラクタ
	CStream() = default;
	explicit CStream(cxx::NamedFileHolder&& file, bool bExceptionMode = false);
	explicit CStream(std::wstring_view path, std::wstring_view mode, bool bExceptionMode = false);

	CStream(const Me&) = delete;
	Me& operator = (const Me&) = delete;

	CStream(Me&& rhs) noexcept;
	Me& operator = (Me&& rhs) noexcept;

	~CStream() override;

	//演算子
	explicit operator bool() const noexcept { return Good(); }

	//オープン・クローズ
	void	Open(std::wstring_view path, std::wstring_view mode);
	void	Close();

	//操作
	void	SeekSet(long offset) const;
	void	SeekEnd(long offset) const;

	//状態
	virtual bool Good() const noexcept { return GetFp() && !Eof(); }
	bool Eof() const noexcept { return !GetFp() || ::feof(GetFp()); }

	//ファイルハンドル
	FILE* GetFp() const noexcept { return get(); }

	//モード
	bool IsExceptionMode() const{ return m_bExceptionMode; }

private:
	bool			m_bExceptionMode = false;
};

class COutputStream : public CStream{
private:
	using Base = CStream;
	using Me = COutputStream;

public:
	using Base::Base;

	//! データを無変換で書き込む。戻り値は書き込んだバイト数。
	int Write(const void* pBuffer, size_t nSizeInBytes)
	{
		size_t nRet = ::fwrite(pBuffer, 1, nSizeInBytes, GetFp());
		if(nRet!=nSizeInBytes && IsExceptionMode())throw CError_FileWrite();
		return static_cast<int>(nRet);
	}
};

#endif /* SAKURA_CSTREAM_0083EDD7_A671_4315_801D_41FED1A2E3DA_H_ */
