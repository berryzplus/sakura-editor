/*!	@file
	@brief Mutex管理

	@author ryoji
	@date 2007.07.05
*/
/*
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CMUTEX_51EDDE78_F635_419A_9E10_159485D0F710_H_
#define SAKURA_CMUTEX_51EDDE78_F635_419A_9E10_159485D0F710_H_
#pragma once

#include "_main/CProcess.h" // cxx::MutexHolder

/** ミューテックスを扱うクラス
	@date 2007.07.05 ryoji 新規作成
*/
class CMutex final : public cxx::MutexHolder
{
private:
	using Base = MutexHolder;
	using Me = CMutex;

public:
	CMutex( BOOL bInitialOwner, LPCWSTR pszName, LPSECURITY_ATTRIBUTES psa = nullptr )
		: MutexHolder(nullptr)
		, m_Name(pszName ? std::optional<std::wstring>(pszName) : std::nullopt)
		, m_bInitialOwner(bInitialOwner)
		, m_pSa(psa)
	{
	}
	CMutex(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CMutex(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CMutex() override = default;

	bool Lock(DWORD dwTimeout = INFINITE) override
	{
		if (!*this) {
			HandleHolder::reset(::CreateMutexW(m_pSa, m_bInitialOwner, m_Name.has_value() ? m_Name.value().c_str() : nullptr));
		}
		return Base::Lock(dwTimeout);
	}

private:
	std::optional<std::wstring> m_Name;	// ミューテックスの名前を保持する
	bool m_bInitialOwner = false;
	LPSECURITY_ATTRIBUTES m_pSa = nullptr;
};

/**	スコープから抜けると同時にロックを解除する．

	@date 2007.07.07 genta 新規作成

	@code
	CMutex aMutex;
	
    void function()
    {
        //  other processing
        {
            LockGuard<CMutex> aGuard(aMutex);
            //  aMutex is locked
            //  do something protected by "aMutex"

        } // aMutex is automatically released
        //  other processing
    }
	@endcode
*/
template<class EXCLUSIVE_OBJECT>
class LockGuard {
	using Me = LockGuard< EXCLUSIVE_OBJECT>;

	EXCLUSIVE_OBJECT& o_;
public:
	LockGuard(EXCLUSIVE_OBJECT& ex) : o_( ex ){
		o_.Lock();
	}
	template<class PARAM>
	LockGuard(EXCLUSIVE_OBJECT& ex, PARAM p) : o_( ex ){
		o_.Lock(p);
	}
	LockGuard(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	LockGuard(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~LockGuard() {
		o_.Unlock();
	}
};

#endif /* SAKURA_CMUTEX_51EDDE78_F635_419A_9E10_159485D0F710_H_ */
