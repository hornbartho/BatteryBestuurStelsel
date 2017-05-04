/*
 * User_Functions.c
 *
 *  Created on: 04 May 2017
 *      Author: Sonja
 */

#include "User_Prototypes.h"
#include "User_Globals.h"

void Initialise_BMS(void)
{
    flagCurrent = 0;

    InitSysCtrl();
    InitI2CGpio();
    Init_Gpio();
    InitAdc();
    InitSpiaGpio();

    DINT;

    InitPieCtrl();

    IER = 0x0000;
    IFR = 0x0000;

    InitPieVectTable();

    EALLOW; // This is needed to write to EALLOW protected registers
    PieVectTable.I2CINT1A = &i2c_int1a_isr;
    PieVectTable.TINT0 = &cpu_timer0_isr;
    PieVectTable.TINT1 = &cpu_timer1_isr;
    PieVectTable.TINT2 = &cpu_timer2_isr;
    PieVectTable.ADCINT1 = &adc_isr;
    EDIS;   // This is needed to disable write to EALLOW protected registers

    I2CA_Init();
    InitCpuTimers();

    ConfigCpuTimer(&CpuTimer0, 60, 500000); //2 hz
    ConfigCpuTimer(&CpuTimer1, 60, 20000);  //50 hz
    ConfigCpuTimer(&CpuTimer2, 60, 500);    //2 Khz         //500

    CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
    CpuTimer1Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
    CpuTimer2Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

    // Enable interrupts required for this example

    // Enable ADC interrupt 10 in the PIE: Group 10 interrupt 1
    PieCtrlRegs.PIEIER10.bit.INTx1 = 1;
    IER |= M_INT10;

    // Enable CPU interrupt 1 in the PIE: Group 1 interrupt 7
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;          //clock
    IER |= M_INT1;
    IER |= M_INT13;
    IER |= M_INT14;

    // Enable I2C interrupt 1 in the PIE: Group 8 interrupt 1
    PieCtrlRegs.PIEIER8.bit.INTx1 = 1;
    // Enable I2C INT8 which is connected to PIE group 8
    IER |= M_INT8;

    ////////////////////////////B. Horn
    EnableInterrupts();
    ////////////////////////////B. Horn

    EINT;
    ERTM;   // Enable Global realtime interrupt DBGM

    CANSetup();
    CANMailboxConfig();
    CANInterruptConfig();

    configADC();

    //Turn on and initialise Bq76940
    Bq76940_Init();

    //  Shut_D_BQ();
}

void Reset_ADC(void)
{
    I2CA_WriteData(0x04,0x8);
    I2CA_WriteData(0x04,0x18);
}

void Init_Gpio(void)
{
    EALLOW;

    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;     //BQenable
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;      //Bq enable

    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0;     //led2
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;      //led2

    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0;     //led1
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;      //led1

    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;     //BT reset
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;      //BT reset
    BTReset = 0;                            //keep BT in reset

    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0;    //12 Aux drive
    GpioCtrlRegs.GPADIR.bit.GPIO15 = 1;     // 12 Aux drive (verander miskien)
    Aux_Control = 1;

    GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 0;    //KeyDrive
    GpioCtrlRegs.GPADIR.bit.GPIO19 = 0;     //(input) key drive (verander miskien)

    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0;    //contactor output
    GpioCtrlRegs.GPADIR.bit.GPIO20 = 1;     // contactor output
    ContactorOut = 0;                       //turn off contactor

    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;    //precharge resistor
    GpioCtrlRegs.GPADIR.bit.GPIO21 = 1;     // precharge resistor
    PreCharge = 1;                          //turn on precharge resistor

    GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 0;    //key switch
    GpioCtrlRegs.GPADIR.bit.GPIO24 = 0;     //key switch

    GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;    //BQ on
    GpioCtrlRegs.GPADIR.bit.GPIO26 = 0;     //BQ on (input)

    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;    //CANenable
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;     //CANenable (output)

    GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 0;    //led3
    GpioCtrlRegs.GPBDIR.bit.GPIO40 = 1;     //led3

    GpioCtrlRegs.GPBMUX1.bit.GPIO44 = 0;    //CScontrol
    GpioCtrlRegs.GPBDIR.bit.GPIO44 = 1;     //CSControl

    EDIS;

    //turn on contactor
    ContactorOut = 1;

    CSControl = 0;  //turn CScontrol on for current measurement
}

void I2CA_Init(void)
{
    // Initialize I2C
    I2caRegs.I2CSAR = 0x08;         // Slave address - EEPROM control code              0x08
    I2caRegs.I2CPSC.all = 6;        // Prescaler - need 7-12 Mhz on module clk          8Mhz
    I2caRegs.I2CCLKL = 40;          // NOTE: must be non zero                           40
    I2caRegs.I2CCLKH = 40;          // NOTE: must be non zero                           40  Moet 95.238kHz tot gevolg h�..
    I2caRegs.I2CIER.all = 0x24;     // Enable SCD & ARDY interrupts     0x24
    I2caRegs.I2CMDR.all = 0x0020;   // Take I2C out of reset
    I2caRegs.I2CFFTX.all = 0x6000;  // Enable FIFO mode and TXFIFO
    I2caRegs.I2CFFRX.all = 0x2040;  // Enable RXFIFO, clear RXFFINT,
}

void CANTransmit(Uint16 Destination, Uint32 TxDataH, Uint32 TxDataL, Uint16 Bytes)      //destination, txdataH, txdataL,  bytes
{
    ECanaRegs.CANME.all = 0x00000006;                   // Disable Tx Mailbox

    ECanaMboxes.MBOX0.MSGCTRL.all = Bytes;              // Transmit 4 bytes of data

    ECanaMboxes.MBOX0.MSGID.all = 0;                    // Standard ID length, acceptance masks used, no remote frames
    ECanaMboxes.MBOX0.MSGID.bit.STDMSGID = Destination; // Load destination address

    ECanaMboxes.MBOX0.MDL.all = TxDataL;
    ECanaMboxes.MBOX0.MDH.all = TxDataH;

    ECanaRegs.CANME.all = 0x00000007;                   // Enable Tx Mailbox

    ECanaRegs.CANTRS.all = 0x00000001;                  // Set transmit request
}

Uint16 I2CA_WriteData(unsigned char Register, unsigned char Data)
{
    unsigned char DataBuffer[4];

    while(I2caRegs.I2CMDR.bit.STP == 1);

    // Setup slave address
    I2caRegs.I2CSAR = 0x08;

    // Check if bus busy
    while(I2caRegs.I2CSTR.bit.BB == 1);

    CurrentMsgPtr = &I2cMsgOut1;

    // Setup number of bytes to send
    I2caRegs.I2CCNT = 3;

    DataBuffer[0] = (0x08) << 1;
    DataBuffer[1] = Register;
    DataBuffer[2] = Data;
    DataBuffer[3] = CRC8(DataBuffer, 3, 7); //bereken crc

    // Setup data to send
    I2caRegs.I2CDXR = Register;
    I2caRegs.I2CDXR = Data;
    I2caRegs.I2CDXR = DataBuffer[3];                            //send crc

    // Send start as master transmitter
    I2caRegs.I2CMDR.all = 0x6E20;

    I2cMsgOut1.MsgStatus = I2C_MSGSTAT_WRITE_BUSY;

    //while(CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_WRITE_BUSY);

    return I2C_SUCCESS;
}// end of write section

Uint16 I2CA_ReadData(struct I2CMSG *msg, unsigned char Register, Uint16 amount)
{
    while(I2caRegs.I2CMDR.bit.STP == 1);

    // Check if bus busy
    while(I2caRegs.I2CSTR.bit.BB == 1);

    // Setup slave address
    I2caRegs.I2CSAR = 0x08;

    // Setup number of bytes to send
    I2caRegs.I2CCNT = 1;

    // Setup data to send
    I2caRegs.I2CDXR = Register;

    I2caRegs.I2CMDR.all = 0x2E20;

    CurrentMsgPtr = &I2cMsgIn1;
    I2cMsgIn1.MsgStatus = I2C_MSGSTAT_SEND_NOSTOP_BUSY;

    while(CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_SEND_NOSTOP_BUSY);

    if(msg->MsgStatus == I2C_MSGSTAT_RESTART)
    {
        while(I2caRegs.I2CMDR.bit.STP == 1);

        // Check if bus busy
        while(I2caRegs.I2CSTR.bit.BB == 1);

        // Setup slave address
        I2caRegs.I2CSAR = 0x08;

        I2caRegs.I2CCNT = 2;                    // Setup how many bytes to expect
        I2caRegs.I2CMDR.all = 0x2C20;           // Send restart as master receiver

        // Update current message pointer and message status
        CurrentMsgPtr->MsgStatus = I2C_MSGSTAT_READ_BUSY;

        while(CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_READ_BUSY);

        Received = DataOut;

        if(amount == 2)
        {
            Received = Received<<8;

            while(I2caRegs.I2CMDR.bit.STP == 1);

            // Setup slave address
            I2caRegs.I2CSAR = 0x08;

            I2caRegs.I2CCNT = 2;                    // Setup how many bytes to expect
            I2caRegs.I2CMDR.all = 0x2C20;           // Send restart as master receiver

            // Update current message pointer and message status
            CurrentMsgPtr->MsgStatus = I2C_MSGSTAT_READ_BUSY;

            while(CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_READ_BUSY);
            Received = Received | DataOut;
        }
        return Received;
    }
    return I2C_SUCCESS;
}
void Shut_D_BQ(void)                        //bq turn off sequence
{
    I2CA_WriteData(0x04,0x00);
    I2CA_WriteData(0x04,0x02);
    I2CA_WriteData(0x04,0x01);
    I2CA_WriteData(0x04,0x02);
}

void CANSlaveReception(void)
{
    Uint32 RxData = 0;
    union bits32 TxData;

    RxData = ECanaMboxes.MBOX1.MDL.all;             // Data taken out of direct mailbox

    switch (RxData)
    {
    case 1: {TxData.asFloat=Voltages[0]; CANTransmit(0, 1, TxData.asUint,5); break;}
    case 2: {TxData.asFloat=Voltages[1]; CANTransmit(0, 2, TxData.asUint,5); break;}
    case 3: {TxData.asFloat=Voltages[2]; CANTransmit(0, 3, TxData.asUint,5); break;}
    case 4: {TxData.asFloat=Voltages[3]; CANTransmit(0, 4, TxData.asUint,5); break;}
    case 5: {TxData.asFloat=Voltages[4]; CANTransmit(0, 5, TxData.asUint,5); break;}
    case 6: {TxData.asFloat=Voltages[5]; CANTransmit(0, 6, TxData.asUint,5); break;}
    case 7: {TxData.asFloat=Voltages[6]; CANTransmit(0, 7, TxData.asUint,5); break;}
    case 8: {TxData.asFloat=Voltages[7]; CANTransmit(0, 8, TxData.asUint,5); break;}
    case 9: {TxData.asFloat=Voltages[8]; CANTransmit(0, 9, TxData.asUint,5); break;}
    case 10: {TxData.asFloat=Voltages[9]; CANTransmit(0, 10, TxData.asUint,5); break;}
    case 11: {TxData.asFloat=Voltages[10]; CANTransmit(0, 11, TxData.asUint,5); break;}
    case 12: {TxData.asFloat=Voltages[11]; CANTransmit(0, 12, TxData.asUint,5); break;}
    case 13: {TxData.asFloat=Voltages[12]; CANTransmit(0, 13, TxData.asUint,5); break;}
    case 14: {TxData.asFloat=Voltages[13]; CANTransmit(0, 14, TxData.asUint,5); break;}
    case 15: {TxData.asFloat=Voltages[14]; CANTransmit(0, 15, TxData.asUint,5); break;}

    case 16: {TxData.asFloat=Current; CANTransmit(0, 16, TxData.asUint,5); break;}

    case 17: {TxData.asFloat=Temperatures[0]; CANTransmit(0, 17, TxData.asUint,5); break;}
    case 18: {TxData.asFloat=Temperatures[1]; CANTransmit(0, 18, TxData.asUint,5); break;}
    case 19: {TxData.asFloat=Temperatures[2]; CANTransmit(0, 19, TxData.asUint, 5); break;}

    case 20: {TxData.asFloat=test_current; CANTransmit(0, 20, TxData.asUint, 5); break;}
    }
}
void  Bq76940_Init(void)
{
    Uint16 temp;
    Uint16 temp2;
    int OV;
    int UV;
    Uint16 Reset;

    BQEnable = 1;                                           //turn on BQ chip
    while(counter_2Hz != 1);                                      //toets delay

    Reset = I2CA_ReadData(&I2cMsgIn1,0x00, 1);

    if(Reset != 00)
        I2CA_WriteData(0x00,(char)Reset);

    I2CA_WriteData(0x04,0x18);                                  //Sit metings aan

    I2CA_WriteData(0x01,0x00);
    I2CA_WriteData(0x02,0x00);
    I2CA_WriteData(0x03,0x00);

    I2CA_WriteData(0x08,0x08);                                  //protect 3

    I2CA_WriteData(0x0B,0x19);

    I2CA_WriteData(0x05,0x03);                                  //turn on outputs (CHG+DSG)       sit miskien eers op 'n latere stadium dit aan? ?

    //lees adc gain en offset
    temp = I2CA_ReadData(&I2cMsgIn1,0x50, 1);

    temp2 = I2CA_ReadData(&I2cMsgIn1,0x59, 1);

    temp2 = temp2 >> 5;
    temp2= temp2 & 0x07;

    temp = temp<<1;
    temp = temp & 0x0C;

    temp = temp2 | temp;

    ADCgain = ((float)temp + 365)* 0.000001;

    ADCoffset = ((I2CA_ReadData(&I2cMsgIn1,0x51, 1))) * 0.001;

    //Over voltage = 3.7 V
    OV = (3.7-ADCoffset)/ADCgain;
    OV = (OV>>4) & 0xFF;

    I2CA_WriteData(0x09, (char)OV);                             //Stel OV_trip op

    //Under voltage = 2.5 V
    UV = (2.5-ADCoffset)/ADCgain;
    UV = (UV>>4) & 0xFF;

    I2CA_WriteData(0x0A,(char)UV);                              //Stel UV_trip op

    BQEnable = 0;                                               //pull low to allow BQ to measure temp
}

void  Read_Cell_Voltages(void)
{
    // Read data from EEPROM section //
    int i;
    Voltage_total = 0;
    Voltage_high = 0;                      //reset values
    Voltage_low = 10;

    float temp_V = 0;

    for(i = 0; i<15; i++)
    {
        //V[i] = I2CA_ReadData(&I2cMsgIn1,0x0C+(i*0x02), 2);

        temp_V = I2CA_ReadData(&I2cMsgIn1,0x0C+(i*0x02), 2);

        temp_V = (ADCgain * temp_V) + ADCoffset;

        Voltages[i] = temp_V;

        Voltage_total = Voltage_total +  Voltages[i];

        if(Voltage_high<Voltages[i])
            Voltage_high = Voltages[i];

        if(Voltage_low>Voltages[i])
            Voltage_low = Voltages[i];
    }
}

void Toggle_LED(void)
{
    GpioDataRegs.GPATOGGLE.bit.GPIO5 = 1;
}

void Process_Voltages(void)
{
    if(Voltage_high > 3.6)         //3.65
    {
        balance = 1;            //start balancing
        flagCharged = 1;        //charged flag to to stop charging
    }

    if(Voltage_low < 2.8 && Voltage_low > 2.6)
    {
        flagDischarged = 1;
        led3 = 1;               //turn on red led
        ContactorOut = 0;       //turn off contactor            //turn off output
    }
    else if(Voltage_low < 2.6)
    {
        flagDischarged = 2;
    }

    if(Voltage_high<3.35 )
        flagCharged = 0;

    if(Voltage_low>2.8 )
        flagDischarged = 0;
}

void Calculate_Current(void)
{
    Current = (test_current-2085)* 0.125;                   //2035    maal, moenie deel nie!!!!     0.0982--200/2048
}

void Read_System_Status(void)
{
    system_status = I2CA_ReadData(&I2cMsgIn1,0x00, 1);

}

void Process_System_Status(void)
{
    if(system_status != 00)
    {
        I2CA_WriteData(0x00, 0x3F);
    }
}

void Read_Temp()
{
    int i;
    int flag = 0;
    float Vts;
    float Rts;

    float temp_T = 0;

    for(i = 0; i<3; i++)
    {
        temp_T = I2CA_ReadData(&I2cMsgIn1, 0x2C+(i*0x02), 2);
        //      test_blah[i] = T[i];

        if(i == 0)
            Vts = (temp_T*ADCgain) + 0.27;
        else
            Vts = temp_T*ADCgain;

        //test1 = Vts;
        Rts = (10000*Vts)/(3.3-Vts);
        //test2 = Rts;

        Temperatures[i] = (1/((log(Rts/10000))/4000+0.003356))-273;
        //  T[i] = T[i] -273;

        if(Temperatures[i]> 70 || Temperatures[i]<0)
        {
            flag = 1;
        }
    }

    if(flag == 1)
        flagTemp = 1;
    else if(flag == 0)
        flagTemp = 0;
}

void Balance(int period, float reference)
{
    static float count = 0;
    float Cell_B_Voltage = reference;
    int i;

    if(balance == 1)
    {
        if(count ==0)       // 4 siklusse met onewe selle
        {
            //reset all balancing
            I2CA_WriteData(0x01,0x00);
            I2CA_WriteData(0x02,0x00);
            I2CA_WriteData(0x03,0x00);
            Cell_B1 = 0;
            Cell_B2 = 0;
            Cell_B3 = 0;

            //setup balancing
            for(i = 0; i<5; i+=2)
            {
                if(Voltages[i] > Cell_B_Voltage)
                {
                    Cell_B1 = (Cell_B1 | (0x01 << i));
                }
            }

            for(i = 5; i<10; i+=2)
            {
                if(Voltages[i] > Cell_B_Voltage)
                {

                    Cell_B2 = (Cell_B2 | (0x01 << (i-5)));
                }
            }

            for(i = 10; i<15; i+=2)
            {
                if(Voltages[i] > Cell_B_Voltage)
                {

                    Cell_B3 = (Cell_B3 | (0x01 << (i-10)));
                }
            }

            if(Cell_B1==0x0 && Cell_B2==0x0 && Cell_B3==0x0)
            {
                count = period;
            }
            else
            {
                I2CA_WriteData(0x01,Cell_B1);
                I2CA_WriteData(0x02,Cell_B2);
                I2CA_WriteData(0x03,Cell_B3);
                count++;
            }

        }
        else if(count ==period) // 4 siklusse met ewe selle
        {
            //reset all balancing
            I2CA_WriteData(0x01,0x00);
            I2CA_WriteData(0x02,0x00);
            I2CA_WriteData(0x03,0x00);
            Cell_B1 = 0;
            Cell_B2 = 0;
            Cell_B3 = 0;

            //setup balancing
            for(i = 1; i<5; i+=2)
            {
                if(Voltages[i] > Cell_B_Voltage)
                {
                    Cell_B1 = (Cell_B1 | (0x01 << i));
                }
            }

            for(i = 6; i<10; i+=2)
            {
                if(Voltages[i] > Cell_B_Voltage)
                {

                    Cell_B2 = (Cell_B2 | (0x01 << (i-5)));
                }
            }

            for(i = 11; i<15; i+=2)
            {
                if(Voltages[i] > Cell_B_Voltage)
                {

                    Cell_B3 = (Cell_B3 | (0x01 << (i-10)));
                }
            }

            if(Cell_B1==0x0 && Cell_B2==0x0 && Cell_B3==0x0)
            {
                count = period*2;
            }
            else
            {
                I2CA_WriteData(0x01,Cell_B1);
                I2CA_WriteData(0x02,Cell_B2);
                I2CA_WriteData(0x03,Cell_B3);
                count++;
            }
        }
        else if(count == period*2)
        {
            balance = 0;

            for(i = 0; i<15; i++)
            {
                if(Voltages[i] > Cell_B_Voltage)
                {
                    balance++;
                }
            }

            if(balance !=0)
            {
                balance = 1;
            }
            else if(balance ==0)                //sit miskien else hier             hierdie is toets fase
            {
                balance = 0;
            }

            count = 0;
            I2CA_WriteData(0x01,0x00);
            I2CA_WriteData(0x02,0x00);
            I2CA_WriteData(0x03,0x00);
            Cell_B1 = 0;
            Cell_B2 = 0;
            Cell_B3 = 0;
        }
        else
        {
            count++;
        }
    }
}



unsigned char CRC8(unsigned char *ptr, unsigned char len,unsigned char key)
{
    unsigned char i;
    unsigned char crc=0;
    while(len--!=0)
    {
        for(i=0x80; i!=0; i/=2)
        {
            if((crc & 0x80) != 0)
            {
                crc *= 2;
                crc ^= key;
            }
            else
                crc *= 2;

            if((*ptr & i)!=0)
                crc ^= key;
        }
        ptr++;
    }
    return(crc);
}

void configADC(void)
{
    EALLOW;
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;    //ADCINT1 trips after AdcResults latch
    AdcRegs.INTSEL1N2.bit.INT1E     = 1;    //Enabled ADCINT1
    AdcRegs.INTSEL1N2.bit.INT1CONT  = 0;    //Disable ADCINT1 Continuous mode
    AdcRegs.INTSEL1N2.bit.INT1SEL   = 1;    //setup EOC1 to trigger ADCINT1 to fire
    AdcRegs.ADCSOC0CTL.bit.CHSEL    = 0x0B; //0     set SOC0 channel select to ADCINB3(dummy sample for rev0 errata workaround)
    AdcRegs.ADCSOC1CTL.bit.CHSEL    = 0x0B; //0     set SOC1 channel select to ADCINB3
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL  = 0x03; //  CPU Timer 0  // hierdie was 2
    AdcRegs.ADCSOC1CTL.bit.TRIGSEL  = 0x03; //
    AdcRegs.ADCSOC0CTL.bit.ACQPS    = 20;   //set SOC0 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)
    AdcRegs.ADCSOC1CTL.bit.ACQPS    = 20;   //set SOC1 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)
    EDIS;
}

void CANSetup(void)
{
    EALLOW;

    // Configure CAN GPIO pins
    GpioCtrlRegs.GPAPUD.bit.GPIO30 = 0;             // Enable pull-up for GPIO30 (CANRXA)
    GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0;             // Enable pull-up for GPIO31 (CANTXA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO30 = 3;           // Asynch qual for GPIO30 (CANRXA)
    GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1;            // Configure GPIO30 for CANRXA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1;            // Configure GPIO31 for CANTXA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;            // Set the enable signal to the CAN transciever as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;             // Set the enable signal to the CAN transciever as output
    GpioDataRegs.GPADAT.bit.GPIO27 = 1;             // Set the enable signal to the CAN transciever high

    // Configure eCAN RX and TX pins
    struct ECAN_REGS ECanaShadow;
    ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;
    ECanaShadow.CANTIOC.bit.TXFUNC = 1;
    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;
    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;
    ECanaShadow.CANRIOC.bit.RXFUNC = 1;
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;

    // Configure Master Control register
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.SCB = 0;                  //  Martin Rademeyer //
    ECanaShadow.CANMC.bit.DBO = 1;                  //  Martin Rademeyer //     1 bartho
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

    // Initialise Message Control registers
    ECanaMboxes.MBOX0.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX1.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX2.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX3.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX4.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX5.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX6.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX7.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX8.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX9.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX10.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX11.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX12.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX13.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX14.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX15.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX16.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX17.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX18.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX19.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX20.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX21.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX22.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX23.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX24.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX25.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX26.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX27.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX28.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX29.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX30.MSGCTRL.all = 0x00000000;
    ECanaMboxes.MBOX31.MSGCTRL.all = 0x00000000;

    // Clear flag registers
    ECanaRegs.CANTA.all = 0xFFFFFFFF;
    ECanaRegs.CANRMP.all = 0xFFFFFFFF;
    ECanaRegs.CANGIF0.all = 0xFFFFFFFF;
    ECanaRegs.CANGIF1.all = 0xFFFFFFFF;

    // Configure bit-timing
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 1 ;
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    do {ECanaShadow.CANES.all = ECanaRegs.CANES.all;}   // Wait until the CPU has been granted permission to change the configuration registers
    while (ECanaShadow.CANES.bit.CCE != 1 );            // Wait for CCE bit to be set..
    ECanaShadow.CANBTC.all = 0;
    ECanaShadow.CANBTC.bit.BRPREG = 3;
    //  ECanaShadow.CANBTC.bit.BRPREG = 15;                 //  Martin Rademeyer //
    //  ECanaShadow.CANBTC.bit.BRPREG = 1;                  //  Martin Rademeyer //
    ECanaShadow.CANBTC.bit.TSEG2REG = 2;                //  Martin Rademeyer //
    ECanaShadow.CANBTC.bit.TSEG1REG = 10;               //  Martin Rademeyer //
    ECanaShadow.CANBTC.bit.SAM = 0;                     //1
    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;

    // Finalise CAN configuration
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 0 ;
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
    do {ECanaShadow.CANES.all = ECanaRegs.CANES.all;}   // Wait until the CPU no longer has permission to change the configuration registers
    while(ECanaShadow.CANES.bit.CCE != 0 );             // Wait for CCE bit to be  cleared..
    ECanaRegs.CANME.all = 0;                            // Disable all Mailboxes - Required before writing the MSGIDs

    EDIS;
}

void CANMailboxConfig(void)
{
    ECanaRegs.CANGAM.all = 0x00000000;              // Global All-Pass Mask (Don't care: 1, Match: 0)

    ECanaRegs.CANMD.all = 0x00000006;               // Message Direction (Rx: 1, Tx: 0)                                 //bartho    0x00000002

    // Tx Mailbox (0x00000001)
    ECanaMboxes.MBOX0.MSGCTRL.all = 0x00000004; // Transmit 4 bytes of data

    // Rx Mailbox (0x00000002)
    ECanaMboxes.MBOX1.MSGID.all = 0;                // Standard ID length, acceptance masks used, no remote frames
    ECanaMboxes.MBOX1.MSGID.bit.STDMSGID = NodeID;  // Current address loaded
    ECanaLAMRegs.LAM1.all = 0x00000000;             // Accept standard IDs with matching address

    ECanaMboxes.MBOX1.MSGCTRL.all = 0x00000004; // Receive 4 bytes of data

    // Rx Mailbox (0x00000003)
    ECanaMboxes.MBOX2.MSGID.all = 0;                // Standard ID length, acceptance masks used, no remote frames      //bartho
    ECanaMboxes.MBOX2.MSGID.bit.STDMSGID = 0x0611;  // Current address loaded                                           //bartho
    ECanaLAMRegs.LAM2.all = 0x00000000;             // Accept standard IDs with matching address                        //bartho

    ECanaMboxes.MBOX2.MSGCTRL.all = 0x00000008; // Receive 8 bytes of data                                              //bartho

    ECanaMboxes.MBOX3.MSGCTRL.all = 0x00000008; // Transmit 8 bytes of data

    ECanaRegs.CANME.all = 0x00000006;               // Enable Rx Mailbox                                                //bartho    0x00000002

    // The Tx Mailbox MSGID has to be set as required and then enabled
}

void CANInterruptConfig(void)
{
    DINT;                                           // Global interrupt disable

    EALLOW;
    ECanaRegs.CANGIM.all = 0x00000003;              // Enable ECAN0INT and ECAN0INT interrupt lines
    ECanaRegs.CANMIM.all = 0x0000000F;              // Allow interrupts for Mailbox 0 and 1 , 2,3
    ECanaRegs.CANMIL.all = 0x00000009;              // Mailbox 0 triggers ECAN1INT line, Mailbox 1 triggers ECAN0INT line       //1 bartho
    PieVectTable.ECAN0INTA = &can_rx_isr;           // Link Rx ISR function
    PieVectTable.ECAN1INTA = &can_tx_isr;           // Link Tx ISR function
    EDIS;

    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;              // Enable Rx interrupt in PIE group 9
    PieCtrlRegs.PIEIER9.bit.INTx6 = 1;              // Enable Tx interrupt in PIE group 9

    IER |= M_INT9;                                  // Enable group 9 interrupts

    EINT;                                           // Global interrupt enable
}

void CANChargerReception(void)
{
    Uint32 RxDataL = 0;
    Uint32 RxDataH = 0;
    float ChgVoltage = 0;
    float ChgCurrent = 0;
    Uint16 ChgStatus = 0;
    Uint16 temp = 0;
    Uint16 temp2 = 0;


    static volatile float Current_max = 28;
    //  float Vreference = 52;

    static volatile int delay = 0;  // miskien >> count 1 cycle from contactor closes till charger starts
    //   >> count 1 cycle from charger stops till contactor opens

    RxDataL = ECanaMboxes.MBOX2.MDL.all;                // Data taken out of direct mailbox
    RxDataH = ECanaMboxes.MBOX2.MDH.all;                // Data taken out of direct mailbox

    //Read Charger Voltage
    temp = RxDataL;
    temp2 = (temp & 0xFF) << 8;
    temp2 = ((temp &0xFF00)>>8) | temp2;
    ChgVoltage = (float)temp2*0.1;

    //Read Charger Current
    temp = (RxDataL& 0xFFFF0000)>>16;
    temp2    = (temp & 0xFF) << 8;
    temp2 = ((temp &0xFF00)>>8) | temp2;
    ChgCurrent = (float)temp2*0.1;

    //Read Charger Status
    ChgStatus = RxDataH & 0xFF;

    if(ChgStatus == 0)                                                                  //Charger ready to charge. No flags set
    {
        if(flagCurrent == 0 && flagTemp == 0 && flagCharged == 0 && KeySwitch == 0)     //check flags to ensure charging is allowed   haal flagvoltage uit
        {
            if(delay == 0)                                                              //sit miskien check in om met die charger Vbat te meet
            {
                ContactorOut = 1;                                                       //turn on contactor
                delay++;
            }
            else if(delay == 1)
            {
                //calculate maximum allowed voltage and possibly current
                //- make function with PI control loop
                if(Voltage_high < 3.55)
                {
                    //current_reference = 52.5;
                    CANTransmit(0x618, 0, ChgCalculator(54, Current_max), 8);               //charging started.. maybe reveiw..
                }
                else
                {
                    //                  current_reference = ChgCurrent + (3.55-Vhigh)*5;                                   //ChgVoltage + error*Kp (1) Proposionele beheerder
                    //                  PI = 1;
                    if(ChgCurrent>4 )                                                       //sit stroom check in om charger te stop blah blah , sit charged flag = 1 /balance
                        CANTransmit(0x618, 0, ChgCalculator(54, Current_max-=2), 8);        //charging started.. maybe reveiw..
                    else
                    {
                        CANTransmit(0x618, 1, ChgCalculator(54, 18), 8);                    //charging stops
                        Current_max = 28;
                        flagCharged = 1;                                                    //status charged
                        balance = 1;                                                        //balance
                    }
                }

                flagDischarged = 0;
            }
        }
        else
        {
            if(delay == 1)                                                              //sit miskien check in om met die charger Vbat te meet
            {
                CANTransmit(0x618,1,ChgCalculator(54, 0),8);                            //disconnect charger
                delay--;
            }
            else if(delay == 0)
            {
                ContactorOut = 0;                                                       //turn off contactor
                CANTransmit(0x618,1,ChgCalculator(54, 0),8);                            //disconnect charger
            }
        }
    }
    else                                                                                //Charger flag set.
    {
        if(delay == 1)
        {
            CANTransmit(0x618,1,ChgCalculator(54, 0),8);                                //disconnect charger
            delay--;
        }
        else if(delay == 0)
        {
            ContactorOut = 0;                                                           //turn off contactor
            CANTransmit(0x618,1,ChgCalculator(54, 0),8);                                //disconnect charger
        }
    }

    Charger_status = ChgStatus;
    toets = ChgVoltage;
    toets2 = ChgCurrent;
}

Uint32 ChgCalculator(float Voltage, float Current)
{
    Uint32 answer= 0;
    Uint16 temp1 = 0;

    Voltage = Voltage*10;
    Current = Current*10;

    temp1 = (Uint16)Current;

    answer = answer | (temp1&0xFF);
    answer = (answer<<8) | (temp1>>8);

    temp1 = (Uint16)Voltage;

    answer = answer<<8 | (temp1&0xFF);
    answer = (answer<<8) | (temp1>>8);

    return answer;
}

__interrupt void  adc_isr(void)
{
    //gee aandag hieraan. doen 'n stroom meting conversion elke teen 1 Hz soos spanning
    //Sit dit dalk deur 'n laag deurlaat filter y(k) = y(k - 1) + a[x(k) - y(k - 1)] met a = 1 - e^WcTs

    //  float measure_C;
    static float current_p;
    //  int toets_stroom = 0;

    //test_current = AdcResult.ADCRESULT1;

    //current_ADC = current_ADC_p + (test_current - current_ADC_p)/2;

    //contactor oop:
    /*  if(ContactorOut == 0)
    {
        Current = 0;
    }
    else    //contactor toe
    {*/
    //Current = (measure_C-1820)* 0.131;                    //2035    maal, moenie deel nie!!!!     0.0982--200/2048

    //  Current = measure_C;

    test_current = current_p + (0.00314*(AdcResult.ADCRESULT1-current_p));     //   0.00314-1Hz     //  0.01249 - 4 Hz      //0.27-100Hz
    current_p=test_current;

    if(AdcResult.ADCRESULT1 > 3500 || AdcResult.ADCRESULT1 < 700)                       ////////////////////////////////////////////////
    {
        //sit uittree af
        ContactorOut = 0;       //turn off contactor
        flagCurrent = 1;
    }                                                               ////////////////////////////////////////////////

    //}
    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;       //Clear ADCINT1 flag reinitialize for next SOC
    PieCtrlRegs.PIEACK.bit.ACK10 = 1;   // Acknowledge interrupt to PIE

    led2 = 0x01^led2;                   //toggle led
}

__interrupt void cpu_timer0_isr(void)
{
    counter_2Hz++;
    CpuTimer0.InterruptCount++;
    // Acknowledge this interrupt to receive more interrupts from group 1
    PieCtrlRegs.PIEACK.bit.ACK1 = 1/* PIEACK_GROUP1*/;
}

__interrupt void cpu_timer1_isr(void)
{
    //check status of all flags as well as the key switch

    counter_50Hz++;

    if(KeySwitch == 1)  //keyswitch == 1
    {
        //  led3 = 1;       //turn on red led

        //binne die keydrive if
        if((flagDischarged == 0) && (flagCurrent == 0)  &&(flagTemp == 0))
        {

            ContactorOut = 1;           //turn on contactor
        }

        //////////////////////////////////////////////
        /*
        if(KeyDrive == 0)   //keydrive == 0
        {


        }
        else if(KeyDrive == 1)  //keydrive == 1
        {
            ContactorOut = 0;       //turn off contactor

            //led3 = 0;     //turn off red led
        }*/
        ////////////////////////////////////////////

    }
    else if(KeySwitch == 0) //keyswitch == 0
    {
        flagCurrent = 0;
        //      ContactorOut = 0;       //turn off contactor

        /*  if((flagDischarged == 0) || (flagCurrent == 0)  ||(flagTemp == 0))
        {

            ContactorOut = 0;           //turn on contactor
        }*/

        //  led3 = 0;       //turn off red led
    }

    CpuTimer1.InterruptCount++;
    // The CPU acknowledges the interrupt.
    EDIS;
}

__interrupt void cpu_timer2_isr(void)
{
    EALLOW;

    CpuTimer2.InterruptCount++;
    // The CPU acknowledges the interrupt.
    EDIS;
}

__interrupt void i2c_int1a_isr(void)     // I2C-A
{
    Uint16 IntSource;

    // Read interrupt source
    IntSource = I2caRegs.I2CISRC.all;

    // Interrupt source = stop condition detected
    if(IntSource == I2C_SCD_ISRC)
    {
        // If completed message was writing data, reset msg to inactive state
        if (CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_WRITE_BUSY)
        {
            CurrentMsgPtr->MsgStatus = I2C_MSGSTAT_INACTIVE;
            //I2cMsgIn1.MsgStatus = I2C_MSGSTAT_SEND_NOSTOP;
        }
        else
        {
            // If a message receives a NACK during the address setup portion of the
            // EEPROM read, the code further below included in the register access ready
            // interrupt source code will generate a stop condition. After the stop
            // condition is received (here), set the message status to try again.
            // User may want to limit the number of retries before generating an error.
            if(CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_SEND_NOSTOP_BUSY)
            {
                CurrentMsgPtr->MsgStatus = I2C_MSGSTAT_RESTART;
            }
            // If completed message was reading EEPROM data, reset msg to inactive state
            // and read data from FIFO.
            else if (CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_READ_BUSY)
            {
                CurrentMsgPtr->MsgStatus = I2C_MSGSTAT_INACTIVE;

                DataOut = I2caRegs.I2CDRR;
                DataOut2 = I2caRegs.I2CDRR;
            }
        }
    }  // end of stop condition detected
    else if(IntSource == 2)
    {
        // Generate some error due to invalid interrupt source
        __asm("   ESTOP0");
    }
    else
    {
        // Generate some error due to invalid interrupt source
        __asm("   ESTOP0");
    }
    // Enable future I2C (PIE Group 8) interrupts
    PieCtrlRegs.PIEACK.bit.ACK8 = 1;
}

__interrupt void can_rx_isr(void)
{
    if (ECanaRegs.CANRMP.all == 0x00000002)
    {
        CANSlaveReception();                        // Handle the received message
    }
    else if (ECanaRegs.CANRMP.all == 0x00000004)
    {

        CANChargerReception();
    }

    ECanaRegs.CANRMP.all = 0xFFFFFFFF;          // Reset receive mailbox flags

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;         // Acknowledge this interrupt to receive more interrupts from group 9
}

__interrupt void can_tx_isr(void)
{
    /*if (ECanaRegs.CANTA.all == 0x00000001)
    {
        ECanaRegs.CANTA.all = 0xFFFFFFFF;           // Reset tranmission flags
    }*/


    ECanaRegs.CANTA.all = 0xFFFFFFFF;           // Reset tranmission flags

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;         // Acknowledge this interrupt to receive more interrupts from group 9
}