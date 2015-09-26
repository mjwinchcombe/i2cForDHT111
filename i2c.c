#include <xc.h>
#include <stdint.h>

#include "i2c.h"

#define I2C_ADDRESS 0x2a
#define I2C_COMMAND_READ_TEMP 0x10
#define I2C_COMMAND_READ_HUMIDITY 0x11

#define I2C_STATUS_MASK 0x3D

typedef enum {IDLE, WAIT_FOR_COMMAND, COMMAND_ID_SET, COMMAND_FINISHED} state_type;


uint8_t requestedCommand;
bool transferActive;
state_type state = IDLE;

/*
 * Default setup is for running on a 16F887, should be customisable. 
 */
void i2c_Init() {
    //TODO Setup the registeres for i2c slave operation.

    //Set clock and datalines to input
    TRISCbits.TRISC3 = 1;
    TRISCbits.TRISC4 = 1;

    //Set address 
    SSPADD = I2C_ADDRESS << 1;
    //Enable slave mode with interrupts
    SSPCON = 0x3e;

    //Enable interrupts
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIE1bits.SSPIE = 1;
}

void i2c_sendDataForRequestedCommand() {
    if (requestedCommand == I2C_COMMAND_READ_TEMP) {
        SSPBUF = 21;
    } else if (requestedCommand == I2C_COMMAND_READ_HUMIDITY) {
        SSPBUF = 88;
    } else {
        SSPBUF = 0xff;
    }
}

/*
 * Called by the interrupt handler when ever an SSP interrupt occurs.  This
 * works through a simple state machine which will collect the command ID 
 * and then cause the response to be transmitted once the data write request
 * has been received.  Functionality verified with i2cget.
 */
void i2c_DataReceived() {
    transferActive = true;
    
    if (SSPCONbits.SSPOV == 0) {
        uint8_t status = SSPSTAT & I2C_STATUS_MASK;
        uint8_t data = SSPBUF;
  
        //Return to idle if stopped
        if (SSPSTATbits.P) {
            state = IDLE;
            requestedCommand = 0;
            return;
        }
        
        switch (state) {
            
            case IDLE:
                if (status == 0x09) {
                    state = WAIT_FOR_COMMAND;
                }
                break;
            case WAIT_FOR_COMMAND:
                if (status == 0x29) {
                    requestedCommand = data;
                    state = COMMAND_ID_SET;
                }
                break;
            case COMMAND_ID_SET:
                if (status == 0x0d) {
                    i2c_sendDataForRequestedCommand();
                    state = COMMAND_FINISHED;
                }
                break;
        }
        
        SSPCONbits.CKP = 1;
        
    } else {
        uint8_t tmp = SSPBUF;
        SSPCONbits.SSPOV = 0;
    }
    
    
}

bool i2c_IsTransferActive() {
    return transferActive;
}