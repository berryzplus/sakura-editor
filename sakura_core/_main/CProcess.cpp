/*!	@file
	@brief プロセス基底クラス

	@author aroka
	@date 2002/01/07 作成
	@date 2002/01/17 修正
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2004, Moca
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CProcess.h"

#include "config/app_constants.h"
#include "config/system_constants.h"
#include "env/CSakuraEnvironment.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "io/CTextStream.h"
#include "util/module.h"

#include "CSelectLang.h"

/*!
 * @brief 編集ウィンドウを列挙するコールバック関数
 *
 * @param hWnd 列挙されたウィンドウのハンドル
 * @param lParam 列挙の呼び出し元から渡されたパラメータ（HWND* を期待）
 * @retval TRUE 列挙続行（編集ウィンドウではなかった。）
 * @retval FALSE 列挙停止（編集ウィンドウが見付かった。）
 */
BOOL CALLBACK EnumEditorWindowProc(
	_In_ HWND   hWnd,
	_In_ LPARAM lParam
)
{
	if (!hWnd || !lParam || !IsSakuraMainWindow(hWnd)) return TRUE;	// 検索続行

	auto phWndFound = std::bit_cast<HWND*>(lParam);

	*phWndFound = hWnd;

	return FALSE;
}

std::vector<std::wstring> SplitLegacyCommandLine(std::wstring_view s)
{
	std::vector<std::wstring> args;

	// 正規表現パターン:
	//   ^(?:-\w+[=:])?  … オプション接頭辞 (-XXX= または -XXX:) は省略可能
	//   "                … 開き引用符
	//   (?:              … 引用符内文字の繰り返し
	//     ""             …   "" → " のエスケープ
	//     | \"           …   \" → " のエスケープ
	//     | \\(?!")      …   " が後続しない単独 \ (リテラル)
	//     | [^"\\]       …   " と \ 以外の通常文字
	//   )*
	//   "                … 閉じ引用符
	std::wregex re(LR"(^(?:-\w+[=:])?"(?:""|\\"|\\(?!")|[^"\\])*")");

	while (!s.empty()) {
		// 先頭の空白を読み飛ばす
		if (const auto p0 = s.find_first_not_of(L' '); std::wstring_view::npos == p0) {
			s = std::wstring_view{};	// 残余が空白のみ → 処理終了
			continue;
		} else if (p0) {
			s = s.substr(p0);
		}

		// 次の区切り文字を探す
		const auto p1 = s.find_first_of(L' ');

		if (std::wstring_view::npos == p1) {
			args.emplace_back(s);	// 残り全部を1つの引数とみなす → 処理終了
			s = std::wstring_view{};
			continue;
		}

		// 引用符を探す
		if (const auto p2 = s.find_first_of(L'"'); std::wstring_view::npos == p2 || p1 < p2) {
			args.emplace_back(s.substr(0, p1));
			s = s.substr(p1);
			continue;
		}

		// 引用符の正規表現とマッチさせる
		std::match_results<std::wstring_view::const_iterator> m;
		if (!std::regex_search(s.begin(), s.end(), m, re)) {
			args.emplace_back(s);	// 残り全部を1つの引数とみなす → 処理終了
			s = std::wstring_view{};
			continue;
		}

		args.emplace_back(s.substr(0, m.length()));
		s = s.substr(m.length());
	}

	return args;
}

/*!
 * WM_CREATEハンドラ
 *
 * WM_CREATEはCreateWindowEx関数によるウインドウ作成中にポストされます。
 * メッセージの戻り値はウインドウの作成を続行するかどうかの判断に使われます。
 *
 * @retval true  ウィンドウの作成を続行する
 * @retval false ウィンドウの作成を中止する
 */
bool CAppMainWnd::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	if (!hWnd || !lpCreateStruct) {
		return false;
	}

	const auto hInstance = lpCreateStruct->hInstance;
	assert(hInstance);

	m_hIcons.Create(hInstance);
	m_cMenuDrawer.Create(CSelectLang::getLangRsrcInstance(), hWnd, &m_hIcons);

	m_pcPropertyManager->Create(hWnd, &m_hIcons, &m_cMenuDrawer);

	return true;
}

/*!
 * @brief サクラエディタのプロセスを起動する
 *
 * @param si スタートアップ情報
 * @param args コマンドライン引数
 * @param optWorkingDir カレントディレクトリ（省略した場合は起動元と同じ）
 * @param optProfileName プロファイル名（省略した場合は指定なし）
 * @return 起動したプロセスのハンドルオブジェクト
 */
/* static */ cxx::ProcessHolder CProcess::CreateSakuraProcess(
	STARTUPINFO& si,
	std::vector<std::wstring>& args,
	const std::optional<std::filesystem::path>& optWorkingDir,
	const std::optional<std::wstring>& optProfileName
)
{
	// プロファイル指定がある場合、argsに追加する
	if (const auto pCommandLine = CCommandLine::getInstance(); (pCommandLine && pCommandLine->IsSetProfile()) || optProfileName.has_value()) {
		const auto profileName = optProfileName.value_or(GetProfileName());
		args.emplace(args.begin(), std::format(LR"(-PROF={})", CCommandLine::QuoteArg(profileName)));
	}

	// 実行ファイルのパスを取得する
	const auto exePath = GetExeFileName();

	// 引数を空白区切りで連結する
	auto strCommandLine = std::accumulate(args.begin(), args.end(), CCommandLine::QuoteArg(exePath.native()), [](const std::wstring& a, std::wstring_view b) { return std::format(LR"({} {})", a, CCommandLine::QuoteArg(b)); });

	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;

	LPCWSTR lpszWorkingDir = nullptr;
	if (optWorkingDir.has_value()) {
		if (const auto attr = ::GetFileAttributesW(optWorkingDir->c_str());
			INVALID_FILE_ATTRIBUTES != attr && (attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			lpszWorkingDir = optWorkingDir->c_str();
		}
	}

	// プロセス情報（出力用構造体なので値は入れない）
	PROCESS_INFORMATION pi{};

	// コントロールプロセスを起動する
	if (!::CreateProcessW(
		exePath.c_str(),		// 実行可能モジュールパス
		strCommandLine.data(),	// コマンドラインバッファ
		nullptr,				// プロセスのセキュリティ記述子
		nullptr,				// スレッドのセキュリティ記述子
		FALSE,					// ハンドルの継承オプション(継承させない)
		dwCreationFlag,			// 作成のフラグ
		nullptr,				// 環境変数(変更しない)
		lpszWorkingDir,			// カレントディレクトリ(変更しない)
		&si,					// スタートアップ情報
		&pi						// プロセス情報(作成されたプロセス情報を格納する構造体)
	))
	{
		cxx::raise_system_error("create process failed.");
	}

	// 開いたハンドルは使わないので閉じておく
	::CloseHandle(pi.hThread);

	return cxx::ProcessHolder{ pi.hProcess, pi.dwProcessId, pi.dwThreadId };
}

/*!
 * @brief コントロールプロセスを起動する
 *
 * @param optProfileName プロファイル名
 * @return 起動したプロセスオブジェクト
 */
/* static */ cxx::ProcessHolder CProcess::CreateControlProcess(
	const std::optional<std::wstring>& optProfileName
)
{
	std::wstring profileName{ optProfileName.value_or(GetProfileName()) };

	// トレイウインドウのクラス名を組み立てる
	std::wstring trayWndClassName{ GSTR_CEDITAPP };
	trayWndClassName += profileName;

	// トレイウインドウを検索する
	if (const auto hTrayWnd = cxx::FindWindowW(trayWndClassName, trayWndClassName)) {
		// トレイウインドウからプロセスIDを取得する
		DWORD dwControlProcessId;
		if (DWORD dwControlThreadId = ::GetWindowThreadProcessId(hTrayWnd, &dwControlProcessId)) {
			if (const auto process = ::OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwControlProcessId)) {
				return cxx::ProcessHolder{ process, dwControlProcessId, dwControlThreadId };
			}
		}
	}

	// 初期化完了イベントの名前を決める
	SFilePath initEventName{ GSTR_EVENT_SAKURA_CP_INITIALIZED };
	initEventName += profileName;

	// プロセス起動前に初期化完了イベントを作成する
	cxx::HandleHolder hEvent = ::CreateEventW(nullptr, TRUE, FALSE, initEventName);
	if (!hEvent || ERROR_ALREADY_EXISTS == ::GetLastError()) {
		cxx::raise_system_error("create event failed.");
	}

	std::vector<std::wstring> commandArgs{ LR"(-NOWIN)" };

	std::wstring title{ L"sakura control process" };

	// スタートアップ情報（入力用構造体なので値を入れる）
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.lpTitle = std::data(title);
	si.wShowWindow = SW_SHOWDEFAULT;

	// コントロールプロセスを起動する
	auto cp = CreateSakuraProcess(si, commandArgs, cxx::GetSystemDirectoryW(), optProfileName);
	if (!cp) return cxx::ProcessHolder{};

	// 初期化完了を待つ
	std::array handles{ hEvent.get(), cp.get() };
	if (const auto dwRet = ::WaitForMultipleObjects(DWORD(std::size(handles)), std::data(handles), FALSE, 15000); WAIT_OBJECT_0 != dwRet) {
		cxx::raise_system_error("waitEvent is timeout.");
	}

	return cxx::ProcessHolder{ cp.release(), cp.dwProcessId, cp.dwThreadId };
}

/*!
 * @brief エディタープロセスを起動する
 *
 * @param args コマンドライン引数
 * @param optWorkingDir カレントディレクトリ（省略した場合は起動元と同じ）
 * @param optProfileName プロファイル名
 * @return 起動したプロセスオブジェクト
 */
/* static */ cxx::EditorProcessHolder CProcess::CreateEditorProcess(
	std::vector<std::wstring>& args,
	std::wstring_view cmdLineoptions,
	const std::optional<std::filesystem::path>& optWorkingDir,
	const std::optional<std::wstring>& optProfileName,
	bool bNewWindow,
	bool sync
)
{
	// グループID
	if (const auto pShareData = GetDllShareDataPtr(); pShareData && pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin) {
		if (bNewWindow) {	// 新規エディタをウインドウで開く
			// 空いているグループIDを使用する
			args.emplace_back(std::format(L"-GROUP={:d}", CAppNodeManager::getInstance()->GetFreeGroupId()));
		} else {
			HWND hWndFound = nullptr;

			// スレッドに含まれるウインドウを列挙する
			::EnumThreadWindows(::GetCurrentThreadId(), EnumEditorWindowProc, LPARAM(&hWndFound));

			// グループIDを親ウィンドウから取得
			if (const auto pNode = CAppNodeManager::getInstance()->GetEditNode(hWndFound); pNode && 0 < pNode->GetGroup()) {
				args.emplace_back(std::format(L"-GROUP={:d}", static_cast<int>(pNode->GetGroup())));
			}
		}
	}

	// 追加のコマンドラインオプション
	CTextOutputStream output{};

	if (!cmdLineoptions.empty()) {
		// Grepなどで入りきらない場合はレスポンスファイルを利用する
		CCommandLineString cCmdLineBuf;
		if (cCmdLineBuf.max_size() < cmdLineoptions.length()) {
			output = CTextOutputStream::CreateTempFile(L"skr_resp", ::GetIniFileName().remove_filename());
			if (!output) {
				ErrorMessage(nullptr, LS(STR_TRAY_RESPONSEFILE));
				return cxx::EditorProcessHolder{};
			}

			// 出力
			output.WriteString(std::data(cmdLineoptions), int(std::size(cmdLineoptions)));

			args.emplace_back(std::format(LR"(-@=\"{:s}\")", output.GetPath().c_str()));

		} else {
			// 追加のコマンドライン文字列をバラして配列の末尾に追加する
			auto opts = SplitLegacyCommandLine(cmdLineoptions);
			args.insert(
				args.end(),
				std::make_move_iterator( opts.begin() ),
				std::make_move_iterator( opts.end() ) );
		}
	}

	// スタートアップ情報（入力用構造体なので値を入れる）
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWDEFAULT;

	// エディタープロセスを起動する
	auto ep = CProcess::CreateSakuraProcess(si, args, optWorkingDir, optProfileName);
	if (!ep) return cxx::EditorProcessHolder{};

	output.Close();

	// 初期化完了イベントを作成する
	SFilePath initEventName{ std::format(GSTR_EVENT_SAKURA_EP_INITIALIZED, ep.dwThreadId) };
	cxx::HandleHolder hEvent = ::CreateEventW(nullptr, TRUE, FALSE, initEventName);

	const auto startTick = ::GetTickCount64();

	// メインウインドウを取得する
	HWND hWndFound = nullptr;
	do {
		Sleep(100);  // 100msスリープしてリトライ

		// スレッドに含まれるウインドウを列挙する
		::EnumThreadWindows(ep.dwThreadId, EnumEditorWindowProc, LPARAM(&hWndFound));
	}
	while (sync && !hWndFound && ::GetTickCount64() - startTick < 30000);

	// 初期化完了を待つ
	if (sync && hWndFound) hEvent.lock();

	// プロセスオブジェクトを返す
	return cxx::EditorProcessHolder{ ep.release(), ep.dwProcessId, ep.dwThreadId, hWndFound };
}

/*!
	@brief プロセス基底クラス
	
	@author aroka
	@date 2002/01/07
*/
CProcess::CProcess(
	HINSTANCE				hInstance,		//!< handle to process instance
	CCommandLineHolder&&	pCommandLine	//!< pointer to command line
)
	: m_hInstance(hInstance)
	, m_pCommandLine(std::move(pCommandLine))
{
}

/*!
	@brief iniファイルパスを取得する
 */
std::filesystem::path CProcess::GetIniFileName() const
{
	if (m_cShareData.IsPrivateSettings()) {
		const DLLSHAREDATA *pShareData = &GetDllShareData();
		return pShareData->m_szPrivateIniFile.c_str();
	}
	return GetExeFileName().replace_extension(L".ini");
}

/*!
 * @brief プロセス実行
 *
 * @param nCmdShow [in] コマンドライン引数の表示オプション
 * @return PostQuitMessage()で指定した終了コード
 * 
 * @author aroka
 * @date 2002/01/16 新規作成
 */
int CProcess::Run(int nCmdShow)
{
	if (InitializeProcess(nCmdShow)) {
		if (const auto pcMainWnd = GetMainWnd()) {
			return pcMainWnd->MessageLoop();
		}
	}

	return 0;
}

/*!
	言語選択後に共有メモリ内の文字列を更新する
*/
void CProcess::RefreshString()
{
	m_cShareData.RefreshString();
}
