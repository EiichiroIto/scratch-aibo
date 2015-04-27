//
// Copyright 2002,2003 Sony Corporation
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef _GameController_h_DEFINED
#define _GameController_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include "UDPConnection.h"
#include "GameControllerConfig.h"
#include "def.h"
#include "RoboCupGameControlData.h"


#ifdef ERS210
/* locations of the buttons that are used (as per previous constants */
static const char* const SENSOR_LOCATOR[] = {
    "PRM:/r6/s1-Sensor:s1",             // BACK SWITCH
    "PRM:/r1/c1/c2/c3/f2-Sensor:f2",    // HEAD TACTILE SENSOR (FRONT)
    "PRM:/r1/c1/c2/c3/f1-Sensor:f1",    // HEAD TACTILE SENSOR (REAR)
};


/* same thing but for the LEDs */
static const char* const LOCATOR[] = {
  "PRM:/r1/c1/c2/c3/l1-LED2:l1", // Eye light bottom left
  "PRM:/r1/c1/c2/c3/l2-LED2:l2", // Eye light middle left
  "PRM:/r1/c1/c2/c3/l3-LED2:l3", // Eye light top left
  "PRM:/r1/c1/c2/c3/l4-LED2:l4", // Eye light bottom right
  "PRM:/r1/c1/c2/c3/l5-LED2:l5", // Eye light middle right
  "PRM:/r1/c1/c2/c3/l6-LED2:l6", // Eye light top right
  "PRM:/r1/c1/c2/c3/l7-LED2:l7", // Mode light
  "PRM:/r6/l1-LED2:l1", // Tail light Blue
  "PRM:/r6/l2-LED2:l2", // Tail light Orange
};
#endif /* ERS210 */
#ifdef ERS7
/* locations of the buttons that are used (as per previous constants */
static const char* const SENSOR_LOCATOR[] = {
    "PRM:/t2-Sensor:t2",                // BACK SENSOR (REAR)
    "PRM:/t3-Sensor:t3",                // BACK SENSOR (MIDDLE)
    "PRM:/t4-Sensor:t4",                // BACK SENSOR (FRONT)
    "PRM:/r1/c1/c2/c3/t1-Sensor:t1",    // HEAD SENSOR
};


/* same thing but for the LEDs */
static const char* const LOCATOR[] = {
    "PRM:/lu-LED3:lu", // Back light (front,  color)
    "PRM:/lv-LED3:lv", // Back light (front,  white)
    "PRM:/lw-LED3:lw", // Back light (middle, color)
    "PRM:/lx-LED3:lx", // Back light (middle, white)
    "PRM:/ly-LED3:ly", // Back light (rear,   color)
    "PRM:/lz-LED3:lz", // Back light (rear,   white)
    "PRM:/r1/c1/c2/c3/la-LED3:la", // Face light1
    "PRM:/r1/c1/c2/c3/lb-LED3:lb", // Face light2
    "PRM:/r1/c1/c2/c3/lc-LED3:lc", // Face light3
    "PRM:/r1/c1/c2/c3/ld-LED3:ld", // Face light4
    "PRM:/r1/c1/c2/c3/le-LED3:le", // Face light5
    "PRM:/r1/c1/c2/c3/lf-LED3:lf", // Face light6
    "PRM:/r1/c1/c2/c3/lg-LED3:lg", // Face light7
    "PRM:/r1/c1/c2/c3/lh-LED3:lh", // Face light8
    "PRM:/r1/c1/c2/c3/li-LED3:li", // Face light9
    "PRM:/r1/c1/c2/c3/lj-LED3:lj", // Face light10
    "PRM:/r1/c1/c2/c3/lk-LED3:lk", // Face light11
    "PRM:/r1/c1/c2/c3/ll-LED3:ll", // Face light12
    "PRM:/r1/c1/c2/c3/lm-LED3:lm", // Face light13
    "PRM:/r1/c1/c2/c3/ln-LED3:ln"  // Face light14 
};
#endif /* ERS7 */


class GameController : public OObject {
public:
    GameController();
    virtual ~GameController() {}

    /* interface functions for RoboCupGameControlData
       use these for on demand information rather than waiting
       for the updates */
    uint32    getKickOffTeam();
    int       getPlayerNumber();
    int       getTeamNumber();
    TeamInfo* getMyTeam();
    RoboCupGameControlData* getGameData();

    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    void SendCont   (ANTENVMSG msg);
    void ReceiveCont(ANTENVMSG msg);
    void CloseCont  (ANTENVMSG msg);
    void UpdatePowerStatus(void* msg);

    void SensorUpdate(const ONotifyEvent& event);
    void LEDUpdate();

private:

    /* local copy of the GameController data */
    RoboCupGameControlData controlData;

    /* UDP related declarations */
    OStatus Bind   (int index);
    OStatus Send   (int index);
    OStatus Receive(int index);
    OStatus Close  (int index);
    OStatus InitUDPBuffer(int index);
    OStatus CreateUDPEndpoint(int index);

    antStackRef   ipstackRef;
    UDPConnection connection[GAMECONTROLLER_CONNECTION_MAX];


    /* data structure processing related declarations */
    bool validatePacket(RoboCupGameControlData *data);
    void processGameData(RoboCupGameControlData* data);
    bool isThisGame(RoboCupGameControlData* data);
    bool dataEqual(void* data, void* prev);
    bool checkHeader(char* header);
    void initGameData();
    void notifyDataObservers();
    void loadTeamCfg();
    void beepOnUnfairPenalize(RoboCupGameControlData* gameData);

    /* sensors related declarations */
    /* the number of SENSOR_TRIGGER_TIMEs the head button needs to be held
       to unpenalise a dog */
    static const int UNPENALISE_HOLDTIME_INITIAL = 30;

    /* the number milliseconds that need to pass for the
       button press to be accepted */
    static const int SENSOR_TRIGGER_TIME = 100;

    /* the number of milliseconds that needs to pass for
       the back buttons to register a penalty */
    static const int PENALTY_TRIGGER_TIME = 1000;

    /* the number of milliseconds between sensor updates
       this is used to calculate how much time a button
       has been pressed down for */
    static const int SENSOR_UPDATES = 30;

#ifdef ERS210
    /* the pressure threshold of the head and back buttons */
    static const int BACK_THRESHOLD = 1;
    static const int HEAD_THRESHOLD = 15;

    /* these constants for are recongising the buttons */
    static const int NUM_SENSORS = 3;
    static const int BACK_SW       = 0;
    static const int HEAD_SENSOR_F = 1;
    static const int HEAD_SENSOR_R = 2;
#endif /* ERS210 */
#ifdef ERS7
    /* the pressure threshold of the head and back buttons */
    static const int BACK_THRESHOLD = 15;
    static const int HEAD_THRESHOLD = 10;

    /* these constants for are recongising the buttons */
    static const int NUM_SENSORS = 4;
    static const int BACK_SW_R   = 0;
    static const int BACK_SW_M   = 1;
    static const int BACK_SW_F   = 2;
    static const int HEAD_SENSOR = 3;
#endif /* ERS7 */

    void InitERSSensorIndex(OSensorFrameVectorData* sensorVec);
    void processSensors(OSensorFrameVectorData* sensorVec);
    int  getSensorValue(OSensorFrameVectorData* sensorVec, int index);
    void updateGameState();
    void manualPenalise(bool penalise);
    void resetButton(int button);
    void swapTeams(int team);
    void rawSwapTeams(RoboCupGameControlData* data);

    TeamInfo* myTeam;
    uint32    teamNumber;
    int       playerNumber;
    int       holdCount;
    bool      initSensorIndex;
    int       sensorIdx[NUM_SENSORS];

    /* bools to fire when buttons have been pressed for a certain amount of time */
    /* keeps track of how many updates this has been pushed down for */
#ifdef ERS210
    bool buttonPressed[NUM_SENSORS];
    int  buttonCount[NUM_SENSORS];
#endif /* ERS210 */
#ifdef ERS7
    bool buttonPressed[NUM_SENSORS];
    int  buttonCount[NUM_SENSORS];
#endif /* ERS7 */

    /* LED related declarations */
    void OpenPrimitives();
    void ClosePrimitives();
    void NewCommandVectorData();
    void setBackLED(int LED, RCRegion* rgn);
    void setFaceLED(RCRegion* rgn);
    void setPowerLED(RCRegion* rgn);
    bool correctPlayerLED(int player, int LED);
    bool correctPowerLED(int remaining, int LED);
    RCRegion* FindFreeRegion();
    void initLEDs(RCRegion* rgn);

#ifdef ERS210
    static const int NUM_LEDS           = 9;
    static const int NUM_COMMAND_VECTOR = 2;
    static const int BLUE_LED           = 7;
    static const int YELLOW_LED         = 6;
    static const int RED_LED            = 8;
#endif /* ERS210 */
#ifdef ERS7
    static const int NUM_LEDS           = 20;
    static const int NUM_COMMAND_VECTOR = 2;
    static const int LED_ON             = 255;
    static const int LED_OFF            = 0;
    static const int BLUE_LED           = 0;
    static const int YELLOW_LED         = 2;
    static const int RED_LED            = 4;
#endif /* ERS7 */

    OPrimitiveID ledID[NUM_LEDS];
    RCRegion*    region[NUM_COMMAND_VECTOR];

    int lastStnNo;
    int lastSideInfo;
    bool yellowLEDOff;
};

#endif
