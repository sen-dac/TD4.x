//============================================================================================
// Application : TD4 Emulator & Assembler for X68000 - TD4.x
// Platform    : SHARP X68000 / Human68k
// Build       : GCC真里子版(ver1.42) + XCライブラリ
//--------------------------------------------------------------------------------------------
// Filename    : dbg.c
// Version     : 1.0.0
// Author      : SEN::DAC
// Description : デバッガモジュール (メイン処理)
//============================================================================================
//============================================================================================
// インクルードファイル
//============================================================================================
#include "dbg.h"

//============================================================================================
// 定数定義 (内部用)
//============================================================================================
// .iniファイル関連
#define INI_FILE_NAME    "td4.ini"     // ファイル名
#define INI_TBL_MAX      2             // テーブル要素数 (扱う項目数)

// クロックモード
#define CLK_1HZ          0             // 1 Hz
#define CLK_10HZ         1             // 10 Hz

// UI表示用 
#define DIS_AS_CODE_LEN  12            // 逆アセンブルコードの文字列最大長

// UI 表示位置 (Y;X)
#define CUI_POS_FILE     "\x1b[5;9H"   // パス/ファイル名
#define CUI_POS_BPADDR_0 "\x1b[12;8H"  // BP $0
#define CUI_POS_BPADDR_1 "\x1b[13;8H"  // BP $1
#define CUI_POS_BPADDR_2 "\x1b[14;8H"  // BP $2
#define CUI_POS_BPADDR_3 "\x1b[15;8H"  // BP $3
#define CUI_POS_BPADDR_4 "\x1b[16;8H"  // BP $4
#define CUI_POS_BPADDR_5 "\x1b[17;8H"  // BP $5
#define CUI_POS_BPADDR_6 "\x1b[18;8H"  // BP $6
#define CUI_POS_BPADDR_7 "\x1b[19;8H"  // BP $7
#define CUI_POS_BPADDR_8 "\x1b[20;8H"  // BP $8
#define CUI_POS_BPADDR_9 "\x1b[21;8H"  // BP $9
#define CUI_POS_BPADDR_A "\x1b[22;8H"  // BP $A
#define CUI_POS_BPADDR_B "\x1b[23;8H"  // BP $B
#define CUI_POS_BPADDR_C "\x1b[24;8H"  // BP $C
#define CUI_POS_BPADDR_D "\x1b[25;8H"  // BP $D
#define CUI_POS_BPADDR_E "\x1b[26;8H"  // BP $E
#define CUI_POS_BPADDR_F "\x1b[27;8H"  // BP $F
#define CUI_POS_MESSAGE  "\x1b[29;9H"  // メッセージ
#define CUI_POS_COMMAND  "\x1b[31;2H"  // コマンド (カーソル0文字位置)

// コマンド一覧表示位置 (Y;X)
#define CUI_POS_CMD_R001 "\x1b[10;39H" // 01行目
#define CUI_POS_CMD_R002 "\x1b[11;39H" // 02行目
#define CUI_POS_CMD_R003 "\x1b[12;39H" // 03行目
#define CUI_POS_CMD_R004 "\x1b[13;39H" // 04行目
#define CUI_POS_CMD_R005 "\x1b[14;39H" // 05行目
#define CUI_POS_CMD_R006 "\x1b[15;39H" // 06行目
#define CUI_POS_CMD_R007 "\x1b[16;39H" // 07行目
#define CUI_POS_CMD_R008 "\x1b[17;39H" // 08行目
#define CUI_POS_CMD_R009 "\x1b[18;39H" // 09行目
#define CUI_POS_CMD_R010 "\x1b[19;39H" // 10行目
#define CUI_POS_CMD_R011 "\x1b[20;39H" // 11行目
#define CUI_POS_CMD_R012 "\x1b[21;39H" // 12行目
#define CUI_POS_CMD_R013 "\x1b[22;39H" // 13行目
#define CUI_POS_CMD_R014 "\x1b[23;39H" // 14行目
#define CUI_POS_CMD_R015 "\x1b[24;39H" // 15行目
#define CUI_POS_CMD_R016 "\x1b[25;39H" // 16行目
#define CUI_POS_CMD_R017 "\x1b[26;39H" // 17行目
#define CUI_POS_CMD_R018 "\x1b[27;39H" // 18行目

// UI 表示行 (Y;1)
#define CUI_ROW_TITLE    "\x1b[1;1H"   // タイトル
#define CUI_ROW_CLOCK    "\x1b[4;1H"   // クロック
#define CUI_ROW_FILE     "\x1b[5;1H"   // パス/ファイル名
#define CUI_ROW_REG      "\x1b[7;1H"   // レジスタ状態
#define CUI_ROW_LHEADER  "\x1b[10;1H"  // リストヘッダ
#define CUI_ROW_ADDR_0   "\x1b[12;1H"  // リスト $0
#define CUI_ROW_ADDR_1   "\x1b[13;1H"  // リスト $1
#define CUI_ROW_ADDR_2   "\x1b[14;1H"  // リスト $2
#define CUI_ROW_ADDR_3   "\x1b[15;1H"  // リスト $3
#define CUI_ROW_ADDR_4   "\x1b[16;1H"  // リスト $4
#define CUI_ROW_ADDR_5   "\x1b[17;1H"  // リスト $5
#define CUI_ROW_ADDR_6   "\x1b[18;1H"  // リスト $6
#define CUI_ROW_ADDR_7   "\x1b[19;1H"  // リスト $7
#define CUI_ROW_ADDR_8   "\x1b[20;1H"  // リスト $8
#define CUI_ROW_ADDR_9   "\x1b[21;1H"  // リスト $9
#define CUI_ROW_ADDR_A   "\x1b[22;1H"  // リスト $A
#define CUI_ROW_ADDR_B   "\x1b[23;1H"  // リスト $B
#define CUI_ROW_ADDR_C   "\x1b[24;1H"  // リスト $C
#define CUI_ROW_ADDR_D   "\x1b[25;1H"  // リスト $D
#define CUI_ROW_ADDR_E   "\x1b[26;1H"  // リスト $E
#define CUI_ROW_ADDR_F   "\x1b[27;1H"  // リスト $F
#define CUI_ROW_MESSAGE  "\x1b[29;1H"  // メッセージ
#define CUI_ROW_CHEADER  "\x1b[31;1H"  // コマンドプロンプト表示行

// アプリのタイトルとバージョン
#define APP_TITLE        "TD4 Emulator & Assembler for X68000 - TD4.x "
#define APP_VERSION      "version 1.00"

// アプリ起動時にファイル名を指定していない場合の表示
#define FN_NO_LOAD       "none"

// コマンドリファレンス表示文字列
#define CMD_REFE_01      "G    : オート(Go)モード開始"
#define CMD_REFE_02      "T    : ステップ(Trace)実行"
#define CMD_REFE_03      "R    : ターゲットCPU リセット"
#define CMD_REFE_04      "O    : ターゲットCPU 動作クロック変更(1Hz / 10Hz)"
#define CMD_REFE_05      "BS n : ブレークポイント設置 (n = 0～F)"
#define CMD_REFE_06      "BC n : ブレークポイント解除 (n = 0～F)"
#define CMD_REFE_07      "BC   : ブレークポイントを全て解除"
#define CMD_REFE_08      "N    : 現在行を実行せずにPCを1つ進める"
#define CMD_REFE_09      "P    : 現在行を実行せずにPCを1つ戻す"
#define CMD_REFE_10      "S fn : プログラムをセーブ (fn = パス/ファイル名.拡張子)"
#define CMD_REFE_11      "L fn : プログラムをロード (fn = パス/ファイル名.拡張子)"
#define CMD_REFE_12      "0～F : INPUTに値を設定 (即時反映)"
#define CMD_REFE_13      "Q    : アプリケーション終了"

//============================================================================================
// 型定義 (内部用)
//============================================================================================
// 逆アセンブル構造体
typedef struct
{
    const char* const szcpc_label; // 逆アセンブル表示用文字列
    const BL bl_operand;           // オペランドがある命令か (TRUE：あり / FALSE：なし)
}
stDISASM;

//============================================================================================
// プロトタイプ宣言 (static)
//============================================================================================
static S1 s1f_s_DbgRunCmdMod(U1* const u1pc_rom);                   // コマンドモードの処理
static VD vdf_s_DbgRunAtMod(U1* const u1pc_rom);                    // オートモードの処理

static VD vdf_s_DbgDrawTitle(VD);                                   // アプリタイトル表示
static VD vdf_s_DbgDrawClock(VD);                                   // クロックモード表示
static VD vdf_s_DbgDrawFName(const char* const szcpc_filename);     // ファイル名表示
static VD vdf_s_DbgDrawReg(VD);                                     // レジスタ状態表示
static VD vdf_s_DbgDrawLHead(VD);                                   // リストヘッダ表示
static VD vdf_s_DbgDrawList(U4 u4_addr, const U1* const u1cpc_rom); // リスト表示 (1行)
static VD vdf_s_DbgDrawCmdReference(VD);                            // コマンドリファレンス表示
static VD vdf_s_DbgDrawMessLabel(VD);                               // メッセージラベル表示
static VD vdf_s_DbgDrawCmdPrompt(VD);                               // コマンドプロンプト表示

static VD vdf_s_DbgDrawRLedHex(U1 u1_reg_id);                       // レジスタ表示(LED & HEX)
static VD vdf_s_DbgDrawFlg(const char* const szcpc_label,           // 1ビットフラグ表示
                           U1 u1_reg, U1 u1_bit); 
static VD vdf_s_DbgDrawRomBin(U4 u4_val, U1 u1_num);                // ROM値を2進表示
static VD vdf_s_DbgDrawUpdateExe(VD);                               // PCとレジスタの表示更新
static VD vdf_s_DbgDrawUpdatePc(U4 u4_addr);                        // PC位置の表示更新

//============================================================================================
// グローバル変数定義 (外部公開)
//============================================================================================

//============================================================================================
// 静的変数定義 (static)
//============================================================================================
// 1Hz 近似値生成用のウェイト設定値
// ラーメンタイマーが3分15秒に近づくように実行環境毎にコンフィグする。
// デフォルトはZ用設定値(46) → .iniファイルから設定値が読み込めた場合は上書きされる。
static U1 u1_s_WaitConfig = 46;

// .ini ファイルから読み取る項目の拡張サンプル兼リザーブ変数
static U1 u1_s_Reserved = 0;

// .ini ファイル設定項目のテーブル (キーワード, 設定先変数)
static stINI_PRM sta_s_IniTable[INI_TBL_MAX] =
{
    { "WAIT",    &u1_s_WaitConfig },
    { "RESERVE", &u1_s_Reserved   },
};

// クロックモード (1Hz : CLK_1HZ / 10Hz : CLK_10HZ)
static U1 u1_s_Clk = CLK_1HZ;

// レジスタラベル名テーブル
static const char* const szacpc_s_RegLabelTbl[CPU_REG_MAX] =
{
    "REG_A : ",           // A レジスタ
    "REG_B : ",           // B レジスタ
    "INPUT : ",           // INPUT  (DIPスイッチ)
    "OUTPUT: ",           // OUTPUT (出力端子)
    "PC    : ",           // プログラムカウンタ
    "SR    : ",           // ステータスレジスタ
};

// リスト表示行テーブル
static const char* const szacpc_s_ListRowTbl[ROM_ADR_MAX] =
{
    CUI_ROW_ADDR_0,       // $0
    CUI_ROW_ADDR_1,       // $1
    CUI_ROW_ADDR_2,       // $2
    CUI_ROW_ADDR_3,       // $3
    CUI_ROW_ADDR_4,       // $4
    CUI_ROW_ADDR_5,       // $5
    CUI_ROW_ADDR_6,       // $6
    CUI_ROW_ADDR_7,       // $7
    CUI_ROW_ADDR_8,       // $8
    CUI_ROW_ADDR_9,       // $9
    CUI_ROW_ADDR_A,       // $A
    CUI_ROW_ADDR_B,       // $B
    CUI_ROW_ADDR_C,       // $C
    CUI_ROW_ADDR_D,       // $D
    CUI_ROW_ADDR_E,       // $E
    CUI_ROW_ADDR_F,       // $F
};

// 1つ前のプログラムカウンタを保持 (PC'→'表示を消すために必要)
static U4 u4_s_PrevPc;

// ターゲット上で動作するメモリ領域のブレークポイント情報
// Note :
// ホストマシン上でのメモリ配置がターゲットCPU上のメモリ配置と
// 一致しなくなるため、メモリ領域と本変数を構造体で纏めてはならない。
// また、本ソフトはエミュレータであり、現物の物理的構造に即して分離させる方針とする。
static BL bla_s_BreakPoint[ROM_ADR_MAX] = { 0 };

// ブレークポイント表示位置テーブル
static const char* const szacpc_s_BpPosTbl[ROM_ADR_MAX] =
{
    CUI_POS_BPADDR_0,     // $0
    CUI_POS_BPADDR_1,     // $1
    CUI_POS_BPADDR_2,     // $2
    CUI_POS_BPADDR_3,     // $3
    CUI_POS_BPADDR_4,     // $4
    CUI_POS_BPADDR_5,     // $5
    CUI_POS_BPADDR_6,     // $6
    CUI_POS_BPADDR_7,     // $7
    CUI_POS_BPADDR_8,     // $8
    CUI_POS_BPADDR_9,     // $9
    CUI_POS_BPADDR_A,     // $A
    CUI_POS_BPADDR_B,     // $B
    CUI_POS_BPADDR_C,     // $C
    CUI_POS_BPADDR_D,     // $D
    CUI_POS_BPADDR_E,     // $E
    CUI_POS_BPADDR_F,     // $F
};

// 逆アセンブル構造体 (NOP)
static const stDISASM stc_s_DisAsmNOP =
{
    "NOP", FALSE            // $0 - NOP (ADD A,#$0とした場合はNOPとして扱われる)
};

// 逆アセンブル構造体テーブル
static const stDISASM stca_s_DisAsmTbl[CPU_INS_MAX] =
{
    { "ADD A,",  TRUE  }, // $0 - ADD A,Im 0000 XXXX
    { "MOV A,B", FALSE }, // $1 - MOV A,B  0001 0000
    { "IN  A",   FALSE }, // $2 - IN  A    0010 0000
    { "MOV A,",  TRUE  }, // $3 - MOV A,Im 0011 XXXX
    { "MOV B,A", FALSE }, // $4 - MOV B,A  0100 0000
    { "ADD B,",  TRUE  }, // $5 - ADD B,Im 0101 XXXX
    { "IN  B",   FALSE }, // $6 - IN  B    0110 0000
    { "MOV B,",  TRUE  }, // $7 - MOV B,Im 0111 XXXX
    { "?:OUT B", FALSE }, // $8 - ???      1000 0000 未定義命令 OUT B と同じ動作をする
    { "OUT B",   FALSE }, // $9 - OUT B    1001 0000
    { "?:OUT ",  TRUE  }, // $A - ???      1010 XXXX 未定義命令 OUT Im と同じ動作をする
    { "OUT ",    TRUE  }, // $B - OUT Im   1011 XXXX
    { "?:JNC ",  TRUE  }, // $C - ???      1100 XXXX 未定義命令 JNC Im と同じ動作をする
    { "?:JMP ",  TRUE  }, // $D - ???      1101 XXXX 未定義命令 JMP Im と同じ動作をする
    { "JNC ",    TRUE  }, // $E - JNC Im   1110 XXXX C = 0 ならジャンプ
    { "JMP ",    TRUE  }, // $F - JMP Im   1111 XXXX 無条件ジャンプ
};

// 動作モード (コマンドモード:DBG_MODE_CMD / オートモード:DBG_MODE_AT)
static U1 u1_s_DbgOpMode = DBG_MODE_CMD;

// エラー命令表示済みフラグ (要初回ON)
static BL bl_s_ErrMesFlg = TRUE;

//============================================================================================
// 関数定義
//============================================================================================
//============================================================================================
// ファイル名の表示更新
//--------------------------------------------------------------------------------------------
// Arguments : const char* const szcpc_filename : (I) 表示するファイル名
// Return    : -
//============================================================================================
VD vdf_g_DbgDrawUpdateFName(const char* const szcpc_filename)
{
    S4 s4_i;

    printf(CUI_POS_FILE);                            // カーソルをファイル名表示位置に移動

    for(s4_i = 0; s4_i < (ROW_CHAR_MAX - 10); s4_i ++)
    {
        printf(" ");                                 // 空白を書き文字を消す
    }

    for(s4_i = 0; s4_i < (ROW_CHAR_MAX - 10); s4_i ++)
    {
        printf("\b");                                // カーソルを戻す (BS)
    }

    printf(szcpc_filename);                          // 再表示
}

//============================================================================================
// リストの表示更新 (1行)
//--------------------------------------------------------------------------------------------
// Arguments : U4              u4_addr   : (I) 更新するROMアドレス
//             const U1* const u1cpc_rom : (I) ROM格納配列の先頭アドレス
// Return    : -
//============================================================================================
VD vdf_g_DbgDrawUpdateList(U4 u4_addr, const U1* const u1cpc_rom)
{
    vdf_s_DbgDrawList(u4_addr, u1cpc_rom);           // 再表示 (1行)
}

//============================================================================================
// クロックのトグルと表示更新
//--------------------------------------------------------------------------------------------
// Arguments : -
// Return    : -
//============================================================================================
VD vdf_g_DbgDrawUpdateClockTgl(VD)
{
    u1_s_Clk ++;                                     // 1Hz → 10Hz / 10Hz → 1Hz
    u1_s_Clk &= 0x01;                                // ビット0のみ有効

    vdf_s_DbgDrawClock();                            // 再表示
}

//============================================================================================
// ブレークポイントの表示更新
//--------------------------------------------------------------------------------------------
// Arguments : U4 u4_addr : (I) 更新するROMアドレス
// Return    : -
//============================================================================================
VD vdf_g_DbgDrawUpdateBp(U4 u4_addr)
{
    if(u4_addr < ROM_ADR_MAX)                        // 引数はROMのアドレス範囲内か？
    {
        printf(szacpc_s_BpPosTbl[u4_addr]);          // カーソルを対象BP表示位置に移動

        if(bla_s_BreakPoint[u4_addr])                // ブレークポイントを表示
        {
            printf(" ● ");                          // TRUE  : ブレークする
        }
        else
        {
            printf(" ○ ");                          // FALSE : ブレークしない
        }
    }
}

//============================================================================================
// メッセージの表示更新
//--------------------------------------------------------------------------------------------
// Arguments : const char* const szcpc_message : (I) 表示するメッセージ
// Return    : -
//============================================================================================
VD vdf_g_DbgDrawUpdateMess(const char* const szcpc_message)
{
    S4 s4_i;

    printf(CUI_POS_MESSAGE);                         // カーソルをメッセージ表示位置に移動

    // メッセージ消去
    for(s4_i = 0; s4_i < (ROW_CHAR_MAX - 10); s4_i ++)
    {
        printf(" ");                                 // 空白を書き文字を消す
    }

    for(s4_i = 0; s4_i < (ROW_CHAR_MAX - 10); s4_i ++)
    {
        printf("\b");                                // カーソルを戻す
    }

    printf(szcpc_message);                           // 再表示
}

//============================================================================================
// 動作モードをセットする
//--------------------------------------------------------------------------------------------
// Arguments : U1 u1_mode : (I) 動作モード 0 : DBG_MODE_CMD コマンドモード
//                                         1 : DBG_MODE_AT  オートモード
// Return    : -
//============================================================================================
VD vdf_g_DbgSetOpMode(U1 u1_mode)
{
    u1_s_DbgOpMode = u1_mode;
}

//============================================================================================
// 1つ前のPC(プログラムカウンタ)を保存する
//--------------------------------------------------------------------------------------------
// Arguments : U4 u4_prev_pc : (I) 保存するPC値
// Return    : -
//============================================================================================
VD vdf_g_DbgSetPrevPC(U4 u4_prev_pc)
{
    u4_s_PrevPc = u4_prev_pc;
}

//============================================================================================
// ブレークポイントを設定する
//--------------------------------------------------------------------------------------------
// Arguments : U4 u4_addr  : (I) ブレークポイントを設定するアドレス 
//             BL bl_state : (I) TRUE  : ブレークする
//                               FALSE : ブレークしない
// Return    : -
//============================================================================================
VD vdf_g_DbgSetBP(U4 u4_addr, BL bl_state)
{
    if(u4_addr < ROM_ADR_MAX)                        // 引数の範囲をチェック
    {
        bla_s_BreakPoint[u4_addr] = bl_state;
    }
}

//============================================================================================
// コマンド位置にカーソルを戻す
//--------------------------------------------------------------------------------------------
// Arguments : -
// Return    : -
//--------------------------------------------------------------------------------------------
// Note      :
// コマンド関数が各UI部品を表示更新した後にカーソル位置に戻すための処理
// 打ち込んだコマンド文字の表示消去も行う
//============================================================================================
VD vdf_g_DbgRetCurPos(VD)
{
    S4 s4_i;

    printf(CUI_POS_COMMAND);                         // コマンド位置にカーソルを戻す

    for(s4_i = 0; s4_i < CMD_BUF_MAX; s4_i ++)
    {
        printf(" ");                                 // 空白を書き文字を消す
    }

    for(s4_i = 0; s4_i < CMD_BUF_MAX; s4_i ++)
    {
        printf("\b");                                // カーソルを戻す (BS)
    }
}

//============================================================================================
// ROMファイルをロード
//--------------------------------------------------------------------------------------------
// Arguments : const char* const szcpc_filename : (I) ロードするパス/ファイル名
//             U1*         const u1pc_rom       : (I) ROM格納配列の先頭アドレス
// Return    : BL                bl_ret         : (O) TRUE  : 成功
//                                                    FALSE : 失敗
//============================================================================================
BL blf_g_DbgLoadRomFile(const char* const szcpc_filename, U1* const u1pc_rom)
{
    BL bl_ret = FALSE;                               // 戻り値格納用
    FILE* fp  = fopen(szcpc_filename, "rb");         // ファイルオープン

    if(fp != NULL)                                   // ファイルオープン成功
    {
        // サイズ分を一括でロード
        if(fread(u1pc_rom, sizeof(U1), ROM_ADR_MAX, fp) == ROM_ADR_MAX)
        {

            bl_ret = TRUE;                           // ロード成功
        }
        fclose(fp);                                  // ファイルクローズ
    }

    return bl_ret;
}

//============================================================================================
// ROMファイルをセーブ
//--------------------------------------------------------------------------------------------
// Arguments : const char* const szcpc_filename : (I) セーブするパス/ファイル名
//             const U1*   const u1cpc_rom      : (I) ROM格納配列の先頭アドレス
// Return    : BL                bl_ret         : (O) TRUE  : 成功
//                                                    FALSE : 失敗
//============================================================================================
BL blf_g_DbgSaveRomFile(const char* const szcpc_filename, const U1* const u1cpc_rom)
{
    BL bl_ret = FALSE;                               // 戻り値格納用
    FILE* fp  = fopen(szcpc_filename, "wb");         // ファイルオープン

    if(fp != NULL)                                   // ファイルオープン成功
    {
        // サイズ分を一括でセーブ
        if(fwrite(u1cpc_rom, sizeof(U1), ROM_ADR_MAX, fp) == ROM_ADR_MAX)
        {
            bl_ret = TRUE;                           // セーブ成功
        }
        fclose(fp);                                  // ファイルクローズ
    }

    return bl_ret;
}

//============================================================================================
// デバッガ初期化処理
//--------------------------------------------------------------------------------------------
// Arguments : int       argc     : (I) アプリ起動時引数 : argv[]の有効要素数
//             char*     argv[]   : (I) アプリ起動時引数 : 引数文字列のポインタテーブル
//             U1* const u1pc_rom : (I) ROM格納配列の先頭アドレス
// Return    : -
//============================================================================================
VD vdf_g_DbgInit(int argc, char* argv[], U1* const u1pc_rom)
{
    S4    s4_i;
    BL    bl_load     = TRUE;                        // ROMファイルロードフラグ = 成功
    char* sz_filename = FN_NO_LOAD;                  // ROMファイル名 = ロードなし

    blf_g_IniFileLoad(INI_FILE_NAME,                 // .iniファイルをロードして設定値を取得
                      sta_s_IniTable,
                      INI_TBL_MAX);
    if (argc > 1)                                    // argcはargv[]の要素数が入っている
    {                                                // argcが 1 以上なら起動時引数がある
        if(blf_g_DbgLoadRomFile(argv[1], u1pc_rom))  // argv[1]～起動時引数の文字列先頭アドレス
        {                                            // ROMファイルロード成功
            sz_filename = argv[1];                   // ROMファイル名 = 起動時引数
        }
        else
        {                                            // ROMファイルロード失敗
            bl_load = FALSE;                         // ROMファイルのロードフラグ = 失敗
        }
    }

    cls();                                           // コンソールをクリア
    vdf_s_DbgDrawTitle();                            // タイトル表示
    vdf_s_DbgDrawClock();                            // クロックモード表示
    vdf_s_DbgDrawFName(sz_filename);                 // ファイル名を表示
    vdf_s_DbgDrawReg();                              // レジスタ状態を表示
    vdf_s_DbgDrawLHead();                            // リストヘッダの表示

    for(s4_i = 0; s4_i < ROM_ADR_MAX; s4_i ++)
    {
        vdf_s_DbgDrawList(s4_i, u1pc_rom);           // リストの表示 (全行)
    }

    vdf_s_DbgDrawCmdReference();                     // コマンドリファレンスを表示
    vdf_s_DbgDrawMessLabel();                        // メッセージラベルを表示

    if(!bl_load)                                     // ROMファイルロード失敗の場合
    {
        vdf_g_DbgDrawUpdateMess(MES_FL_FAILURE);     // ROMファイルロード失敗のメッセージ表示
    }
    
    vdf_s_DbgDrawCmdPrompt();                        // コマンドプロンプトを表示
}

//============================================================================================
// デバッガ実行処理
//--------------------------------------------------------------------------------------------
// Arguments : U1* const u1pc_rom : (I) ROM格納配列の先頭アドレス
// Return    : S1        s1_ret   : (O) S1_RET_ERR3 : エラーコード 3
//                                      S1_RET_ERR2 : エラーコード 2
//                                      S1_RET_ERR1 : エラーコード 1
//                                      S1_RET_QUIT : アプリ終了通知
//                                      S1_RET_DONE : 実行済み / 成功
//                                      S1_RET_SKIP : コマンド未処理
//============================================================================================
S1 s1f_g_DbgRun(U1* const u1pc_rom)
{
    S1 s1_ret = S1_RET_SKIP;

    // コマンドモードの場合の処理
    if(u1_s_DbgOpMode == DBG_MODE_CMD) s1_ret = s1f_s_DbgRunCmdMod(u1pc_rom);

    // オートモードの場合の処理
    else if(u1_s_DbgOpMode == DBG_MODE_AT) vdf_s_DbgRunAtMod(u1pc_rom);

    // コマンドモードの戻り値がアプリ終了通知だった場合
    if(s1_ret == S1_RET_QUIT) printf("q\n");

    return s1_ret;
}

//============================================================================================
// ターゲットCPUを1クロック実行
//--------------------------------------------------------------------------------------------
// Arguments : U1* const u1pc_rom : (I) ROM格納配列の先頭アドレス
// Return    : -
//============================================================================================
VD vdf_g_DbgExe1Clk(U1* const u1pc_rom)
{
    U1 u1_ret = CPU_RET_UINS;                        // ターゲットCPU命令の戻り値

    vdf_g_DbgSetPrevPC(u4f_g_CpuDebugGetPc());       // 1つ前のPCを記録
    u1_ret = u1f_g_CpuExe1Clk(u1pc_rom);             // ターゲットCPUを1クロック実行

    if((u1_ret & CPU_RET_B_UINS) ||                  // 未定義命令の場合 もしくは
       (u1_ret & CPU_RET_B_UIMM))                    // 不正Im命令の場合
    {
        beep();                                      // ブザー吹鳴

        if(u1_ret & CPU_RET_B_UINS)                  // 未定義命令の場合
        {
            vdf_g_DbgDrawUpdateMess(MES_UNDEF_INS);  // 未定義命令のメッセージ
        }
        else                                         // 不正Im命令の場合
        {
            vdf_g_DbgDrawUpdateMess(MES_UNDEF_IMM);  // 不正Im命令のメッセージ
        }

        if(u1_s_DbgOpMode == DBG_MODE_AT)            // オートモードなら
        {
            bl_s_ErrMesFlg = TRUE;                   // エラー命令表示済みフラグ ON
        }
    }
    else                                             // エラー命令ではない場合 かつ
    if(bl_s_ErrMesFlg &&                             // エラー命令表示済みフラグON かつ
           (u1_s_DbgOpMode == DBG_MODE_AT))          // オートモードなら
    {
        vdf_g_DbgDrawUpdateMess(MES_EXEC_AUTO);      // オートモード実行中のメッセージ
        bl_s_ErrMesFlg = FALSE;                      // エラー命令表示済みフラグ OFF
    }

    if(u4f_g_CpuGetOutput() & 0x08) beep();          // Output Bit3 が1ならブザー吹鳴
}

//============================================================================================
// コマンドモードの処理
//--------------------------------------------------------------------------------------------
// Arguments : U1* const u1pc_rom : (I) ROM格納配列の先頭アドレス
// Return    : S1        s1_ret   : (O) S1_RET_ERR3 : エラーコード 3
//                                      S1_RET_ERR2 : エラーコード 2
//                                      S1_RET_ERR1 : エラーコード 1
//                                      S1_RET_QUIT : アプリ終了通知
//                                      S1_RET_DONE : 実行済み / 成功
//                                      S1_RET_SKIP : コマンド未処理
//============================================================================================
static S1 s1f_s_DbgRunCmdMod(U1* const u1pc_rom)
{
    S1 s1_ret = S1_RET_SKIP;                         // 戻り値：コマンド未処理

    s1_ret = s1f_g_DbCmdRec(u1pc_rom);               // コマンド入力受付
    vdf_s_DbgDrawUpdateExe();                        // PCとレジスタの表示更新
    vdf_g_DbgRetCurPos();                            // カーソル位置を戻す

    return s1_ret;
}

//============================================================================================
// オートモードの処理
//--------------------------------------------------------------------------------------------
// Arguments : U1* const u1pc_rom : (I) ROM格納配列の先頭アドレス
// Return    : -
//============================================================================================
static VD vdf_s_DbgRunAtMod(U1* const u1pc_rom)
{
    U4 u4_pc = u4f_g_CpuDebugGetPc();                // プログラムカウンタ取得
    static U1 u1s_cnt = 0;                           // ウェイト用カウント

    if(u1_s_Clk == CLK_1HZ)                          // 1Hzモードの場合
    {
        vdf_g_CrtVblank();                           // 60(55.4)fps (時間計測用)
        u1s_cnt ++;                                  // ウェイト用カウント加算
    }
    else                                             // 10Hzモードの場合
    {
        u1s_cnt = u1_s_WaitConfig;                   // ウェイト無視
    }

    if(u1s_cnt >= u1_s_WaitConfig)                   // ウェイト時間を満たした場合
    {
        if(bla_s_BreakPoint[u4_pc])                  // 実行する行にBPがある場合
        {
            u1_s_DbgOpMode = DBG_MODE_CMD;           // コマンドモードに切替
            vdf_g_DbgDrawUpdateMess(MES_BREAK);      // ブレークのメッセージを表示
            vdf_g_DbgRetCurPos();                    // カーソル位置を戻す
        }
        else                                         // 実行する行にBPがない場合
        {
            vdf_g_DbgExe1Clk(u1pc_rom);              // ターゲットCPUを1クロック実行
            vdf_s_DbgDrawUpdateExe();                // PCとレジスタの表示更新
            vdf_g_DbgRetCurPos();                    // カーソル位置を戻す
        }
        u1s_cnt = 0;                                 // ウェイト用カウントをクリア
    }

    if(BITSNS(KG_0) == KG_0_KC_ESC)                  // ESCキーが押された場合
    {
        u1_s_DbgOpMode = DBG_MODE_CMD;               // コマンドモードに切替
        vdf_g_DbgDrawUpdateMess(MES_STOP_AUTO);      // オートモード停止のメッセージ
        vdf_g_DbgRetCurPos();                        // カーソル位置を戻す
        KFLUSHIO(KFL_BUFCLR);                        // キーバッファのクリア
        u1s_cnt = 0;                                 // ウェイト用カウントをクリア
        bl_s_ErrMesFlg = TRUE;                       // 初回はONである必要があるので戻す
    }
}

//============================================================================================
// アプリケーションのタイトルを表示
//--------------------------------------------------------------------------------------------
// Arguments : -
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawTitle(VD)
{
    printf(CUI_ROW_TITLE);                           // カーソルをタイトル表示行に移動
    printf("Original TD4 architecture designed by Kaoru Tonami\n");
    printf(APP_TITLE);                               // アプリケーションのタイトルを表示
    printf(APP_VERSION);                             // アプリケーションのバージョンを表示
    printf(" (c) 2026 SEN::DAC");
}

//============================================================================================
// クロックモードを表示
//--------------------------------------------------------------------------------------------
// Arguments : -
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawClock(VD)
{
    printf(CUI_ROW_CLOCK);                           // カーソルをクロック表示行に移動

    if(u1_s_Clk == CLK_1HZ)
    {
        printf("Clock : 1  Hz");                     // 1 Hz モードであることを表示

        // 1MHzモードでは、校正値nにより概ねの時間を再現している旨を表示
        printf(" approx timing (cal.%d)", u1_s_WaitConfig);
    }
    else
    {
        printf("Clock : 10 Hz");                     // 10 Hz モードであることを表示

        // 10MHzモードでは、タイミングが正しく再現されていない旨を表示
        printf(" no timing correction   ");
    }
}

//============================================================================================
// ファイル名を表示
//--------------------------------------------------------------------------------------------
// Arguments : const char* const szcpc_filename : (I) 表示するファイル名
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawFName(const char* const szcpc_filename)
{
    printf(CUI_ROW_FILE);                            // カーソルをファイル名表示行に移動
    printf("File  : ");                              // ラベルを表示
    printf(szcpc_filename);                          // ファイル名を表示
}

//============================================================================================
// レジスタ状態を表示
//--------------------------------------------------------------------------------------------
// Arguments : -
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawReg(VD)
{
    printf(CUI_ROW_REG);                             // カーソルをレジスタ状態表示行に移動

    // 表示イメージ REG_A : ○○○○ $00  REG_B : ●●●● $0F  C_FLG : ○ 0
    vdf_s_DbgDrawRLedHex(CPU_REG_A);
    printf("  ");
    vdf_s_DbgDrawRLedHex(CPU_REG_B);
    printf("  ");
    vdf_s_DbgDrawFlg("C_FLG : ", CPU_REG_SR, 0);     // SR:bit0 Cフラグを表示
    printf("\n");

    // 表示イメージ INPUT : ●○●○ $0A  OUTPUT: ○●○● $05  BUZZER: ● 1
    vdf_s_DbgDrawRLedHex(CPU_REG_I);
    printf("  ");
    vdf_s_DbgDrawRLedHex(CPU_REG_O);
    printf("  ");
    vdf_s_DbgDrawFlg("BUZZER: ", CPU_REG_O, 3);      // OUTPUT:bit3 ブザーを表示
    printf("\n");
}

//============================================================================================
// リストヘッダを表示
//--------------------------------------------------------------------------------------------
// Arguments : -
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawLHead(VD)
{
    printf(CUI_ROW_LHEADER);                         // カーソルをリストヘッダ表示行に移動

    printf("PC ADR: BP CODE(BIN)  CODE(ASM)   \n");
    printf("----------------------------------\n");

}

//============================================================================================
// リストを表示 (1行)
//--------------------------------------------------------------------------------------------
// Arguments : U4              u4_addr   : (I) ROM内のアドレス
//             const U1* const u1cpc_rom : (I) ROM格納配列の先頭アドレス
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawList(U4 u4_addr, const U1* const u1cpc_rom)
{
    S4 s4_i;

    if(u4_addr < ROM_ADR_MAX)                                        // 引数の範囲チェック
    {
        printf(szacpc_s_ListRowTbl[u4_addr]);                        // リスト行にカーソル移動

        // PC 表示
        if(u4_addr == u4f_g_CpuDebugGetPc()) printf("→ ");          // PCと一致  ：矢印を表示
        else                                 printf("   ");          // PCと不一致：空白を表示

        // ADDR:BP 表示
        printf("$%01X :", u4_addr);                                  // アドレスを表示
        if(bla_s_BreakPoint[u4_addr]) printf(" ● ");                // TRUE  : ブレークする
        else                          printf(" ○ ");                // FALSE : ブレークしない

        // CODE(BIN) マシン語コード
        vdf_s_DbgDrawRomBin((u1cpc_rom[u4_addr] & 0xF0) >> 4, 3);    // 上位ニブルを2進表示
        printf(" ");                                                 // 間に空白を入れる
        vdf_s_DbgDrawRomBin(u1cpc_rom[u4_addr] & 0x0F, 3);           // 下位ニブル2進表示
        printf("  ");                                                // 間に空白を入れる

        // CODE(ASM) 逆アセンブラ (更新用消去)
        for(s4_i = 0; s4_i < DIS_AS_CODE_LEN; s4_i ++) printf(" ");  // 空白を書き文字を消す
        for(s4_i = 0; s4_i < DIS_AS_CODE_LEN; s4_i ++) printf("\b"); // カーソルを戻す (BS)

        // CODE(ASM) 逆アセンブラ 表示
        if(u1cpc_rom[u4_addr] == 0x00)                               // NOP命令の場合
        {
            printf(stc_s_DisAsmNOP.szcpc_label);                     // 逆アセンブラ表示
        }
        else                                                         // NOP命令ではない場合
        {
            // 逆アセンブラ表示
            printf(stca_s_DisAsmTbl[(u1cpc_rom[u4_addr] & 0xF0) >> 4].szcpc_label);

            // オペランドがある命令の場合
            if(stca_s_DisAsmTbl[(u1cpc_rom[u4_addr] & 0xF0) >> 4].bl_operand)
            {
                printf("$%X",u1cpc_rom[u4_addr] & 0x0F);             // オペランドを表示
            }
            // オペランドがないはずの命令に指定されている場合 (不正Im命令)
            else if(u1cpc_rom[u4_addr] & 0x0F)
            {
                printf("[!]");                                       // 不正Im命令の表示
            }
        }
    }
}

//============================================================================================
// コマンドリファレンスを表示
//--------------------------------------------------------------------------------------------
// Arguments : -
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawCmdReference(VD)
{
    // 表示位置; 表示内容;
    printf(CUI_POS_CMD_R001); printf("<< コマンド一覧 >>");

    printf(CUI_POS_CMD_R003); printf(CMD_REFE_01);
    printf(CUI_POS_CMD_R004); printf(CMD_REFE_02);
    printf(CUI_POS_CMD_R005); printf(CMD_REFE_03);
    printf(CUI_POS_CMD_R006); printf(CMD_REFE_04);
    printf(CUI_POS_CMD_R007); printf(CMD_REFE_05);
    printf(CUI_POS_CMD_R008); printf(CMD_REFE_06);
    printf(CUI_POS_CMD_R009); printf(CMD_REFE_07);
    printf(CUI_POS_CMD_R010); printf(CMD_REFE_08);
    printf(CUI_POS_CMD_R011); printf(CMD_REFE_09);
    printf(CUI_POS_CMD_R012); printf(CMD_REFE_10);
    printf(CUI_POS_CMD_R013); printf(CMD_REFE_11);
    printf(CUI_POS_CMD_R014); printf(CMD_REFE_12);
    printf(CUI_POS_CMD_R015); printf(CMD_REFE_13);

    printf(CUI_POS_CMD_R017);
    printf("アセンブラ(マニュアル参照)入力で現在PC位置に反映されます。");
    printf(CUI_POS_CMD_R018);
    printf("↑↓キーで入力履歴(2文字以上)が呼び出せます。");
}

//============================================================================================
// メッセージ(INFO)ラベルを表示
//--------------------------------------------------------------------------------------------
// Arguments : -
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawMessLabel(VD)
{
    printf(CUI_ROW_MESSAGE);                         // メッセージ表示行にカーソル移動
    printf(" INFO : ");                              // ラベル表示
}

//============================================================================================
// コマンドプロンプトを表示
//--------------------------------------------------------------------------------------------
// Arguments : -
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawCmdPrompt(VD)
{
    printf(CUI_ROW_CHEADER);                         // コマンドプロンプト表示行にカーソル移動
    printf("-");                                     // ヘッダ表示
}

//============================================================================================
// レジスタ値を表示 (LED風2進数 と 16進表示の併記)
//--------------------------------------------------------------------------------------------
// Arguments : U1 u1_reg_id : (I) 対象のレジスタID
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawRLedHex(U1 u1_reg_id)
{
    U4 u4_msk = 1 << u1f_g_CpuDebugGetRegBitTop(u1_reg_id); // ビット毎取得用

    printf(szacpc_s_RegLabelTbl[u1_reg_id]);                // レジスタ名表示

    while(u4_msk > 0)                                       // LED風 2進数表示
    {
        if(u4f_g_CpuDebugReadReg(u1_reg_id) & u4_msk)
        {
            printf("●");                                   // 1 の場合LED点灯
        }
        else
        {
            printf("○");                                   // 0 の場合LED消灯
        }
        u4_msk /= 2;                                        // 次のビットへ
    }

    printf(" $%X", u4f_g_CpuDebugReadReg(u1_reg_id));       // 16進表示
}

//============================================================================================
// 1ビットフラグを表示 (LED ON/OFF, 0/1)
//--------------------------------------------------------------------------------------------
// Arguments : const char* const szcpc_label : (I) 表示用ラベル
//             U1                u1_reg_id   : (I) 対象のレジスタID
//             U1                u1_bit      : (I) 対象ビット番号
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawFlg(const char* const szcpc_label, U1 u1_reg_id, U1 u1_bit)
{
    U4 msk = 1 << u1_bit;                            // 対象ビットのみを取り出すマスク

    printf(szcpc_label);                             // ラベル表示

    if(u4f_g_CpuDebugReadReg(u1_reg_id) & msk)
    {
        printf("● 1");                              // 1 の場合LED点灯
    }
    else
    {
        printf("○ 0");                              // 0 の場合LED消灯
    }
}

//============================================================================================
// ROM値を2進表示
//--------------------------------------------------------------------------------------------
// Arguments : U4 u4_val : 表示対象となる値
//             U1 u1_num : 表示するビット数
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawRomBin(U4 u4_val, U1 u1_num)
{
    U4 u4_msk = 1 << u1_num;                         // 表示ビット数の最上位ビットのみ1を立てる

    while(u4_msk > 0)                                // 最下位ビットの表示完了までループ
    {                                                // 上位のビットから順に表示
        if(u4_val & u4_msk) printf("1");             // 対象ビットが 1
        else                printf("0");             // 対象ビットが 0
        u4_msk /= 2;                                 // 次(1つ下位)のビットへ
    }
}

//============================================================================================
// PCとレジスタの表示更新 (1ステップ実行時の描画更新用)
//--------------------------------------------------------------------------------------------
// Arguments : -
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawUpdateExe(VD)
{
    vdf_s_DbgDrawUpdatePc(u4_s_PrevPc);              // 前回PCのリスト表示を更新
    vdf_s_DbgDrawUpdatePc(u4f_g_CpuDebugGetPc());    // 現在PCのリスト表示を更新

    vdf_s_DbgDrawReg();                              // レジスタ状態表示を更新
}

//============================================================================================
// PC位置の更新
//--------------------------------------------------------------------------------------------
// Arguments : U4  u4_addr : (I) ROM内のアドレス
// Return    : -
//============================================================================================
static VD vdf_s_DbgDrawUpdatePc(U4 u4_addr)
{
    printf(szacpc_s_ListRowTbl[u4_addr]);            // カーソルをリスト表示行に移動

    if(u4_addr == u4f_g_CpuDebugGetPc())
    {
        printf("→ ");                               // 引数が現在のPCと一致ならカーソルを表示
    }
    else
    {
        printf("   ");                               // 引数が現在のPCと不一致なら空白を表示
    }
}

