#include "AD9959.h"

uint8_t CSR_DATA0[1] = {0x10};                                               // Enable CH0
uint8_t CSR_DATA1[1] = {0x20};                                               // Enable CH1
uint8_t CSR_DATA2[1] = {0x40};                                               // Enable CH2
uint8_t CSR_DATA3[1] = {0x80};                                               // Enable CH3

uint8_t FR2_DATA[2]     = {0x00, 0x00};                                      // default Value = 0x0000
uint8_t CFR_DATA[3]     = {0x00, 0x03, 0x02};                                // default Value = 0x000302
uint8_t CPOW0_DATA[2]   = {0x00, 0x00};                                      // default Value = 0x0000   @ = POW/2^14*360
uint8_t LSRR_DATA[2]    = {0x00, 0x00};                                      // default Value = 0x----
uint8_t RDW_DATA[4]     = {0x00, 0x00, 0x00, 0x00};                          // default Value = 0x--------
uint8_t FDW_DATA[4]     = {0x00, 0x00, 0x00, 0x00};                          // default Value = 0x--------

uint32_t SinFre[4]      = {10000000, 10000000, 200000000, 40000};
uint32_t SinAmp[4]      = {9215, 9215, 9215, 9215};
uint32_t SinPhr[4]      = {0, 4095, 4095 * 3, 4095 * 2};

/**
  * @brief  Initializing IO port and reset device
  * @funna  Init_AD9959
  * @param  None
  * @retval None
  */
void Init_AD9959(void)
{
    uint8_t FR1_DATA[3] = {0xD3, 0x00, 0x00};                                  //20 frequency doubling
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);	                   //PD port clock enable

    GPIO_InitStructure.GPIO_Pin   = (0x4FFF << 0);                             //Initialize PD0~12
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    InitIO_9959();
    InitReset();

    WriteData_AD9959(FR1_ADD, 3, FR1_DATA, 1);

    Write_frequence(0, SinFre[0]);
    Write_frequence(1, SinFre[1]);
    Write_frequence(2, SinFre[2]);
    Write_frequence(3, SinFre[3]);

    Write_Phase(3, SinPhr[3]);
    Write_Phase(0, SinPhr[0]);
    Write_Phase(1, SinPhr[1]);
    Write_Phase(2, SinPhr[2]);

    Write_Amplitude(3, SinAmp[3]);
    Write_Amplitude(0, SinAmp[0]);
    Write_Amplitude(1, SinAmp[1]);
    Write_Amplitude(2, SinAmp[2]);

}

/**
  * @brief  dedicated delay for AD9959
  * @funna  delay_9959
  * @param  length
  * @retval None
  */
void delay_9959(uint32_t length)
{
    length = length * 12;
    while(length--);
}

/**
  * @brief  Initialization of IO port state
  * @funna  InitIO_9959
  * @param  None
  * @retval None
  */
void InitIO_9959(void)
{
    AD9959_PWR = 0;
    CS = 1;
    SCLK = 0;
    UPDATE = 0;
    PS0 = 0;
    PS1 = 0;
    PS2 = 0;
    PS3 = 0;
    SDIO0 = 0;
    SDIO1 = 0;
    SDIO2 = 0;
    SDIO3 = 0;
}

/**
  * @brief  Reset AD9959
  * @funna  InitReset
  * @param  None
  * @retval None
  */
void InitReset(void)
{
    RESET = 0;
    delay_9959(1);
    RESET = 1;
    delay_9959(30);
    RESET = 0;
}

/**
  * @brief  Upadta
  * @funna  IO_Update
  * @param  None
  * @retval None
  */
void IO_Update(void)
{
    UPDATE = 0;
    delay_9959(4);
    UPDATE = 1;
    delay_9959(4);
    UPDATE = 0;
}

/**
  * @brief  Read data
  * @funna  ReadData_9959
  * @param  RegisterAddress, NumberofRegisters, *RegisterData
  * @retval None
  */
void ReadData_9959(uint8_t RegisterAddress, uint8_t NumberofRegisters, uint8_t *RegisterData)
{
    uint8_t ControlValue = 0;
    uint8_t RegisterIndex = 0;
    uint8_t ReceiveData = 0;
    uint8_t i = 0;
    GPIO_InitTypeDef GPIO_InitStructure;
    //Create the 8-bit header
    ControlValue = RegisterAddress;
    SCLK = 1;
    delay_9959(0x20);
    CS = 1;
    delay_9959(0x20);
    CS = 0;
    delay_9959(0x20);


    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    delay_9959(0x20);
    //Write out the control word
    for(i = 0; i < 8; i++)
    {
        SCLK = 0;
        if(0x80 == (ControlValue & 0x80))
        {
            SDIO0 = 1;	        //Send one to SDIO pin
        }
        else
        {
            SDIO0 = 0;	        //Send zero to SDIO pin
        }
        delay_9959(0x20);
        SCLK = 1;
        delay_9959(0x20);
        ControlValue <<= 1;	    //Rotate data
    }

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    delay_9959(0x20);
    for (RegisterIndex = NumberofRegisters; RegisterIndex > 0; RegisterIndex--)
    {
        for(i = 0; i < 8; i++)
        {
            SCLK = 0;
            ReceiveData <<= 1;		//Rotate data
            if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3))
            {
                ReceiveData |= 1;
            }
            delay_9959(0x20);
            SCLK = 1;
            delay_9959(0x20);
        }
        *(RegisterData + RegisterIndex - 1) = ReceiveData;
    }
    delay_9959(0x20);
    CS = 1;	                        //bring CS high again
}

/**
  * @brief  Write data
  * @funna  WriteData_9959
  * @param  RegisterAddress, NumberofRegisters, *RegisterData, temp(1:update   0:update)
  * @retval None
  */
void WriteData_AD9959(uint8_t RegisterAddress, uint8_t NumberofRegisters, uint8_t *RegisterData, uint8_t temp)
{
    uint8_t  ControlValue = 0;
    uint8_t  ValueToWrite = 0;
    uint8_t  RegisterIndex = 0;
    uint8_t  i = 0;

    ControlValue = RegisterAddress;
    //Write address
    SCLK = 0;
    CS = 0;
    for(i = 0; i < 8; i++)
    {
        SCLK = 0;
        if(0x80 == (ControlValue & 0x80))
            SDIO0 = 1;
        else
            SDIO0 = 0;
        SCLK = 1;
        ControlValue <<= 1;
        delay_9959(2);
    }

    SCLK = 0;
    //Write data
    for (RegisterIndex = 0; RegisterIndex < NumberofRegisters; RegisterIndex++)
    {
        ValueToWrite = RegisterData[RegisterIndex];
        for (i = 0; i < 8; i++)
        {
            SCLK = 0;
            if(0x80 == (ValueToWrite & 0x80))
                SDIO0 = 1;
            else
                SDIO0 = 0;
            SCLK = 1;
            ValueToWrite <<= 1;
            delay_9959(2);
        }
        SCLK = 0;
    }
    if(temp == 1)
        IO_Update();
    CS = 1;
}

/**
  * @brief  Set up the frequency
  * @funna  Write_frequence
  * @param  channel, freq
  * @retval None
  */
void Write_frequence(uint8_t Channel, uint32_t Freq)
{
    uint8_t CFTW0_DATA[4] = {0x00, 0x00, 0x00, 0x00};
    uint32_t Temp;
    Temp = (uint32_t)Freq * 8.589934592;            //The input frequency factor is divided into four bytes.  4.294967296=(2^32)/500000000
    CFTW0_DATA[3] = (uint8_t)Temp;
    CFTW0_DATA[2] = (uint8_t)(Temp >> 8);
    CFTW0_DATA[1] = (uint8_t)(Temp >> 16);
    CFTW0_DATA[0] = (uint8_t)(Temp >> 24);
    if(Channel == 0)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA0, 0); //Control register writes to CH0 channel
        WriteData_AD9959(CFTW0_ADD, 4, CFTW0_DATA, 1); //CTW0 address 0x04.Output CH0 setting frequency
    }
    else if(Channel == 1)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA1, 0); //Control register writes to CH1 channel
        WriteData_AD9959(CFTW0_ADD, 4, CFTW0_DATA, 1); //CTW0 address 0x04.Output CH1 setting frequency
    }
    else if(Channel == 2)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA2, 0); //Control register writes to CH2 channel
        WriteData_AD9959(CFTW0_ADD, 4, CFTW0_DATA, 1); //CTW0 address 0x04.Output CH2 setting frequency
    }
    else if(Channel == 3)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA3, 0); //Control register writes to CH3 channel
        WriteData_AD9959(CFTW0_ADD, 4, CFTW0_DATA, 1); //CTW0 address 0x04.Output CH3 setting frequency
    }
}

/**
  * @brief  Set up the amplitude
  * @funna  Write_Amplitude
  * @param  channel, ampli
  * @retval None
  */
void Write_Amplitude(uint8_t Channel, uint16_t Ampli)
{
    uint16_t A_temp;                                //=0x23ff;
    uint8_t ACR_DATA[3] = {0x00, 0x00, 0x00};       //default Value = 0x--0000 Rest = 18.91/Iout

    A_temp = Ampli | 0x1000;
    ACR_DATA[2] = (uint8_t)A_temp;                  //Low bit data
    ACR_DATA[1] = (uint8_t)(A_temp >> 8);           //High bit data
    if(Channel == 0)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA0, 0);
        WriteData_AD9959(ACR_ADD, 3, ACR_DATA, 1);
    }
    else if(Channel == 1)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA1, 0);
        WriteData_AD9959(ACR_ADD, 3, ACR_DATA, 1);
    }
    else if(Channel == 2)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA2, 0);
        WriteData_AD9959(ACR_ADD, 3, ACR_DATA, 1);
    }
    else if(Channel == 3)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA3, 0);
        WriteData_AD9959(ACR_ADD, 3, ACR_DATA, 1);
    }
}

/**
  * @brief  Set up the Phase
  * @funna  Write_Phase
  * @param  channel, Phase
  * @retval None
  */
void Write_Phase(uint8_t Channel, uint16_t Phase)
{
    uint16_t P_temp = 0;
    P_temp = (uint16_t)Phase;
    CPOW0_DATA[1] = (uint8_t)P_temp;
    CPOW0_DATA[0] = (uint8_t)(P_temp >> 8);
    if(Channel == 0)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA0, 0);
        WriteData_AD9959(CPOW0_ADD, 2, CPOW0_DATA, 1);
    }
    else if(Channel == 1)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA1, 0);
        WriteData_AD9959(CPOW0_ADD, 2, CPOW0_DATA, 1);
    }
    else if(Channel == 2)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA2, 0);
        WriteData_AD9959(CPOW0_ADD, 2, CPOW0_DATA, 1);
    }
    else if(Channel == 3)
    {
        WriteData_AD9959(CSR_ADD, 1, CSR_DATA3, 0);
        WriteData_AD9959(CPOW0_ADD, 2, CPOW0_DATA, 1);
    }
}









