//
// Copyright 2005 (C) Eiichiro ITO, GHC02331@nifty.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Eiichiro ITO, 15 October 2005
// mailto: GHC02331@nifty.com
//
// 条件・コマンド定義ファイル
// 作成日：2003/08/27
// 更新日：2003/10/17 敵ゴール、味方プレイヤー、イベント、LED左右追加
// 更新日：2003/12/17 (3a) 足先を見る追加
// 更新日：2005/09/15 カウンター条件、コマンド追加
// 更新日：2006/04/17 ビーコンコマンド追加
// 更新日：2007/04/13 記憶範囲の条件を追加、記憶条件やイベント条件を削除

#ifndef COMMAND_h_DEFINED
#define COMMAND_h_DEFINED

// 条件種別一覧 ------------------------------------------------------
const int condSensor = 2;		// センサー条件
const int condPSDSensor = 3;		// 距離センサー条件
const int condEvent = 5;		// イベント用条件
const int condTimer1 = 6;		// タイマー1条件
const int condTimer2 = 13;		// タイマー2条件
const int condVisionRange = 20;		// 視覚範囲
const int condVisionNearRange = 21;	// 視覚範囲(近距離)
const int condMemoryRange = 22;		// 記憶範囲
const int condVision = 30;		// 視覚条件
//const int condMemory = 31;		// 記憶条件
const int condCounter1 = 41;		// カウンター1条件
const int condCounter2 = 42;		// カウンター2条件
const int condCounter3 = 43;		// カウンター3条件
const int condReturn = 97;		// 復帰条件
const int condMessage = 98;		// メッセージ条件
const int condMisc = 99;		// その他条件

// 【センサー条件】
const int scodeBacksw = 0;		// 背中スイッチが押された
const int scodeTactF = 1;		// 頭(前)スイッチが押された
const int scodeTactR = 2;		// 頭(後)スイッチが押された
const int scodeTin = 3;			// 舌スイッチが押された
const int scodeLegRF = 4;		// 右前足スイッチが押された
const int scodeLegLF = 5;		// 左前足スイッチが押された
const int scodeLegRR = 6;		// 右後足スイッチが押された
const int scodeLegLR = 7;		// 左後足スイッチが押された
// 【距離センサー条件】
//   パラメータ指定による
// 【タイマー条件】
//   パラメータ指定による
// 【復帰条件】
const int rcodeNone = 0;		// 復帰番号なし
const int rcodeNumber0 = 1;		// 復帰番号=0
const int rcodeNumber1 = 2;		// 復帰番号=1
const int rcodeNumber2 = 3;		// 復帰番号=2
const int rcodeNumber3 = 4;		// 復帰番号=3
const int rcodeNumber4 = 5;		// 復帰番号=4
const int rcodeNumber5 = 6;		// 復帰番号=5
const int rcodeNumber6 = 7;		// 復帰番号=6
const int rcodeNumber7 = 8;		// 復帰番号=7
const int rcodeNumber8 = 9;		// 復帰番号=8
const int rcodeNumber9 = 10;		// 復帰番号=9
// 【メッセージ条件】
const int mscodeNone = 0;		// メッセージなし
const int mscodeMessage0 = 1;		// メッセージ=0
const int mscodeMessage1 = 2;		// メッセージ=1
const int mscodeMessage2 = 3;		// メッセージ=2
const int mscodeMessage3 = 4;		// メッセージ=3
const int mscodeMessage4 = 5;		// メッセージ=4
const int mscodeMessage5 = 6;		// メッセージ=5
const int mscodeMessage6 = 7;		// メッセージ=6
const int mscodeMessage7 = 8;		// メッセージ=7
const int mscodeMessage8 = 9;		// メッセージ=8
const int mscodeMessage9 = 10;		// メッセージ=9
// 【その他条件】
const int micodeCompleteHeading = 0;	// 周囲を見回す動作が完了した
const int micodeKickoff = 1;		// 自チームキックオフか
const int micodeWin = 2;		// 勝っている
const int micodeLose = 3;		// 負けている
const int micodeDraw = 4;		// 引き分けている
// 【視覚条件】
const int vicodeBallLost = 1;		// ボールが見えない
const int vicodeMikataGoalLost = 11;	// 味方ゴールが見えない
const int vicodeTekiGoalLost = 21;	// 敵ゴールが見えない
const int vicodeLeftBeaconLost = 23;	// 左ビーコンが見えない
const int vicodeRightBeaconLost = 13;	// 右ビーコンが見えない

// 首動作コマンド一覧 ------------------------------------------------------
const int hcmdRelease = -3;             // 開放
const int hcmdInitialize = -2;          // 初期化
const int hcmdSetVerboseMode = -1;	// verboseMode設定
const int hcmdNone = 0;			// 何もしない
const int hcmdDown = 1;			// 完全に下を向く
const int hcmdTrackBall = 2;		// ボールを追いかける
const int hcmdSearchBallSlow = 3;	// ゆっくりボールを探す
const int hcmdNormal = 4;		// 標準姿勢になる
const int hcmdSearchBall = 5;		// ボールを探す
const int hcmdStop = 11;		// 首の動きを止める
//const int hcmdResetComplete = 15;	// Complete情報をクリアする
const int hcmdTakeLayerM = 17;		// LayerM画像を保存する
const int hcmdTakeLayerC = 18;		// LayerC画像を保存する
const int hcmdSearchAny = 24;		// ゴールやボールを探す
const int hcmdSearchGoalB = 25;		// 青ゴールを探す
const int hcmdSearchMikataGoal = 25;	// 味方ゴールを探す
const int hcmdSearchGoalY = 26;		// 黄ゴールを探す
const int hcmdSearchTekiGoal = 26;	// 敵ゴールを探す
const int hcmdTrackGoalB = 27;		// 青ゴールを目で追う
const int hcmdTrackMikataGoal = 27;	// 味方ゴールを目で追う
const int hcmdTrackGoalY = 28;		// 黄ゴールを目で追う
const int hcmdTrackTekiGoal = 28;	// 敵ゴールを目で追う
const int hcmdTrackBallUpper = 30;	// ボールの少し上を追いかける
const int hcmdTrackBeaconBY = 31;	// 青黄ビーコンを目で追う
const int hcmdTrackRightBeacon = 31;	// 右ビーコンを目で追う
const int hcmdTrackBeaconYB = 33;	// 黄青ビーコンを目で追う
const int hcmdTrackLeftBeacon = 33;	// 左ビーコンを目で追う
const int hcmdSearchBeacon = 35;	// ビーコンを探す
const int hcmdSearchGoal = 36;		// ゴールを探す
const int hcmdSearchBeaconBY = 37;	// 青黄ビーコンを探す
const int hcmdSearchRightBeacon = 37;	// 右ビーコンを探す
const int hcmdSearchBeaconYB = 38;	// 黄青ビーコンを探す
const int hcmdSearchLeftBeacon = 38;	// 左ビーコンを探す
const int hcmdFaceToBall = 40;		// ボールの位置を向く
const int hcmdReadHeadings = 97;	// 首モーションファイルを読み込む
const int hcmdExecute = 98;		// 首モーションを実行する
const int hcmdSetHeading = 99;		// 関節値を設定する
// 表情コマンド一覧 ------------------------------------------------------
const int fcmdNone = -1;		// 直前の表情を維持する
const int fcmdOff = 0;			// 表情を消す
const int fcmdEarMove1 = 1;		// 耳を１回動作
const int fcmdEarMove2 = 2;		// 耳を２回動作
const int fcmdEarUp = 3;		// 耳を上げる
const int fcmdEarDown = 4;		// 耳を下げる
const int fcmdSerious = 5;		// 真剣な顔
const int fcmdLaugh = 6;		// 笑った顔
const int fcmdCry = 7;			// 泣いた顔
const int fcmdAll = 8;			// 全てのランプを点灯する
const int fcmdULeft = 9;		// 左側(上)のランプを点灯する
const int fcmdURight = 10;		// 右側(上)のランプを点灯する
const int fcmdMLeft = 11;		// 左側(中)のランプを点灯する
const int fcmdMRight = 12;		// 右側(中)のランプを点灯する
const int fcmdLLeft = 13;		// 左側(下)のランプを点灯する
const int fcmdLRight = 14;		// 右側(下)のランプを点灯する
#ifdef ERS210
const int fcmdModeOn = 15;		// モードランプを点ける
const int fcmdModeOff = 16;		// モードランプを消す
const size_t NumFaceCommands = 17;	// 表情コマンドの個数
#endif // ERS210
#ifdef ERS7
const int fcmdLanOn = 15;		// 無線ランプを点ける
const int fcmdLanOff = 16;		// 無線ランプを消す
const int fcmdHeadColor = 17;		// 頭ランプ橙
const int fcmdHeadWhite = 18;		// 頭ランプ白
const int fcmdHeadOff = 19;		// 頭ランプを消す
const int fcmdModeRed = 20;		// モードランプ赤
const int fcmdModeGreen = 21;		// モードランプ緑
const int fcmdModeBlue = 22;		// モードランプ青
const int fcmdModeOff = 23;		// モードランプを消す
const int fcmdToLeft = 24;		// 左へ
const int fcmdToRight = 25;		// 右へ
const size_t NumFaceCommands = 26;	// 表情コマンドの個数
#endif // ERS7

// 内部コマンド一覧 ------------------------------------------------------
const int icmdNone = -1;		// なにもしない
const int icmdResetTimer1 = 0;		// タイマー1をリセットする
const int icmdClearReturnNo = 1;	// 復帰番号をクリアする
const int icmdClearMessageNo = 2;	// メッセージ番号をクリアする
const int icmdMessage0 = 3;		// メッセージ0を送信する
const int icmdMessage1 = 4;		// メッセージ1を送信する
const int icmdMessage2 = 5;		// メッセージ2を送信する
const int icmdMessage3 = 6;		// メッセージ3を送信する
const int icmdMessage4 = 7;		// メッセージ4を送信する
const int icmdMessage5 = 8;		// メッセージ5を送信する
const int icmdMessage6 = 9;		// メッセージ6を送信する
const int icmdMessage7 = 10;		// メッセージ7を送信する
const int icmdMessage8 = 11;		// メッセージ8を送信する
const int icmdMessage9 = 12;		// メッセージ9を送信する
const int icmdResetTimer2 = 13;		// タイマー2をリセットする
const int icmdCounter1Reset = 20;	// カウンター1をリセットする
const int icmdCounter1Inc = 21;		// カウンター1を+1する
const int icmdCounter1Dec = 22;		// カウンター1を-1する
const int icmdCounter2Reset = 30;	// カウンター2をリセットする
const int icmdCounter2Inc = 31;		// カウンター2を+1する
const int icmdCounter2Dec = 32;		// カウンター2を-1する
const int icmdCounter3Reset = 40;	// カウンター3をリセットする
const int icmdCounter3Inc = 41;		// カウンター3を+1する
const int icmdCounter3Dec = 42;		// カウンター3を-1する
const int icmdStartLog = 99;		// 記録を開始する
const int icmdEndLog = 98;		// 記録を終了して保存する
const int icmdResetLog = 97;		// 記録をリセットする

#endif // COMMAND_h_DEFINED
