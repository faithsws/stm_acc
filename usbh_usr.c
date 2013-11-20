/**
  ******************************************************************************
  * @file    Audio_playback_and_record/src/usbh_usr.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   This file includes the usb host user callbacks
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbh_usr.h"
#include "stm32f4xx_it.h"
#include "f_accessory.h"
#include "string.h"
extern  USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern  USBH_HOST                    USB_Host;
extern void print_string(char const * string);
/** @addtogroup STM32F4-Discovery_Audio_Player_Recorder
  * @{
  */

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint8_t Command_index = 0;
/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */
FATFS fatfs;
FIL file;
FIL fileR;
DIR dir;
FILINFO fno;

USBH_Usr_cb_TypeDef USR_Callbacks =
{
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  USBH_USR_MSC_Application,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
};

extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern __IO uint8_t AudioPlayStart ;
uint8_t joystick_use = 0x00;
uint8_t lcdLineNo = 0x00;
extern __IO uint8_t RepeatState ;
extern __IO uint8_t LED_Toggle;
static uint8_t USBH_USR_ApplicationState = USH_USR_FS_INIT;
extern __IO uint32_t WaveDataLength ;
extern __IO uint16_t Time_Rec_Base;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/**
  * @brief  USBH_USR_Init
  * @param  None
  * @retval None
  */
void USBH_USR_Init(void)
{
  print_string(__func__);
}

/**
  * @brief  USBH_USR_DeviceAttached
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceAttached(void)
{
  print_string(__func__);
  /* TIM Interrupts enable */
  TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
}

/**
  * @brief  USBH_USR_UnrecoveredError
  * @param  None
  * @retval None
  */
void USBH_USR_UnrecoveredError (void)
{
  print_string(__func__);
}

/**
  * @brief  USBH_DisconnectEvent
  *         Device disconnect event
  * @param  None
  * @retval Staus
  */
void USBH_USR_DeviceDisconnected (void)
{
  print_string(__func__);
  /* Disable the Timer */
  TIM_ITConfig(TIM4, TIM_IT_CC1 , DISABLE);

  
}

/**
  * @brief  USBH_USR_ResetUSBDevice
  * @param  None
  * @retval None
  */
void USBH_USR_ResetDevice(void)
{
  /* callback for USB-Reset */
  print_string(__func__);
}


/**
  * @brief  USBH_USR_DeviceSpeedDetected
  *         Displays the message on LCD for device speed
  * @param  Device speed:
  * @retval None
  */
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
  print_string(__func__);
  char buf[20];
  sprintf(buf,"%d",DeviceSpeed);
  print_string(buf);
}

char Is_AccessoryDevice(void *DeviceDesc)
{
    USBH_DevDesc_TypeDef * desc = (USBH_DevDesc_TypeDef *)DeviceDesc;
    if(desc->idVendor == 0x18D1)
    {
        if((desc->idProduct == 0x2D00) || (desc->idProduct == 0x2D01))
          return 1;
        else
          return 0;
    }
    else
      return 0;
}
void Send_String(USB_OTG_CORE_HANDLE *pdev,USBH_HOST  * phost, uint16_t index , char * str)
{
  phost->Control.setup.b.bmRequestType = USB_H2D |  USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_VENDOR;
  phost->Control.setup.b.bRequest = ACCESSORY_SEND_STRING;
  phost->Control.setup.b.wValue.w = 0;
  phost->Control.setup.b.wIndex.w = index;
  phost->Control.setup.b.wLength.w = strlen(str) +1;  
    
  USBH_CtlReq(pdev, phost, str , strlen(str) +1 );          
 
   
  //USBH_BulkSendData (pdev, str, strlen(str) +1 ,phost->Control.hc_num_out);
   //USB_OTG_WritePacket(pdev,str ,phost->Control.hc_num_out,strlen(str) +1 );
}
void Switch_Device(USB_OTG_CORE_HANDLE *pdev,USBH_HOST  * phost)
{

    Send_String(pdev,phost, ACCESSORY_STRING_MANUFACTURER, "Google, Inc.");
    Send_String(pdev,phost, ACCESSORY_STRING_MODEL, "DemoKit");
    Send_String(pdev,phost,ACCESSORY_STRING_DESCRIPTION, "DemoKit Arduino Board");
    Send_String(pdev,phost,ACCESSORY_STRING_VERSION, "1.0");
    Send_String(pdev,phost,ACCESSORY_STRING_URI, "http://www.android.com");
    Send_String(pdev,phost,ACCESSORY_STRING_SERIAL, "0000000012345678");
  
    phost->Control.setup.b.bmRequestType = USB_H2D |  USB_REQ_RECIPIENT_DEVICE | USB_REQ_TYPE_VENDOR;
    phost->Control.setup.b.bRequest = ACCESSORY_START;
    phost->Control.setup.b.wValue.w = 0;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = 0;  
    
    USBH_CtlReq(pdev, phost, 0 , 0 );          
}
/**
  * @brief  USBH_USR_Device_DescAvailable
  * @param  device descriptor
  * @retval None
  */
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
  USBH_DevDesc_TypeDef * desc = (USBH_DevDesc_TypeDef *)DeviceDesc;
  print_string(__func__);
  
  char buf[20];
  sprintf(buf,"idVendor = %04x",desc->idVendor);
  print_string(buf);
  
  sprintf(buf,"idProduct = %04x",desc->idProduct);
  print_string(buf);
  
}

/**
  * @brief  USBH_USR_DeviceAddressAssigned
  *         USB device is successfully assigned the Address
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceAddressAssigned(void)
{
  print_string(__func__);
  if(Is_AccessoryDevice(&(USB_Host.device_prop.Dev_Desc)) == 0)
  {
        print_string("Not wanted device");
        Switch_Device(&USB_OTG_Core,&USB_Host);
  }
   
}

/**
  * @brief  USBH_USR_Conf_Desc
  * @param  Configuration descriptor
  * @retval None
  */
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
    USBH_InterfaceDesc_TypeDef *itfDesc,
    USBH_EpDesc_TypeDef *epDesc)
{
  print_string(__func__);
  
  
}

/**
  * @brief  USBH_USR_Manufacturer_String
  * @param  Manufacturer String
  * @retval None
  */
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
  print_string(__func__);
  print_string(ManufacturerString);
}

/**
  * @brief  USBH_USR_Product_String
  * @param  Product String
  * @retval None
  */
void USBH_USR_Product_String(void *ProductString)
{
  print_string(__func__);
  print_string(ProductString);
}

/**
  * @brief  USBH_USR_SerialNum_String
  * @param  SerialNum_String
  * @retval None
  */
void USBH_USR_SerialNum_String(void *SerialNumString)
{
  print_string(__func__);
  print_string(SerialNumString);
}
extern int My_Application(void);
/**
  * @brief  EnumerationDone 
  *         User response request is displayed to ask application jump to class
  * @param  None
  * @retval None
  */
void USBH_USR_EnumerationDone(void)
{
  print_string(__func__);
  /* 0.5 seconds delay */
  USB_OTG_BSP_mDelay(500);
  //My_Application();
 // USBH_USR_MSC_Application();
} 

/**
  * @brief  USBH_USR_DeviceNotSupported
  *         Device is not supported
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceNotSupported(void)
{
  print_string(__func__);
}


/**
  * @brief  USBH_USR_UserInput
  *         User Action for application state entry
  * @param  None
  * @retval USBH_USR_Status : User response for key button
  */
USBH_USR_Status USBH_USR_UserInput(void)
{
  print_string(__func__);
  /* callback for Key botton: set by software in this case */
  return USBH_USR_RESP_OK;
}

/**
  * @brief  USBH_USR_OverCurrentDetected
  *         Over Current Detected on VBUS
  * @param  None
  * @retval None
  */
void USBH_USR_OverCurrentDetected (void)
{
  print_string(__func__);
}

/**
  * @brief  USBH_USR_MSC_Application
  * @param  None
  * @retval Staus
  */
int USBH_USR_MSC_Application(void)
{
  
  print_string(__func__);
  char buf[100];
  ACC_Write(&USB_OTG_Core,buf,3);
#if 0
  switch (USBH_USR_ApplicationState)
  {
    case USH_USR_FS_INIT:

      /* Initialises the File System*/
      if (f_mount( 0, &fatfs ) != FR_OK ) 
      {
        /* efs initialisation fails*/
        return(-1);
      }
      
      /* Flash Disk is write protected */
      if (USBH_MSC_Param.MSWriteProtect == DISK_WRITE_PROTECTED)
      {
        while(1)
        {
          /* Red LED On */
          STM_EVAL_LEDOn(LED5);
        }
      }
      /* Go to menu */
      USBH_USR_ApplicationState = USH_USR_AUDIO;
      break;

    case USH_USR_AUDIO:

      /* Go to Audio menu */
      COMMAND_AudioExecuteApplication();

      /* Set user initialization flag */
      USBH_USR_ApplicationState = USH_USR_FS_INIT;
      break;

    default:
      break;
  }
#endif
  return(0);
}

/**
  * @brief  COMMAND_AudioExecuteApplication
  * @param  None
  * @retval None
  */
void COMMAND_AudioExecuteApplication(void)
{
  /* Execute the command switch the command index */
  switch (Command_index)
  {
  /* Start Playing from USB Flash memory */
  case CMD_PLAY:
    if (RepeatState == 0)
      WavePlayerStart();
    break;
    /* Start Recording in USB Flash memory */ 
  case CMD_RECORD:
    RepeatState = 0;
    WaveRecorderUpdate();
    break;  
  default:
    break;
  }
}

/**
  * @brief  USBH_USR_DeInit
  *         Deint User state and associated variables
  * @param  None
  * @retval None
  */
void USBH_USR_DeInit(void)
{
  print_string(__func__);
  USBH_USR_ApplicationState = USH_USR_FS_INIT;
}

/**
  * @}
  */



/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
