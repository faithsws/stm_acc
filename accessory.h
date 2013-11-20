#ifndef __ACCESSORY_H__
#define __ACCESSORY_H__

#include "usbh_core.h"
#include "usbh_stdreq.h"
#include "usb_bsp.h"
#include "usbh_ioreq.h"
#include "usbh_hcs.h"

#define USBH_ACC_SEND                 1
#define USBH_ACC_SENT                 2
#define USBH_ACC_RECV                   3
#define USBH_ACC_RECVED        4
#define USBH_ACC_ERROR_IN       5
#define USBH_ACC_ERROR_OUT      6


typedef struct _ACC_Process
{
  uint8_t              hc_num_in; 
  uint8_t              hc_num_out; 
  uint8_t              MSBulkOutEp;
  uint8_t              MSBulkInEp;
  uint16_t             MSBulkInEpSize;
  uint16_t             MSBulkOutEpSize;
  uint8_t              buff[USBH_MSC_MPS_SIZE];
  uint8_t              maxLun;
}
ACC_Machine_TypeDef; 

typedef enum {
  USBH_ACC_OK = 0,
  USBH_ACC_FAIL = 1,
  USBH_ACC_PHASE_ERROR = 2,
  USBH_ACC_BUSY = 3
}USBH_ACC_Status_TypeDef;


typedef struct _ACCXfer
{
uint8_t ACCState;
uint8_t ACCStateBkp;
uint8_t ACCStateCurrent;


uint8_t RWState;
uint8_t RWStateBkp;


uint8_t CmdStateMachine;
uint8_t* pRxTxBuff;
uint16_t DataLength;
uint8_t BOTXferErrorCount;
uint8_t ACCXferStatus;
} USBH_AccXfer_TypeDef;

typedef enum
{
  USBH_ACC_INIT_STATE = 0,                        
  USBH_ACC_USB_TRANSFERS,        
  USBH_ACC_DEFAULT_APPLI_STATE,  
  USBH_ACC_CTRL_ERROR_STATE,
  USBH_ACC_UNRECOVERED_STATE
}
ACCState;

typedef enum {
  CMD_ACC_UNINITIALIZED_STATE =0,
  CMD_ACC_SEND_STATE,
  CMD_ACC_WAIT_STATUS
} CMD_ACC_STATES_TypeDef;  

extern USBH_Class_cb_TypeDef  Accessory_cb;
extern ACC_Machine_TypeDef    ACC_Machine;

#endif