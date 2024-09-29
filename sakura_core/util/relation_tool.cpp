/*! @file */
/*
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
#include "StdAfx.h"
#include "relation_tool.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         CSubject                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CSubject::~CSubject()
{
	//リスナを解除
	for (auto listenerRef : m_vListenersRef) {
		_RemoveListener(listenerRef);
	}
	m_vListenersRef.clear();
}

void CSubject::_AddListener(CListener* pcListener)
{
	//既に追加済みなら何もしない
	const auto cend = m_vListenersRef.cend();
	if (const auto found = std::find(m_vListenersRef.cbegin(), cend, pcListener);
		found != cend)
	{
		return;
	}

	//追加
	m_vListenersRef.push_back(pcListener);
}

void CSubject::_RemoveListener(const CListener* pcListener)
{
	const auto cend = m_vListenersRef.cend();
	const auto found = std::find(m_vListenersRef.cbegin(), cend, pcListener);
	if (found != cend) {
		m_vListenersRef.erase(found);
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         CListener                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CSubject* CListener::_Listen(CSubject* pcSubject)
{
	auto pOld = _GetListeningSubject();

	//古いサブジェクトを解除
	m_pcSubjectRef.reset();

	//新しく設定
	if (pcSubject) {
		pcSubject->_AddListener(this);
		m_pcSubjectRef.reset(pcSubject);
	}

	return pOld;
}
