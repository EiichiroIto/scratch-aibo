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
// �����ޥ������ե�����
// ��������2003/08/27
// ��������2003/10/17 Ũ�����롢̣���ץ쥤�䡼�����٥�ȡ�LED�����ɲ�
// ��������2003/12/17 (3a) ­��򸫤��ɲ�
// ��������2005/09/15 �����󥿡������ޥ���ɲ�
// ��������2006/04/17 �ӡ����󥳥ޥ���ɲ�
// ��������2007/04/13 �����ϰϤξ����ɲá��������䥤�٥�Ⱦ�����

#ifndef COMMAND_h_DEFINED
#define COMMAND_h_DEFINED

// �����̰��� ------------------------------------------------------
const int condSensor = 2;		// ���󥵡����
const int condPSDSensor = 3;		// ��Υ���󥵡����
const int condEvent = 5;		// ���٥���Ѿ��
const int condTimer1 = 6;		// �����ޡ�1���
const int condTimer2 = 13;		// �����ޡ�2���
const int condVisionRange = 20;		// ����ϰ�
const int condVisionNearRange = 21;	// ����ϰ�(���Υ)
const int condMemoryRange = 22;		// �����ϰ�
const int condVision = 30;		// ��о��
//const int condMemory = 31;		// �������
const int condCounter1 = 41;		// �����󥿡�1���
const int condCounter2 = 42;		// �����󥿡�2���
const int condCounter3 = 43;		// �����󥿡�3���
const int condReturn = 97;		// �������
const int condMessage = 98;		// ��å��������
const int condMisc = 99;		// ����¾���

// �ڥ��󥵡�����
const int scodeBacksw = 0;		// ���楹���å��������줿
const int scodeTactF = 1;		// Ƭ(��)�����å��������줿
const int scodeTactR = 2;		// Ƭ(��)�����å��������줿
const int scodeTin = 3;			// �她���å��������줿
const int scodeLegRF = 4;		// ����­�����å��������줿
const int scodeLegLF = 5;		// ����­�����å��������줿
const int scodeLegRR = 6;		// ����­�����å��������줿
const int scodeLegLR = 7;		// ����­�����å��������줿
// �ڵ�Υ���󥵡�����
//   �ѥ�᡼������ˤ��
// �ڥ����ޡ�����
//   �ѥ�᡼������ˤ��
// ����������
const int rcodeNone = 0;		// �����ֹ�ʤ�
const int rcodeNumber0 = 1;		// �����ֹ�=0
const int rcodeNumber1 = 2;		// �����ֹ�=1
const int rcodeNumber2 = 3;		// �����ֹ�=2
const int rcodeNumber3 = 4;		// �����ֹ�=3
const int rcodeNumber4 = 5;		// �����ֹ�=4
const int rcodeNumber5 = 6;		// �����ֹ�=5
const int rcodeNumber6 = 7;		// �����ֹ�=6
const int rcodeNumber7 = 8;		// �����ֹ�=7
const int rcodeNumber8 = 9;		// �����ֹ�=8
const int rcodeNumber9 = 10;		// �����ֹ�=9
// �ڥ�å���������
const int mscodeNone = 0;		// ��å������ʤ�
const int mscodeMessage0 = 1;		// ��å�����=0
const int mscodeMessage1 = 2;		// ��å�����=1
const int mscodeMessage2 = 3;		// ��å�����=2
const int mscodeMessage3 = 4;		// ��å�����=3
const int mscodeMessage4 = 5;		// ��å�����=4
const int mscodeMessage5 = 6;		// ��å�����=5
const int mscodeMessage6 = 7;		// ��å�����=6
const int mscodeMessage7 = 8;		// ��å�����=7
const int mscodeMessage8 = 9;		// ��å�����=8
const int mscodeMessage9 = 10;		// ��å�����=9
// �ڤ���¾����
const int micodeCompleteHeading = 0;	// ���Ϥ򸫲�ư���λ����
const int micodeKickoff = 1;		// �������७�å����դ�
const int micodeWin = 2;		// ���äƤ���
const int micodeLose = 3;		// �餱�Ƥ���
const int micodeDraw = 4;		// ����ʬ���Ƥ���
// �ڻ�о���
const int vicodeBallLost = 1;		// �ܡ��뤬�����ʤ�
const int vicodeMikataGoalLost = 11;	// ̣�������뤬�����ʤ�
const int vicodeTekiGoalLost = 21;	// Ũ�����뤬�����ʤ�
const int vicodeLeftBeaconLost = 23;	// ���ӡ����󤬸����ʤ�
const int vicodeRightBeaconLost = 13;	// ���ӡ����󤬸����ʤ�

// ��ư��ޥ�ɰ��� ------------------------------------------------------
const int hcmdRelease = -3;             // ����
const int hcmdInitialize = -2;          // �����
const int hcmdSetVerboseMode = -1;	// verboseMode����
const int hcmdNone = 0;			// ���⤷�ʤ�
const int hcmdDown = 1;			// �����˲������
const int hcmdTrackBall = 2;		// �ܡ�����ɤ�������
const int hcmdSearchBallSlow = 3;	// ��ä���ܡ����õ��
const int hcmdNormal = 4;		// ɸ������ˤʤ�
const int hcmdSearchBall = 5;		// �ܡ����õ��
const int hcmdStop = 11;		// ���ư����ߤ��
//const int hcmdResetComplete = 15;	// Complete����򥯥ꥢ����
const int hcmdTakeLayerM = 17;		// LayerM��������¸����
const int hcmdTakeLayerC = 18;		// LayerC��������¸����
const int hcmdSearchAny = 24;		// �������ܡ����õ��
const int hcmdSearchGoalB = 25;		// �ĥ������õ��
const int hcmdSearchMikataGoal = 25;	// ̣���������õ��
const int hcmdSearchGoalY = 26;		// ���������õ��
const int hcmdSearchTekiGoal = 26;	// Ũ�������õ��
const int hcmdTrackGoalB = 27;		// �ĥ�������ܤ��ɤ�
const int hcmdTrackMikataGoal = 27;	// ̣����������ܤ��ɤ�
const int hcmdTrackGoalY = 28;		// ����������ܤ��ɤ�
const int hcmdTrackTekiGoal = 28;	// Ũ��������ܤ��ɤ�
const int hcmdTrackBallUpper = 30;	// �ܡ���ξ�������ɤ�������
const int hcmdTrackBeaconBY = 31;	// �Ĳ��ӡ�������ܤ��ɤ�
const int hcmdTrackRightBeacon = 31;	// ���ӡ�������ܤ��ɤ�
const int hcmdTrackBeaconYB = 33;	// ���ĥӡ�������ܤ��ɤ�
const int hcmdTrackLeftBeacon = 33;	// ���ӡ�������ܤ��ɤ�
const int hcmdSearchBeacon = 35;	// �ӡ������õ��
const int hcmdSearchGoal = 36;		// �������õ��
const int hcmdSearchBeaconBY = 37;	// �Ĳ��ӡ������õ��
const int hcmdSearchRightBeacon = 37;	// ���ӡ������õ��
const int hcmdSearchBeaconYB = 38;	// ���ĥӡ������õ��
const int hcmdSearchLeftBeacon = 38;	// ���ӡ������õ��
const int hcmdFaceToBall = 40;		// �ܡ���ΰ��֤����
const int hcmdReadHeadings = 97;	// ��⡼�����ե�������ɤ߹���
const int hcmdExecute = 98;		// ��⡼������¹Ԥ���
const int hcmdSetHeading = 99;		// �����ͤ����ꤹ��
// ɽ�𥳥ޥ�ɰ��� ------------------------------------------------------
const int fcmdNone = -1;		// ľ����ɽ���ݻ�����
const int fcmdOff = 0;			// ɽ���ä�
const int fcmdEarMove1 = 1;		// ���򣱲�ư��
const int fcmdEarMove2 = 2;		// ���򣲲�ư��
const int fcmdEarUp = 3;		// ����夲��
const int fcmdEarDown = 4;		// ���򲼤���
const int fcmdSerious = 5;		// �����ʴ�
const int fcmdLaugh = 6;		// �Фä���
const int fcmdCry = 7;			// �㤤����
const int fcmdAll = 8;			// ���ƤΥ��פ���������
const int fcmdULeft = 9;		// ��¦(��)�Υ��פ���������
const int fcmdURight = 10;		// ��¦(��)�Υ��פ���������
const int fcmdMLeft = 11;		// ��¦(��)�Υ��פ���������
const int fcmdMRight = 12;		// ��¦(��)�Υ��פ���������
const int fcmdLLeft = 13;		// ��¦(��)�Υ��פ���������
const int fcmdLRight = 14;		// ��¦(��)�Υ��פ���������
#ifdef ERS210
const int fcmdModeOn = 15;		// �⡼�ɥ��פ�������
const int fcmdModeOff = 16;		// �⡼�ɥ��פ�ä�
const size_t NumFaceCommands = 17;	// ɽ�𥳥ޥ�ɤθĿ�
#endif // ERS210
#ifdef ERS7
const int fcmdLanOn = 15;		// ̵�����פ�������
const int fcmdLanOff = 16;		// ̵�����פ�ä�
const int fcmdHeadColor = 17;		// Ƭ������
const int fcmdHeadWhite = 18;		// Ƭ������
const int fcmdHeadOff = 19;		// Ƭ���פ�ä�
const int fcmdModeRed = 20;		// �⡼�ɥ�����
const int fcmdModeGreen = 21;		// �⡼�ɥ�����
const int fcmdModeBlue = 22;		// �⡼�ɥ�����
const int fcmdModeOff = 23;		// �⡼�ɥ��פ�ä�
const int fcmdToLeft = 24;		// ����
const int fcmdToRight = 25;		// ����
const size_t NumFaceCommands = 26;	// ɽ�𥳥ޥ�ɤθĿ�
#endif // ERS7

// �������ޥ�ɰ��� ------------------------------------------------------
const int icmdNone = -1;		// �ʤˤ⤷�ʤ�
const int icmdResetTimer1 = 0;		// �����ޡ�1��ꥻ�åȤ���
const int icmdClearReturnNo = 1;	// �����ֹ�򥯥ꥢ����
const int icmdClearMessageNo = 2;	// ��å������ֹ�򥯥ꥢ����
const int icmdMessage0 = 3;		// ��å�����0����������
const int icmdMessage1 = 4;		// ��å�����1����������
const int icmdMessage2 = 5;		// ��å�����2����������
const int icmdMessage3 = 6;		// ��å�����3����������
const int icmdMessage4 = 7;		// ��å�����4����������
const int icmdMessage5 = 8;		// ��å�����5����������
const int icmdMessage6 = 9;		// ��å�����6����������
const int icmdMessage7 = 10;		// ��å�����7����������
const int icmdMessage8 = 11;		// ��å�����8����������
const int icmdMessage9 = 12;		// ��å�����9����������
const int icmdResetTimer2 = 13;		// �����ޡ�2��ꥻ�åȤ���
const int icmdCounter1Reset = 20;	// �����󥿡�1��ꥻ�åȤ���
const int icmdCounter1Inc = 21;		// �����󥿡�1��+1����
const int icmdCounter1Dec = 22;		// �����󥿡�1��-1����
const int icmdCounter2Reset = 30;	// �����󥿡�2��ꥻ�åȤ���
const int icmdCounter2Inc = 31;		// �����󥿡�2��+1����
const int icmdCounter2Dec = 32;		// �����󥿡�2��-1����
const int icmdCounter3Reset = 40;	// �����󥿡�3��ꥻ�åȤ���
const int icmdCounter3Inc = 41;		// �����󥿡�3��+1����
const int icmdCounter3Dec = 42;		// �����󥿡�3��-1����
const int icmdStartLog = 99;		// ��Ͽ�򳫻Ϥ���
const int icmdEndLog = 98;		// ��Ͽ��λ������¸����
const int icmdResetLog = 97;		// ��Ͽ��ꥻ�åȤ���

#endif // COMMAND_h_DEFINED
