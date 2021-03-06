/*******************************************************************************
* File Name: OTAMandatory.h
*
* Version 1.30
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project. They are mandatory for OTA functionality.
*
********************************************************************************
* Copyright 2014-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <cytypes.h>


#define BLE_PACKET_SIZE_MAX                 (144u)

#if defined(__ARMCC_VERSION)

extern unsigned long Image$$DATA$$ZI$$Limit;

__attribute__ ((section(".bootloaderruntype"), zero_init))
extern volatile uint32 CyReturnToBootloaddableAddress;

#endif /*__ARMCC_VERSION*/




/* Buffer for received data */
extern uint8 packetRX[BLE_PACKET_SIZE_MAX];
extern uint32 packetRXSize;
extern uint32 packetRXFlag;

extern uint8 packetTX[BLE_PACKET_SIZE_MAX];
extern uint32 packetTXSize;

/* [] END OF FILE */
