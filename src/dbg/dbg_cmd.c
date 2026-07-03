//============================================================================================
// Application : TD4 Emulator & Assembler for X68000 - TD4.x
// Platform    : SHARP X68000 / Human68k
// Build       : GCC真里子版(ver1.42) + XCライブラリ
//--------------------------------------------------------------------------------------------
// Filename    : dbg_cmd.c
// Version     : 1.0.0
// Author      : SEN::DAC
// Description : デバッガモジュール (コマンド処理)
//============================================================================================
//============================================================================================
// インクルードファイル
//============================================================================================
#include "dbg_cmd.h"

//============================================================================================
// 定数定義 (内部用)
//============================================================================================
// コマンド応答判定
#define CHA_CODE_UP    0x17    // K_KEYINP()戻り値：↑キー
#define CHA_CODE_DOWN  0x05    // K_KEYINP()戻り値：↓キー
#define CHA_CODE_BS    0x08    // K_KEYINP()戻り値：BSキー
#define CHA_CODE_ESC   0x1B    // K_KEYINP()戻り値：ESCキー
#define CHA_CODE_CR    0x0D    // K_KEYINP()戻り値：CR リターンキー
#define CHA_CODE_TAB   0x09    // K_KEYINP()戻り値：TABキー (拡張用：未使用)
#define CHA_CODE_LEFT  0x13    // K_KEYINP()戻り値：←キー  (拡張用：未使用)
#define CHA_CODE_RIGHT 0x04    // K_KEYINP()戻り値：→キー  (拡張用：未使用)

// コマンド応答関数テーブル 要素数
#define CMD_ACK_NUM    6

// コマンド実行関数テーブル 要素数
#define CMD_EXE_DBG    12      // デバッガ操作コマンド数
#define CMD_EXE_INP    16      // INPUTレジスタ設定コマンド数
#define CMD_EXE_ASM    13      // アセンブリ言語入力コマンド数
#define CMD_EXE_NUM    (CMD_EXE_DBG + CMD_EXE_INP + CMD_EXE_ASM)

// コマンド有効文字範囲
#define CMD_CHAR_MIN   32      // ' '：コマンド認識文字コード最小値
#define CMD_CHAR_MAX   126     // '~'：コマンド認識文字コード最大値

//============================================================================================
// 型定義 (内部用)
//============================================================================================

//============================================================================================
// プロトタイプ宣言 (static)
//============================================================================================
// コマンド応答関数
static BL blf_s_DbgCmdAck_CHAR(U1 u1_code, U1* u1p_buf, U1* u1p_index);   // コマンド有効文字
static BL blf_s_DbgCmdAck_UP__(U1 u1_code, U1* u1p_buf, U1* u1p_index);   // ↑ ：ヒストリ NEW
static BL blf_s_DbgCmdAck_DOWN(U1 u1_code, U1* u1p_buf, U1* u1p_index);   // ↓ ：ヒストリ OLD
static BL blf_s_DbgCmdAck_BS__(U1 u1_code, U1* u1p_buf, U1* u1p_index);   // BS ：入力後退
static BL blf_s_DbgCmdAck_ESC_(U1 u1_code, U1* u1p_buf, U1* u1p_index);   // ESC：誤入力防止
static BL blf_s_DbgCmdAck_CR__(U1 u1_code, U1* u1p_buf, U1* u1p_index);   // CR ：入力完結

// コマンド実行関数 (デバッガ操作)
static S1 s1f_s_DbgCmdExe_BS_n__(const U1* const u1cpc_cmd, U1* u1p_rom); // BS n BP設置
static S1 s1f_s_DbgCmdExe_BC_n__(const U1* const u1cpc_cmd, U1* u1p_rom); // BC n BP解除
static S1 s1f_s_DbgCmdExe_BC____(const U1* const u1cpc_cmd, U1* u1p_rom); // BC   BP全解除
static S1 s1f_s_DbgCmdExe_S_fn__(const U1* const u1cpc_cmd, U1* u1p_rom); // S fn：セーブ
static S1 s1f_s_DbgCmdExe_L_fn__(const U1* const u1cpc_cmd, U1* u1p_rom); // L fn：ロード
static S1 s1f_s_DbgCmdExe_T_____(const U1* const u1cpc_cmd, U1* u1p_rom); // T：1行実行
static S1 s1f_s_DbgCmdExe_N_____(const U1* const u1cpc_cmd, U1* u1p_rom); // N：PC ++ 
static S1 s1f_s_DbgCmdExe_P_____(const U1* const u1cpc_cmd, U1* u1p_rom); // P：PC --
static S1 s1f_s_DbgCmdExe_G_____(const U1* const u1cpc_cmd, U1* u1p_rom); // G：連続実行
static S1 s1f_s_DbgCmdExe_R_____(const U1* const u1cpc_cmd, U1* u1p_rom); // R：リセット
static S1 s1f_s_DbgCmdExe_O_____(const U1* const u1cpc_cmd, U1* u1p_rom); // O：クロック変更
static S1 s1f_s_DbgCmdExe_Q_____(const U1* const u1cpc_cmd, U1* u1p_rom); // Q：アプリ終了

// コマンド実行関数 (INPUTレジスタ設定)
static S1 s1f_s_DbgCmdExe_0_____(const U1* const u1cpc_cmd, U1* u1p_rom); // 0：INPUT = 0
static S1 s1f_s_DbgCmdExe_1_____(const U1* const u1cpc_cmd, U1* u1p_rom); // 1：INPUT = 1
static S1 s1f_s_DbgCmdExe_2_____(const U1* const u1cpc_cmd, U1* u1p_rom); // 2：INPUT = 2
static S1 s1f_s_DbgCmdExe_3_____(const U1* const u1cpc_cmd, U1* u1p_rom); // 3：INPUT = 3
static S1 s1f_s_DbgCmdExe_4_____(const U1* const u1cpc_cmd, U1* u1p_rom); // 4：INPUT = 4
static S1 s1f_s_DbgCmdExe_5_____(const U1* const u1cpc_cmd, U1* u1p_rom); // 5：INPUT = 5
static S1 s1f_s_DbgCmdExe_6_____(const U1* const u1cpc_cmd, U1* u1p_rom); // 6：INPUT = 6
static S1 s1f_s_DbgCmdExe_7_____(const U1* const u1cpc_cmd, U1* u1p_rom); // 7：INPUT = 7
static S1 s1f_s_DbgCmdExe_8_____(const U1* const u1cpc_cmd, U1* u1p_rom); // 8：INPUT = 8
static S1 s1f_s_DbgCmdExe_9_____(const U1* const u1cpc_cmd, U1* u1p_rom); // 9：INPUT = 9
static S1 s1f_s_DbgCmdExe_A_____(const U1* const u1cpc_cmd, U1* u1p_rom); // A：INPUT = A
static S1 s1f_s_DbgCmdExe_B_____(const U1* const u1cpc_cmd, U1* u1p_rom); // B：INPUT = B
static S1 s1f_s_DbgCmdExe_C_____(const U1* const u1cpc_cmd, U1* u1p_rom); // C：INPUT = C
static S1 s1f_s_DbgCmdExe_D_____(const U1* const u1cpc_cmd, U1* u1p_rom); // D：INPUT = D
static S1 s1f_s_DbgCmdExe_E_____(const U1* const u1cpc_cmd, U1* u1p_rom); // E：INPUT = E
static S1 s1f_s_DbgCmdExe_F_____(const U1* const u1cpc_cmd, U1* u1p_rom); // F：INPUT = F

// コマンド実行関数 (アセンブリ言語入力)
static S1 s1f_s_DbgCmdExe_NOP___(const U1* const u1cpc_cmd, U1* u1p_rom); // NOP
static S1 s1f_s_DbgCmdExe_ADD_AI(const U1* const u1cpc_cmd, U1* u1p_rom); // ADD A,Im
static S1 s1f_s_DbgCmdExe_MOV_AB(const U1* const u1cpc_cmd, U1* u1p_rom); // MOV A,B
static S1 s1f_s_DbgCmdExe_IN_A__(const U1* const u1cpc_cmd, U1* u1p_rom); // IN A
static S1 s1f_s_DbgCmdExe_MOV_AI(const U1* const u1cpc_cmd, U1* u1p_rom); // MOV A,Im
static S1 s1f_s_DbgCmdExe_MOV_BA(const U1* const u1cpc_cmd, U1* u1p_rom); // MOV B,A
static S1 s1f_s_DbgCmdExe_ADD_BI(const U1* const u1cpc_cmd, U1* u1p_rom); // ADD B,Im
static S1 s1f_s_DbgCmdExe_IN_B__(const U1* const u1cpc_cmd, U1* u1p_rom); // IN B
static S1 s1f_s_DbgCmdExe_MOV_BI(const U1* const u1cpc_cmd, U1* u1p_rom); // MOV B,Im
static S1 s1f_s_DbgCmdExe_OUT_B_(const U1* const u1cpc_cmd, U1* u1p_rom); // OUT B
static S1 s1f_s_DbgCmdExe_OUT_I_(const U1* const u1cpc_cmd, U1* u1p_rom); // OUT Im
static S1 s1f_s_DbgCmdExe_JNC_I_(const U1* const u1cpc_cmd, U1* u1p_rom); // JNC Im
static S1 s1f_s_DbgCmdExe_JMP_I_(const U1* const u1cpc_cmd, U1* u1p_rom); // JMP Im

//============================================================================================
// グローバル変数定義 (外部公開)
//============================================================================================

//============================================================================================
// 静的変数定義 (static)
//============================================================================================
// コマンド応答関数テーブル
static BL
(*const blfpac_s_DbgCmdAckFuncTbl[CMD_ACK_NUM])(U1 u1_code,U1* u1p_buf, U1* u1p_index) =
{
    blf_s_DbgCmdAck_CHAR,   // コマンド有効文字
    blf_s_DbgCmdAck_UP__,   // ↑  ：ヒストリ NEW
    blf_s_DbgCmdAck_DOWN,   // ↓  ：ヒストリ OLD
    blf_s_DbgCmdAck_BS__,   // BS  ：カーソル後退
    blf_s_DbgCmdAck_ESC_,   // ESC ：誤入力防止措置
    blf_s_DbgCmdAck_CR__,   // CR  ：入力完結
};

// コマンド実行関数テーブル
static S1
(*const s1fpac_s_DbgCmdExeFuncTbl[CMD_EXE_NUM])(const U1* const u1cpc_cmd, U1* u1p_rom) =
{
    // デバッガ操作
    s1f_s_DbgCmdExe_BS_n__, // BS n : ブレークポイント設置
    s1f_s_DbgCmdExe_BC_n__, // BC n : ブレークポイント解除
    s1f_s_DbgCmdExe_BC____, // BC   : ブレークポイントを全て解除
    s1f_s_DbgCmdExe_S_fn__, // S fn : ファイル保存
    s1f_s_DbgCmdExe_L_fn__, // L fn : ファイル読み込み
    s1f_s_DbgCmdExe_T_____, // T    : 現在PC行の命令のみ実行 (実行後にPCが進む)
    s1f_s_DbgCmdExe_N_____, // N    : 命令実行せずPCを1つ進める
    s1f_s_DbgCmdExe_P_____, // P    : 命令実行せずPCを1つ戻す
    s1f_s_DbgCmdExe_G_____, // G    : 連続実行モードにする (ESCキー押下で解除)
    s1f_s_DbgCmdExe_R_____, // R    : ターゲットCPUをリセット
    s1f_s_DbgCmdExe_O_____, // O    : 動作クロックをトグル (1Hz / 10Hz)
    s1f_s_DbgCmdExe_Q_____, // Q    ：アプリ終了

    // INPUTレジスタ設定
    s1f_s_DbgCmdExe_0_____, // 0：INPUT = 0
    s1f_s_DbgCmdExe_1_____, // 1：INPUT = 1
    s1f_s_DbgCmdExe_2_____, // 2：INPUT = 2
    s1f_s_DbgCmdExe_3_____, // 3：INPUT = 3
    s1f_s_DbgCmdExe_4_____, // 4：INPUT = 4
    s1f_s_DbgCmdExe_5_____, // 5：INPUT = 5
    s1f_s_DbgCmdExe_6_____, // 6：INPUT = 6
    s1f_s_DbgCmdExe_7_____, // 7：INPUT = 7
    s1f_s_DbgCmdExe_8_____, // 8：INPUT = 8
    s1f_s_DbgCmdExe_9_____, // 9：INPUT = 9
    s1f_s_DbgCmdExe_A_____, // A：INPUT = A
    s1f_s_DbgCmdExe_B_____, // B：INPUT = B
    s1f_s_DbgCmdExe_C_____, // C：INPUT = C
    s1f_s_DbgCmdExe_D_____, // D：INPUT = D
    s1f_s_DbgCmdExe_E_____, // E：INPUT = E
    s1f_s_DbgCmdExe_F_____, // F：INPUT = F

    // アセンブリ言語入力
    s1f_s_DbgCmdExe_NOP___, // NOP
    s1f_s_DbgCmdExe_ADD_AI, // ADD A,Im
    s1f_s_DbgCmdExe_MOV_AB, // MOV A,B
    s1f_s_DbgCmdExe_IN_A__, // IN  A
    s1f_s_DbgCmdExe_MOV_AI, // MOV A,Im
    s1f_s_DbgCmdExe_MOV_BA, // MOV B,A
    s1f_s_DbgCmdExe_ADD_BI, // ADD B,Im
    s1f_s_DbgCmdExe_IN_B__, // IN  B
    s1f_s_DbgCmdExe_MOV_BI, // MOV B,Im
    s1f_s_DbgCmdExe_OUT_B_, // OUT B
    s1f_s_DbgCmdExe_OUT_I_, // OUT Im
    s1f_s_DbgCmdExe_JNC_I_, // JNC Im
    s1f_s_DbgCmdExe_JMP_I_, // JMP Im
};

//============================================================================================
// 関数定義
//============================================================================================
//============================================================================================
// コマンド入力受付
//--------------------------------------------------------------------------------------------
// Arguments : U1* u1p_rom : (I) ROM格納配列の先頭アドレス
// Return    : S1  s1_ret  : (O) S1_RET_ERR3 : エラーコード 3
//                               S1_RET_ERR2 : エラーコード 2
//                               S1_RET_ERR1 : エラーコード 1
//                               S1_RET_QUIT : アプリ終了通知
//                               S1_RET_DONE : 実行済み / 成功
//                               S1_RET_SKIP : 未処理  (条件不一致)
//============================================================================================
S1 s1f_g_DbCmdRec(U1* u1p_rom)
{
    S4 s4_i;
    S1 s1_ret  = S1_RET_SKIP;                  // 戻り値：未処理
    BL bl_done = FALSE;                        // 入力完結フラグ
    U1 u1_code;                                // K_KEYINP()の戻り値(文字コード)受け取り用
    U1 u1a_buf[CMD_BUF_MAX] = { 0 };           // コマンド入力の文字列を受け取るバッファ
    U1 u1_index = 0;                           // バッフアのインデックス

    vdf_g_HisBegin();                          // ヒストリ準備

    while(1)                                   // 入力受付
    {
        u1_code = (U1)K_KEYINP();              // キー入力を1つ受け取るまで止まる(文字コード)

        for(s4_i = 0; s4_i < CMD_ACK_NUM; s4_i ++)
        {                                      // キー入力内容(文字コード)毎の処理
            if((*blfpac_s_DbgCmdAckFuncTbl[s4_i])(u1_code, u1a_buf, &u1_index))
            {
                bl_done = TRUE;                // 入力完結 (リターンキー押下)
                break;
            }
        }
        if(bl_done) break;                     // 入力完結 コマンド実行に進む
    }

    vdf_g_DbgDrawUpdateMess(MES_CLEAR);        // メッセージ消去

    for(s4_i = 0; s4_i < CMD_EXE_NUM; s4_i ++) // コマンド実行
    {
        s1_ret = (*s1fpac_s_DbgCmdExeFuncTbl[s4_i])(u1a_buf, u1p_rom);
        if((s1_ret == S1_RET_DONE) ||          // 実行済み
            s1_ret == S1_RET_QUIT) break;      // アプリ終了通知
    }

    return s1_ret;
}

//============================================================================================
// コマンドとして有効な文字キーの押下
//--------------------------------------------------------------------------------------------
// Arguments : U1  u1_code   : (I)   キー入力1つ分の文字コード
//             U1* u1p_buf   : (O)   入力文字コードを順次格納し文字列とするためのバッファ
//             U1* u1p_index : (I/O) 現在のバッファのインデックス
// Return    : BL  bl_ret    : (O)   TRUE  : 入力完結 (リターンキーが押下された)
//                                   FALSE : 入力途中 (リターンキー以外は常にFALSE)
//============================================================================================
static BL blf_s_DbgCmdAck_CHAR(U1 u1_code, U1* u1p_buf, U1* u1p_index)
{
    BL bl_ret = FALSE;                                  // 戻り値格納

    if(u1_code >= CMD_CHAR_MIN && u1_code <= CMD_CHAR_MAX)
    {
        if((*u1p_index) < CMD_BUF_MAX - 1)              // バッファ容量内であること
        {
            vdf_g_HisDisable();                         // ヒストリ状態を無効にする
            u1p_buf[(*u1p_index)] = u1_code;            // 1文字をバッファに格納
            putchar(u1p_buf[(*u1p_index)]);             // 1文字を表示
            (*u1p_index) ++;                            // 次のindexへ移動
        }
    }

    return bl_ret;
}

//============================================================================================
// ↑キーの押下 最新のヒストリから表示
//--------------------------------------------------------------------------------------------
// Arguments : U1  u1_code   : (I)   キー入力1つ分の文字コード
//             U1* u1p_buf   : (O)   入力文字コードを順次格納し文字列とするためのバッファ
//             U1* u1p_index : (I/O) 現在のバッファのインデックス
// Return    : BL  bl_ret    : (O)   TRUE  : 入力完結 (リターンキーが押下された)
//                                   FALSE : 入力途中 (リターンキー以外は常にFALSE)
//============================================================================================
static BL blf_s_DbgCmdAck_UP__(U1 u1_code, U1* u1p_buf, U1* u1p_index)
{
    BL bl_ret = FALSE;                                  // 戻り値格納

    if(u1_code == CHA_CODE_UP)                          // ↑キーが押されたら
    {
        if(blf_g_HisGetState() || (*u1p_index) <= 0)    // ヒストリ状態 または 0文字なら
        {
            vdf_g_DbgRetCurPos();                       // 前回ヒストリ文字列消去
            vdf_g_HisKeyUp(u1p_buf, u1p_index);         // ヒストリ↑押下時の処理
        }
    }

    return bl_ret;
}

//============================================================================================
// ↓キーの押下 最古のヒストリから表示
//--------------------------------------------------------------------------------------------
// Arguments : U1  u1_code   : (I)   キー入力1つ分の文字コード
//             U1* u1p_buf   : (O)   入力文字コードを順次格納し文字列とするためのバッファ
//             U1* u1p_index : (I/O) 現在のバッファのインデックス
// Return    : BL  bl_ret    : (O)   TRUE  : 入力完結 (リターンキーが押下された)
//                                   FALSE : 入力途中 (リターンキー以外は常にFALSE)
//============================================================================================
static BL blf_s_DbgCmdAck_DOWN(U1 u1_code, U1* u1p_buf, U1* u1p_index)
{
    BL bl_ret = FALSE;                                  // 戻り値格納

    if(u1_code == CHA_CODE_DOWN)                        // ↓キーが押されたら
    {
        if(blf_g_HisGetState() || (*u1p_index) <= 0)    // ヒストリ状態 または 0文字なら
        {
            vdf_g_DbgRetCurPos();                       // 前回ヒストリ文字列消去
            vdf_g_HisKeyDown(u1p_buf, u1p_index);       // ヒストリ↓押下時の処理
        }
    }

    return bl_ret;
}

//============================================================================================
// BSキーの押下 カーソル後退
//--------------------------------------------------------------------------------------------
// Arguments : U1  u1_code   : (I)   キー入力1つ分の文字コード
//             U1* u1p_buf   : (O)   入力文字コードを順次格納し文字列とするためのバッファ
//             U1* u1p_index : (I/O) 現在のバッファのインデックス
// Return    : BL  bl_ret    : (O)   TRUE  : 入力完結 (リターンキーが押下された)
//                                   FALSE : 入力途中 (リターンキー以外は常にFALSE)
//============================================================================================
static BL blf_s_DbgCmdAck_BS__(U1 u1_code, U1* u1p_buf, U1* u1p_index)
{
    BL bl_ret = FALSE;                                  // 戻り値格納

    if(u1_code == CHA_CODE_BS)                          // BSキーが押されたら
    {
        if((*u1p_index) > 0)                            // 既にバッファにデータがあるなら
        {
            vdf_g_HisDisable();                         // ヒストリ状態を無効にする
            (*u1p_index) --;                            // 末尾のデータを無かった事にする
            printf("\b \b");                            // 後方の1文字を消去
        }
    }

    return bl_ret;
}

//============================================================================================
// ESCキーのキーコードが入った場合 誤入力防止措置 (ESCキー以外の押下でも入る)
//--------------------------------------------------------------------------------------------
// Arguments : U1  u1_code   : (I)   キー入力1つ分の文字コード
//             U1* u1p_buf   : (O)   入力文字コードを順次格納し文字列とするためのバッファ
//             U1* u1p_index : (I/O) 現在のバッファのインデックス
// Return    : BL  bl_ret    : (O)   TRUE  : 入力完結 (リターンキーが押下された)
//                                   FALSE : 入力途中 (リターンキー以外は常にFALSE)
//--------------------------------------------------------------------------------------------
// Note      :
// ファンクションキー, Insert(INS)キー, Home(HOME)キーは、ESCキーと同じキーコードを先頭とした
// 2バイト組み合わせとなり、これが動作を阻害する。
// このため、K_KEYINP()を空コールして一度読み捨てる。
//============================================================================================
static BL blf_s_DbgCmdAck_ESC_(U1 u1_code, U1* u1p_buf, U1* u1p_index)
{
    BL bl_ret = FALSE;                                  // 戻り値格納

    if(u1_code == CHA_CODE_ESC)                         // ESCキーのコードの場合
    {
        K_KEYINP();                                     // 空コールして読み捨てる
    }

    return bl_ret;
}

//============================================================================================
// リターンキーの押下 入力完結
//--------------------------------------------------------------------------------------------
// Arguments : U1  u1_code   : (I)   キー入力1つ分の文字コード
//             U1* u1p_buf   : (O)   入力文字コードを順次格納し文字列とするためのバッファ
//             U1* u1p_index : (I/O) 現在のバッファのインデックス
// Return    : BL  bl_ret    : (O)   TRUE  : 入力完結 (リターンキーが押下された)
//                                   FALSE : 入力途中 (リターンキー以外は常にFALSE)
//============================================================================================
static BL blf_s_DbgCmdAck_CR__(U1 u1_code, U1* u1p_buf, U1* u1p_index)
{
    BL bl_ret = FALSE;                                  // 戻り値格納

    if(u1_code == CHA_CODE_CR)                          // リターンキーが押されたら
    {
        u1p_buf[(*u1p_index)] = '\0';                   // 末尾にNULL文字を付与する
        blf_g_HisWrite(u1p_buf);                        // ヒストリへの書き込み
        bl_ret = TRUE;                                  // 入力完結 (リターンキー押下)
    }

    return bl_ret;
}

//============================================================================================
// BS n コマンド - Brake point Set ブレークポイント設置
// 入力例 : -BS n
// 引数   : n = ブレークポイントを設定するROMアドレス (0～F)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列 (ファイル名を含む)
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_BS_n__(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret  = S1_RET_SKIP;                           // 戻り値：未処理
    U4 u4_addr = 0;                                     // 設置先アドレス

    if((u1cpc_cmd[0] == 'B'  || u1cpc_cmd[0] == 'b') && // コマンド判別
       (u1cpc_cmd[1] == 'S'  || u1cpc_cmd[1] == 's') &&
        u1cpc_cmd[2] == ' '  &&
      ((u1cpc_cmd[3] >= '0'  && u1cpc_cmd[3] <= '9') ||
       (u1cpc_cmd[3] >= 'a'  && u1cpc_cmd[3] <= 'f') ||
       (u1cpc_cmd[3] >= 'A'  && u1cpc_cmd[3] <= 'F')) &&
       (u1cpc_cmd[4] == '\n' || u1cpc_cmd[4] == '\0'))
    {
        if(u1cpc_cmd[3] >= '0'  && u1cpc_cmd[3] <= '9') // 引数判別
        {
            u4_addr = (U4)u1cpc_cmd[3] - '0';
        }
        else if (u1cpc_cmd[3] >= 'a'  && u1cpc_cmd[3] <= 'f')
        {
            u4_addr = (U4)u1cpc_cmd[3] - 'a' + 10;
        }
        else if (u1cpc_cmd[3] >= 'A'  && u1cpc_cmd[3] <= 'F')
        {
            u4_addr = (U4)u1cpc_cmd[3] - 'A' + 10;
        }

        vdf_g_DbgSetBP(u4_addr, TRUE);                  // ブレークポイント設置
        vdf_g_DbgDrawUpdateBp(u4_addr);                 // ブレークポイントの表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// BC n コマンド - Brake point Clear ブレークポイント解除
// 入力例 : -BC n
// 引数   : n = ブレークポイントを解除するROMアドレス (0～F)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列 (ファイル名を含む)
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_BC_n__(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret  = S1_RET_SKIP;                           // 戻り値：未処理
    U4 u4_addr = 0;                                     // 設置先アドレス

    if((u1cpc_cmd[0] == 'B'  || u1cpc_cmd[0] == 'b') && // コマンド判別
       (u1cpc_cmd[1] == 'C'  || u1cpc_cmd[1] == 'c') &&
        u1cpc_cmd[2] == ' '  &&
      ((u1cpc_cmd[3] >= '0'  && u1cpc_cmd[3] <= '9') ||
       (u1cpc_cmd[3] >= 'a'  && u1cpc_cmd[3] <= 'f') ||
       (u1cpc_cmd[3] >= 'A'  && u1cpc_cmd[3] <= 'F')) &&
       (u1cpc_cmd[4] == '\n' || u1cpc_cmd[4] == '\0'))
    {
        if(u1cpc_cmd[3] >= '0'  && u1cpc_cmd[3] <= '9') // 引数判別
        {
            u4_addr = (U4)u1cpc_cmd[3] - '0';
        }
        else if (u1cpc_cmd[3] >= 'a'  && u1cpc_cmd[3] <= 'f')
        {
            u4_addr = (U4)u1cpc_cmd[3] - 'a' + 10;
        }
        else if (u1cpc_cmd[3] >= 'A'  && u1cpc_cmd[3] <= 'F')
        {
            u4_addr = (U4)u1cpc_cmd[3] - 'A' + 10;
        }

        vdf_g_DbgSetBP(u4_addr, FALSE);                 // ブレークポイント解除
        vdf_g_DbgDrawUpdateBp(u4_addr);                 // ブレークポイントの表示更新

        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// BC コマンド - Brake point Clear ブレークポイント解除
// 入力例 : -BC
// 引数   : なし (全てのBPを解除)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列 (ファイル名を含む)
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_BC____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S4 s4_i;
    S1 s1_ret  = S1_RET_SKIP;                           // 戻り値：未処理
    U4 u4_addr = 0;                                     // 設置先アドレス

    if((u1cpc_cmd[0] == 'B'  || u1cpc_cmd[0] == 'b') && // コマンド判別
       (u1cpc_cmd[1] == 'C'  || u1cpc_cmd[1] == 'c') && // 引数なし
       (u1cpc_cmd[2] == '\n' || u1cpc_cmd[2] == '\0'))
    {
        for(s4_i = 0; s4_i < ROM_ADR_MAX; s4_i ++)
        {
            vdf_g_DbgSetBP(s4_i, FALSE);                // ブレークポイント解除
            vdf_g_DbgDrawUpdateBp(s4_i);                // ブレークポイントの表示更新
        }
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// S コマンド - Save ファイル保存
// 入力例 : -S fn
// 引数   : fn = パスおよびファイル名 (./rom/ramen_timer.tdx など)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const  u1cpc_cmd : (I) コマンド文字列 (ファイル名を含む)
//             U1*              u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1               s1_ret    : (O) S1_RET_SKIP : 未処理
//                                              S1_RET_DONE : ファイルセーブ成功
//                                              S1_RET_ERR1 : ファイル名にNULL文字が無い
//                                              S1_RET_ERR2 : ファイル名に不正な文字がある
//                                              S1_RET_ERR3 : ファイルセーブ失敗
//============================================================================================
static S1 s1f_s_DbgCmdExe_S_fn__(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S4 s4_i;
    S1 s1_ret = S1_RET_SKIP;                                      // 戻り値：未処理

    if((u1cpc_cmd[0] == 'S'  || u1cpc_cmd[0] == 's') &&           // コマンド判別
        u1cpc_cmd[1] == ' '  &&                                   // 最低1文字以上必要
       (u1cpc_cmd[2] >= CMD_CHAR_MIN) && (u1cpc_cmd[2] <= CMD_CHAR_MAX))
    {
        for(s4_i = 3; s4_i < CMD_BUF_MAX; s4_i ++)                // NULL文字はあるか？
        {
            if(u1cpc_cmd[s4_i] == '\0') break;
        }
        if(s4_i == CMD_BUF_MAX)                                   // エラー : NULL文字なし
        {
            vdf_g_DbgDrawUpdateMess(MES_FN_ER_NULL);              // エラーメッセージ表示
            s1_ret = S1_RET_ERR1;                                 // 戻り値：エラーコード 1
        }
        else                                                      // NULL文字あり
        {
            // NULL文字までの間にパス名/ファイル名として不正な文字がないかチェック
            for(s4_i = 3; u1cpc_cmd[s4_i] != '\0'; s4_i ++)
            {
                if((u1cpc_cmd[s4_i] < CMD_CHAR_MIN) || (u1cpc_cmd[s4_i] > CMD_CHAR_MAX)) break;
            }

            if(u1cpc_cmd[s4_i] != '\0')                           // エラー : 不正な文字を検出
            {
                vdf_g_DbgDrawUpdateMess(MES_FN_ER_NSTD);          // エラーメッセージ表示
                s1_ret = S1_RET_ERR2;                             // 戻り値：エラーコード 2
            }
            else                                                  // 不正な文字が無かった場合
            {
                if(!blf_g_DbgSaveRomFile(&u1cpc_cmd[2], u1p_rom)) // ファイルセーブ
                {                                                 // エラー : 失敗
                    vdf_g_DbgDrawUpdateMess(MES_FS_FAILURE);      // エラーメッセージ表示
                    s1_ret = S1_RET_ERR3;                         // 戻り値：エラーコード 3
                }
                else                                              // 成功
                {
                    vdf_g_DbgDrawUpdateMess(MES_FS_SUCCESS);      // 成功メッセージ表示
                    vdf_g_DbgDrawUpdateFName(&u1cpc_cmd[2]);      // ファイル名表示を更新
                    s1_ret = S1_RET_DONE;                         // 戻り値：成功
                }
            }
        }
    }

    return s1_ret;
}

//============================================================================================
// L コマンド - Load ファイル読み込み
// 入力例 : -L fn
// 引数   : fn = パスおよびファイル名 (./rom/ramen_timer.tdx など)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列 (ファイル名を含む)
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : ファイルロード成功
//                                             S1_RET_ERR1 : ファイル名にNULL文字が無い
//                                             S1_RET_ERR2 : ファイル名に不正な文字がある
//                                             S1_RET_ERR3 : ファイルロード失敗
//============================================================================================
static S1 s1f_s_DbgCmdExe_L_fn__(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S4 s4_i;
    S1 s1_ret = S1_RET_SKIP;                                      // 戻り値：未処理

    if((u1cpc_cmd[0] == 'L'  || u1cpc_cmd[0] == 'l') &&           // コマンド判別
        u1cpc_cmd[1] == ' '  &&                                   // 最低1文字以上必要
       (u1cpc_cmd[2] >= CMD_CHAR_MIN) && (u1cpc_cmd[2] <= CMD_CHAR_MAX))
    {
        for(s4_i = 3; s4_i < CMD_BUF_MAX; s4_i ++)                // NULL文字はあるか？
        {
            if(u1cpc_cmd[s4_i] == '\0') break;
        }
        if(s4_i == CMD_BUF_MAX)                                   // エラー : NULL文字なし
        {
            vdf_g_DbgDrawUpdateMess(MES_FN_ER_NULL);              // エラーメッセージ表示
            s1_ret = S1_RET_ERR1;                                 // 戻り値：エラーコード 1
        }
        else                                                      // NULL文字あり
        {
            // NULL文字までの間にパス名/ファイル名として不正な文字がないかチェック
            for(s4_i = 3; u1cpc_cmd[s4_i] != '\0'; s4_i ++)
            {
                if((u1cpc_cmd[s4_i] < CMD_CHAR_MIN) || (u1cpc_cmd[s4_i] > CMD_CHAR_MAX)) break;
            }

            if(u1cpc_cmd[s4_i] != '\0')                           // エラー : 不正な文字を検出
            {
                vdf_g_DbgDrawUpdateMess(MES_FN_ER_NSTD);          // エラーメッセージ表示
                s1_ret = S1_RET_ERR2;                             // 戻り値：エラーコード 2
            }
            else                                                  // 不正な文字が無かった場合
            {
                if(!blf_g_DbgLoadRomFile(&u1cpc_cmd[2], u1p_rom)) // ファイルロード
                {                                                 // エラー : 失敗
                    vdf_g_DbgDrawUpdateMess(MES_FL_FAILURE);      // エラーメッセージ表示
                    s1_ret = S1_RET_ERR3;                         // 戻り値：エラーコード 3
                }
                else                                              // 成功
                {
                    vdf_g_DbgDrawUpdateMess(MES_FL_SUCCESS);      // 成功メッセージ表示
                    vdf_g_DbgDrawUpdateFName(&u1cpc_cmd[2]);      // ファイル名表示を更新

                    for(s4_i = 0; s4_i < ROM_ADR_MAX; s4_i ++)
                    {
                        vdf_g_DbgDrawUpdateList(s4_i, u1p_rom);   // リストの表示を更新
                        s1_ret = S1_RET_DONE;                     // 戻り値：成功
                    }
                }
            }
        }
    }

    return s1_ret;
}

//============================================================================================
// T コマンド - Trace 現在PC行の命令のみ実行 (実行後はPCが変化)
// 入力例 : -T
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_T_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret      = S1_RET_SKIP;                       // 戻り値：未処理
    S1 s1_inst_ret = CPU_RET_UINS;                      // ターゲットCPU命令の戻り値

    if((u1cpc_cmd[0] == 'T'  || u1cpc_cmd[0] == 't') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_DbgExe1Clk(u1p_rom);                      // ターゲットCPUを1クロック実行
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// N コマンド - Next 命令実行せずPCを1つ進める
// 入力例 : -N
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_N_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if((u1cpc_cmd[0] == 'N'  || u1cpc_cmd[0] == 'n') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_DbgSetPrevPC(u4f_g_CpuDebugGetPc());      // 1つ前のプログラムカウンタを記録
        vdf_g_CpuDebugIncPc();                          // PCを1つ進める (デバッグ機能)
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// P コマンド - Prev 命令実行せずPCを1つ戻す
// 入力例 : -P
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_P_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if((u1cpc_cmd[0] == 'P'  || u1cpc_cmd[0] == 'p') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_DbgSetPrevPC(u4f_g_CpuDebugGetPc());      // 1つ前のプログラムカウンタを記録
        vdf_g_CpuDebugDecPc();                          // PCを1つ戻す (デバッグ機能)
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// G コマンド - Go オートモードにする (ESCキー押下で解除)
// 入力例 : -G
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_G_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret      = S1_RET_SKIP;                       // 戻り値：未処理
    S1 s1_inst_ret = CPU_RET_UINS;                      // ターゲットCPU命令の戻り値

    if((u1cpc_cmd[0] == 'G'  || u1cpc_cmd[0] == 'g') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_DbgSetOpMode(DBG_MODE_AT);                // デバッガをオートモードに変更
        vdf_g_DbgExe1Clk(u1p_rom);                      // ターゲットCPUを1クロック実行
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// R コマンド - Reset ターゲットCPUをリセット
// 入力例 : -R
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_R_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if((u1cpc_cmd[0] == 'R'  || u1cpc_cmd[0] == 'r') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_DbgSetPrevPC(u4f_g_CpuDebugGetPc());      // 1つ前のプログラムカウンタを記録
        vdf_g_CpuReset(RST_TYPE_WARM);                  // ターゲットCPUを端子リセット
        vdf_g_DbgDrawUpdateMess(MES_RESET);             // リセットのメッセージ表示
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// O コマンド - Oscillation 動作クロックをトグル (1Hz / 10Hz)
// 入力例 : -O
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_O_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if((u1cpc_cmd[0] == 'O'  || u1cpc_cmd[0] == 'o') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_DbgDrawUpdateClockTgl();                  // クロックのトグルと表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// Q コマンド - Quit アプリ終了
// 入力例 : -Q
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_QUIT : アプリ終了通知
//============================================================================================
static S1 s1f_s_DbgCmdExe_Q_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if((u1cpc_cmd[0] == 'Q'  || u1cpc_cmd[0] == 'q') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_DbgDrawUpdateMess(MES_QUIT);              // アプリ終了のメッセージ表示
        s1_ret = S1_RET_QUIT;                           // 戻り値：アプリ終了通知
    }

    return s1_ret;
}

//============================================================================================
// 0 コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0x0 を設定する。
// 入力例 : -0
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_0_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if(u1cpc_cmd[0] == '0'  &&                          // コマンド判別
      (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))   // 引数なし
    {
        vdf_g_CpuSetInput(0x0);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// 1 コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0x1 を設定する。
// 入力例 : -1
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_1_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if(u1cpc_cmd[0] == '1'  &&                          // コマンド判別
      (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))   // 引数なし
    {
        vdf_g_CpuSetInput(0x1);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// 2 コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0x2 を設定する。
// 入力例 : -2
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_2_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if(u1cpc_cmd[0] == '2'  &&                          // コマンド判別
      (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))   // 引数なし
    {
        vdf_g_CpuSetInput(0x2);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// 3 コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0x3 を設定する。
// 入力例 : -3
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_3_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if(u1cpc_cmd[0] == '3'  &&                          // コマンド判別
      (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))   // 引数なし
    {
        vdf_g_CpuSetInput(0x3);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// 4 コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0x4 を設定する。
// 入力例 : -4
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_4_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if(u1cpc_cmd[0] == '4'  &&                          // コマンド判別
      (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))   // 引数なし
    {
        vdf_g_CpuSetInput(0x4);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// 5 コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0x5 を設定する。
// 入力例 : -5
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_5_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if(u1cpc_cmd[0] == '5'  &&                          // コマンド判別
      (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))   // 引数なし
    {
        vdf_g_CpuSetInput(0x5);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// 6 コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0x6 を設定する。
// 入力例 : -5
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_6_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if(u1cpc_cmd[0] == '6'  &&                          // コマンド判別
      (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))   // 引数なし
    {
        vdf_g_CpuSetInput(0x6);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// 7 コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0x7 を設定する。
// 入力例 : -7
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_7_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if(u1cpc_cmd[0] == '7'  &&                          // コマンド判別
      (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))   // 引数なし
    {
        vdf_g_CpuSetInput(0x7);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// 8 コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0x8 を設定する。
// 入力例 : -8
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_8_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if(u1cpc_cmd[0] == '8'  &&                          // コマンド判別
      (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))   // 引数なし
    {
        vdf_g_CpuSetInput(0x8);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// 9 コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0x9 を設定する。
// 入力例 : -9
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_9_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if(u1cpc_cmd[0] == '9'  &&                          // コマンド判別
      (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))   // 引数なし
    {
        vdf_g_CpuSetInput(0x9);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// A コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0xA を設定する。
// 入力例 : -A
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_A_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if((u1cpc_cmd[0] == 'A'  || u1cpc_cmd[0] == 'a') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_CpuSetInput(0xA);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// B コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0xB を設定する。
// 入力例 : -B
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_B_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if((u1cpc_cmd[0] == 'B'  || u1cpc_cmd[0] == 'b') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_CpuSetInput(0xB);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// C コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0xC を設定する。
// 入力例 : -C
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_C_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if((u1cpc_cmd[0] == 'C'  || u1cpc_cmd[0] == 'c') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_CpuSetInput(0xC);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// D コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0xD を設定する。
// 入力例 : -D
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_D_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if((u1cpc_cmd[0] == 'D'  || u1cpc_cmd[0] == 'd') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_CpuSetInput(0xD);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// E コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0xE を設定する。
// 入力例 : -E
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_E_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if((u1cpc_cmd[0] == 'E'  || u1cpc_cmd[0] == 'e') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_CpuSetInput(0xE);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// F コマンド - INPUTレジスタ(外部入力DIPスイッチ)に 0xF を設定する。
// 入力例 : -F
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_F_____(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理

    if((u1cpc_cmd[0] == 'F'  || u1cpc_cmd[0] == 'f') && // コマンド判別
       (u1cpc_cmd[1] == '\n' || u1cpc_cmd[1] == '\0'))  // 引数なし
    {
        vdf_g_CpuSetInput(0xF);                         // INPUTレジスタに値を反映させる
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// NOP コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : NOP
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_NOP___(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理
    U4 u4_pc  = u4f_g_CpuDebugGetPc();                  // プログラムカウンタ取得

    if((u1cpc_cmd[0] == 'N'  || u1cpc_cmd[0] == 'n') && // 命令判別 
       (u1cpc_cmd[1] == 'O'  || u1cpc_cmd[1] == 'o') && // 引数なし
       (u1cpc_cmd[2] == 'P'  || u1cpc_cmd[2] == 'p') &&
       (u1cpc_cmd[3] == '\n' || u1cpc_cmd[3] == '\0'))
    {
        u1p_rom[u4_pc] = 0x00;                          // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// ADD A,Im コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : ADD A,$n
// 引数   : n = Aレジスタに加算する値 (0～F)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_ADD_AI(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret     = S1_RET_SKIP;                        // 戻り値：未処理
    U4 u4_pc      = u4f_g_CpuDebugGetPc();              // プログラムカウンタ取得
    U1 u1_opcode  = 0x0;                                // オペコード(命令：上位ニブル)
    U1 u1_operand = 0x0;                                // オペランド(引数：下位ニブル)
    U1 u1_code    = 0x00;                               // マシン語コード(命令 & 引数)

    if((u1cpc_cmd[0] == 'A'  || u1cpc_cmd[0] == 'a') && // 命令判別
       (u1cpc_cmd[1] == 'D'  || u1cpc_cmd[1] == 'd') &&
       (u1cpc_cmd[2] == 'D'  || u1cpc_cmd[2] == 'd') &&
        u1cpc_cmd[3] == ' '  &&
       (u1cpc_cmd[4] == 'A'  || u1cpc_cmd[4] == 'a') &&
        u1cpc_cmd[5] == ','  &&
        u1cpc_cmd[6] == '$'  &&
      ((u1cpc_cmd[7] >= '0'  && u1cpc_cmd[7] <= '9') ||
       (u1cpc_cmd[7] >= 'a'  && u1cpc_cmd[7] <= 'f') ||
       (u1cpc_cmd[7] >= 'A'  && u1cpc_cmd[7] <= 'F')) &&
       (u1cpc_cmd[8] == '\n' || u1cpc_cmd[8] == '\0'))
    {
        if(u1cpc_cmd[7] >= '0'  && u1cpc_cmd[7] <= '9') // 引数判別
        {
            u1_operand = u1cpc_cmd[7] - '0';
        }
        else if (u1cpc_cmd[7] >= 'a'  && u1cpc_cmd[7] <= 'f')
        {
            u1_operand = u1cpc_cmd[7] - 'a' + 10;
        }
        else if (u1cpc_cmd[7] >= 'A'  && u1cpc_cmd[7] <= 'F')
        {
            u1_operand = u1cpc_cmd[7] - 'A' + 10;
        }

        u1_code |= (u1_opcode << 4);                    // 命令は上位ニブル
        u1_code |= u1_operand;                          // 引数を合成
        u1p_rom[u4_pc] = u1_code;                       // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// MOV A,B コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : MOV A,B
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_MOV_AB(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理
    U4 u4_pc  = u4f_g_CpuDebugGetPc();                  // プログラムカウンタ取得

    if((u1cpc_cmd[0] == 'M'  || u1cpc_cmd[0] == 'm') && // 命令判別
       (u1cpc_cmd[1] == 'O'  || u1cpc_cmd[1] == 'o') && // 引数なし
       (u1cpc_cmd[2] == 'V'  || u1cpc_cmd[2] == 'v') &&
        u1cpc_cmd[3] == ' '  &&
       (u1cpc_cmd[4] == 'A'  || u1cpc_cmd[4] == 'a') &&
        u1cpc_cmd[5] == ','  &&
       (u1cpc_cmd[6] == 'B'  || u1cpc_cmd[6] == 'b') &&
       (u1cpc_cmd[7] == '\n' || u1cpc_cmd[7] == '\0'))
    {
        u1p_rom[u4_pc] = 0x10;                          // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// IN A コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : IN A
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_IN_A__(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理
    U4 u4_pc  = u4f_g_CpuDebugGetPc();                  // プログラムカウンタ取得

    if((u1cpc_cmd[0] == 'I'  || u1cpc_cmd[0] == 'i') && // 命令判別
       (u1cpc_cmd[1] == 'N'  || u1cpc_cmd[1] == 'n') && // 引数なし
        u1cpc_cmd[2] == ' '  &&
       (u1cpc_cmd[3] == 'A'  || u1cpc_cmd[3] == 'a') &&
       (u1cpc_cmd[4] == '\n' || u1cpc_cmd[4] == '\0'))
    {
        u1p_rom[u4_pc] = 0x20;                          // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// MOV A,Im コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : MOV A,$n
// 引数   : n = Aレジスタに格納する値 (0～F)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_MOV_AI(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret     = S1_RET_SKIP;                        // 戻り値：未処理
    U4 u4_pc      = u4f_g_CpuDebugGetPc();              // プログラムカウンタ取得
    U1 u1_opcode  = 0x3;                                // オペコード(命令：上位ニブル)
    U1 u1_operand = 0x0;                                // オペランド(引数：下位ニブル)
    U1 u1_code    = 0x00;                               // マシン語コード(命令 & 引数)

    if((u1cpc_cmd[0] == 'M'  || u1cpc_cmd[0] == 'm') && // 命令判別
       (u1cpc_cmd[1] == 'O'  || u1cpc_cmd[1] == 'o') &&
       (u1cpc_cmd[2] == 'V'  || u1cpc_cmd[2] == 'v') &&
        u1cpc_cmd[3] == ' '  &&
       (u1cpc_cmd[4] == 'A'  || u1cpc_cmd[4] == 'a') &&
        u1cpc_cmd[5] == ','  &&
        u1cpc_cmd[6] == '$'  &&
      ((u1cpc_cmd[7] >= '0'  && u1cpc_cmd[7] <= '9') ||
       (u1cpc_cmd[7] >= 'a'  && u1cpc_cmd[7] <= 'f') ||
       (u1cpc_cmd[7] >= 'A'  && u1cpc_cmd[7] <= 'F')) &&
       (u1cpc_cmd[8] == '\n' || u1cpc_cmd[8] == '\0'))
    {
        if(u1cpc_cmd[7] >= '0'  && u1cpc_cmd[7] <= '9') // 引数判別
        {
            u1_operand = u1cpc_cmd[7] - '0';
        }
        else if (u1cpc_cmd[7] >= 'a'  && u1cpc_cmd[7] <= 'f')
        {
            u1_operand = u1cpc_cmd[7] - 'a' + 10;
        }
        else if (u1cpc_cmd[7] >= 'A'  && u1cpc_cmd[7] <= 'F')
        {
            u1_operand = u1cpc_cmd[7] - 'A' + 10;
        }

        u1_code |= (u1_opcode << 4);                    // 命令は上位ニブル
        u1_code |= u1_operand;                          // 引数を合成
        u1p_rom[u4_pc] = u1_code;                       // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// MOV B,A コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : MOV B,A
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_MOV_BA(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理
    U4 u4_pc  = u4f_g_CpuDebugGetPc();                  // プログラムカウンタ取得

    if((u1cpc_cmd[0] == 'M'  || u1cpc_cmd[0] == 'm') && // 命令判別
       (u1cpc_cmd[1] == 'O'  || u1cpc_cmd[1] == 'o') && // 引数なし
       (u1cpc_cmd[2] == 'V'  || u1cpc_cmd[2] == 'v') &&
        u1cpc_cmd[3] == ' '  &&
       (u1cpc_cmd[4] == 'B'  || u1cpc_cmd[4] == 'b') &&
        u1cpc_cmd[5] == ','  &&
       (u1cpc_cmd[6] == 'A'  || u1cpc_cmd[6] == 'a') &&
       (u1cpc_cmd[7] == '\n' || u1cpc_cmd[7] == '\0'))
    {
        u1p_rom[u4_pc] = 0x40;                          // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// ADD B,Im コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : ADD B,$n
// 引数   : n = Bレジスタに加算する値 (0～F)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_ADD_BI(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret     = S1_RET_SKIP;                        // 戻り値：未処理
    U4 u4_pc      = u4f_g_CpuDebugGetPc();              // プログラムカウンタ取得
    U1 u1_opcode  = 0x5;                                // オペコード(命令：上位ニブル)
    U1 u1_operand = 0x0;                                // オペランド(引数：下位ニブル)
    U1 u1_code    = 0x00;                               // マシン語コード(命令 & 引数)

    if((u1cpc_cmd[0] == 'A'  || u1cpc_cmd[0] == 'a') && // 命令判別
       (u1cpc_cmd[1] == 'D'  || u1cpc_cmd[1] == 'd') &&
       (u1cpc_cmd[2] == 'D'  || u1cpc_cmd[2] == 'd') &&
        u1cpc_cmd[3] == ' '  &&
       (u1cpc_cmd[4] == 'B'  || u1cpc_cmd[4] == 'b') &&
        u1cpc_cmd[5] == ','  &&
        u1cpc_cmd[6] == '$'  &&
      ((u1cpc_cmd[7] >= '0'  && u1cpc_cmd[7] <= '9') ||
       (u1cpc_cmd[7] >= 'a'  && u1cpc_cmd[7] <= 'f') ||
       (u1cpc_cmd[7] >= 'A'  && u1cpc_cmd[7] <= 'F')) &&
       (u1cpc_cmd[8] == '\n' || u1cpc_cmd[8] == '\0'))
    {
        if(u1cpc_cmd[7] >= '0'  && u1cpc_cmd[7] <= '9') // 引数判別
        {
            u1_operand = u1cpc_cmd[7] - '0';
        }
        else if (u1cpc_cmd[7] >= 'a'  && u1cpc_cmd[7] <= 'f')
        {
            u1_operand = u1cpc_cmd[7] - 'a' + 10;
        }
        else if (u1cpc_cmd[7] >= 'A'  && u1cpc_cmd[7] <= 'F')
        {
            u1_operand = u1cpc_cmd[7] - 'A' + 10;
        }

        u1_code |= (u1_opcode << 4);                    // 命令は上位ニブル
        u1_code |= u1_operand;                          // 引数を合成
        u1p_rom[u4_pc] = u1_code;                       // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// IN B コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : IN B
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_IN_B__(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理
    U4 u4_pc  = u4f_g_CpuDebugGetPc();                  // プログラムカウンタ取得

    if((u1cpc_cmd[0] == 'I'  || u1cpc_cmd[0] == 'i') && // 命令判別
       (u1cpc_cmd[1] == 'N'  || u1cpc_cmd[1] == 'n') && // 引数なし
        u1cpc_cmd[2] == ' '  &&
       (u1cpc_cmd[3] == 'B'  || u1cpc_cmd[3] == 'b') &&
       (u1cpc_cmd[4] == '\n' || u1cpc_cmd[4] == '\0'))
    {
        u1p_rom[u4_pc] = 0x60;                          // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// MOV B,Im コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : MOV B,$n
// 引数   : n = Bレジスタに格納する値 (0～F)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_MOV_BI(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret     = S1_RET_SKIP;                        // 戻り値：未処理
    U4 u4_pc      = u4f_g_CpuDebugGetPc();              // プログラムカウンタ取得
    U1 u1_opcode  = 0x7;                                // オペコード(命令：上位ニブル)
    U1 u1_operand = 0x0;                                // オペランド(引数：下位ニブル)
    U1 u1_code    = 0x00;                               // マシン語コード(命令 & 引数)

    if((u1cpc_cmd[0] == 'M'  || u1cpc_cmd[0] == 'm') && // 命令判別
       (u1cpc_cmd[1] == 'O'  || u1cpc_cmd[1] == 'o') &&
       (u1cpc_cmd[2] == 'V'  || u1cpc_cmd[2] == 'v') &&
        u1cpc_cmd[3] == ' '  &&
       (u1cpc_cmd[4] == 'B'  || u1cpc_cmd[4] == 'b') &&
        u1cpc_cmd[5] == ','  &&
        u1cpc_cmd[6] == '$'  &&
      ((u1cpc_cmd[7] >= '0'  && u1cpc_cmd[7] <= '9') ||
       (u1cpc_cmd[7] >= 'a'  && u1cpc_cmd[7] <= 'f') ||
       (u1cpc_cmd[7] >= 'A'  && u1cpc_cmd[7] <= 'F')) &&
       (u1cpc_cmd[8] == '\n' || u1cpc_cmd[8] == '\0'))
    {
        if(u1cpc_cmd[7] >= '0'  && u1cpc_cmd[7] <= '9') // 引数判別
        {
            u1_operand = u1cpc_cmd[7] - '0';
        }
        else if (u1cpc_cmd[7] >= 'a'  && u1cpc_cmd[7] <= 'f')
        {
            u1_operand = u1cpc_cmd[7] - 'a' + 10;
        }
        else if (u1cpc_cmd[7] >= 'A'  && u1cpc_cmd[7] <= 'F')
        {
            u1_operand = u1cpc_cmd[7] - 'A' + 10;
        }

        u1_code |= (u1_opcode << 4);                    // 命令は上位ニブル
        u1_code |= u1_operand;                          // 引数を合成
        u1p_rom[u4_pc] = u1_code;                       // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// OUT B コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : OUT B
// 引数   : なし
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_OUT_B_(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret = S1_RET_SKIP;                            // 戻り値：未処理
    U4 u4_pc  = u4f_g_CpuDebugGetPc();                  // プログラムカウンタ取得

    if((u1cpc_cmd[0] == 'O'  || u1cpc_cmd[0] == 'o') && // 命令判別
       (u1cpc_cmd[1] == 'U'  || u1cpc_cmd[1] == 'u') && // 引数なし
       (u1cpc_cmd[2] == 'T'  || u1cpc_cmd[2] == 't') &&
        u1cpc_cmd[3] == ' '  &&
       (u1cpc_cmd[4] == 'B'  || u1cpc_cmd[4] == 'b') &&
       (u1cpc_cmd[5] == '\n' || u1cpc_cmd[5] == '\0'))
    {
        u1p_rom[u4_pc] = 0x90;                          // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// OUT Im コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : OUT $n
// 引数   : n = Oレジスタに格納する値 (0～F)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_OUT_I_(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret     = S1_RET_SKIP;                        // 戻り値：未処理
    U4 u4_pc      = u4f_g_CpuDebugGetPc();              // プログラムカウンタ取得
    U1 u1_opcode  = 0xB;                                // オペコード(命令：上位ニブル)
    U1 u1_operand = 0x0;                                // オペランド(引数：下位ニブル)
    U1 u1_code    = 0x00;                               // マシン語コード(命令 & 引数)

    if((u1cpc_cmd[0] == 'O'  || u1cpc_cmd[0] == 'o') && // 命令判別
       (u1cpc_cmd[1] == 'U'  || u1cpc_cmd[1] == 'u') &&
       (u1cpc_cmd[2] == 'T'  || u1cpc_cmd[2] == 't') &&
        u1cpc_cmd[3] == ' '  &&
        u1cpc_cmd[4] == '$'  &&
      ((u1cpc_cmd[5] >= '0'  && u1cpc_cmd[5] <= '9') ||
       (u1cpc_cmd[5] >= 'a'  && u1cpc_cmd[5] <= 'f') ||
       (u1cpc_cmd[5] >= 'A'  && u1cpc_cmd[5] <= 'F')) &&
       (u1cpc_cmd[6] == '\n' || u1cpc_cmd[6] == '\0'))
    {
        if(u1cpc_cmd[5] >= '0'  && u1cpc_cmd[5] <= '9') // 引数判別
        {
            u1_operand = u1cpc_cmd[5] - '0';
        }
        else if (u1cpc_cmd[5] >= 'a'  && u1cpc_cmd[5] <= 'f')
        {
            u1_operand = u1cpc_cmd[5] - 'a' + 10;
        }
        else if (u1cpc_cmd[5] >= 'A'  && u1cpc_cmd[5] <= 'F')
        {
            u1_operand = u1cpc_cmd[5] - 'A' + 10;
        }

        u1_code |= (u1_opcode << 4);                    // 命令は上位ニブル
        u1_code |= u1_operand;                          // 引数を合成
        u1p_rom[u4_pc] = u1_code;                       // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// JNC Im コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : JNC $n
// 引数   : n = Cフラグが0のときにジャンプする先のアドレス (0～F)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_JNC_I_(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret     = S1_RET_SKIP;                        // 戻り値：未処理
    U4 u4_pc      = u4f_g_CpuDebugGetPc();              // プログラムカウンタ取得
    U1 u1_opcode  = 0xE;                                // オペコード(命令：上位ニブル)
    U1 u1_operand = 0x0;                                // オペランド(引数：下位ニブル)
    U1 u1_code    = 0x00;                               // マシン語コード(命令 & 引数)

    if((u1cpc_cmd[0] == 'J'  || u1cpc_cmd[0] == 'j') && // 命令判別
       (u1cpc_cmd[1] == 'N'  || u1cpc_cmd[1] == 'n') &&
       (u1cpc_cmd[2] == 'C'  || u1cpc_cmd[2] == 'c') &&
        u1cpc_cmd[3] == ' '  &&
        u1cpc_cmd[4] == '$'  &&
      ((u1cpc_cmd[5] >= '0'  && u1cpc_cmd[5] <= '9') ||
       (u1cpc_cmd[5] >= 'a'  && u1cpc_cmd[5] <= 'f') ||
       (u1cpc_cmd[5] >= 'A'  && u1cpc_cmd[5] <= 'F')) &&
       (u1cpc_cmd[6] == '\n' || u1cpc_cmd[6] == '\0'))
    {
        if(u1cpc_cmd[5] >= '0'  && u1cpc_cmd[5] <= '9') // 引数判別
        {
            u1_operand = u1cpc_cmd[5] - '0';
        }
        else if (u1cpc_cmd[5] >= 'a'  && u1cpc_cmd[5] <= 'f')
        {
            u1_operand = u1cpc_cmd[5] - 'a' + 10;
        }
        else if (u1cpc_cmd[5] >= 'A'  && u1cpc_cmd[5] <= 'F')
        {
            u1_operand = u1cpc_cmd[5] - 'A' + 10;
        }

        u1_code |= (u1_opcode << 4);                    // 命令は上位ニブル
        u1_code |= u1_operand;                          // 引数を合成
        u1p_rom[u4_pc] = u1_code;                       // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}

//============================================================================================
// JMP Im コマンド - 現在PC位置のコードを当該命令に書き換える
// 入力例 : JNC $n
// 引数   : n = 無条件ジャンプする先のアドレス (0～F)
//--------------------------------------------------------------------------------------------
// Arguments : const U1* const u1cpc_cmd : (I) コマンド文字列
//             U1*             u1p_rom   : (I) プログラム保持領域のポインタ
// Return    : S1              s1_ret    : (O) S1_RET_SKIP : 未処理
//                                             S1_RET_DONE : 実行済み
//============================================================================================
static S1 s1f_s_DbgCmdExe_JMP_I_(const U1* const u1cpc_cmd, U1* u1p_rom)
{
    S1 s1_ret     = S1_RET_SKIP;                        // 戻り値：未処理
    U4 u4_pc      = u4f_g_CpuDebugGetPc();              // プログラムカウンタ取得
    U1 u1_opcode  = 0xF;                                // オペコード(命令：上位ニブル)
    U1 u1_operand = 0x0;                                // オペランド(引数：下位ニブル)
    U1 u1_code    = 0x00;                               // マシン語コード(命令 & 引数)

    if((u1cpc_cmd[0] == 'J'  || u1cpc_cmd[0] == 'j') && // 命令判別
       (u1cpc_cmd[1] == 'M'  || u1cpc_cmd[1] == 'm') &&
       (u1cpc_cmd[2] == 'P'  || u1cpc_cmd[2] == 'p') &&
        u1cpc_cmd[3] == ' '  &&
        u1cpc_cmd[4] == '$'  &&
      ((u1cpc_cmd[5] >= '0'  && u1cpc_cmd[5] <= '9') ||
       (u1cpc_cmd[5] >= 'a'  && u1cpc_cmd[5] <= 'f') ||
       (u1cpc_cmd[5] >= 'A'  && u1cpc_cmd[5] <= 'F')) &&
       (u1cpc_cmd[6] == '\n' || u1cpc_cmd[6] == '\0'))
    {
        if(u1cpc_cmd[5] >= '0'  && u1cpc_cmd[5] <= '9') // 引数判別
        {
            u1_operand = u1cpc_cmd[5] - '0';
        }
        else if (u1cpc_cmd[5] >= 'a'  && u1cpc_cmd[5] <= 'f')
        {
            u1_operand = u1cpc_cmd[5] - 'a' + 10;
        }
        else if (u1cpc_cmd[5] >= 'A'  && u1cpc_cmd[5] <= 'F')
        {
            u1_operand = u1cpc_cmd[5] - 'A' + 10;
        }

        u1_code |= (u1_opcode << 4);                    // 命令は上位ニブル
        u1_code |= u1_operand;                          // 引数を合成
        u1p_rom[u4_pc] = u1_code;                       // 命令書き換え
        vdf_g_DbgDrawUpdateList(u4_pc, u1p_rom);        // 表示更新
        s1_ret = S1_RET_DONE;                           // 戻り値：実行済み
    }

    return s1_ret;
}
