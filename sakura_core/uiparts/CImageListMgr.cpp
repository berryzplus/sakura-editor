/*!	@file
	@brief ImageListの取り扱い

	@author genta
	@date Oct. 11, 2000 genta
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2000, jepro
	Copyright (C) 2001, GAE, jepro
	Copyright (C) 2003, Moca, genta, wmlhq
	Copyright (C) 2007, ryoji
	Copyright (C) 2010, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "uiparts/CImageListMgr.h"

#include "env/CommonSetting.h"
#include "util/module.h"
#include "debug/CRunningTimer.h"
#include "sakura_rc.h"
#include "config/system_constants.h"

//  2010/06/29 syat MAX_X, MAX_Yの値をCommonSettings.hに移動
//	Jul. 21, 2003 genta 他でも使うので関数の外に出した
//	Oct. 21, 2000 JEPRO 設定
const int MAX_X = MAX_TOOLBAR_ICON_X;
const int MAX_Y = MAX_TOOLBAR_ICON_Y;	//2002.01.17

// 加算演算子（仮定義）
constexpr EFunctionCode operator + (EFunctionCode a, int b) {
	return EFunctionCode(WORD(a) + b);
}

//	キーワード：アイコン順序(アイコンインデックス)
//	Sept. 16, 2000 Jepro note: アイコン登録メニュー
//	以下の登録はツールバーだけでなくアイコンをもつすべてのメニューで利用されている
//	数字はビットマップリソースのIDB_MYTOOLに登録されているアイコンの先頭からの順番のようである
//	アイコンをもっと登録できるように横幅を16dotsx218=2048dotsに拡大
//	縦も15dotsから16dotsにして「プリンター」アイコンや「ヘルプ1」の、下が欠けている部分を補ったが15dotsまでしか表示されないらしく効果なし
//	→
//	Sept. 17, 2000 縦16dot目を表示できるようにした
//	修正したファイルにはJEPRO_16thdotとコメントしてあるのでもし間違っていたらそれをキーワードにして検索してください(Sept. 28, 2000現在 6箇所変更)
//	IDB_MYTOOLの16dot目に見やすいように横16dotsづつの区切りになる目印を付けた
//
//	Sept. 16, 2000 見やすいように横に20個(あるいは32個)づつに配列しようとしたが配列構造を変えなければうまく格納できないので
//	それを解決するのが先決(→げんた氏改修版ur3β13で解決)
//
//	Sept. 16, 2000 JEPRO できるだけ系ごとに集まるように順番を大幅に入れ替えた  それに伴いCShareData.cppで設定している初期設定値も変更
//	Oct. 22, 2000 JEPRO アイコンのビットマップリソースの2次元配置が可能になったため根本的に配置転換した
//	・配置の基本は「コマンド一覧」に入っている機能(コマンド)順	なお「コマンド一覧」自体は「メニューバー」の順におおよそ準拠している
//	・アイコンビットマップファイルには横32個X15段ある(2010.06.26 13段から拡張)
//	・互換性と新コマンド追加の両立の都合で飛び地あり
//	・メニューに属する系および各系の段との関係は次の通り(2012.03.10 現在)：
//		ファイル----- ファイル操作系	(1段目32個: 1-32)
//		編集--------- 編集系			(2段目32個: 33-64)
//		移動--------- カーソル移動系	(3段目32個: 65-96)
//		選択--------- 選択系			(4段目32個: 97-128)
//					+ 矩形選択系		(5段目32個: 129-160) //(注. 矩形選択系のほとんどは未実装)
//					+ クリップボード系	(6段目24個: 161-184)
//			★挿入系					(6段目残りの8個: 185-192)
//		変換--------- 変換系			(7段目32個: 193-224)
//		検索--------- 検索系			(8段目32個: 225-256)
//		ツール------- モード切り替え系	(9段目4個: 257-260)
//					+ 設定系			(9段目次の16個: 261-276)
//					+ マクロ系			(9段目最後の11個: 277-287)
//					+ 外部マクロ		(12段目32個: 353-384/13段目19個: 385-403)
//					+ カスタムメニュー	(10段目25個: 289-313)
//		ウィンドウ--- ウィンドウ系		(11段目22個: 321-342)
//					+ タブ系			(10段目残りの7個: 314-320/9段目最期の1個: 288)
//		ヘルプ------- 支援				(11段目残りの10個: 343-352)
//	注1.「挿入系」はメニューでは「編集」に入っている
//	注2.「コマンド一覧」に入ってないコマンドもわかっている範囲で位置予約にしておいた
//  注3. F_DISABLE は未定義用(ダミーとしても使う)
//	注4. ユーザー用に確保された場所は特にないので各段の空いている後ろの方を使ってください。
//	注5. アイコンビットマップの有効段数は、CImageListMgr の MAX_Y です。

std::array<EFunctionCode, MAX_TOOLBAR_ICON_COUNT> CImageListMgr::gm_toolIcons = {
/*  0 */	F_SEPARATOR,				//区切り線

/* ファイル操作系(1段目32個: 1-32) */
/*  1 */	F_FILENEW,					//新規作成
/*  2 */	F_FILEOPEN,					//開く
/*  3 */	F_FILESAVE,					//上書き保存
/*  4 */	F_FILESAVEAS_DIALOG,		//名前を付けて保存
/*  5 */	F_FILECLOSE,				//閉じて(無題)
/*  6 */	F_FILECLOSE_OPEN,			//閉じて開く
/*  7 */	F_FILE_REOPEN_SJIS,			//SJISで開き直す
/*  8 */	F_FILE_REOPEN_JIS,			//JISで開き直す
/*  9 */	F_FILE_REOPEN_EUC,			//EUCで開き直す
/* 10 */	F_FILE_REOPEN_UNICODE,		//Unicodeで開き直す
/* 11 */	F_FILE_REOPEN_UTF8,			//UTF-8で開き直す
/* 12 */	F_FILE_REOPEN_UTF7,			//UTF-7で開き直す
/* 13 */	F_PRINT,					//印刷
/* 14 */	F_PRINT_PREVIEW,			//印刷プレビュー
/* 15 */	F_PRINT_PAGESETUP,			//印刷ページ設定
/* 16 */	F_OPEN_HfromtoC,			//同名のC/C++ヘッダー(ソース)を開く
/* 17 */	F_0,						//同名のC/C++ヘッダーファイルを開く
/* 18 */	F_0,						//同名のC/C++ソースファイルを開く
/* 19 */	F_ACTIVATE_SQLPLUS,			//Oracle SQL*Plusをアクティブ表示 */
/* 20 */	F_PLSQL_COMPILE_ON_SQLPLUS,	//Oracle SQL*Plusで実行 */
/* 21 */	F_BROWSE,					//ブラウズ
/* 22 */	F_PROPERTY_FILE,			//ファイルのプロパティ
/* 23 */	F_VIEWMODE,					//ビューモード
/* 24 */	F_FILE_REOPEN_UNICODEBE,	//UnicodeBEで開き直す
/* 25 */	F_FILEOPEN_DROPDOWN,		//開く(ドロップダウン)
/* 26 */	F_FILE_REOPEN,				//開きなおす
/* 27 */	F_EXITALL,					//サクラエディタの全終了
/* 28 */	F_FILESAVECLOSE,			//保存して閉じる
/* 29 */	F_FILENEW_NEWWINDOW,		//新規ウインドウを開く
/* 30 */	F_FILESAVEALL,				//全て上書き保存
/* 31 */	F_EXITALLEDITORS,			//編集の全終了
/* 32 */	F_FILE_REOPEN_CESU8,		//CESU-8で開きなおす

/* 編集系(2段目32個: 32-64) */
/* 33 */	F_UNDO,						//元に戻す(Undo)
/* 34 */	F_REDO,						//やり直し(Redo)
/* 35 */	F_DELETE,					//削除
/* 36 */	F_DELETE_BACK,				//カーソル前を削除
/* 37 */	F_WordDeleteToStart,		//単語の左端まで削除
/* 38 */	F_WordDeleteToEnd,			//単語の右端まで削除
/* 39 */	F_WordDelete,				//単語削除
/* 40 */	F_WordCut,					//単語切り取り
/* 41 */	F_LineDeleteToStart,		//行頭まで削除(改行単位)
/* 42 */	F_LineDeleteToEnd,			//行末まで削除(改行単位)
/* 43 */	F_LineCutToStart,			//行頭まで切り取り(改行単位)
/* 44 */	F_LineCutToEnd,				//行末まで切り取り(改行単位)
/* 45 */	F_DELETE_LINE,				//行削除(折り返し単位)
/* 46 */	F_CUT_LINE,					//行切り取り(改行単位)
/* 47 */	F_DUPLICATELINE,			//行の二重化(折り返し単位)
/* 48 */	F_INDENT_TAB,				//TABインデント
/* 49 */	F_UNINDENT_TAB,				//逆TABインデント
/* 50 */	F_INDENT_SPACE,				//SPACEインデント
/* 51 */	F_UNINDENT_SPACE,			//逆SPACEインデント
/* 52 */	F_0/*F_WORDSREFERENCE*/,	//単語リファレンス
/* 53 */	F_LTRIM,					//LTRIM
/* 54 */	F_RTRIM,					//RTRIM
/* 55 */	F_SORT_ASC,					//SORT_ASC
/* 56 */	F_SORT_DESC,				//SORT_DES
/* 57 */	F_MERGE,					//MERGE
/* 58 */	F_RECONVERT,				//再変換
/* 59 */	F_0,						//ダミー
/* 60 */	F_0,						//ダミー
/* 61 */	F_0,						//ダミー
/* 62 */	F_0,						//ダミー
/* 63 */	F_PROFILEMGR,				//プロファイルマネージャ
/* 64 */	F_FILE_REOPEN_LATIN1,		//Latin1で開き直す

/* カーソル移動系(3段目32個: 65-96) */
/* 65 */	F_OPEN_FOLDER_IN_EXPLORER,	//ファイルの場所を開く
/* 66 */	F_OPEN_COMMAND_PROMPT,		//コマンドプロンプトを開く
/* 67 */	F_OPEN_POWERSHELL,			//PowerShellを開く
/* 68 */	F_UP,						//カーソル上移動
/* 69 */	F_DOWN,						//カーソル下移動
/* 70 */	F_LEFT,						//カーソル左移動
/* 71 */	F_RIGHT,					//カーソル右移動
/* 72 */	F_UP2,						//カーソル上移動(２行ごと)
/* 73 */	F_DOWN2,					//カーソル下移動(２行ごと)
/* 74 */	F_WORDLEFT,					//単語の左端に移動
/* 75 */	F_WORDRIGHT,				//単語の右端に移動
/* 76 */	F_GOLINETOP,				//行頭に移動(折り返し単位)
/* 77 */	F_GOLINEEND,				//行末に移動(折り返し単位)
/* 78 */	F_HalfPageUp,				//半ページアップ
/* 79 */	F_HalfPageDown,				//半ページダウン
/* 80 */	F_1PageUp,					//１ページアップ
/* 81 */	F_1PageDown,				//１ページダウン
/* 82 */	F_OPEN_COMMAND_PROMPT_AS_ADMIN,	//管理者としてコマンドプロンプトを開く
/* 83 */	F_OPEN_POWERSHELL_AS_ADMIN,	//管理者としてPowerShellを開く
/* 84 */	F_GOFILETOP,				//ファイルの先頭に移動
/* 85 */	F_GOFILEEND,				//ファイルの最後に移動
/* 86 */	F_CURLINECENTER,			//カーソル行をウィンドウ中央へ
/* 87 */	F_JUMPHIST_PREV,			//移動履歴: 前へ
/* 88 */	F_JUMPHIST_NEXT,			//移動履歴: 次へ
/* 89 */	F_WndScrollDown,			//テキストを１行下へスクロール
/* 90 */	F_WndScrollUp,				//テキストを１行上へスクロール
/* 91 */	F_GONEXTPARAGRAPH,			//次の段落へ
/* 92 */	F_GOPREVPARAGRAPH,			//前の段落へ
/* 93 */	F_JUMPHIST_SET,				//現在位置を移動履歴に登録
/* 94 */	F_MODIFYLINE_PREV,			//前の変更行へ
/* 95 */	F_MODIFYLINE_NEXT,			//次の変更行へ
/* 96 */	F_0,						//ダミー

/* 選択系(4段目32個: 97-128) */
/* 97 */	F_SELECTWORD,				//現在位置の単語選択
/* 98 */	F_SELECTALL,				//すべて選択
/* 99 */	F_BEGIN_SEL,				//範囲選択開始
/* 100 */	F_UP_SEL,					//(範囲選択)カーソル上移動
/* 101 */	F_DOWN_SEL,					//(範囲選択)カーソル下移動
/* 102 */	F_LEFT_SEL,					//(範囲選択)カーソル左移動
/* 103 */	F_RIGHT_SEL,				//(範囲選択)カーソル右移動
/* 104 */	F_UP2_SEL,					//(範囲選択)カーソル上移動(２行ごと)
/* 105 */	F_DOWN2_SEL,				//(範囲選択)カーソル下移動(２行ごと)
/* 106 */	F_WORDLEFT_SEL,				//(範囲選択)単語の左端に移動
/* 107 */	F_WORDRIGHT_SEL,			//(範囲選択)単語の右端に移動
/* 108 */	F_GOLINETOP_SEL,			//(範囲選択)行頭に移動(折り返し単位)
/* 109 */	F_GOLINEEND_SEL,			//(範囲選択)行末に移動(折り返し単位)
/* 110 */	F_HalfPageUp_Sel,			//(範囲選択)半ページアップ
/* 111 */	F_HalfPageDown_Sel,			//(範囲選択)半ページダウン
/* 112 */	F_1PageUp_Sel,				//(範囲選択)１ページアップ
/* 113 */	F_1PageDown_Sel,			//(範囲選択)１ページダウン
/* 114 */	F_0/*F_DISPLAYTOP_SEL*/,	//(範囲選択)画面の先頭に移動(未実装)
/* 115 */	F_0/*F_DISPLAYEND_SEL*/,	//(範囲選択)画面の最後に移動(未実装)
/* 116 */	F_GOFILETOP_SEL,			//(範囲選択)ファイルの先頭に移動
/* 117 */	F_GOFILEEND_SEL,			//(範囲選択)ファイルの最後に移動
/* 118 */	F_GONEXTPARAGRAPH_SEL,		//(範囲選択)次の段落へ
/* 119 */	F_GOPREVPARAGRAPH_SEL,		//(範囲選択)前の段落へ
/* 120 */	F_SELECTLINE,				//1行選択
/* 121 */	F_FUNCLIST_PREV,			//前の関数リストマーク
/* 122 */	F_FUNCLIST_NEXT,			//次の関数リストマーク
/* 123 */	F_0,						//ダミー
/* 124 */	F_0,						//ダミー
/* 125 */	F_0,						//ダミー
/* 126 */	F_MODIFYLINE_PREV_SEL,		//(範囲選択)前の変更行へ
/* 127 */	F_MODIFYLINE_NEXT_SEL,		//(範囲選択)次の変更行へ
/* 128 */	F_0,	//ダミー

/* 矩形選択系(5段目32個: 129-160) */ //(注. 矩形選択系のほとんどは未実装)
/* 129 */	F_0,						//ダミー
/* 130 */	F_0/*F_BOXSELALL*/,			//矩形ですべて選択
/* 131 */	F_BEGIN_BOX,				//矩形範囲選択開始
/* 132 */	F_UP_BOX,					//(矩形選択)カーソル上移動
/* 133 */	F_DOWN_BOX,					//(矩形選択)カーソル下移動
/* 134 */	F_LEFT_BOX,					//(矩形選択)カーソル左移動
/* 135 */	F_RIGHT_BOX,				//(矩形選択)カーソル右移動
/* 136 */	F_UP2_BOX,					//(矩形選択)カーソル上移動(２行ごと)
/* 137 */	F_DOWN2_BOX,				//(矩形選択)カーソル下移動(２行ごと)
/* 138 */	F_WORDLEFT_BOX,				//(矩形選択)単語の左端に移動
/* 139 */	F_WORDRIGHT_BOX,			//(矩形選択)単語の右端に移動
/* 140 */	F_GOLINETOP_BOX,			//(矩形選択)行頭に移動(折り返し単位)
/* 141 */	F_GOLINEEND_BOX,			//(矩形選択)行末に移動(折り返し単位)
/* 142 */	F_HalfPageUp_BOX,			//(矩形選択)半ページアップ
/* 143 */	F_HalfPageDown_BOX,			//(矩形選択)半ページダウン
/* 144 */	F_1PageUp_BOX,				//(矩形選択)１ページアップ
/* 145 */	F_1PageDown_BOX,			//(矩形選択)１ページダウン
/* 146 */	F_0/*F_DISPLAYTOP_BOX*/,	//(矩形選択)画面の先頭に移動(未実装)
/* 147 */	F_0/*F_DISPLAYEND_BOX*/,	//(矩形選択)画面の最後に移動(未実装)
/* 148 */	F_GOFILETOP_BOX,			//(矩形選択)ファイルの先頭に移動
/* 149 */	F_GOFILEEND_BOX,			//(矩形選択)ファイルの最後に移動
/* 150 */	F_GOLOGICALLINETOP_BOX,		//(矩形選択)行頭に移動(改行単位)
/* 151 */	F_0/*F_GOLOGICALLINEEND_BOX*/,	//ダミー
/* 152 */	F_0,						//ダミー
/* 153 */	F_0,						//ダミー
/* 154 */	F_0,						//ダミー
/* 155 */	F_0,						//ダミー
/* 156 */	F_0,						//ダミー
/* 157 */	F_0,						//ダミー
/* 158 */	F_0,						//ダミー
/* 159 */	F_0,						//ダミー
/* 160 */	F_0,						//ダミー

/* クリップボード系(6段目24個: 161-184) */
/* 161 */	F_CUT,						//切り取り(選択範囲をクリップボードにコピーして削除)
/* 162 */	F_COPY,						//コピー(選択範囲をクリップボードにコピー)
/* 163 */	F_COPY_CRLF,				//CRLF改行でコピー
/* 164 */	F_PASTE,					//貼り付け(クリップボードから貼り付け)
/* 165 */	F_PASTEBOX,					//矩形貼り付け(クリップボードから貼り付け)
/* 166 */	F_0/*F_INSTEXT_W*/,			//テキストを貼り付け	(未公開コマンド？未完成？)
/* 167 */	F_0/*F_ADDTAIL_W*/,			//最後にテキストを追加	(未公開コマンド？未完成？)
/* 168 */	F_COPYLINES,				//選択範囲内全行コピー
/* 169 */	F_COPYLINESASPASSAGE,		//選択範囲内全行引用符付きコピー
/* 170 */	F_COPYLINESWITHLINENUMBER,	//選択範囲内全行行番号付きコピー
/* 171 */	F_COPYPATH,					//このファイルのパス名をコピー
/* 172 */	F_COPYTAG,					//このファイルのパス名とカーソル位置をコピー
/* 173 */	F_CREATEKEYBINDLIST,		//キー割り当て一覧をコピー
/* 174 */	F_COPYFNAME,				//このファイル名をクリップボードにコピー
/* 175 */	F_COPY_ADDCRLF,				//折り返し位置に改行をつけてコピー
/* 176 */	F_COPY_COLOR_HTML,			//選択範囲内色付きHTMLコピー
/* 177 */	F_COPY_COLOR_HTML_LINENUMBER,	//選択範囲内行番号色付きHTMLコピー
/* 178 */	F_COPYDIRPATH,				//このファイルのフォルダー名をクリップボードにコピー
/* 179 */	F_0,						//ダミー
/* 180 */	F_0,						//ダミー
/* 181 */	F_0,						//ダミー
/* 182 */	F_CHGMOD_EOL_CRLF,
/* 183 */	F_CHGMOD_EOL_LF,
/* 184 */	F_CHGMOD_EOL_CR,

/* 挿入系(6段目残り8個: 185-192) */
/* 185 */	F_INS_DATE,					//日付挿入
/* 186 */	F_INS_TIME,					//時刻挿入
/* 187 */	F_CTRL_CODE_DIALOG,			//コントロールコードの入力(ダイアログ)
/* 188 */	F_INS_FILE_USED_RECENTLY,	//最近使ったファイル挿入
/* 189 */	F_INS_FOLDER_USED_RECENTLY,	//最近使ったフォルダー挿入
/* 190 */	F_0,						//ダミー
/* 191 */	F_0,						//ダミー
/* 192 */	F_0,						//ダミー

/* 変換系(7段目32個: 193-224) */
/* 193 */	F_TOLOWER,					//小文字
/* 194 */	F_TOUPPER,					//大文字
/* 195 */	F_TOHANKAKU,				//全角→半角
/* 196 */	F_TOZENKAKUKATA,			//半角＋全ひら→全角・カタカナ
/* 197 */	F_TOZENKAKUHIRA,			//半角＋全カタ→全角・ひらがな
/* 198 */	F_HANKATATOZENKATA,			//半角カタカナ→全角カタカナ
/* 199 */	F_HANKATATOZENHIRA,			//半角カタカナ→全角ひらがな
/* 200 */	F_TABTOSPACE,				//TAB→空白
/* 201 */	F_CODECNV_AUTO2SJIS,		//自動判別→SJISコード変換
/* 202 */	F_CODECNV_EMAIL,			//E-Mail(JIS→SIJIS)コード変換
/* 203 */	F_CODECNV_EUC2SJIS,			//EUC→SJISコード変換
/* 204 */	F_CODECNV_UNICODE2SJIS,		//Unicode→SJISコード変換
/* 205 */	F_CODECNV_UTF82SJIS,		//UTF-8→SJISコード変換
/* 206 */	F_CODECNV_UTF72SJIS,		//UTF-7→SJISコード変換
/* 207 */	F_CODECNV_SJIS2JIS,			//SJIS→JISコード変換
/* 208 */	F_CODECNV_SJIS2EUC,			//SJIS→EUCコード変換
/* 209 */	F_CODECNV_SJIS2UTF8,		//SJIS→UTF-8コード変換
/* 210 */	F_CODECNV_SJIS2UTF7,		//SJIS→UTF-7コード変換
/* 211 */	F_BASE64DECODE,				//Base64デコードして保存
/* 212 */	F_UUDECODE,					//uudecodeしてファイルに保存
/* 213 */	F_SPACETOTAB,				//空白→TAB
/* 214 */	F_TOZENEI,					//半角英数→全角英数
/* 215 */	F_TOHANEI,					//全角英数→半角英数
/* 216 */	F_CODECNV_UNICODEBE2SJIS,	//UnicodeBE→SJISコード変換
/* 217 */	F_TOHANKATA,				//全角カタカナ→半角カタカナ
/* 218 */	F_FILETREE,					//ファイルツリー表示
/* 219 */	F_SHOWMINIMAP,				//ミニマップを表示
/* 220 */	F_0,						//ダミー
/* 221 */	F_0,						//ダミー
/* 222 */	F_0,						//ダミー
/* 223 */	F_TAGJUMP_CLOSE,			//閉じてタグジャンプ(元ウィンドウclose)
/* 224 */	F_OUTLINE_TOGGLE,			//アウトライン解析(toggle)

/* 検索系(8段目32個: 225-256) */
/* 225 */	F_SEARCH_DIALOG,			//検索(単語検索ダイアログ)
/* 226 */	F_SEARCH_NEXT,				//次を検索
/* 227 */	F_SEARCH_PREV,				//前を検索
/* 228 */	F_REPLACE_DIALOG,			//置換
/* 229 */	F_SEARCH_CLEARMARK,			//検索マークのクリア
/* 230 */	F_GREP_DIALOG,				//Grep
/* 231 */	F_JUMP_DIALOG,				//指定行へジャンプ
/* 232 */	F_OUTLINE,					//アウトライン解析
/* 233 */	F_TAGJUMP,					//タグジャンプ機能
/* 234 */	F_TAGJUMPBACK,				//タグジャンプバック機能
/* 235 */	F_COMPARE,					//ファイル内容比較
/* 236 */	F_BRACKETPAIR,				//対括弧の検索
/* 237 */	F_BOOKMARK_SET,				//ブックマーク設定・解除
/* 238 */	F_BOOKMARK_NEXT,			//次のブックマークへ
/* 239 */	F_BOOKMARK_PREV,			//前のブックマークへ
/* 240 */	F_BOOKMARK_RESET,			//ブックマークの全解除
/* 241 */	F_BOOKMARK_VIEW,			//ブックマークの一覧
/* 242 */	F_DIFF_DIALOG,				//DIFF差分表示
/* 243 */	F_DIFF_NEXT,				//次の差分へ
/* 244 */	F_DIFF_PREV,				//前の差分へ
/* 245 */	F_DIFF_RESET,				//差分の全解除
/* 246 */	F_SEARCH_BOX,				//検索(ボックス)
/* 247 */	F_JUMP_SRCHSTARTPOS,		//検索開始位置へ戻る
/* 248 */	F_TAGS_MAKE,				//タグファイルの作成
/* 249 */	F_DIRECT_TAGJUMP,			//ダイレクトタグジャンプ
/* 250 */	F_ISEARCH_NEXT,				//前方インクリメンタルサーチ
/* 251 */	F_ISEARCH_PREV,				//後方インクリメンタルサーチ
/* 252 */	F_ISEARCH_REGEXP_NEXT,		//正規表現前方インクリメンタルサーチ
/* 253 */	F_ISEARCH_REGEXP_PREV,		//正規表現前方インクリメンタルサーチ
/* 254 */	F_ISEARCH_MIGEMO_NEXT,		//MIGEMO前方インクリメンタルサーチ
/* 255 */	F_ISEARCH_MIGEMO_PREV,		//MIGEMO前方インクリメンタルサーチ
/* 256 */	F_TAGJUMP_KEYWORD,			//キーワードを指定してダイレクトタグジャンプ

/* モード切り替え系(9段目4個: 257-260) */
/* 257 */	F_CHGMOD_INS,				//挿入／上書きモード切り替え
/* 258 */	F_CANCEL_MODE,				//各種モードの取り消し
/* 259 */	F_CHG_CHARSET,				//文字コードセット指定
/* 260 */	F_GREP_REPLACE_DLG,			//Grep置換

/* 設定系(9段目次の16個: 261-276) */
/* 261 */	F_SHOWTOOLBAR,				//ツールバーの表示
/* 262 */	F_SHOWFUNCKEY,				//ファンクションキーの表示
/* 263 */	F_SHOWSTATUSBAR,			//ステータスバーの表示
/* 264 */	F_TYPE_LIST,				//タイプ別設定一覧
/* 265 */	F_OPTION_TYPE,				//タイプ別設定
/* 266 */	F_OPTION,					//共通設定
/* 267 */	F_FONT,						//フォント設定
/* 268 */	F_WRAPWINDOWWIDTH,			//現在のウィンドウ幅で折り返し
/* 269 */	F_FAVORITE,					//履歴の管理
/* 270 */	F_SHOWTAB,					//タブの表示
/* 271 */	F_0,						//ダミー
/* 272 */	F_TOGGLE_KEY_SEARCH,		//キーワードヘルプ自動表示
/* 273 */	F_TMPWRAPNOWRAP,			//折り返さない（一時設定）
/* 274 */	F_TMPWRAPSETTING,			//指定桁で折り返す（一時設定）
/* 275 */	F_TMPWRAPWINDOW,			//右端で折り返す（一時設定）
/* 276 */	F_SELECT_COUNT_MODE,		//文字カウント方法

/* マクロ系(9段目最後の12個: 277-288) */
/* 277 */	F_RECKEYMACRO,				//キーマクロの記録開始／終了
/* 278 */	F_SAVEKEYMACRO,				//キーマクロの保存
/* 279 */	F_LOADKEYMACRO,				//キーマクロの読み込み
/* 280 */	F_EXECKEYMACRO,				//キーマクロの実行
/* 281 */	F_EXECMD_DIALOG,			//外部コマンド実行
/* 282 */	F_EXECEXTMACRO,				//名前を指定してマクロ実行
/* 283 */	F_PLUGCOMMAND,				//プラグインコマンド用に予約
/* 284 */	F_0,						//ダミー
/* 285 */	F_0,						//ダミー
/* 286 */	F_0,						//ダミー
/* 287 */	F_0,						//ダミー
/* 288 */	F_TAB_CLOSEOTHER,			//このタブ以外を閉じる

/* カスタムメニュー(10段目25個: 289-313) */
/* 289 */	F_MENU_RBUTTON,				//右クリックメニュー
/* 290 */	F_CUSTMENU_1,				//カスタムメニュー1
/* 291 */	F_CUSTMENU_2,				//カスタムメニュー2
/* 292 */	F_CUSTMENU_3,				//カスタムメニュー3
/* 293 */	F_CUSTMENU_4,				//カスタムメニュー4
/* 294 */	F_CUSTMENU_5,				//カスタムメニュー5
/* 295 */	F_CUSTMENU_6,				//カスタムメニュー6
/* 296 */	F_CUSTMENU_7,				//カスタムメニュー7
/* 297 */	F_CUSTMENU_8,				//カスタムメニュー8
/* 298 */	F_CUSTMENU_9,				//カスタムメニュー9
/* 299 */	F_CUSTMENU_10,				//カスタムメニュー10
/* 300 */	F_CUSTMENU_11,				//カスタムメニュー11
/* 301 */	F_CUSTMENU_12,				//カスタムメニュー12
/* 302 */	F_CUSTMENU_13,				//カスタムメニュー13
/* 303 */	F_CUSTMENU_14,				//カスタムメニュー14
/* 304 */	F_CUSTMENU_15,				//カスタムメニュー15
/* 305 */	F_CUSTMENU_16,				//カスタムメニュー16
/* 306 */	F_CUSTMENU_17,				//カスタムメニュー17
/* 307 */	F_CUSTMENU_18,				//カスタムメニュー18
/* 308 */	F_CUSTMENU_19,				//カスタムメニュー19
/* 309 */	F_CUSTMENU_20,				//カスタムメニュー20
/* 310 */	F_CUSTMENU_21,				//カスタムメニュー21
/* 311 */	F_CUSTMENU_22,				//カスタムメニュー22
/* 312 */	F_CUSTMENU_23,				//カスタムメニュー23
/* 313 */	F_CUSTMENU_24,				//カスタムメニュー24

/* ウィンドウ系(10段目7個: 314-320) */
/* 314 */	F_TAB_MOVERIGHT,			//タブを右に移動
/* 315 */	F_TAB_MOVELEFT,				//タブを左に移動
/* 316 */	F_TAB_SEPARATE,				//新規グループ
/* 317 */	F_TAB_JOINTNEXT,			//次のグループに移動
/* 318 */	F_TAB_JOINTPREV,			//前のグループに移動
/* 319 */	F_TAB_CLOSERIGHT,			//右をすべて閉じる
/* 320 */	F_TAB_CLOSELEFT,			//左をすべて閉じる

/* ウィンドウ系(11段目22個: 321-342) */
/* 321 */	F_SPLIT_V,					//上下に分割
/* 322 */	F_SPLIT_H,					//左右に分割
/* 323 */	F_SPLIT_VH,					//縦横に分割
/* 324 */	F_WINCLOSE,					//ウィンドウを閉じる
/* 325 */	F_WIN_CLOSEALL,				//すべてのウィンドウを閉じる
/* 329 */	F_NEXTWINDOW,				//次のウィンドウ
/* 330 */	F_PREVWINDOW,				//前のウィンドウ
/* 326 */	F_CASCADE,					//重ねて表示
/* 237 */	F_TILE_V,					//上下に並べて表示
/* 328 */	F_TILE_H,					//左右に並べて表示
/* 331 */	F_MAXIMIZE_V,				//縦方向に最大化
/* 332 */	F_MAXIMIZE_H,				//横方向に最大化
/* 333 */	F_MINIMIZE_ALL,				//すべて最小化
/* 334 */	F_REDRAW,					//再描画
/* 335 */	F_WIN_OUTPUT,				//アウトプットウィンドウ表示
/* 336 */	F_BIND_WINDOW,				//結合して表示
/* 337 */	F_TOPMOST,					//常に手前に表示
/* 338 */	F_DLGWINLIST,				//ウィンドウ一覧表示
/* 339 */	F_WINLIST,					//ウィンドウ一覧ポップアップ表示
/* 340 */	F_GROUPCLOSE,				//グループを閉じる
/* 341 */	F_NEXTGROUP,				//次のグループ
/* 342 */	F_PREVGROUP,				//前のグループ

/* 支援(11段目残りの10個: 343-352) */
/* 343 */	F_HOKAN,					//入力補完
/* 344 */	F_HELP_CONTENTS,			//ヘルプ目次
/* 345 */	F_HELP_SEARCH,				//ヘルプキーワード検索
/* 346 */	F_MENU_ALLFUNC,				//コマンド一覧
/* 347 */	F_EXTHELP1,					//外部ヘルプ１
/* 348 */	F_EXTHTMLHELP,				//外部HTMLヘルプ
/* 349 */	F_ABOUT,					//バージョン情報
/* 350 */	F_0,						//ダミー
/* 351 */	F_0,						//ダミー
/* 352 */	F_0,						//ダミー


/* 外部マクロ(12段目31個: 353-383) */
/* 353 */	F_USERMACRO_0 + 0,			//外部マクロ①
/* 354 */	F_USERMACRO_0 + 1,			//外部マクロ②
/* 355 */	F_USERMACRO_0 + 2,			//外部マクロ③
/* 356 */	F_USERMACRO_0 + 3,			//外部マクロ④
/* 357 */	F_USERMACRO_0 + 4,			//外部マクロ⑤
/* 358 */	F_USERMACRO_0 + 5,			//外部マクロ⑥
/* 359 */	F_USERMACRO_0 + 6,			//外部マクロ⑦
/* 360 */	F_USERMACRO_0 + 7,			//外部マクロ⑧
/* 361 */	F_USERMACRO_0 + 8,			//外部マクロ⑨
/* 362 */	F_USERMACRO_0 + 9,			//外部マクロ⑩
/* 363 */	F_USERMACRO_0 + 10,			//外部マクロ⑪
/* 364 */	F_USERMACRO_0 + 11,			//外部マクロ⑫
/* 365 */	F_USERMACRO_0 + 12,			//外部マクロ⑬
/* 366 */	F_USERMACRO_0 + 13,			//外部マクロ⑭
/* 367 */	F_USERMACRO_0 + 14,			//外部マクロ⑮
/* 368 */	F_USERMACRO_0 + 15,			//外部マクロ⑯
/* 369 */	F_USERMACRO_0 + 16,			//外部マクロ⑰
/* 370 */	F_USERMACRO_0 + 17,			//外部マクロ⑱
/* 371 */	F_USERMACRO_0 + 18,			//外部マクロ⑲
/* 372 */	F_USERMACRO_0 + 19,			//外部マクロ⑳
/* 373 */	F_USERMACRO_0 + 20,			//外部マクロ21
/* 374 */	F_USERMACRO_0 + 21,			//外部マクロ22
/* 375 */	F_USERMACRO_0 + 22,			//外部マクロ23
/* 376 */	F_USERMACRO_0 + 23,			//外部マクロ24
/* 377 */	F_USERMACRO_0 + 24,			//外部マクロ25
/* 378 */	F_USERMACRO_0 + 25,			//外部マクロ26
/* 379 */	F_USERMACRO_0 + 26,			//外部マクロ27
/* 380 */	F_USERMACRO_0 + 27,			//外部マクロ28
/* 381 */	F_USERMACRO_0 + 28,			//外部マクロ29
/* 382 */	F_USERMACRO_0 + 29,			//外部マクロ30
/* 383 */	F_USERMACRO_0 + 30,			//外部マクロ31


/* 384 */	F_TOOLBARWRAP,	//追加マクロ用icon位置兼、折返ツールバーボタンID

/* 外部マクロ(13段目19個: 385-403) */
/* 385 */	F_USERMACRO_0 + 31,			//外部マクロ32
/* 386 */	F_USERMACRO_0 + 32,			//外部マクロ33
/* 387 */	F_USERMACRO_0 + 33,			//外部マクロ34
/* 388 */	F_USERMACRO_0 + 34,			//外部マクロ35
/* 389 */	F_USERMACRO_0 + 35,			//外部マクロ36
/* 390 */	F_USERMACRO_0 + 36,			//外部マクロ37
/* 391 */	F_USERMACRO_0 + 37,			//外部マクロ38
/* 392 */	F_USERMACRO_0 + 38,			//外部マクロ39
/* 393 */	F_USERMACRO_0 + 39,			//外部マクロ40
/* 394 */	F_USERMACRO_0 + 40,			//外部マクロ41
/* 395 */	F_USERMACRO_0 + 41,			//外部マクロ42
/* 396 */	F_USERMACRO_0 + 42,			//外部マクロ43
/* 397 */	F_USERMACRO_0 + 43,			//外部マクロ44
/* 398 */	F_USERMACRO_0 + 44,			//外部マクロ45
/* 399 */	F_USERMACRO_0 + 45,			//外部マクロ46
/* 400 */	F_USERMACRO_0 + 46,			//外部マクロ47
/* 401 */	F_USERMACRO_0 + 47,			//外部マクロ48
/* 402 */	F_USERMACRO_0 + 48,			//外部マクロ49
/* 403 */	F_USERMACRO_0 + 49,			//外部マクロ50
/* 404 */	F_0,						//ダミー
};

/*!
 * 機能コードとアイコン番号の対応表を取得する
 */
/*static*/ std::map<int, int> CImageListMgr::GetFuncIcons()
{
	std::map<int, int> buttonIds;
	for (size_t i = 0; i < std::size(gm_toolIcons); ++i) {
		buttonIds.try_emplace(gm_toolIcons[i], i);
	}
	return buttonIds;
}

/*!	領域を指定色で塗りつぶす

	@author Nakatani
*/
static void FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr)
{
//	ASSERT_VALID(this);
//	ASSERT(m_hDC != NULL);

	RECT rect;
	::SetBkColor( hdc, clr );
	::SetRect( &rect, x, y, x + cx, y + cy );
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rect, nullptr, 0, nullptr );
}

/*! リソースに埋め込まれたmytool.bmpを読み込む
 */
static inline
HBITMAP LoadMyToolFromModule( HINSTANCE hInstance )
{
	//	リソースからBitmapを読み込む
	HANDLE hRscbmp = ::LoadImageW(
		hInstance,
		MAKEINTRESOURCE( IDB_MYTOOL ),
		IMAGE_BITMAP,
		0,
		0,
		LR_CREATEDIBSECTION
	);

	return (HBITMAP)hRscbmp;
}

static
HBITMAP ConvertTo32bppBMP(HBITMAP hbmpSrc)
{
	BITMAP bmp;
	if (0 == GetObject(hbmpSrc, sizeof(BITMAP), &bmp )) {
		return hbmpSrc;
	}
	if (bmp.bmBitsPixel == 32) {
		return hbmpSrc;
	}
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = bmp.bmWidth;
	bmi.bmiHeader.biHeight = bmp.bmHeight;
	bmi.bmiHeader.biPlanes = bmp.bmPlanes;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
	HBITMAP hdib = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, nullptr, nullptr, 0);
	if (hdib == nullptr) {
		return hbmpSrc;
	}
	HDC hdcSrc = CreateCompatibleDC(nullptr);
	if (!hdcSrc) {
		DeleteObject(hdib);
		return hbmpSrc;
	}
	HDC hdcDst = CreateCompatibleDC(nullptr);
	if (!hdcDst) {
		DeleteDC(hdcSrc);
		DeleteObject(hdib);
		return hbmpSrc;
	}
	HGDIOBJ hbmpSrcOld = SelectObject(hdcSrc, hbmpSrc);
	HGDIOBJ hbmpDstOld = SelectObject(hdcDst, hdib);
	BitBlt(hdcDst, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcSrc, 0, 0, SRCCOPY);
	SelectObject(hdcSrc, hbmpSrcOld);
	SelectObject(hdcDst, hbmpDstOld);
	DeleteDC(hdcSrc);
	DeleteDC(hdcDst);
	DeleteObject(hbmpSrc);
	return hdib;
}


/*
	@brief Image Listの作成
	
	リソースまたはファイルからbitmapを読み込んで
	描画用に保持する．
	
	@param hInstance [in] bitmapリソースを持つインスタンス
	
	@date 2003.07.21 genta ImageListの構築は行わない．代わりにbitmapをそのまま保持する．
*/
bool CImageListMgr::Create(HINSTANCE hInstance)
{
	MY_RUNNINGTIMER( cRunningTimer, L"CImageListMgr::Create" );
	if( m_hIconBitmap != nullptr ){	//	既に構築済みなら無視する
		return true;
	}

	HBITMAP	hRscbmp;			//	リソースから読み込んだひとかたまりのBitmap

	//	From Here 2001.7.1 GAE
	//	2001.7.1 GAE リソースをローカルファイル(sakuraディレクトリ) my_icons.bmp から読めるように
	// 2007.05.19 ryoji 設定ファイル優先に変更
	WCHAR szPath[_MAX_PATH];
	GetInidirOrExedir( szPath, FN_TOOL_BMP );
	hRscbmp = (HBITMAP)::LoadImage( nullptr, szPath, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS );

	if( hRscbmp == nullptr ) {	// ローカルファイルの読み込み失敗時はリソースから取得
		//	リソースからBitmapを読み込む
		//	2003.09.29 wmlhq 環境によってアイコンがつぶれる
		hRscbmp = LoadMyToolFromModule( hInstance );
		if( hRscbmp == nullptr ){
			return false;
		}
	}

	hRscbmp = ConvertTo32bppBMP(hRscbmp);

	//	To Here 2001.7.1 GAE

	//	2003.07.21 genta
	//	ImageListへの登録部分は当然ばっさり削除
		
	//	もはや処理とは無関係だが，後学のためにコメントのみ残しておこう
	//---------------------------------------------------------
	//	BitmapがMemoryDCにAssignされている間はbitmapハンドルを
	//	使っても正しいbitmapが取得できない．
	//	つまり，DCへの描画命令を発行してもその場でBitmapに
	//	反映されるわけではない．
	//	BitmapをDCから取り外して初めて内容の保証ができる

	//	DCのmap/unmapが速度に大きく影響するため，
	//	横長のBitmapを作って一括登録するように変更
	//	これによって250msecくらい速度が改善される．
	//---------------------------------------------------------

	// システムのスモールアイコンサイズを取得する
	m_cx = ::GetSystemMetrics( SM_CXSMICON );
	m_cy = ::GetSystemMetrics( SM_CYSMICON );

	// アイコンサイズが異なる場合、拡大縮小する
	hRscbmp = ResizeToolIcons( hRscbmp, m_cTrans );
	if( hRscbmp == nullptr ){
		//	リソースからBitmapを読み込む
		hRscbmp = LoadMyToolFromModule( hInstance );
		if( hRscbmp == nullptr ){
			return false;
		}

		hRscbmp = ConvertTo32bppBMP(hRscbmp);

		// アイコンサイズが異なる場合、拡大縮小する
		hRscbmp = ResizeToolIcons( hRscbmp, m_cTrans );
		if( hRscbmp == nullptr ){
			return false;
		}
	}

	// クラスメンバに変更を保存する
	m_hIconBitmap = hRscbmp;

	return true;
}

/*! RGBQUADラッパー
 *  STLコンテナに入れられるよう == 演算子を実装したもの。
 */
struct MyRGBQUAD : tagRGBQUAD
{
	using tagRGBQUAD::rgbRed;
	using tagRGBQUAD::rgbGreen;
	using tagRGBQUAD::rgbBlue;
	using tagRGBQUAD::rgbReserved;

	MyRGBQUAD() noexcept
		: tagRGBQUAD()
	{
		rgbBlue = 0;
		rgbGreen = 0;
		rgbRed = 0;
		rgbReserved = 0;
	}
	bool operator == ( const RGBQUAD &rhs ) const noexcept
	{
		return rgbBlue == rhs.rgbBlue
			&& rgbGreen == rhs.rgbGreen
			&& rgbRed == rhs.rgbRed
			&& rgbReserved == rhs.rgbReserved;
	}
	bool operator != ( const RGBQUAD &rhs ) const noexcept
	{
		return !(*this == rhs);
	}
	operator COLORREF ( void ) const noexcept
	{
		return RGB( rgbRed, rgbGreen, rgbBlue );
	}
};

// HLS色情報タプル
typedef std::tuple<double, double, double> _HlsTuple;
enum { HLS_H, HLS_S, HLS_L, };

/*!
 * @brief RGB⇒HLS(円柱モデル)変換する
 */
_HlsTuple ToHLS( const COLORREF color )
{
	auto R = (double) GetRValue( color ) / 255.;
	auto G = (double) GetGValue( color ) / 255.;
	auto B = (double) GetBValue( color ) / 255.;
	auto MIN = std::min( { R, G, B } );
	auto MAX = std::max( { R, G, B } );
	auto M = MAX + MIN;
	auto m = MAX - MIN;
	double H;
	if ( MIN == MAX ) {
		H = std::numeric_limits<double>::infinity();
	}
	else if ( MIN == B ) {
		H = 60. * (m == 0 ? 0 : ((G - R) / m)) + 60.;
	}
	else if ( MIN == R ) {
		H = 60. * (m == 0 ? 0 : ((B - G) / m)) + 180.;
	}
	else if ( MIN == G ) {
		H = 60. * (m == 0 ? 0 : ((R - B) / m)) + 300.;
	}
	auto L = M / 2.;
	auto S = M == 0 ? 0 : m / (1 - std::abs( M - 1 ));
	return std::make_tuple( H, S, L );
}

/*!
 * @brief HLS(円柱モデル)⇒RGB変換する
 */
COLORREF FromHLS( const _HlsTuple &hls )
{
	auto H = std::get<HLS_H>( hls );
	auto S = std::get<HLS_S>( hls );
	auto L = std::get<HLS_L>( hls );

	// 彩度の範囲を補正する
	if ( S < 0 ) S = 0;
	if ( 1 < S ) S = 1;

	// 輝度の範囲を補正する
	if ( L < 0 ) L = 0;
	if ( 1 < L ) L = 1;

	// 色相が無効値（＝白黒）の場合
	if ( std::isinf( H ) ) {
		return RGB( L * 255, L * 255, L * 255 );
	}

	// 色相の範囲を補正する
	while ( H < 0 ) H = 360 - H;
	while ( 360 <= H ) H = H - 360;

	double R, G, B;
	double MIN = L + S * (1 - std::abs( 2 * L - 1 )) / 2;
	double MAX = L - S * (1 - std::abs( 2 * L - 1 )) / 2;
	if ( H < 60 ) {
		R = MAX;
		G = MAX + (MAX - MIN) * H / 60;
		B = MIN;
	}
	else if ( H < 120 ) {
		R = MIN + (MAX - MIN) * (120 - H) / 60;
		G = MAX;
		B = MIN;
	}
	else if ( H < 180 ) {
		R = MIN;
		G = MAX;
		B = MIN + (MAX - MIN) * (H - 120) / 60;
	}
	else if ( H < 240 ) {
		R = MIN;
		G = MIN + (MAX - MIN) * (240 - H) / 60;
		B = MAX;
	}
	else if ( H < 300 ) {
		R = MIN + (MAX - MIN) * (H - 240) / 60;
		G = MIN;
		B = MAX;
	}
	else { //if ( H < 360 ) {
		R = MAX;
		G = MIN;
		B = MIN + (MAX - MIN) * (360 - H) / 60;
	}
	return RGB( R * 255, G * 255, B * 255 );
}

/*! ビットマップの表示 灰色を透明描画

	@author Nakatani
	@date 2003.07.21 genta 以前のCMenuDrawerより移転復活
	@date 2003.08.27 Moca 背景は透過処理に変更し、colBkColorを削除
	@date 2010.01.30 syat 透明にする色を引数に移動
*/
void CImageListMgr::MyBitBlt(
	HDC drawdc, 
	int nXDest, 
	int nYDest, 
	int nWidth, 
	int nHeight, 
	int nXSrc, 
	int nYSrc
) const
{
	// 仮想DCを生成してビットマップを展開する
	const HBITMAP &bmpSrc = m_hIconBitmap;
	HDC hdcSrc = ::CreateCompatibleDC( drawdc );
	HGDIOBJ bmpSrcOld = ::SelectObject( hdcSrc, bmpSrc );

	// 透過色の変数名が分かりづらいので別名定義する
	const COLORREF &cTransparent = m_cTrans;

	// 透過色を考慮して転送
	::TransparentBlt( drawdc, nXDest, nYDest, nWidth, nHeight,
		hdcSrc, nXSrc, nYSrc, cx(), cy(), cTransparent );

	// 後始末
	::SelectObject( hdcSrc, bmpSrcOld );
	::DeleteDC( hdcSrc );
	return;
}


/*! メニューアイコンの淡色表示

	@author Nakatani
	
	@date 2003.07.21 genta 以前のCMenuDrawerより移転復活
	@date 2003.08.27 Moca 背景色は透過処理する
*/
void CImageListMgr::MyDitherBlt( HDC drawdc, int nXDest, int nYDest,
	int nWidth, int nHeight, int nXSrc, int nYSrc ) const
{
	// 仮想DCを生成してビットマップを展開する
	const HBITMAP &bmpSrc = m_hIconBitmap;
	HDC hdcSrc = ::CreateCompatibleDC( drawdc );
	HGDIOBJ bmpSrcOld = ::SelectObject( hdcSrc, bmpSrc );

	// 作業DCを作成
	HDC hdcWork = ::CreateCompatibleDC( drawdc );

	// DIB作成
	BITMAPINFO bmi;
	char* pBits;
	BITMAPINFOHEADER& bmih = bmi.bmiHeader;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = nWidth;
	assert(nHeight > 0);
	bmih.biHeight = -nHeight; // top down
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;
	const int lineStride = ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) / 8);
	bmih.biSizeImage = lineStride * nHeight;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;
	HBITMAP bmpWork = ::CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, (void**)&pBits, nullptr, 0);
	HGDIOBJ bmpWorkOld = ::SelectObject( hdcWork, bmpWork );

	// 作業DCに転送
	::StretchBlt( hdcWork, 0, 0, nWidth, nHeight,
		hdcSrc, nXSrc, nYSrc, cx(), cy(), SRCCOPY );

	// ディザカラーを決める
	// 淡色テキスト色が背景色と同じなら灰色に避ける、違うなら淡色テキストを使う。
	COLORREF grayText = ::GetSysColor( COLOR_GRAYTEXT );
	COLORREF btnFace = ::GetSysColor( COLOR_3DFACE );
	COLORREF textColor = grayText == btnFace ? RGB( 0x80, 0x80, 0x80 ) : grayText;
	auto textColorH = ToHLS( textColor );
	double textColorL;
	{
		auto r = GetRValue( textColor );
		auto g = GetGValue( textColor );
		auto b = GetBValue( textColor );
		textColorL = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0; //[0,1]
	}
	double textColorR = (1.0 - textColorL) / 255.0;

	// ディザカラー256諧調の配列を作る
	std::array<COLORREF, 0x100> ditherColors;
	for ( size_t i = 0; i < ditherColors.size(); ++i ) {
		auto ditherColorH( textColorH );
		std::get<HLS_L>(ditherColorH) = textColorL + i * textColorR;
		ditherColors[i] = FromHLS( ditherColorH );
	}

	// 透過色の変数名が分かりづらいので別名定義する
	const COLORREF cTransparent = m_cTrans;

	// スキャンライン全行を順に取得して処理する
	for (auto n = 0; n < nHeight; ++n) {

		// スキャンラインを1ピクセルずつ処理する
		auto pixels = reinterpret_cast<MyRGBQUAD*>(pBits);
		for ( auto m = 0; m < nWidth; ++m ) {
			MyRGBQUAD& px = pixels[m];

			// 透過色はスキップする
			if ( px == cTransparent ) continue;

			// ピクセル色をディザカラーに変換する
			auto r = px.rgbRed;
			auto g = px.rgbGreen;
			auto b = px.rgbBlue;
			auto mono = (77 * r + 150 * g + 29 * b) >> 8; //[0,255]

			// ディザカラーを書き込む
			px.rgbRed = GetRValue( ditherColors[mono] );
			px.rgbGreen = GetGValue( ditherColors[mono] );
			px.rgbBlue = GetBValue( ditherColors[mono] );
		}

		pBits += lineStride;
	}

	// 背景を透過させつつ転送
	::TransparentBlt( drawdc, nXDest, nYDest, nWidth, nHeight,
		hdcWork, 0, 0, nWidth, nHeight, cTransparent );

	// 後始末
	::SelectObject( hdcWork, bmpWorkOld );
	::DeleteObject( bmpWork );
	::DeleteDC( hdcWork );
	::SelectObject( hdcSrc, bmpSrcOld );
	::DeleteDC( hdcSrc );
	return;
}

/*!
 * @brief アイコンの描画
 *
 * 指定されたDCの指定された座標にアイコンを描画する．
 *
 * @param [in] drawdc 描画するDevice Context
 * @param [in] x 描画するX座標
 * @param [in] y 描画するY座標
 * @param [in] imageNo 描画するアイコン番号
 * @param [in] fStyle 描画スタイル
 * @param [in] cx アイコン幅
 * @param [in] cy アイコン高さ
 * @note 描画スタイルとして有効なのは，ILD_NORMAL, ILD_MASK
 * 
 * @date 2003.07.21 genta 独自描画ルーチンを使う
 * @date 2003.08.30 genta 背景色を指定する引数を追加
 * @date 2003.09.06 genta Mocaさんの背景色透過処理に伴い，背景色引数削除
 * @date 2007.11.02 ryoji アイコン番号が負の場合は描画しない
 */
bool CImageListMgr::DrawToolIcon( HDC drawdc, LONG x, LONG y,
	int imageNo, DWORD fStyle, LONG cx, LONG cy ) const
{
	if ( m_hIconBitmap == nullptr )
		return false;
	if ( imageNo < 0 || m_nIconCount < imageNo )
		return false;

	if ( (fStyle&ILD_MASK) == ILD_MASK ) {
		MyDitherBlt( drawdc, x, y, cx, cy,
			(imageNo % MAX_X) * m_cx, (imageNo / MAX_X) * m_cy );
	} else {
		MyBitBlt( drawdc, x, y, cx, cy,
			(imageNo % MAX_X) * m_cx, (imageNo / MAX_X) * m_cy );
	}
	return true;
}

/*!	アイコン数を返す

	@date 2003.07.21 genta 個数を自分で管理する必要がある．
*/
int CImageListMgr::Count() const
{
	return m_nIconCount;
//	return MAX_X * MAX_Y;
}

/*!
 * @brief アイコンを追加してそのIDを返す
 */
int CImageListMgr::Add( const WCHAR* szPath )
{
	if ( (m_nIconCount % MAX_X) == 0 ) {
		Extend();
	}

	//アイコンを読み込む
	HBITMAP bmpSrc = (HBITMAP)::LoadImage( nullptr, szPath, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION );

	if( bmpSrc == nullptr ) {
		return -1;
	}

	int imageNo = m_nIconCount++;

	// 仮想DCを生成して読込んだビットマップを展開する
	HDC hdcSrc = ::CreateCompatibleDC( nullptr );
	HGDIOBJ bmpSrcOld = ::SelectObject( hdcSrc, bmpSrc );

	//取得した画像の(0,0)の色を背景色として使う
	COLORREF cTransParent = ::GetPixel( hdcSrc, 0, 0 );

	// DIBセクションからサイズを取得する
	LONG nWidth, nHeight;
	{
		// DIBセクションを取得する
		DIBSECTION di = {};
		if ( !::GetObject( bmpSrc, sizeof( di ), &di ) ) {
			DEBUG_TRACE( L"GetObject() failed." );
			::SelectObject( hdcSrc, bmpSrcOld );
			::DeleteDC( hdcSrc );
			::DeleteObject( bmpSrc );
			return -1;
		}

		nWidth = di.dsBm.bmWidth;
		nHeight = di.dsBm.bmHeight;
		if ( nWidth != nHeight ) {
			DEBUG_TRACE( L"tool bitmap size is unexpected." );
			::SelectObject( hdcSrc, bmpSrcOld );
			::DeleteDC( hdcSrc );
			::DeleteObject( bmpSrc );
			return -1;
		}
	}

	// 作業DCの内容を出力DCに転送
	HDC hdcDst = ::CreateCompatibleDC( nullptr );
	HGDIOBJ hbmDstOld = ::SelectObject( hdcDst, m_hIconBitmap );
	::TransparentBlt( hdcDst, (imageNo % MAX_X) * cx(), (imageNo / MAX_X) * cy(), cx(), cy(),
		hdcSrc, 0, 0, nWidth, nHeight, cTransParent );

	// 後始末
	::SelectObject( hdcDst, hbmDstOld );
	::DeleteDC( hdcDst );
	::SelectObject( hdcSrc, bmpSrcOld );
	::DeleteDC( hdcSrc );

	::DeleteObject( bmpSrc );

	return imageNo;
}

// ツールイメージをリサイズする
HBITMAP CImageListMgr::ResizeToolIcons(
	HBITMAP		bmpSrc,				//!< [in] 変換前Bmpのハンドル
	COLORREF&	clrTransparent		//!< [out] 透過色
) const noexcept
{
	// 引数チェック
	if( bmpSrc == nullptr ){
		DEBUG_TRACE( L"tool bitmap is required." );
		return nullptr;
	}

	// DIBセクションを取得する
	DIBSECTION di = {};
	if ( !::GetObject( bmpSrc, sizeof( di ), &di ) ) {
		DEBUG_TRACE( L"GetObject() failed." );

		// 変換前Bmpを削除する
		::DeleteObject( bmpSrc );

		return nullptr;
	}

	// DIBセクションからサイズを取得する
	const auto bmWidth = di.dsBm.bmWidth;
	const auto bmHeight = di.dsBm.bmHeight;

	// 内部ビットマップの列数/段数は固定。
	const int cols = MAX_X;
	const int rows = MAX_Y;

	// アイコンサイズは固定。
	const int cx = 16;
	const int cy = cx;

	// 仮想DCを作成
	HDC hdcSrc = ::CreateCompatibleDC( nullptr );	//	転送元用
	if( hdcSrc == nullptr ){

		// 変換前Bmpを削除する
		::DeleteObject( bmpSrc );

		return nullptr;
	}

	//	まずbitmapをdcにmapする
	//	こうすることでCreateCompatibleBitmapで
	//	bmpSrcと同じ形式のbitmapを作れる．
	//	単にCreateCompatibleDC(0)で取得したdcや
	//	スクリーンのDCに対してCreateCompatibleBitmapを
	//	使うとモノクロBitmapになる．
	HGDIOBJ hFOldbmp = ::SelectObject( hdcSrc, bmpSrc );
	if( hFOldbmp == nullptr ){
		DEBUG_TRACE( L"SelectObject() failed." );

		// 変換前Bmpを削除する
		::DeleteObject( bmpSrc );

		// 仮想DCを削除する
		::DeleteDC( hdcSrc );

		return nullptr;
	}

	//	仮想DC(=変換前Bmp)の(0,0)の色を背景色として使う
	clrTransparent = ::GetPixel( hdcSrc, 0, 0 );
		
	const int cxSmIcon = m_cx;
	const int cySmIcon = m_cy;

	// アイコンサイズが異なる場合、拡大縮小する
	if ( cx != cxSmIcon ) {
		// 作業DCを作成する
		HDC hdcWork = ::CreateCompatibleDC( hdcSrc );
		HBITMAP bmpWork = ::CreateCompatibleBitmap( hdcSrc, cxSmIcon * cols, cySmIcon * rows );
		HGDIOBJ bmpWorkOld = ::SelectObject( hdcWork, bmpWork );

		// 作業DCを透過色で塗りつぶす
		{
			HBRUSH hBrush = ::CreateSolidBrush( clrTransparent );
			HGDIOBJ hBrushOld = ::SelectObject( hdcWork, hBrush );
			::PatBlt( hdcWork, 0, 0, cxSmIcon * cols, cySmIcon * rows, PATCOPY );
			::SelectObject( hdcWork, hBrushOld );
			::DeleteObject( hBrush );
		}

		// ざっくり拡大縮小すると位置がずれるので1個ずつ変換する
		for ( int row = 0; row < rows; ++row ) {
			for ( int col = 0; col < cols; ++col ) {
				// 拡大・縮小する
				::TransparentBlt(
					hdcWork,
					col * cxSmIcon,
					row * cySmIcon,
					cxSmIcon,
					cySmIcon,
					hdcSrc,
					col * cx,
					row * cy,
					cx,
					cy,
					m_cTrans
				);
			}
		}

		// 作業DCで元Bmpを選択して変換後Bmpを解放する
		::SelectObject( hdcWork, bmpWorkOld );

		// 作業DCを削除する
		::DeleteDC( hdcWork );

		// 仮想DCで元Bmpを選択して変換前Bmpを解放する
		::SelectObject( hdcSrc, hFOldbmp );

		// 仮想DCを削除する
		::DeleteDC( hdcSrc );

		// 変換前Bmpを削除する
		::DeleteObject( bmpSrc );

		return bmpWork;
	}

	// 仮想DCで元Bmpを選択して変換前Bmpを解放する
	::SelectObject( hdcSrc, hFOldbmp );

	// 仮想DCを削除する
	::DeleteDC( hdcSrc );

	return bmpSrc;
}

// ビットマップを一行（MAX_X個）拡張する
void CImageListMgr::Extend(bool bExtend)
{
	int curY = m_nIconCount / MAX_X;
	if( curY < MAX_Y )
		curY = MAX_Y;

	HDC hSrcDC = ::CreateCompatibleDC( nullptr );
	HBITMAP hSrcBmpOld = (HBITMAP)::SelectObject( hSrcDC, m_hIconBitmap );

	//1行拡張したビットマップを作成
	HDC hDestDC = ::CreateCompatibleDC( hSrcDC );
	HBITMAP hDestBmp = ::CreateCompatibleBitmap( hSrcDC, MAX_X * cx(), (curY + (bExtend ? 1 : 0)) * cy() );
	HBITMAP hDestBmpOld = (HBITMAP)::SelectObject( hDestDC, hDestBmp );

	::BitBlt( hDestDC, 0, 0, MAX_X * cx(), curY * cy(), hSrcDC, 0, 0, SRCCOPY );

	//拡張した部分は透過色で塗る
	if( bExtend ){
		FillSolidRect( hDestDC, 0, curY * cy(), MAX_X * cx(), cy(), m_cTrans );
	}

	::SelectObject( hSrcDC, hSrcBmpOld );
	::DeleteObject( m_hIconBitmap );
	::DeleteDC( hSrcDC );

	::SelectObject( hDestDC, hDestBmpOld );
	::DeleteDC( hDestDC );

	//ビットマップの差し替え
	m_hIconBitmap = hDestBmp;
}

void CImageListMgr::ResetExtend()
{
	m_nIconCount = MAX_TOOLBAR_ICON_COUNT;
	Extend(false);
}
