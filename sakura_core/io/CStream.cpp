/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "io/CStream.h"

#include "util/std_macro.h"

#include <fcntl.h>
#include <io.h>

//	::fflush(m_hFile);
//  ネットワーク上のファイルを扱っている場合、
//	書き込み後にFlushを行うとデットロックが発生することがあるので、
//	Close時に::fflushを呼び出してはいけません。
//  詳細：http://www.microsoft.com/japan/support/faq/KBArticles2.asp?URL=/japan/support/kb/articles/jp288/7/94.asp

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CStream::CStream(cxx::NamedFileHolder&& file, bool bExceptionMode)
	: Base(std::move(file))
	, m_bExceptionMode(bExceptionMode)
{
}

CStream::CStream(std::wstring_view path, std::wstring_view mode, bool bExceptionMode)
	: m_bExceptionMode(bExceptionMode)
{
	Open(path, mode);
}

CStream::CStream(Me&& rhs) noexcept
	: Base(std::move(rhs))
	, m_bExceptionMode(rhs.m_bExceptionMode)
{
	rhs.m_bExceptionMode = false;
}

CStream& CStream::operator = (Me&& rhs) noexcept
{
	if (this == &rhs) {
		return *this;
	}

	Base::operator = (std::move(rhs));
	m_bExceptionMode = rhs.m_bExceptionMode;
	rhs.m_bExceptionMode = false;

	return *this;
}

CStream::~CStream()
{
	Close();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    オープン・クローズ                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CStream::Open(
	std::wstring_view path,
	std::wstring_view mode
)
{
	//オープン
	Base::operator = (FileHolder::OpenFilePath(path, mode));

	//エラー処理
	if (!GetFp() && IsExceptionMode()) {
		throw CError_FileOpen();
	}
}

void CStream::Close()
{
	//クローズ
	reset(nullptr);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           操作                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CStream::SeekSet(	//!< シーク
	long offset	//!< ストリーム先頭からのオフセット 
) const
{
	::fseek(GetFp(), offset, SEEK_SET);
}

void CStream::SeekEnd(   //!< シーク
	long offset //!< ストリーム終端からのオフセット
) const
{
	::fseek(GetFp(), offset, SEEK_END);
}
