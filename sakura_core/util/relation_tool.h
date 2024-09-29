/*! @file */
/*
	関連の管理
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_RELATION_TOOL_4B723E5C_5042_4F93_8899_EE2077DB8CFE_H_
#define SAKURA_RELATION_TOOL_4B723E5C_5042_4F93_8899_EE2077DB8CFE_H_
#pragma once

class CListener;

//! 複数のCListenerからウォッチされる
class CSubject {
	using Me = CSubject;

public:
	//コンストラクタ・デストラクタ
	CSubject() = default;
	CSubject(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	~CSubject();

	//公開インターフェース
	int GetListenerCount() const { return (int)m_vListenersRef.size(); }

	//管理用
	void _AddListener(CListener* pcListener);
	void _RemoveListener(const CListener* pcListener);

protected:
	CListener* _GetListener(int nIndex) const { return m_vListenersRef[nIndex]; }

private:
	std::vector<CListener*> m_vListenersRef;
};

struct sublect_releaser {
	CListener* pcListener;

	explicit sublect_releaser(CListener& listener)
		: pcListener(&listener)
	{
	}

	void operator ()(CSubject* pcSubject) const {
		pcSubject->_RemoveListener(pcListener);
	}
};

//! 1つのCSubjectをウォッチする
class CListener {
	using Me = CListener;

public:
	CListener() = default;
	CListener(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	~CListener() = default;

protected:
	CSubject* _Listen(CSubject* pcSubject); //!< 直前にウォッチしていたサブジェクトを返す
	CSubject* _GetListeningSubject() const { return m_pcSubjectRef.get(); }

private:
	using CSubjectHolder = std::unique_ptr<CSubject, sublect_releaser>;
	CSubjectHolder m_pcSubjectRef = CSubjectHolder(static_cast<CSubject*>(nullptr), sublect_releaser(*this));
};

template <class LISTENER> class CSubjectT : public CSubject {
public:
	LISTENER* GetListener(int nIndex) const {
		return static_cast<LISTENER*>(CSubject::_GetListener(nIndex));
	}
};

template <class SUBJECT, std::enable_if_t<std::is_base_of_v<CSubject, SUBJECT>, int> = 0>
class CListenerT : public CListener {
public:
	SUBJECT* Listen(SUBJECT* pcSubject) {
		return static_cast<SUBJECT*>(CListener::_Listen(static_cast<SUBJECT*>(pcSubject)));
	}
	SUBJECT* GetListeningSubject() const {
		return static_cast<SUBJECT*>(CListener::_GetListeningSubject());
	}
};

#endif /* SAKURA_RELATION_TOOL_4B723E5C_5042_4F93_8899_EE2077DB8CFE_H_ */
