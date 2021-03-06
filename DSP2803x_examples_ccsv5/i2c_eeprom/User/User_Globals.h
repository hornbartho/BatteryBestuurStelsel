/*
 * User_Globals.h
 *
 *  Created on: 04 May 2017
 *      Author: Sonja
 */

#ifndef USER_GLOBALS_H_
#define USER_GLOBALS_H_

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "math.h"
#include <stdio.h>
#include <float.h>
#include <string.h>
#include "User_Defines.h"
//#include "User_Queue.h"

extern struct I2CMSG I2cMsgOut1;
extern struct I2CMSG I2cMsgIn1;

extern struct I2CMSG *CurrentMsgPtr;               // Used in interrupts

extern struct BMS_STATUS_FLAGS;

extern volatile Uint16 DataOut;
extern volatile Uint16 DataOut2;
extern volatile Uint16 Received;

/////////////////////////////////////////////
extern volatile Uint16 flagTemp;
extern volatile Uint16 flagCurrent;
extern volatile Uint16 flagVoltage;

extern volatile Uint16 flagCharged;
extern volatile Uint16 flagDischarged;
extern volatile Uint16 balance;
extern volatile Uint16 flagPreCharge;
/////////////////////////////////////////////

extern volatile Uint16 counter_2Hz;

extern float Ireference;

extern volatile Uint16 Charger_status;
extern volatile int system_status;

extern volatile float ChargerVoltage;
extern volatile float ChargerCurrent;
 extern volatile  Uint16 ChargerDebug;
extern volatile float Current;
extern volatile int current_int;

extern volatile float Voltages[15];
extern volatile float Voltage_avg;
extern volatile float Voltage_high;
extern volatile float Voltage_high_cell;
extern volatile float Voltage_low;
extern volatile float Voltage_low_cell;
extern volatile float Temperatures[16];
extern volatile float Temperature_avg;
extern volatile float Temperatures_resistance[14];
extern volatile float Temperature_high;
extern volatile float Temperature_low;
extern volatile float Temperature_high_cell;
extern volatile float Temperature_low_cell;

extern volatile float ADCgain;
extern volatile float ADCoffset;
extern volatile float I;
extern volatile float Voltage_total;
extern volatile float Ah;
extern volatile int rus;

extern Uint32 ref;

extern volatile int CANcounter;

extern volatile float test_current;

extern volatile float test_blah[3];

extern int Cell_B1;
extern int Cell_B2;
extern int Cell_B3;

extern Uint16 NodeID;
extern volatile Uint16 counter_50Hz;

extern volatile float Auxilliary_Voltage;
extern volatile Uint16 Auxilliary_counter;


//defines:
extern volatile float Vmax;
extern volatile float Vmin;
extern volatile float Vcritical;
extern volatile float Vcharge;
extern volatile float Vbalance;

extern volatile int Imin;
extern volatile int Imax;
extern volatile float Ifilter;

extern volatile float Tmax;
extern volatile float Tmin;

extern volatile float SOC;
//extern volatile float SOH;

extern volatile float Vauxmin;
extern volatile float AuxChargeTime;

extern volatile float Vchargedflagreset;
extern volatile float Vdischargedflagreset;

extern volatile float kp_constant;
extern volatile float kp_multiplier;
extern volatile float balancing_upper_level;
extern volatile float balancing_bottom_level;



/* Declare variable using above structure and the function datapoints */
/* These coordinates correspond to the points illustrated in the above graph */
extern  long sine_x[5];
extern  long sine_y[5];

extern  struct table_1d sine_table;

extern long time_x[4];
extern long I_y[4];

extern struct table_1d trip_table;

extern long time2_x[2];
extern long I2_y[2];

extern struct table_1d trip2_table;


extern volatile float Current_CAL;
//extern volatile Uint16 Current_Sum;
//extern volatile Uint16 Current_Counter;


//extern volatile Uint16 Toets_current_sum[10];


extern volatile float SOC_t;
extern volatile float Wsoc;
extern volatile float SOCv;
extern volatile float SOCc;


//extern float ChgCurrent;

extern volatile float SOH_avg;
extern volatile float SOH_max;
extern volatile float SOH_max_cell;


extern volatile float Voltages_old[15];
extern volatile float Current_old;
extern volatile	float resistance;
extern volatile	float resistance_temp;
extern volatile float dI;

extern union bits32 TxData;

extern int toets;
extern int toets2;

extern int temptimer;

extern volatile Uint32 CAN_Charger_dataL;
extern volatile Uint32 CAN_Charger_dataH;

extern volatile Uint32 System_State;

extern volatile Uint32 BMS_Status;
extern volatile long trip_counter;
extern volatile Uint32 testcounter;

extern volatile long testvariable;
extern volatile long testvariable2;

extern volatile long testtrip;

extern volatile long timecounter;
extern volatile long timecounterseconds;

extern volatile Uint16 Charging_animation;

extern volatile Uint32 PreCharge_Timer;
extern volatile Uint16 Charger_Error_flag;

extern const long delay_PreCharge;

#endif /* USER_GLOBALS_H_ */
