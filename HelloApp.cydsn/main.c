/*******************************************************************************
* File Name: main.c
*
* Version: 1.30
*
* Description:
*  BLE HID keyboard example project that supports both input and output reports
*  in boot and protocol mode. The example also demonstrates handling suspend 
*  event from the central device and enters low power mode when suspended.
*  This functionality is implemented based on APIs that are in bootloader 
*  project and shared with bootloadable.
*
* References:
*  BLUETOOTH SPECIFICATION Version 4.1
*  HID Usage Tables spec ver 1.12
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2014-2015, Cypress Semiconductor Corporation.  All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include "main.h"

volatile uint32 mainTimer = 0u;

/*******************************************************************************
* Function Name: main()
********************************************************************************
*
* Summary:
*   This is main() function. It configures BLE component and starts it.
*   Also it tracks trigger event (button press) and switches to bootloader
*   application.
*
*******************************************************************************/
int main()
{
    CYBLE_LP_MODE_T lpMode;
    CYBLE_BLESS_STATE_T blessState;
 
#if !defined(__ARMCC_VERSION)
    InitializeBootloaderSRAM();
#endif

    PrintProjectHeader();
    AART_Start();
    AART_UartPutString("HelloApp");
   
    ConfigureSharedPins();
   
    CyGlobalIntEnable;  

    /* Start CYBLE component and register generic event handler */
    CyBle_Start(AppCallBack);

    WDT_Start();

    while(1) 
    {           
        if(CyBle_GetState() != CYBLE_STATE_INITIALIZING)
        {
            /* Enter DeepSleep mode between connection intervals */
            lpMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
            CyGlobalIntDisable;
            blessState = CyBle_GetBleSsState();

            if(lpMode == CYBLE_BLESS_DEEPSLEEP) 
            {   
                if(blessState == CYBLE_BLESS_STATE_ECO_ON || blessState == CYBLE_BLESS_STATE_DEEPSLEEP)
                {
                    CySysPmDeepSleep();
                }
            }
            else
            {
                if(blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
                {
                    CySysPmSleep();
                }
            }
            CyGlobalIntEnable;
        }
        if((CyBle_GetState() == CYBLE_STATE_CONNECTED) && (suspend != CYBLE_HIDS_CP_SUSPEND))
        {
            if(mainTimer != 0u)
            {
                mainTimer = 0u;                
                
                SimulateBattery();
                CyBle_ProcessEvents();
                if(keyboardSimulation == ENABLED)
                {
                    SimulateKeyboard();
                }
            }
        }
        CyBle_ProcessEvents();
        
        BootloaderSwitch();
    }   
}


/*******************************************************************************
* Function Name: AppCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*  event - event code
*  eventParam - event parameters
*
*******************************************************************************/
void AppCallBack(uint32 event, void* eventParam)
{
    CYBLE_API_RESULT_T apiResult;
    CYBLE_GAP_BD_ADDR_T localAddr;
    
    switch (event)
    {
        /**********************************************************
        *                       General Events
        ***********************************************************/
        case CYBLE_EVT_STACK_ON: /* This event is received when the component is Started */
            /* Enter into discoverable mode so that remote can search it. */
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if(apiResult != CYBLE_ERROR_OK)
            {
            }
            localAddr.type = 0u;
            CyBle_GetDeviceAddress(&localAddr);
            break;
        case CYBLE_EVT_TIMEOUT: 
            break;
        case CYBLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
            break;
            
        /* This event will be triggered by host stack if BLE stack is busy or not busy.
         *  Parameter corresponding to this event will be the state of BLE stack.
         *  BLE stack busy = CYBLE_STACK_STATE_BUSY,
         *  BLE stack not busy = CYBLE_STACK_STATE_FREE 
         */
        case CYBLE_EVT_STACK_BUSY_STATUS:
            break;
        case CYBLE_EVT_HCI_STATUS:
            break;
            
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
        case CYBLE_EVT_GAP_AUTH_REQ:
            break;
        case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
            break;
        case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            break;
        case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
            break;
        case CYBLE_EVT_GAP_AUTH_COMPLETE:
            break;
        case CYBLE_EVT_GAP_AUTH_FAILED:
            break;
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
            {   
                /* Fast and slow advertising period complete, go to low power  
                 * mode (Hibernate mode) and wait for an external
                 * user event to wake up the device again */
                Advertising_LED_Write(LED_OFF);
                Disconnect_LED_Write(LED_OFF);
                CapsLock_LED_Write(LED_OFF);
                Bootloader_Service_Activation_ClearInterrupt();
                Wakeup_Interrupt_ClearPending();
                Wakeup_Interrupt_Start();
                CySysPmHibernate();
            }
            break;
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            Disconnect_LED_Write(LED_OFF);
            Advertising_LED_Write(LED_OFF);
            break;
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if(apiResult != CYBLE_ERROR_OK)
            {
            }
            Disconnect_LED_Write(LED_ON);
            break;
        case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
            { 
                uint16 mtu;
                CyBle_GattGetMtuSize(&mtu);
            }
            break;
        case CYBLE_EVT_GATTS_WRITE_REQ:
            (void)CyBle_GattsWriteRsp(((CYBLE_GATTS_WRITE_REQ_PARAM_T *)eventParam)->connHandle);
            break;
        case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
            break;
        case CYBLE_EVT_GAPC_CONNECTION_UPDATE_COMPLETE:
            break;
            
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CYBLE_EVT_GATT_CONNECT_IND:
            /* Register service specific callback functions */
            HidsInit();
            BasInit();
            ScpsInit();
            break;
        case CYBLE_EVT_GATT_DISCONNECT_IND:
            break;
            
        /**********************************************************
        *                       Other Events
        ***********************************************************/
        default:
            break;
    }

}


/* [] END OF FILE */
