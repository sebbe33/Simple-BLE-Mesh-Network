/*

Copyright (c) 2013 RedBearLab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/*********************************************************************
* INCLUDES
*/

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"

#include "gatt.h"

#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
//#include "devinfoservice.h"

#include "peripheralObserverProfile.h"

#include "gapbondmgr.h"

#include "biscuit.h"
#include "mesh_service.h"
#include "npi.h"

#include "i2c.h"
#include "eeprom.h"
#include "string.h"

#include "print_uart.h"
#include "mesh_transport_network_protocol.h"

#include "applications.h"
#include "relay_switch_application.h"
#include "dimmer_application.h"
#include "advertising_queue.h"
/*********************************************************************
* MACROS
*/

/*********************************************************************
* CONSTANTS
*/
#define APPLICATIONS_LENGTH     2

//#define IS_SERVER 
#define DEBUG_PRINT

#define MESH_IDENTIFIER         0xBC
#define MESH_MESSAGE_FLAG_OFFSET        4
#define NODE_NAME_MAX_SIZE      20
#define NETWORK_NAME_MAX_SIZE   20
  
#define NETWORK_ID_ADR		1
#define NODE_ID_ADR		5
#define NETWORK_NAME_ADR	9
#define NODE_NAME_ADR		9 + NETWORK_NAME_MAX_SIZE

// Uncomment this to burn default values into persistent memory
// #define BURN_DEFAULTS
#define DEFAULT_NODE_NAME               "Dimmer node"
#define DEFAULT_NODE_ID                 131
#define DEFAULT_NETWORK_NAME            "BT Mesh Network"
#define DEFAULT_NETWORK_ID              999

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD                   2500

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL 70
#define DEFAULT_ADVERTISING_IN_CONNECTION_INTERVAL 165

#define FORWARDING_INTERVAL 90
#define FORWARDING_IN_CONNECTION_INTERVAL 300

// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     16

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     16

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

#define INVALID_CONNHANDLE                    0xFFFF

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15        

#define MAX_RX_LEN                            30
#define SBP_RX_TIME_OUT                       5

#define DEFAULT_SCAN_DURATION                 100

// Discovey mode (limited, general, all)
#define DEFAULT_DISCOVERY_MODE                DEVDISC_MODE_ALL

// TRUE to use active scan
#define DEFAULT_DISCOVERY_ACTIVE_SCAN         FALSE

// TRUE to use white list during discovery
#define DEFAULT_DISCOVERY_WHITE_LIST          FALSE

// Maximum number of scan responses
#define DEFAULT_MAX_SCAN_RES                  8

#define DEFAULT                               1
#define FORWARD                               2

#define ADV_PERIOD_INACTIVE                   5000
#define ADV_PERIOD                            180
#define ADV_PERIOD_EAGER                      500

#define QUEUE_SIZE 5
/*********************************************************************
* TYPEDEFS
*/

/*********************************************************************
* GLOBAL VARIABLES
*/

/*********************************************************************
* EXTERNAL VARIABLES
*/

/*********************************************************************
* EXTERNAL FUNCTIONS
*/ 

/*********************************************************************
* LOCAL VARIABLES
*/
static uint8 biscuit_TaskID;   // Task ID for internal task/event processing
static gaprole_States_t gapProfileState = GAPROLE_INIT;
static uint8 isObserving = FALSE;
static uint8 isAdvertisingPeriodically = TRUE;
static Application applications[APPLICATIONS_LENGTH];
static uint8 advertisingQueue[QUEUE_SIZE][32];
static uint8 queueIndex = 0;
static uint8 isProcessingQueue = FALSE;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // Tx power level
  //0x02,   // length of this data
  //GAP_ADTYPE_POWER_LEVEL,
  //0,       // 0dBm
  
  // service UUID, to notify central devices what services are included
  // in this peripheral
  17,   // length of this data
  GAP_ADTYPE_128BIT_COMPLETE,      // some of the UUID's, but not all
  MESH_SERV_UUID,
  
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[31] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
  
  3,
  MESH_IDENTIFIER, 0, 0,
  
  // complete name 
  20,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
};


// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "EHMARINE";
uint8 isForwarding = FALSE;

uint8 count = 0;
/*********************************************************************
* LOCAL FUNCTIONS
*/
static void biscuit_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );
static void performPeriodicTask( void );
static void meshServiceChangeCB( uint8 paramID );
static void simpleBLEObserverEventCB( observerRoleEvent_t *pEvent );
static void advertiseCallback(uint8* data, uint8 length, uint16 delay);
static void messageCallback(uint16 source, uint8* data, uint8 length);
static void dataHandler( uint8 port, uint8 events );
static void processClientMessage(uint8* data, uint8 length);
static void applicationClientResponseCallback(uint8* data, uint8 length);
static void UARTWriteWrapper(uint8* data, uint8 length);
/*********************************************************************
* PROFILE CALLBACKS
*/

// GAP Role Callbacks
static gapRolesCBs_t biscuit_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL,                            // When a valid RSSI is read from controller (not used by application)
  simpleBLEObserverEventCB
};

// GAP Bond Manager Callbacks
static gapBondCBs_t biscuit_BondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};

// Simple GATT Profile Callbacks
static meshServiceCBs_t biscuit_MESHServiceCBs =
{
  meshServiceChangeCB    // Charactersitic value change callback
};
/*********************************************************************
* PUBLIC FUNCTIONS
*/

/*********************************************************************
* @fn      SimpleBLEPeripheral_Init
*
* @brief   Initialization function for the Simple BLE Peripheral App Task.
*          This is called during initialization and should contain
*          any application specific initialization (ie. hardware
*          initialization/setup, table initialization, power up
*          notificaiton ... ).         
*
* @param   task_id - the ID assigned by OSAL.  This ID should be
*                    used to send messages and set timers.
*
* @return  none
*/
void Biscuit_Init( uint8 task_id )
{
  biscuit_TaskID = task_id;
  
  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
  // Setup the GAP Peripheral Role Profile
  {
    // Device starts advertising upon initialization
    uint8 initial_advertising_enable = FALSE;
    
    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;
    
    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;
    
    // Set the GAP Role Parametersuint8 initial_advertising_enable = TRUE;
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );
    
    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    
    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }
  
  i2c_init();
  
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );
  
#ifdef BURN_DEFAULTS
  uint8 networkName[20] = DEFAULT_NETWORK_NAME;
  uint8 nodeName[20] = DEFAULT_NODE_NAME;
  eeprom_write_long(NETWORK_ID_ADR, DEFAULT_NETWORK_ID);
  eeprom_write_long(NODE_ID_ADR, DEFAULT_NODE_ID);
  eeprom_write_bytes(NETWORK_NAME_ADR, networkName, sizeof(networkName));
  eeprom_write_bytes(NODE_NAME_ADR, nodeName, sizeof(nodeName));
#else
  uint16 networkID = (uint16) eeprom_read_long(NETWORK_ID_ADR);
  // Write network ID to advertising data
  *((uint16*) &advertData[5]) = networkID;
  uint16 nodeID = (uint16) eeprom_read_long(NODE_ID_ADR);
  eeprom_read_bytes(NETWORK_NAME_ADR, &advertData[9], 20);
  initializeMeshConnectionProtocol(networkID,nodeID,&advertiseCallback, 
                                   &messageCallback, &osal_GetSystemClock);
#endif
  
  GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
  
  
  // Setup observer related GAP profile properties
  {
    uint8 scanRes = DEFAULT_MAX_SCAN_RES;
    GAPRole_SetParameter(GAPOBSERVERROLE_MAX_SCAN_RES, sizeof( uint8 ), &scanRes);
  }
  
  // Set advertising interval
  {
    uint16 interval = DEFAULT_ADVERTISING_INTERVAL;
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, interval );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, interval );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, interval );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, interval );
    interval = DEFAULT_ADVERTISING_IN_CONNECTION_INTERVAL;
    GAP_SetParamValue( TGAP_CONN_ADV_INT_MIN, interval );
    GAP_SetParamValue( TGAP_CONN_ADV_INT_MAX, interval );
  }
  
  {
    GAP_SetParamValue( TGAP_GEN_DISC_SCAN, DEFAULT_SCAN_DURATION );
    GAP_SetParamValue( TGAP_LIM_DISC_SCAN, DEFAULT_SCAN_DURATION );
    GAP_SetParamValue( TGAP_GEN_DISC_SCAN_WIND, 30 );
    GAP_SetParamValue( TGAP_GEN_DISC_SCAN_INT,  35);
  }
  
  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }
  
  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  //DevInfo_AddService();                           // Device Information Service
  MESH_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile
  
  // Register callback with MESHService
  VOID MESH_RegisterAppCBs( &biscuit_MESHServiceCBs );
  
  P0SEL = 0; // Configure Port 0 as GPIO
  P1SEL = 0; // Configure Port 1 as GPIO
  P2SEL = 0; // Configure Port 2 as GPIO
  
  P0DIR = 0xFF;
  P1DIR = 0xFF; 
  P2DIR = 0x1F; 

  P0 = 0x80; 
  P1 = 0;   // All pins on port 1 to low
  P2 = 0;   // All pins on port 2 to low
  
  // Initialize serial interface
  P1SEL = 0x30;
  P1DIR |= 0x02;
  P1_1 = 1;
  PERCFG |= 1;
  NPI_InitTransport(dataHandler);
  
  //Set baudrate
  {
    U0GCR &= 0xE0;      // Default baudrate 57600
    U0GCR |= 0x0A;
    U0BAUD = 216;
  }
  
  //Set txPower
  HCI_EXT_SetTxPowerCmd( HCI_EXT_TX_POWER_0_DBM );
  
  // Initialze applications
  initializeRelaySwitchApp(applicationClientResponseCallback, sendStatelessMessage);
  applications[0].code = RELAY_SWITCH_APPLICATION_CODE;
  applications[0].fun = processIcomingMessageRelaySwitch;
  
  initializeDimmerApp(applicationClientResponseCallback, sendStatelessMessage, 
                      UARTWriteWrapper);
  applications[1].code = DIMMER_APPLICATION_CODE;
  applications[1].fun = processIncomingMessageDimmer;
  
  // Setup a delayed profile startup
  osal_set_event( biscuit_TaskID, SBP_START_DEVICE_EVT );
}

/*********************************************************************
* @fn      Biscuit_ProcessEvent
*
* @brief   Simple BLE Peripheral Application Task event processor.  This function
*          is called to process all events for the task.  Events
*          include timers, messages and any other user defined events.
*
* @param   task_id  - The OSAL assigned task ID.
* @param   events - events to process.  This is a bit map and can
*                   contain more than one event.
*
* @return  events not processed
*/
uint16 Biscuit_ProcessEvent( uint8 task_id, uint16 events )
{
  
  VOID task_id; // OSAL required parameter that isn't used in this function
  
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;
    
    if ( (pMsg = osal_msg_receive( biscuit_TaskID )) != NULL )
    {
      biscuit_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );
      
      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }
    
    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
  
  if ( events & SBP_ADV_IN_CONNECTION_EVT )
  {
    uint8 turnOnAdv = TRUE;
    // Turn on advertising while in a connection
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );
    
    return (events ^ SBP_ADV_IN_CONNECTION_EVT);
  }
  
  if ( events & SBP_START_DEVICE_EVT )
  {
    // Start the Device
    VOID GAPRole_StartDevice( &biscuit_PeripheralCBs);
    
    // Start Bond Manager
    VOID GAPBondMgr_Register( &biscuit_BondMgrCBs );
    
    // Set timer for first ic event
    osal_start_timerEx( biscuit_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );
    
    //Start observing
    osal_start_timerEx(biscuit_TaskID, SBP_START_OBSERVING, 25);
    
#ifndef IS_SERVER
    // Start periodic advertisement
    osal_start_timerEx( biscuit_TaskID, SBP_START_ADV_PERIOD, ADV_PERIOD_EAGER);
#endif
    
    return ( events ^ SBP_START_DEVICE_EVT );
  }
  
  if ( events & SBP_PERIODIC_EVT )
  {
    // Restart timer
    if ( SBP_PERIODIC_EVT_PERIOD )
    {
      osal_start_timerEx( biscuit_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );
    }
    
    // Perform periodic application task
    performPeriodicTask();
    
    return (events ^ SBP_PERIODIC_EVT);
  }
  
  if (events & SBP_FORWARDING_DONE_EVENT) 
  {
    if(isProcessingQueue == TRUE) {
      // There are items in the queue, call the process queue event
      // to process the queue
      osal_set_event(biscuit_TaskID, SBP_PROCESS_QUEUE_EVENT);
    } else {
      isForwarding = FALSE;
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &isForwarding);   
    } 
  }
  
  if(events & SBP_START_OBSERVING)
  {
    if(isObserving == 0){
      // Start observing again
      GAPObserverRole_StartDiscovery( DEFAULT_DISCOVERY_MODE,
                                     DEFAULT_DISCOVERY_ACTIVE_SCAN,
                                     DEFAULT_DISCOVERY_WHITE_LIST ); 
    }    
    isObserving = 1;
  }
  
  if((events & SBP_START_ADV_PERIOD) && isAdvertisingPeriodically == TRUE)
  {
    if(isForwarding == FALSE){
      // Set back the default advertising data and advertising period
      GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
      
      // Turn on advertisements
      uint8 dummy = TRUE;
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &dummy);  
      osal_start_timerEx( biscuit_TaskID, SBP_STOP_ADV_PERIOD, ADV_PERIOD );
    } else{
      osal_start_timerEx( biscuit_TaskID, SBP_START_ADV_PERIOD, ADV_PERIOD_EAGER );
    }
  }
  
  if((events & SBP_STOP_ADV_PERIOD) && isAdvertisingPeriodically == TRUE)
  {
    if(isForwarding == FALSE){
      // Turn on advertisements
      uint8 dummy = FALSE;
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &dummy);  
    }     
    
    osal_start_timerEx( biscuit_TaskID, SBP_START_ADV_PERIOD, ADV_PERIOD_INACTIVE );
    
  }
  
  if(events & SBP_PROCESS_QUEUE_EVENT)
  {
    isForwarding = TRUE;
    
    // Set advertise data to the first message in the queue
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, advertisingQueue[0][31]+4, advertisingQueue[0]);
    
    //Start advertising
    uint8 dummy = TRUE;
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &dummy );
    
    if(queueIndex > 0) {
      // Decrement all in queue
      for(uint8 i = 0; i < queueIndex; i++) {
        memcpy(advertisingQueue[i], advertisingQueue[i+1], 31);
      }
      queueIndex--;
      osal_start_timerEx( biscuit_TaskID, SBP_PROCESS_QUEUE_EVENT, 
                         isAdvertisingPeriodically == TRUE? FORWARDING_INTERVAL : FORWARDING_IN_CONNECTION_INTERVAL );
    } else {
      isForwarding = FALSE;
      isProcessingQueue = FALSE;
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &isForwarding);  
    }
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
* @fn      biscuit_ProcessOSALMsg
*
* @brief   Process an incoming task message.
*
* @param   pMsg - message to process
*
* @return  none
*/
static void biscuit_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
    
  default:
    // do nothing
    break;
  }
}


/*********************************************************************
* @fn      peripheralStateNotificationCB
*
* @brief   Notification from the profile of a state change.
*
* @param   newState - new state
*
* @return  none
*/
static void peripheralStateNotificationCB( gaprole_States_t newState )
{  
  switch ( newState )
  {
  case GAPROLE_STARTED:
    {
      /*uint8 ownAddress[B_ADDR_LEN];
      uint8 systemId[DEVINFO_SYSTEM_ID_LEN];
      
      GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);
      
      // use 6 bytes of device address for 8 bytes of system ID value
      systemId[0] = ownAddress[0];
      systemId[1] = ownAddress[1];
      systemId[2] = ownAddress[2];
      
      // set middle bytes to zero
      systemId[4] = 0x00;
      systemId[3] = 0x00;
      
      // shift three bytes up
      systemId[7] = ownAddress[5];
      systemId[6] = ownAddress[4];
      systemId[5] = ownAddress[3];
      
      DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);*/
    }
    break;
    
  case GAPROLE_ADVERTISING:
    {
#ifdef DEBUG_PRINT
      debugPrintLine("GAPROLE_ADVERTISING");
#endif
    }
    break;
    
  case GAPROLE_CONNECTED:
    { 
      if(isAdvertisingPeriodically == TRUE) {
        P0_7 = 0; // Turn on blue led to indicate connection
        // If we're advertising periodically, turn it off while in connection
        isAdvertisingPeriodically = FALSE;
        osal_stop_timerEx(biscuit_TaskID, SBP_START_ADV_PERIOD);
        osal_stop_timerEx(biscuit_TaskID, SBP_STOP_ADV_PERIOD);
      }
#ifdef DEBUG_PRINT    
      debugPrintLine("GAPROLE_CONNECTED");
#endif
    }
    break;
    
  case GAPROLE_CONNECTED_ADV:
    {
#ifdef DEBUG_PRINT
      debugPrintLine("GAPROLE_CONNECTED_ADV");
#endif
    }
    break;      
  case GAPROLE_WAITING:
    {      
#ifdef DEBUG_PRINT
      debugPrintLine("GAPROLE_WAITING"); 
#endif
      if(isAdvertisingPeriodically == FALSE) {
        P0_7 = 1; // Turn off blue led to indicate connection
        // Restart periodic advertisement after disconnection
        osal_stop_timerEx(biscuit_TaskID, SBP_START_ADV_PERIOD);
        osal_stop_timerEx(biscuit_TaskID, SBP_STOP_ADV_PERIOD);
        osal_start_timerEx(biscuit_TaskID, SBP_START_ADV_PERIOD, ADV_PERIOD_EAGER);
        isAdvertisingPeriodically = TRUE;
      }
    }
    break;
    
  case GAPROLE_WAITING_AFTER_TIMEOUT:
    {
#ifdef DEBUG_PRINT
      debugPrintLine("GAPROLE_WAITING_AFTER_TIMEOUT");
#endif
    }
    break;
    
  case GAPROLE_ERROR:
    {
#ifdef DEBUG_PRINT
      debugPrintLine("GAPROLE ERROR");
#endif
    }
    break;
  }
  
  gapProfileState = newState;
}

/*********************************************************************
* @fn      simpleBLEObserverEventCB
*
* @brief   Observer event callback function.
*
* @param   pEvent - pointer to event structure
*
* @return  none
*/

static void simpleBLEObserverEventCB( observerRoleEvent_t *pEvent )
{
  switch ( pEvent->gap.opcode )
  {
  case GAP_DEVICE_INIT_DONE_EVENT:  
    {
    }
    break;
    
  case GAP_DEVICE_INFO_EVENT:
    {
      uint8* data = pEvent->deviceInfo.pEvtData;
      uint8  dataLen = pEvent->deviceInfo.dataLen;
      processIncomingMessage(&data[MESH_MESSAGE_FLAG_OFFSET], dataLen-MESH_MESSAGE_FLAG_OFFSET);
    }
    break;
    
  case GAP_DEVICE_DISCOVERY_EVENT:
    {
      if(isObserving == 1){
        GAPObserverRole_StartDiscovery( DEFAULT_DISCOVERY_MODE,
                                       DEFAULT_DISCOVERY_ACTIVE_SCAN,
                                       DEFAULT_DISCOVERY_WHITE_LIST );
      }
    }
    break;
  }
}



/*********************************************************************
* @fn      performPeriodicTask
*
* @brief   Perform a periodic application task. This function gets
*          called every five seconds as a result of the SBP_PERIODIC_EVT
*          OSAL event. In this example, the value of the third
*          characteristic in the SimpleGATTProfile service is retrieved
*          from the profile, and then copied into the value of the
*          the fourth characteristic.
*
* @param   none
*
* @return  none
*/
uint8 hejsan = 0;
static void performPeriodicTask( void )
{
  periodicTask();
}

/*********************************************************************
* @fn      meshServiceChangeCB
*
* @brief   Callback from SimpleBLEProfile indicating a value change
*
* @param   paramID - parameter ID of the value that was changed.
*
* @return  none
*/
static void meshServiceChangeCB( uint8 paramID )
{
  uint8 data[26];
  uint8 len = paramID;
  
  if (paramID == MESSAGE_READY)
  {
    
    MESH_GetParameter(RX_MESSAGE_CHAR, &len, data);
    uint8 length = data[0];
    processClientMessage(data, length); 
  }
  else if (paramID == MESH_RX_NOTI_ENABLED)
  {
    GAPRole_SendUpdateParam( DEFAULT_DESIRED_MAX_CONN_INTERVAL, DEFAULT_DESIRED_MIN_CONN_INTERVAL,
                            DEFAULT_DESIRED_SLAVE_LATENCY, DEFAULT_DESIRED_CONN_TIMEOUT, GAPROLE_RESEND_PARAM_UPDATE );
  }
  else if (paramID == JOIN_GROUP_SET)
  {
    uint16 newGroup;
    MESH_GetParameter(JOIN_GROUP_CHAR, &len, &newGroup);
    joinGroup(newGroup);
    
  }
  else if (paramID == LEAVE_GROUP_SET)
  {
    uint16 oldGroup;
    MESH_GetParameter(LEAVE_GROUP_CHAR, &len, &oldGroup);
    leaveGroup(oldGroup);
    
  }
  else if (paramID == DEV_NAME_CHANGED)
  {
    // Write new node name to persistent memory
    MESH_GetParameter(DEV_NAME_CHAR, &len, data);
    if(len > NODE_NAME_MAX_SIZE) {
      len = NODE_NAME_MAX_SIZE;
    }
    eeprom_write_bytes(NODE_NAME_ADR, data, len);
  }
  else if (paramID == NETWORK_SET)
  {
    MESH_GetParameter(NETWORK_CHAR, &len, data);
    if(len > NETWORK_NAME_MAX_SIZE) {
        len = NETWORK_NAME_MAX_SIZE;
    }
    eeprom_write_bytes(NETWORK_NAME_ADR, data, len);
  }
}

static uint8 rxBuffer[30];
static uint8 rxBufferCurrentIndex = 0;
static uint8 bytesToReceiveOnRx = 0;
/*********************************************************************
* @fn      dataHandler
*
* @brief   Callback from UART indicating a data coming
*
* @param   port - data port.
*
* @param   events - type of data.
*
* @return  none
*/
static void dataHandler( uint8 port, uint8 events )
{  
  if((events & HAL_UART_RX_TIMEOUT) == HAL_UART_RX_TIMEOUT)
  {
    uint8 len = NPI_RxBufLen();
    if(len > 30) {
      len = 30;
    }
    uint8 buf[30];
    NPI_ReadTransport( buf, len );
    uint8 i = 0;
    
    // if new message
    if(bytesToReceiveOnRx == 0) {
      // disregard nonsence data and loop to the indicator
      while(i < len && buf[i] != 0xE6) {
          i++;
      }

      if(i == len) {
        // Invalid message
        return;
      } else if (buf[i] == 0xE6) {
        // Indicator has been received. Check length byte:
        if(i+2 < len) {
          // Length byte and first message byte received
          bytesToReceiveOnRx = buf[i+1];
          // Set i to point at first content byte
          i += 2; 
        } else if(i+1 < len) {
          // Only length byte received
          bytesToReceiveOnRx = buf[i+1];
          return;
        } else {
          // no length byte received. Indicate that it shall be received next time
          bytesToReceiveOnRx = 0xFF;
          return;
        }
      } 
    } else if(bytesToReceiveOnRx == 0xFF) {
        // about to receive the length of the message
        bytesToReceiveOnRx = buf[0];
        // set i to point at the first content byte
        i++;
    }

    if(rxBufferCurrentIndex + (len - i) < bytesToReceiveOnRx) {
      // if a partial message has been received -> copy local buffer to RXBuffer
      memcpy(&rxBuffer[rxBufferCurrentIndex], &buf[i], len - i);
      rxBufferCurrentIndex += len - i;
    } else if (0 < rxBufferCurrentIndex) {
      // if the rest of the partial message has been received -> copy and send it
      memcpy(&rxBuffer[rxBufferCurrentIndex], &buf[i], len - i);
      processClientMessage(rxBuffer, rxBuffer[0]);
      rxBufferCurrentIndex = 0;
      bytesToReceiveOnRx = 0;
    } else {
      // If the whole messaged could be processed directly
      processClientMessage(&buf[i], buf[i]);
      rxBufferCurrentIndex = 0;
      bytesToReceiveOnRx = 0;
    }
    
  }
}

/*********************************************************************
*********************************************************************/

static void processClientMessage(uint8* data, uint8 length) 
{
    uint8 type = data[1];
    uint16 dest = *((uint16*) &data[2]);
    uint8* message = &data[4];
    switch(type)
    {
    case BROADCAST:
      {
        message = &data[2];
        broadcastMessage(message, length);
        break;
      }
    case GROUP_BROADCAST:
      {
        broadcastGroupMessage(dest, message, length);
        break;
      }
    case STATELESS_MESSAGE:
      {
        sendStatelessMessage(dest, message, length);
        break;
      }
    case STATEFUL_MESSAGE:
      {
        sendStatefulMessage(dest, message, length);
        break;
      }
    }		
}

static void advertiseCallback(uint8* data, uint8 length, uint16 delay)
{
  if(queueIndex == QUEUE_SIZE) {
      // disregard, queue is full
      return;
  }
  
  advertisingQueue[queueIndex][0] = 0x02;
  advertisingQueue[queueIndex][1] = GAP_ADTYPE_FLAGS;
  advertisingQueue[queueIndex][2] = DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED;
  advertisingQueue[queueIndex][3] = 27;
    
  // If is already forwarding
  if(queueIndex > 0 || isForwarding == TRUE) {
    // Add message to queue
    osal_memcpy(&advertisingQueue[queueIndex][4], data, length);
    // save the length of the message in the last byte of the array
    advertisingQueue[queueIndex][31] = length;
    queueIndex++;
    
    isProcessingQueue = TRUE;
    return;
  }
  
  // If there is no mesg in queue advertise directly
  isForwarding = TRUE;
  isObserving = 0;
  GAPObserverRole_StopDiscovery();

  osal_memcpy(&advertisingQueue[queueIndex][MESH_MESSAGE_FLAG_OFFSET], data, length);
   
  GAPRole_SetParameter( GAPROLE_ADVERT_DATA, length+MESH_MESSAGE_FLAG_OFFSET, advertisingQueue[queueIndex]);
  //Start advertising
  uint8 dummy = TRUE;
  GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &dummy );
  
  // Start forwarding done event timer
  osal_start_timerEx(biscuit_TaskID, SBP_FORWARDING_DONE_EVENT, 
                     isAdvertisingPeriodically == TRUE? FORWARDING_INTERVAL : FORWARDING_IN_CONNECTION_INTERVAL);
  
  // Start delayed observing
  osal_start_timerEx(biscuit_TaskID, SBP_START_OBSERVING, 15);
#ifdef DEBUG_PRINT
  debugPrintLine("Forw");
#endif
}

static void messageCallback(uint16 source, uint8* data, uint8 length)
{
#ifdef DEBUG_PRINT
  debugPrintLine("Got message");
#endif
  for(uint8 i = 0; i < sizeof(applications); i++) {
    if(applications[i].code == data[0]) {
      applications[i].fun(source, &data[1], length - 1);
      break;
    }
  }
  
}

/**
  * An adapter function for applications to write to the GATT server.
  * Calls the appropriate characteristic with the data delivered from the app.
  */
static void applicationClientResponseCallback(uint8* data, uint8 length) 
{
#ifdef IS_SERVER
  HalUARTWrite(NPI_UART_PORT, data, length); 
  uint8 token[2] = "\r\n";
  HalUARTWrite(NPI_UART_PORT, token, 2); 
#else
    debugPrintLine("set tx"); 
    MESH_SetParameter(TX_MESSAGE_CHAR,length, data);
#endif
}

static void UARTWriteWrapper(uint8* data, uint8 length) 
{
  HalUARTWrite(NPI_UART_PORT, data, length); 
}
