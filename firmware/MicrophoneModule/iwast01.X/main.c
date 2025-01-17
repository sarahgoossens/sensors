/*  ____  ____      _    __  __  ____ ___
 * |  _ \|  _ \    / \  |  \/  |/ ___/ _ \
 * | | | | |_) |  / _ \ | |\/| | |  | | | |
 * | |_| |  _ <  / ___ \| |  | | |__| |_| |
 * |____/|_| \_\/_/   \_\_|  |_|\____\___/
 *                           research group
 *                             dramco.be/
 *
 *  KU Leuven - Technology Campus Gent,
 *  Gebroeders De Smetstraat 1,
 *  B-9000 Gent, Belgium
 *
 *         File: main.c
 *      Created: 2019-09-11
 *       Author: Geoffrey Ottoy
 *      Version: 0.2
 *
 *  Description: Generic I2C sensor interface for 
 *                  "IoT with a SOFT touch"
 *               (DRAMCO / KU Leuven TCG project)
 *
 */

#include "mcc_generated_files/mcc.h"
#include "global.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mcc_generated_files/pin_manager.h"

// include sensor
#include "Sensors/sound_level.h"

Device_API_t sensorAPI = SENSOR_API;

#ifndef TYPE_BYTE
    #error "TYPE_BYTE is not defined"
#elif (~(~TYPE_BYTE + 0) == 0 && ~(~TYPE_BYTE + 1) == 1)
    #error "TYPE_BYTE is defined, but has no value"
#endif

#ifndef M_NR
    #error "M_NR is not defined"
#elif (~(~M_NR + 0) == 0 && ~(~M_NR + 1) == 1)
    #error "M_NR is defined, but has no value"
#endif

#ifndef SLAVE_ADDRESS
    #error "SLAVE_ADDRESS is not defined"
#elif (~(~SLAVE_ADDRESS + 0) == 0 && ~(~SLAVE_ADDRESS + 1) == 1)
    #error "SLAVE_ADDRESS is defined, but has no value"
#endif

void toggleInt(void);

uint8_t measurementData[2 * M_NR];
uint8_t mDataLength;

/* Main application
 */
void main(void)
{
    // initialize the device
    SYSTEM_Initialize(SLAVE_ADDRESS);

    // When using interrupts, you need to set the Global and Peripheral Interrupt Enable bits
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    while(1)
    {
        // I2C Slave operation (respond to commands)
        if(I2C1_CommandReceived()){
            uint8_t cmd;
            uint8_t len;
            uint8_t data[20];
            I2C1_GetCommand(&cmd, data, &len);
            
            switch(cmd){
                // POLL
                case CMD_POLL:{ // master polls address -> respond with ack
                    uint8_t ack = DEFAULT_ACK;
                    I2C1_SetTransmitData(&ack, 1);
                    SLEEP();

                } break;
                
                // TYPE
                case CMD_DEVICE_TYPE:{ // master requests sensor type -> respond with type
                    uint8_t type = TYPE_BYTE;
                    I2C1_SetTransmitData(&type, 1);
                    SLEEP();
                } break;

                // NR OF METRICS
                case CMD_GET_M_NR:{ // master requests measurement data length -> respond with length
                    uint8_t mnr = M_NR;
                    I2C1_SetTransmitData(&mnr, 1);
                    SLEEP();
                } break;

                // MEASURE
                case CMD_START_MEASUREMENT:{ // master forces measurement -> respond with ack
                    uint8_t metric = data[0];
                    uint8_t ack = DEFAULT_ACK;
                    I2C1_SetTransmitData(&ack, 1);
                    sensorAPI.Measure(metric, measurementData, &mDataLength);
                    /*for (int i=0; i<600; i++){}*/
                } break;
                
                // GET DATA
                case CMD_GET_M_DATA:{ // master requests measurement data -> send data
                    I2C1_SetTransmitData(measurementData, mDataLength);
                    //for (int i=0; i<600; i++){}
                    SLEEP();     
                } break;
                
                // INT TOGGLE
                case CMD_INT_TOGGLE:{ //toggle the INT line      
                    uint8_t ack = 0xAA;
                    I2C1_SetTransmitData(&ack, 1); 
                    toggleInt();
                    SLEEP();
                } break;
                
                // DO NOTHING
                default:{
                } break;
            }
        }
    }
}

//----------------------Toggle the interrupt line -----------------------------
void toggleInt(){
    READY_SetLow();
    __delay_ms(1);                          
    READY_SetHigh();
}

/**
 End of File
*/
