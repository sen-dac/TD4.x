//============================================================================================
// Application : TD4 Emulator & Assembler for X68000 - TD4.x
// Platform    : SHARP X68000 / Human68k
// Build       : GCC真里子版(ver1.42) + XCライブラリ
//--------------------------------------------------------------------------------------------
// Filename    : cpu.h
// Version     : 1.0.0
// Author      : SEN::DAC
// Description : ターゲットCPUの定義と処理
//============================================================================================
//============================================================================================
// 2重インクルード防止
//============================================================================================
#ifndef _CPU_H_
#define _CPU_H_

//============================================================================================
// インクルードファイル
//============================================================================================
#include "../StdAfx.h"

//============================================================================================
// 定数定義
//============================================================================================
// 命令関数 属性ビット (フラグ / マスク)
#define CPU_RET_B_JUMP (1 << 0)                          // ジャンプ命令ビット
#define CPU_RET_B_UINS (1 << 1)                          // 未定義命令ビット
#define CPU_RET_B_UIMM (1 << 2)                          // 不正Imビット

// 命令関数 戻り値 (CPU命令属性)
#define CPU_RET_NORM   0                                 // 0 : 通常命令
#define CPU_RET_NJMP   CPU_RET_B_JUMP                    // 1 : 通常ジャンプ命令
#define CPU_RET_UINS   CPU_RET_B_UINS                    // 2 : 未定義命令
#define CPU_RET_UJMP   (CPU_RET_B_UINS | CPU_RET_B_JUMP) // 3 : 未定義ジャンプ命令
#define CPU_RET_UIMM   CPU_RET_B_UIMM                    // 4 : 不正Im命令

// リセット要因 (ソフトウェアリセットは上層の管轄とする)
#define RST_TYPE_COLD  0       // パワーオンリセット
#define RST_TYPE_WARM  1       // 端子リセット

// レジスタ 属性
#define ATTR_R         0       // Read Onry  属性
#define ATTR_W         1       // Write Onry 属性
#define ATTR_RW        2       // RW 属性


// アドレスバスのビット幅 (Configuration)
#define CPU_ADR_BBW    4

// CPUが扱えるアドレス空間サイズ (Configuration)
#define CPU_ADR_MAX    (1 << CPU_ADR_BBW)

// CPU命令数 (Configuration)
#define CPU_INS_MAX    16

// レジスタID (Configuration)
#define CPU_REG_A      0       // Aレジスタ
#define CPU_REG_B      1       // Bレジスタ
#define CPU_REG_I      2       // Input
#define CPU_REG_O      3       // Output
#define CPU_REG_PC     4       // プログラムカウンタ
#define CPU_REG_SR     5       // ステータスレジスタ
#define CPU_REG_MAX    6       // レジスタ数

//============================================================================================
// 型定義
//============================================================================================
// レジスタ構造体
// 拡張性のためレジスタ幅は32ビット幅(4バイト)で定義し、有効ビットのみをマスクして扱う。
typedef struct
{
    const U4 u4c_cold_rst_val; // パワーオンリセット値
    const U4 u4c_warm_rst_val; // 端子リセット値
    U4 u4_val;                 // 現在の値
    U4 u4_max;                 // レジスタ最大値 / 有効ビット取得マスク
    U1 u1_bit_top;             // 有効ビット最上位番号
    U1 u1_rw;                  // R/W属性
}
stREG;

//============================================================================================
// extern
//============================================================================================

//============================================================================================
// プロトタイプ宣言
//============================================================================================
// デバッグ機能
VD vdf_g_CpuDebugIncPc(VD);                      // PCを1つ進める
VD vdf_g_CpuDebugDecPc(VD);                      // PCを1つ戻す
U4 u4f_g_CpuDebugGetPc(VD);                      // PCの値を取得する
U1 u1f_g_CpuDebugGetRegBitTop(U1 u1_reg);        // レジスタ最上位有効ビット番号を取得
U4 u4f_g_CpuDebugReadReg(U1 u1_reg);             // レジスタ直接 Read
BL blf_g_CpuDebugWriteReg(U1 u1_reg, U4 u4_val); // レジスタ直接 Write

// ターゲットCPU動作
VD vdf_g_CpuSetInput(U4 u4_val);                 // 入力ポート(Iレジスタ)に値を設定する
U4 u4f_g_CpuGetOutput(VD);                       // 出力ポート(Oレジスタ)の値を取得する
VD vdf_g_CpuReset(U1 u1_rst_type);               // CPUリセット(COLD / WARM)
U1 u1f_g_CpuExe1Clk(U1* u1p_rom);                // CPUを1クロック実行

#endif // _CPU_H_
