/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"

#include <Windows.h>
#include <DbgHelp.h>

#include <string>

namespace cxx {

/*!
 * @brief スタックトレースを出力するクラス
 */
class DbgHelp final : public cxx::ResourceHolder<&::FreeLibrary>
{
private:
	using Base = cxx::ResourceHolder<&::FreeLibrary>;
	using Me = DbgHelp;

public:
	DbgHelp();

	/*!
	 * @brief DLLシンボルのアドレスを取得する
	 *
	 * @param[in] procName シンボル名
	 * @return DLLシンボルのアドレス
	 */
	template <typename T>
	T LoadSymbol(
		const std::string& procName
	) const
	{
		return std::bit_cast<T>(::GetProcAddress(
			get(),
			procName.c_str()
		));
	}
};

}	// namespace cxx
