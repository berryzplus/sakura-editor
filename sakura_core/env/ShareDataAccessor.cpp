/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
 */
#include "StdAfx.h"
#include "env/ShareDataAccessor.hpp"

#include "env/DLLSHAREDATA.h"

/*!
 * 共有メモリのアドレスを取得します。
 */
DLLSHAREDATA* ShareDataAccessor::GetShareData() const
{
	// 共有メモリのアドレスを取得します。
	return &::GetDllShareData();
}

/*!
 * 共有メモリのアドレスを更新します。
 *
 * このメソッドはCShareData専用です。
 */
void ShareDataAccessor::SetShareData(DLLSHAREDATA* pShareData) const
{
	// 共有メモリのアドレスを更新します。
	::SetDllShareData(pShareData);
}

const ShareDataAccessor& GetShareDataAccessor() {
	static std::unique_ptr<ShareDataAccessor> ShareDataAccessor_;
	if (!ShareDataAccessor_) {
		ShareDataAccessor_ = std::make_unique<ShareDataAccessor>();
	}
	return *ShareDataAccessor_;
}
