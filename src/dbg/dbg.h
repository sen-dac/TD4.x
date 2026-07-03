//============================================================================================
// Application : TD4 Emulator & Assembler for X68000 - TD4.x
// Platform    : SHARP X68000 / Human68k
// Build       : GCC真里子版(ver1.42) + XCライブラリ
//--------------------------------------------------------------------------------------------
// Filename    : dbg.h
// Version     : 1.0.0
// Author      : SEN::DAC
// Description : デバッガモジュール (メイン処理)
//============================================================================================
//============================================================================================
// 2重インクルード防止
//============================================================================================
#ifndef _DBG_H_
#define _DBG_H_

//============================================================================================
// インクルードファイル
//============================================================================================
#include "../StdAfx.h"
#include "../lib/inifile.h"
#include "../cpu/cpu.h"
#include "../mem/mem.h"
#include "dbg_cmd.h"
#include "../main.h"
#include "../lib/keycode.h"

//============================================================================================
// 定数定義
//============================================================================================
// CUI上で1行に表示できる文字の最大数
#define ROW_CHAR_MAX   96

// デバッガの動作モード
#define DBG_MODE_CMD   0 // コマンドモード
#define DBG_MODE_AT    1 // オートモード

// メッセージ欄表示文字列
#define MES_UNDEF_INS  "[ERROR] 未定義の命令が実行されました。"
#define MES_UNDEF_IMM  "[ERROR] 不正なIm値が設定されています。"
#define MES_FN_ER_NULL "[ERROR] ファイル名の終端にNULL文字がありません。"
#define MES_FN_ER_NSTD "[ERROR] ファイル名として不正な文字が含まれています。"
#define MES_FS_FAILURE "[ERROR] ファイルのセーブに失敗しました。"
#define MES_FL_FAILURE "[ERROR] ファイルのロードに失敗しました。"
#define MES_FS_SUCCESS "ファイルのセーブに成功しました。"
#define MES_FL_SUCCESS "ファイルのロードに成功しました。"
#define MES_EXEC_AUTO  "オート(Go)モード実行中...ESCキーで停止します。"
#define MES_STOP_AUTO  "オート(Go)モードを停止しました。"
#define MES_BREAK      "ブレークしました。"
#define MES_RESET      "リセットしました。"
#define MES_QUIT       "終了します。"
#define MES_CLEAR       ""

//============================================================================================
// 型定義
//============================================================================================

//============================================================================================
// extern
//============================================================================================

//============================================================================================
// プロトタイプ宣言
//============================================================================================
VD vdf_g_DbgDrawUpdateFName(const char* const szcpc_filename); // ファイル名の表示更新
VD vdf_g_DbgDrawUpdateList(U4 u4_addr,                         // リストの表示更新
                           const U1* const u1cpc_rom);
VD vdf_g_DbgDrawUpdateClockTgl(VD);                            // クロックのトグルと表示更新
VD vdf_g_DbgDrawUpdateBp(U4 u4_addr);                          // ブレークポイントの表示更新
VD vdf_g_DbgDrawUpdateMess(const char* const szcpc_message);   // メッセージの表示更新

VD vdf_g_DbgSetOpMode(U1 u1_mode);                             // 動作モードをセットする
VD vdf_g_DbgSetPrevPC(U4 u4_prev_pc);                          // 1つ前のPCを保存する
VD vdf_g_DbgSetBP(U4 u4_addr, BL bl_state);                    // ブレークポイントを設定する
VD vdf_g_DbgRetCurPos(VD);                                     // コマンド位置にカーソルを戻す
BL blf_g_DbgLoadRomFile(const char* const szcpc_filename,      // ROMファイルをロード
                        U1* const u1pc_rom);
BL blf_g_DbgSaveRomFile(const char* const szcpc_filename,      // ROMファイルをセーブ
                        const U1* const u1cpc_rom);

VD vdf_g_DbgInit(int argc, char* argv[], U1* const u1pc_rom);  // デバッガ初期化処理
S1 s1f_g_DbgRun(U1* const u1pc_rom);                           // デバッガ実行処理
VD vdf_g_DbgExe1Clk(U1* const u1pc_rom);                       // ターゲットCPUを1クロック実行


#endif // _DBG_H_
