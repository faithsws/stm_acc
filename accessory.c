#include "accessory.h"

extern void print_string(char const * string);

__ALIGN_BEGIN ACC_Machine_TypeDef         ACC_Machine __ALIGN_END ;

USBH_AccXfer_TypeDef USBH_ACC_XferParam; 
extern  USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern  USBH_HOST                    USB_Host;


static USBH_Status InterfaceInit ( USB_OTG_CORE_HANDLE *pdev, void *phost)
{
    print_string(__func__);
    
    USBH_HOST *pphost = phost;
    if(pphost->device_prop.Ep_Desc[0][0].bEndpointAddress & 0x80)
    {
      ACC_Machine.MSBulkInEp = (pphost->device_prop.Ep_Desc[0][0].bEndpointAddress);
      ACC_Machine.MSBulkInEpSize  = pphost->device_prop.Ep_Desc[0][0].wMaxPacketSize;
    }
    else
    {
      ACC_Machine.MSBulkOutEp = (pphost->device_prop.Ep_Desc[0][0].bEndpointAddress);
      ACC_Machine.MSBulkOutEpSize  = pphost->device_prop.Ep_Desc[0] [0].wMaxPacketSize;      
    }
    
    if(pphost->device_prop.Ep_Desc[0][1].bEndpointAddress & 0x80)
    {
      ACC_Machine.MSBulkInEp = (pphost->device_prop.Ep_Desc[0][1].bEndpointAddress);
      ACC_Machine.MSBulkInEpSize  = pphost->device_prop.Ep_Desc[0][1].wMaxPacketSize;      
    }
    else
    {
      ACC_Machine.MSBulkOutEp = (pphost->device_prop.Ep_Desc[0][1].bEndpointAddress);
      ACC_Machine.MSBulkOutEpSize  = pphost->device_prop.Ep_Desc[0][1].wMaxPacketSize;      
    }
    
    ACC_Machine.hc_num_out = USBH_Alloc_Channel(pdev, 
                                                ACC_Machine.MSBulkOutEp);
    ACC_Machine.hc_num_in = USBH_Alloc_Channel(pdev,
                                                ACC_Machine.MSBulkInEp);  
    
    /* Open the new channels */
    USBH_Open_Channel  (pdev,
                        ACC_Machine.hc_num_out,
                        pphost->device_prop.address,
                        pphost->device_prop.speed,
                        EP_TYPE_BULK,
                        ACC_Machine.MSBulkOutEpSize);  
    
    USBH_Open_Channel  (pdev,
                        ACC_Machine.hc_num_in,
                        pphost->device_prop.address,
                        pphost->device_prop.speed,
                        EP_TYPE_BULK,
                        ACC_Machine.MSBulkInEpSize);    
    
    
    return USBH_OK ;
}
static void InterfaceDeInit ( USB_OTG_CORE_HANDLE *pdev,void *phost)
{
    print_string(__func__);
    if ( ACC_Machine.hc_num_out)
  {
    USB_OTG_HC_Halt(pdev, ACC_Machine.hc_num_out);
    USBH_Free_Channel  (pdev, ACC_Machine.hc_num_out);
    ACC_Machine.hc_num_out = 0;     /* Reset the Channel as Free */
  }
   
  if ( ACC_Machine.hc_num_in)
  {
    USB_OTG_HC_Halt(pdev, ACC_Machine.hc_num_in);
    USBH_Free_Channel  (pdev, ACC_Machine.hc_num_in);
    ACC_Machine.hc_num_in = 0;     /* Reset the Channel as Free */
  } 
}

static USBH_Status ClassRequest(USB_OTG_CORE_HANDLE *pdev , 
                                        void *phost)
{   
  
  USBH_Status status = USBH_OK ;
  print_string(__func__);  
  USBH_ACC_XferParam.ACCState = USBH_ACC_INIT_STATE;
  return status; 
}

void USBH_ACC_HandleXfer (USB_OTG_CORE_HANDLE *pdev ,USBH_HOST *phost)
{
  uint8_t xferDirection, index;
  static uint8_t error_direction;
  USBH_Status status;
  
  URB_STATE URB_Status = URB_IDLE;
  
  if(HCD_IsDeviceConnected(pdev))
  {  
    
    switch (USBH_ACC_XferParam.RWState)
    {
    case USBH_ACC_SEND: 
      USBH_BulkSendData (pdev,USBH_ACC_XferParam.pRxTxBuff,
                         USBH_ACC_XferParam.DataLength , 
                         ACC_Machine.hc_num_out);
      
      USBH_ACC_XferParam.RWState = USBH_ACC_SENT;
      USBH_ACC_XferParam.RWStateBkp = USBH_ACC_SEND;
      break;
      
    case USBH_ACC_SENT:
      URB_Status = HCD_GetURB_State(pdev , ACC_Machine.hc_num_out);
      
      if(URB_Status == URB_DONE)
      { 
            USBH_ACC_XferParam.ACCXferStatus = USBH_ACC_OK;
        
      }   
      else if(URB_Status == URB_NOTREADY)
      {
        USBH_ACC_XferParam.RWState  = USBH_ACC_XferParam.RWStateBkp;    
      }     
      else if(URB_Status == URB_STALL)
      {
        //error_direction = USBH_MSC_DIR_OUT;
        USBH_ACC_XferParam.RWState  = USBH_ACC_ERROR_OUT;
      }
      break;
      
    case USBH_ACC_RECV:
              //BOTStallErrorCount = 0;
        USBH_ACC_XferParam.RWStateBkp = USBH_ACC_RECV;    
        
       
        USBH_BulkReceiveData (pdev,USBH_ACC_XferParam.pRxTxBuff,USBH_ACC_XferParam.DataLength ,  
			     ACC_Machine.hc_num_in);
          
          
          /* If value was 0, and successful transfer, then change the state */
          USBH_ACC_XferParam.RWState = USBH_ACC_RECVED;
      
      break;   
      
      
    case USBH_ACC_RECVED:

      URB_Status = HCD_GetURB_State(pdev , ACC_Machine.hc_num_in);
      /* Decode CSW */
      if(URB_Status == URB_DONE)
      {
        //BOTStallErrorCount = 0;
        
        //USBH_ACC_XferParam.ACCState = USBH_ACC_XferParam.ACCStateCurrent ;
        
        USBH_ACC_XferParam.ACCXferStatus = USBH_ACC_OK;
      }
      else if(URB_Status == URB_STALL)     
      {
        //error_direction = USBH_MSC_DIR_IN;
        USBH_ACC_XferParam.RWState  = USBH_ACC_ERROR_IN;
      }
      break;
      
    case USBH_ACC_ERROR_IN: 
      
      break;
      
    case USBH_ACC_ERROR_OUT: 
      
      break;
      
    default:      
      break;
    }
  }
}



static USBH_Status MachineHandle(USB_OTG_CORE_HANDLE *pdev , void   *phost)
{
    //print_string(__func__);
    USBH_HOST *pphost = phost;
    
    USBH_Status status = USBH_BUSY;
    uint8_t accStatus = USBH_ACC_BUSY;   
    uint8_t appliStatus = 0;
    
    if(HCD_IsDeviceConnected(pdev))
    { 
        switch(USBH_ACC_XferParam.ACCState)
        {
        case USBH_ACC_INIT_STATE:
          USBH_ACC_XferParam.CmdStateMachine = CMD_ACC_SEND_STATE;  
          USBH_ACC_XferParam.ACCState = USBH_ACC_DEFAULT_APPLI_STATE;
          break;
          
        case USBH_ACC_USB_TRANSFERS:
          USBH_ACC_HandleXfer(pdev , phost);
          break;
          
        case USBH_ACC_DEFAULT_APPLI_STATE:
          
           appliStatus = pphost->usr_cb->USBH_USR_MSC_Application();
          if(appliStatus == 0)
          {
            USBH_ACC_XferParam.ACCState = USBH_ACC_DEFAULT_APPLI_STATE;
          }
          else if (appliStatus == 1) 
          {
            /* De-init requested from application layer */
            status =  USBH_APPLY_DEINIT;
          }
          
          break;
        case USBH_ACC_UNRECOVERED_STATE:
      
            status = USBH_UNRECOVERED_ERROR;
      
            break;
      
        default:
            break; 
        }
    
    }
    
    
    
    return status ;
}

uint8_t USBH_ACC_Write(USB_OTG_CORE_HANDLE *pdev,
                        uint8_t *dataBuffer,
                        uint32_t nbOfbytes)
{
  static USBH_ACC_Status_TypeDef status = USBH_ACC_BUSY;
  status = USBH_ACC_BUSY;
  
  if(HCD_IsDeviceConnected(pdev))
  {
    switch(USBH_ACC_XferParam.CmdStateMachine)
    {
    case CMD_ACC_SEND_STATE:
           
      USBH_ACC_XferParam.pRxTxBuff = dataBuffer;
      USBH_ACC_XferParam.DataLength = nbOfbytes;
      USBH_ACC_XferParam.RWState = USBH_ACC_SEND;
      /* Start the transfer, then let the state machine 
      magage the other transactions */
      USBH_ACC_XferParam.ACCState = USBH_ACC_USB_TRANSFERS;
      USBH_ACC_XferParam.ACCXferStatus = USBH_ACC_BUSY;
      USBH_ACC_XferParam.CmdStateMachine = CMD_ACC_WAIT_STATUS;
      
      status = USBH_ACC_BUSY;
      
      break;
      
    case CMD_ACC_WAIT_STATUS:
      
      if((USBH_ACC_XferParam.ACCXferStatus == USBH_ACC_OK) && \
        (HCD_IsDeviceConnected(pdev)))
      { 
        /* Commands successfully sent and Response Received  */       
        USBH_ACC_XferParam.CmdStateMachine = CMD_ACC_SEND_STATE;
        status = USBH_ACC_OK;      
      }
      else if (( USBH_ACC_XferParam.ACCXferStatus == USBH_ACC_FAIL ) && \
        (HCD_IsDeviceConnected(pdev)))
      {
        /* Failure Mode */
        USBH_ACC_XferParam.CmdStateMachine = CMD_ACC_SEND_STATE;
      }
      
      else if ( USBH_ACC_XferParam.ACCXferStatus == USBH_ACC_PHASE_ERROR )
      {
        /* Failure Mode */
        USBH_ACC_XferParam.CmdStateMachine = CMD_ACC_SEND_STATE;
        status = USBH_ACC_PHASE_ERROR;    
      }
      else
      {
        /* Wait for the Commands to get Completed */
        /* NO Change in state Machine */
      }
      break;
      
    default:
      break;
    }
  }
  return status;
}
uint8_t USBH_ACC_Read(USB_OTG_CORE_HANDLE *pdev,
                        uint8_t *dataBuffer,
                        uint32_t nbOfbytes)
{
  static USBH_ACC_Status_TypeDef status = USBH_ACC_BUSY;
  status = USBH_ACC_BUSY;
  
  if(HCD_IsDeviceConnected(pdev))
  {
    switch(USBH_ACC_XferParam.CmdStateMachine)
    {
    case CMD_ACC_SEND_STATE:
           
      USBH_ACC_XferParam.pRxTxBuff = dataBuffer;
      USBH_ACC_XferParam.DataLength = nbOfbytes;
      USBH_ACC_XferParam.RWState = USBH_ACC_RECV;
      /* Start the transfer, then let the state machine 
      magage the other transactions */
      USBH_ACC_XferParam.ACCState = USBH_ACC_USB_TRANSFERS;
      USBH_ACC_XferParam.ACCXferStatus = USBH_ACC_BUSY;
      USBH_ACC_XferParam.CmdStateMachine = CMD_ACC_WAIT_STATUS;
      
      status = USBH_ACC_BUSY;
      
      break;
      
    case CMD_ACC_WAIT_STATUS:
      
      if((USBH_ACC_XferParam.ACCXferStatus == USBH_ACC_OK) && \
        (HCD_IsDeviceConnected(pdev)))
      { 
        /* Commands successfully sent and Response Received  */       
        USBH_ACC_XferParam.CmdStateMachine = CMD_ACC_SEND_STATE;
        status = USBH_ACC_OK;      
      }
      else if (( USBH_ACC_XferParam.ACCXferStatus == USBH_ACC_FAIL ) && \
        (HCD_IsDeviceConnected(pdev)))
      {
        /* Failure Mode */
        USBH_ACC_XferParam.CmdStateMachine = CMD_ACC_SEND_STATE;
      }
      
      else if ( USBH_ACC_XferParam.ACCXferStatus == USBH_ACC_PHASE_ERROR )
      {
        /* Failure Mode */
        USBH_ACC_XferParam.CmdStateMachine = CMD_ACC_SEND_STATE;
        status = USBH_ACC_PHASE_ERROR;    
      }
      else
      {
        /* Wait for the Commands to get Completed */
        /* NO Change in state Machine */
      }
      break;
      
    default:
      break;
    }
  }
  return status;
}

uint8_t ACC_Read(USB_OTG_CORE_HANDLE *pdev,
                        uint8_t *dataBuffer,
                        uint32_t nbOfbytes)
{
    uint8_t status = USBH_ACC_OK;
    if(HCD_IsDeviceConnected(&USB_OTG_Core))
    {  
    
        do
        {
          status = USBH_ACC_Read(&USB_OTG_Core, dataBuffer, nbOfbytes);
          print_string("USBH_ACC_Read  end");
          USBH_ACC_HandleXfer(&USB_OTG_Core ,&USB_Host);
          print_string("USBH_ACC_Read  end");
          if(!HCD_IsDeviceConnected(&USB_OTG_Core))
          { 
            return USBH_ACC_FAIL;
          }      
    }
    while(status == USBH_ACC_BUSY );
  }
  
  return status;
}
uint8_t ACC_Write(USB_OTG_CORE_HANDLE *pdev,
                        uint8_t *dataBuffer,
                        uint32_t nbOfbytes)
{
    uint8_t status = USBH_ACC_OK;
    if(HCD_IsDeviceConnected(&USB_OTG_Core))
    {  
    
        do
        {
          status = USBH_ACC_Write(&USB_OTG_Core, dataBuffer, nbOfbytes);
          print_string("USBH_ACC_Write  end");
          USBH_ACC_HandleXfer(&USB_OTG_Core ,&USB_Host);
          print_string("USBH_ACC_HandleXfer  end");
          if(!HCD_IsDeviceConnected(&USB_OTG_Core))
          { 
            return USBH_ACC_FAIL;
          }      
    }
    while(status == USBH_ACC_BUSY );
  }
  
  return status;
}
USBH_Class_cb_TypeDef  Accessory_cb = 
{
  InterfaceInit,
  InterfaceDeInit,
  ClassRequest,
  MachineHandle,
};

