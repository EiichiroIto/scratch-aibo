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

#include <OPENR/OPENRAPI.h>
#include <OPENR/OSyslog.h>
#include "PowerMonitor.h"
#include "entry.h"

PowerMonitor::PowerMonitor ()
{
}

OStatus
PowerMonitor::DoInit(const OSystemEvent& event)
{
    return oSUCCESS;
}

OStatus
PowerMonitor::DoStart(const OSystemEvent& event)
{
    OStatus result;
    OPowerStatus currentStatus;

    result = OPENR::GetPowerStatus(&currentStatus);
    if (result == oSUCCESS) {
        PrintPowerStatus(currentStatus);
    } else {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "PowerMonitor::DoStart()",
                  "OPENR::GetPowerStatus() FAILED", result));
    }

    OPowerStatus observationStatus;
    observationStatus.Set(orsbALL,
                          obsbALL,
                          opsoREMAINING_CAPACITY_NOT_NOTIFY,
                          opsoTEMPERATURE_NOT_NOTIFY,
                          opsoTIME_DIF_NOT_NOTIFY,
                          opsoVOLUME_NOT_NOTIFY);
    
    OServiceEntry entry(myOID_, Extra_Entry[entryUpdatePowerStatus]);
    result = OPENR::ObservePowerStatus(observationStatus, entry);
    if(result != oSUCCESS) {
        OSYSLOG1((osyslogERROR, "%s : %s %d",
                  "PowerMonitor::DoStart()",
                  "OPENR::ObservePowerStatus() FAILED", result));
        return oFAIL;
    }

    return oSUCCESS;
}    

OStatus
PowerMonitor::DoStop(const OSystemEvent& event)
{
    return oSUCCESS;
}

OStatus
PowerMonitor::DoDestroy(const OSystemEvent& event)
{
    return oSUCCESS;
}

void
PowerMonitor::UpdatePowerStatus(void* msg)
{
    OPowerStatusMessage *pwsMsg = (OPowerStatusMessage*)msg;
    OPowerStatus status = pwsMsg->powerStatus;

    if (status.robotStatus & orsbPAUSE ||
        status.robotStatus & orsbBATTERY_CAPACITY_LOW) {

        PrintPowerStatus(status);

        OServiceEntry entry(myOID_, Extra_Entry[entryUpdatePowerStatus]);
        OStatus result = OPENR::UnobservePowerStatus(entry);
        if(result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "PowerMonitor::UpdatePowerStatus()",
                      "OPENR::UnobservePowerStatus() FAILED", result));
        }

        result = OPENR::SetMotorPower(opowerOFF);
        if(result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "PowerMonitor::UpdatePowerStatus()",
                      "OPENR::SetMotorPower() FAILED", result));
        }

        OBootCondition bootCond(obcbPAUSE_SW);
        result = OPENR::Shutdown(bootCond);
        if(result != oSUCCESS) {
            OSYSLOG1((osyslogERROR, "%s : %s %d",
                      "PowerMonitor::UpdatePowerStatus()",
                      "OPENR::Shutdown() FAILED", result));
            return;
        }

        OSYSDEBUG(("#########################\n"));
        OSYSDEBUG(("###  SYSTEM SHUTDOWN  ###\n"));
        OSYSDEBUG(("#########################\n"));
    }
}

void
PowerMonitor::PrintPowerStatus(const OPowerStatus& ps)
{
    OSYSDEBUG(("[OPowerStatus]\n"));
    OSYSDEBUG(("  robotStatus          : 0x%08x\n", ps.robotStatus));
    OSYSDEBUG(("  batteryStatus        : 0x%04x\n", ps.batteryStatus));
    OSYSDEBUG(("  remainingCapacity    : %d\n", ps.remainingCapacity));
    OSYSDEBUG(("  temperature          : %d\n", ps.temperature));
    OSYSDEBUG(("  fullyChargedCapacity : %d\n", ps.fullyChargedCapacity));
    OSYSDEBUG(("  voltage              : %d\n", ps.voltage));
    OSYSDEBUG(("  current              : %d\n", ps.current));
    OSYSDEBUG(("  timeDif              : %d\n", ps.timeDif));
    OSYSDEBUG(("  volume               : %d\n", ps.volume));

    OSYSDEBUG(("[ORobotStatus]\n"));
    if(ps.robotStatus & orsbPAUSE) {
        OSYSDEBUG(("  PAUSE\n"));
    }
    if(ps.robotStatus & orsbMOTOR_POWER) {
        OSYSDEBUG(("  MOTOR_POWER\n"));
    }
    if(ps.robotStatus & orsbVIBRATION_DETECT) {
        OSYSDEBUG(("  VIBRATION_DETECT\n"));
    }
    if(ps.robotStatus & orsbEX_PORT_CONNECTED) {
        OSYSDEBUG(("  EX_PORT_CONNECTED\n"));
    }
    if(ps.robotStatus & orsbSTATION_CONNECTED) {
        OSYSDEBUG(("  STATION_CONNECTED\n"));
    }
    if(ps.robotStatus & orsbEX_POWER_CONNECTED) {
        OSYSDEBUG(("  EX_POWER_CONNECTED\n"));
    }
    if(ps.robotStatus & orsbBATTERY_CONNECTED) {
        OSYSDEBUG(("  BATTERY_CONNECTED\n"));
    }
    if(ps.robotStatus & orsbBATTERY_CHARGING) {
        OSYSDEBUG(("  BATTERY_CHARGING\n"));
    }
    if(ps.robotStatus & orsbBATTERY_CAPACITY_FULL) {
        OSYSDEBUG(("  BATTERY_CAPACITY_FULL\n"));
    }
    if(ps.robotStatus & orsbBATTERY_CAPACITY_LOW) {
        OSYSDEBUG(("  BATTERY_CAPACITY_LOW\n"));
    }
    if(ps.robotStatus & orsbBATTERY_OVER_CURRENT) {
        OSYSDEBUG(("  BATTERY_OVER_CURRENT\n"));
    }
    if(ps.robotStatus & orsbBATTERY_OVER_TEMP_DISCHARGING) {
        OSYSDEBUG(("  BATTERY_OVER_TEMP_DISCHARGING\n"));
    }
    if(ps.robotStatus & orsbBATTERY_OVER_TEMP_CHARGING) {
        OSYSDEBUG(("  BATTERY_OVER_TEMP_CHARGING\n"));        
    }
    if(ps.robotStatus & orsbBATTERY_ERROR_OF_CHARGING) {
        OSYSDEBUG(("  BATTERY_ERROR_OF_CHARGING\n")); 
    }
    if(ps.robotStatus & orsbERROR_OF_PLUNGER) {
        OSYSDEBUG(("  ERROR_OF_PLUNGER\n"));
    }
    if(ps.robotStatus & orsbOPEN_R_POWER_GOOD) {
        OSYSDEBUG(("  OPEN_R_POWER_GOOD\n"));
    }
    if(ps.robotStatus & orsbERROR_OF_FAN) {
        OSYSDEBUG(("  ERROR_OF_FAN\n"));
    }
    if(ps.robotStatus & orsbDATA_STREAM_FROM_STATION) {
        OSYSDEBUG(("  DATA_STREAM_FROM_STATION\n"));
    }
    if(ps.robotStatus & orsbREGISTER_UPDATED_BY_STATION) {
        OSYSDEBUG(("  REGISTER_UPDATED_BY_STATION\n"));
    }
    if(ps.robotStatus & orsbRTC_ERROR) {
        OSYSDEBUG(("  RTC_ERROR\n"));
    }
    if(ps.robotStatus & orsbRTC_OVERFLOW) {
        OSYSDEBUG(("  RTC_OVERFLOW\n"));
    }
    if(ps.robotStatus & orsbRTC_RESET) {
        OSYSDEBUG(("  RTC_RESET\n"));
    }
    if(ps.robotStatus & orsbRTC_SET) {
        OSYSDEBUG(("  RTC_SET\n"));
    }
    if(ps.robotStatus & orsbSPECIAL_MODE) {
        OSYSDEBUG(("  SPECIAL_MODE\n"));
    }
    if(ps.robotStatus & orsbBMN_DEBUG_MODE) {
        OSYSDEBUG(("  BMN_DEBUG_MODE\n"));
    }
    if(ps.robotStatus & orsbCHARGER_STATUS) {
        OSYSDEBUG(("  CHARGER_STATUS\n"));
    }
    if(ps.robotStatus & orsbPLUNGER) {
        OSYSDEBUG(("  PLUNGER\n"));
    }
    if(ps.robotStatus & orsbSUSPENDED) {
        OSYSDEBUG(("  SUSPENDED\n"));
    }
    if(ps.robotStatus & orsbSPECIAL_DATA_READ_REQ) {
        OSYSDEBUG(("  SPECIAL_DATA_READ_REQ\n"));
    }
}
