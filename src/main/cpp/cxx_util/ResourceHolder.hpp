/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

namespace cxx_util {

/*!
 * リソースハンドルを保持するスマートポインタ
 */
template<typename T, auto Deleter>
struct ResourceHolder
{
	using holder_type = std::unique_ptr<std::remove_pointer_t<T>, decltype(Deleter)>;
	holder_type holder;

	/* implicit */ ResourceHolder(T t) noexcept
		: holder(t, Deleter)
	{
	}

	/* implicit */ operator T() const noexcept { return holder.get(); }
};

} // end of namespace cxx_util
