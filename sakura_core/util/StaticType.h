/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_
#define SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_
#pragma once

#include "util/string_ex.h"
#include "debug/Debug2.h"

#include <array>
#include <initializer_list>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

/*!
 * @brief ヒープを用いないvector
 *
 * std::arrayが存在しなかった時代に作られたもの。
 * 生配列の代わりとして使えるよう改造しており、原形を留めていない。
 *
 * @author kobake
 * @date 2007.09.23 kobake 作成
 */
template <class ELEMENT_TYPE, int MAX_SIZE, class SET_TYPE = const ELEMENT_TYPE&>
class StaticVector final {
public:
	//型
	using ElementType = ELEMENT_TYPE;

private:
	using ArrayType = std::array<ElementType, MAX_SIZE>;

	using Me = StaticVector<ElementType, MAX_SIZE, SET_TYPE>;

public:
	static int max_size() noexcept { return MAX_SIZE; }

	StaticVector() = default;

	template<std::ranges::sized_range T>
	constexpr explicit StaticVector(const T& source)
	{
		const auto sourceSize = static_cast<int>(std::size(source));
		if (MAX_SIZE < sourceSize) {
			throw std::out_of_range(std::format("source has too many elements. (elements: {}, allowed: {})", sourceSize, MAX_SIZE));
		}

		m_nCount = sourceSize;

		std::ranges::copy(source, m_aElements.begin());
	}

	constexpr explicit StaticVector(std::initializer_list<ElementType> source)
		: StaticVector(std::span(source.begin(), source.size()))
	{
	}

	//属性
	constexpr int size() const noexcept { return m_nCount; }

	constexpr auto begin() noexcept { return m_aElements.begin(); }
	constexpr auto end() noexcept { return m_aElements.begin() + MAX_SIZE; }

	auto begin() const noexcept { return m_aElements.begin(); }
	auto end() const noexcept { return m_aElements.begin() + m_nCount; }

	constexpr       auto* data()        noexcept { return std::data(m_aElements); }
	constexpr const auto* data()  const noexcept { return std::data(m_aElements); }

	/* implicit */ constexpr operator std::span<ElementType, MAX_SIZE>() & noexcept { return std::span<ElementType, MAX_SIZE>{ data(), MAX_SIZE }; }
	/* implicit */ constexpr operator std::span<ElementType>() & noexcept { return operator std::span<ElementType, MAX_SIZE>(); }
	/* implicit */ constexpr operator const ElementType*() const & noexcept { return data(); }

	//要素アクセス
	ElementType& operator[](size_t nIndex) noexcept
	{
		assert(nIndex <= size_t(MAX_SIZE));

		const auto index = static_cast<int>(nIndex);
		if (m_nCount <= index && index < MAX_SIZE) {
			try {
				resize(index + 1);
			}
			catch (const std::out_of_range&) {
				// 例外を握りつぶす(絶対に発生しない想定)
			}
		}

		return m_aElements[index];
	}

	constexpr const ElementType& operator[](size_t nIndex) const
	{
		if (size_t(MAX_SIZE) <= nIndex) {
			throw std::out_of_range("nIndex is out of range.");
		}

		const auto index = static_cast<int>(nIndex);
		return m_aElements[index];
	}

	//操作
	void clear() noexcept { m_nCount=0; }

	template<typename ... Args>
	void emplace_back(Args&& ...args)
	{
		const auto index = m_nCount;
		resize(index + 1);
		m_aElements[index] = ElementType(std::forward<Args>(args)...);
	}

	void push_back(SET_TYPE e)
	{
		const auto index = m_nCount;
		resize(index + 1);
		m_aElements[index] = e;
	}

	constexpr void resize(size_t nNewSize)
	{
		const auto newSize = static_cast<int>(nNewSize);
		if (MAX_SIZE < newSize) {
			throw std::out_of_range("nNewSize is out of range.");
		}
		m_nCount = newSize;
	}
	
	//! 要素数が0でも要素へのポインタを取得
	ElementType* dataPtr() noexcept { return &m_aElements.front();}

	//特殊
	int& _GetSizeRef(){ return m_nCount; }
	void SetSizeLimit(){
		if( MAX_SIZE < m_nCount ){
			m_nCount = MAX_SIZE;
		}else if( m_nCount < 0 ){
			m_nCount = 0;
		}
	}

private:
	int         m_nCount = 0;
	ArrayType	m_aElements{};
};

/*!
 * @brief ヒープを用いない文字列クラス
 *
 * 格納文字数を制限することによりクラス内だけデータ領域が完結する疑似文字列クラス。
 * C++標準の文字列クラスはヒープ領域にデータを格納するので、共有メモリに配置できない。
 *
 * @author kobake
 * @date 2007.09.23 kobake 作成
 */
template <int N_BUFFER_COUNT>
class StaticString{
private:
	//テンプレート定数名が長過ぎて不便なので、エイリアスを切る
	static constexpr auto N = N_BUFFER_COUNT;

	using ArrayType = std::array<WCHAR, N>;
	using Traits = std::char_traits<WCHAR>;

	using Me = StaticString<N>;

public:
	static constexpr auto BUFFER_COUNT = N_BUFFER_COUNT;

	static constexpr auto size() noexcept { return BUFFER_COUNT; }

	//コンストラクタ・デストラクタ
	StaticString() = default;

	constexpr explicit StaticString(
		std::wstring_view source
	)
	{
		if (STRUNCATE == assign(source)) {
			throw std::out_of_range(std::format("source string is too long. (length:{}, capacity: {})", std::size(source), size()));
		}
	}

	/*!
	 * 文字列を末尾に追加する
	 *
	 * @retval 0 成功
	 * @retval STRUNCATE 切り詰め発生
	 */
	constexpr errno_t append(std::wstring_view src) noexcept
	{
		const auto len = length();
		const auto count = std::min<size_t>(std::size(src), size() - len - 1);
		Traits::move(data() + len, std::data(src), count);
		Traits::assign(data()[len + count], L'\0');
		return count < std::size(src) ? STRUNCATE : 0;
	}

	/*!
	 * 文字列を代入する
	 *
	 * @retval 0 成功
	 * @retval STRUNCATE 切り詰め発生
	 */
	constexpr errno_t assign(std::wstring_view src) noexcept
	{
		const auto count = std::min<size_t>(std::size(src), size() - 1);
		Traits::move(data(), std::data(src), count);
		Traits::assign(data()[count], L'\0');
		return count < std::size(src) ? STRUNCATE : 0;
	}

	/*!
	 * 文字列長を取得する
	 */
	constexpr size_t length() const noexcept
	{
		const auto pos = Traits::find(data(), size(), L'\0');
		return pos ? static_cast<size_t>(pos - data()) : size() - 1;
	}

	constexpr bool empty() const noexcept { return 0 == m_szData[0]; }

	constexpr auto begin() noexcept { return m_szData.begin(); }
	constexpr auto end() noexcept { return m_szData.end() - 1; }

	auto begin() const noexcept { return m_szData.begin(); }
	auto end() const noexcept { return m_szData.begin() + length(); }

	constexpr       WCHAR* data()        noexcept { return std::data(m_szData); }
	constexpr const WCHAR* data()  const noexcept { return std::data(m_szData); }
	constexpr const WCHAR* c_str() const noexcept { return data(); }

	constexpr /* implicit */ operator std::span<WCHAR, N>()       & noexcept { return std::span<WCHAR, N>{ data(), N }; }
	constexpr /* implicit */ operator std::span<WCHAR>()          & noexcept { return operator std::span<WCHAR, N>(); }
	constexpr /* implicit */ operator std::wstring_view()   const & noexcept { return std::wstring_view{ data(), length() }; }

	explicit operator std::filesystem::path() const & noexcept { return static_cast<std::wstring_view>(*this); }

	constexpr Me& operator = (std::wstring_view rhs) noexcept { assign(rhs); return *this; }
	constexpr Me& operator = (const std::filesystem::path& path) noexcept { assign(path.wstring()); return *this; }

	constexpr Me& operator += (std::wstring_view rhs) noexcept { append(rhs); return *this; }

	//クラス属性
	size_t GetBufferCount() const{ return N_BUFFER_COUNT; }

	//データアクセス
	WCHAR*       GetBufferPointer()      { return data(); }
	const WCHAR* GetBufferPointer() const{ return data(); }

	//簡易データアクセス
	constexpr /* implicit */ operator       WCHAR*()       & noexcept { return data(); }
	constexpr /* implicit */ operator const WCHAR*() const & noexcept { return data(); }

	WCHAR At(int nIndex) const{ return m_szData[nIndex]; }

	//簡易コピー
	void Assign(const WCHAR* src) noexcept { assign(std::wstring_view{ src ? src : L"" }); }

	Me& operator = (const WCHAR* src){ Assign(src); return *this; }

	//各種メソッド
	int Length() const noexcept { return static_cast<int>(length()); }

private:
	ArrayType	m_szData{};
};

template<int N> inline errno_t wcscpy_s(StaticString<N>& dst, std::wstring_view src)        noexcept { return dst.assign(src); }
template<int N> inline errno_t wcscat_s(StaticString<N>& dst, std::wstring_view src)        noexcept { return dst.append(src); }

template<int N> inline errno_t wcsncpy_s(StaticString<N>& dst, std::wstring_view src, size_t count) noexcept { if (_TRUNCATE != count && count < std::size(src)) src = src.substr(0, count); return wcscpy_s(dst, src); }
template<int N> inline errno_t wcsncat_s(StaticString<N>& dst, std::wstring_view src, size_t count) noexcept { if (_TRUNCATE != count && count < std::size(src)) src = src.substr(0, count); return wcscat_s(dst, src); }

template<int N>
inline int vswprintf_s(StaticString<N>& buf, const WCHAR* format, va_list& v) noexcept {
	return ::_vsnwprintf_s(std::data(buf), std::size(buf), _TRUNCATE, format, v);
}

template<int N, typename... Params>
inline int swprintf_s(StaticString<N>& buf, const WCHAR* format, Params&&... params) noexcept {
	return ::_snwprintf_s(std::data(buf), _TRUNCATE, std::size(buf), format, std::forward<Params>(params)...);
}

#endif /* SAKURA_STATICTYPE_54CC2BD5_4C7C_4584_B515_EF8C533B90EA_H_ */
