
#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "hal/hal.h"
#include "hal/micro/cortexm3/usb/em_usb.h"
#if defined(CORTEXM3_EM35X_USB) && EM_SERIAL3_ENABLED
#include "descriptors.h"

/**************************************************************************//**
 * @brief
 *   Handle USB setup commands. Implements CDC class specific commands.
 *
 * @param[in] setup Pointer to the setup packet received.
 *
 * @return USB_STATUS_OK if command accepted.
 *         USB_STATUS_REQ_UNHANDLED when command is unknown, the USB device
 *         stack will handle the request.
 *****************************************************************************/
int SetupCmd(const USB_Setup_TypeDef *setup)
{
  int retVal = USB_STATUS_REQ_UNHANDLED;

  if ((setup->Type == USB_SETUP_TYPE_CLASS) &&
      (setup->Recipient == USB_SETUP_RECIPIENT_INTERFACE))
  {
    switch (setup->bRequest)
    {
    case USB_CDC_GETLINECODING:

      #ifdef USB_DEBUG_CDC
      DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "CDC_GETLINECODING -> \r\n");
      #endif
      /********************/
      if ((setup->wValue == 0) &&
          (setup->wIndex == 0) &&               /* Interface no.            */
          (setup->wLength == 7) &&              /* Length of cdcLineCoding  */
          (setup->Direction == USB_SETUP_DIR_IN))
      {
        /* Send current settings to USB host. */
        // USBD_Write(0, (void*) &cdcLineCoding, 7, NULL);
        USBD_Write(0,NULL,0,NULL);
        retVal = USB_STATUS_OK;
      }
      break;

    case USB_CDC_SETLINECODING:

      #ifdef USB_DEBUG_CDC
      DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "CDC_SETLINECODING -> \r\n");
      #endif
      /********************/
      if ((setup->wValue == 0) &&
          (setup->wIndex == 0) &&               /* Interface no.            */
          (setup->wLength == 7) &&              /* Length of cdcLineCoding  */
          (setup->Direction != USB_SETUP_DIR_IN))
      {
        /* Get new settings from USB host. */
        // USBD_Read(0, (void*) &cdcLineCoding, 7, LineCodingReceived);

        
        USBD_Write(0,NULL,0,NULL);

        // USBD_Read(0,NULL,7,NULL);
        retVal = USB_STATUS_OK;
      }
      break;

    case USB_CDC_SETCTRLLINESTATE:
      #ifdef USB_DEBUG_CDC
      DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "CDC_SETLINESTATE -> \r\n");
      #endif
      /********************/
      if ((setup->wIndex == 0) &&               /* Interface no.  */
          (setup->wLength == 0))                /* No data        */
      {
        /* Do nothing ( Non compliant behaviour !! ) */
        USBD_Write(0,NULL,0,NULL);
        retVal = USB_STATUS_OK;
      }
      break;
    }
  }

  return retVal;
}

void stateChange(USBD_State_TypeDef oldState, USBD_State_TypeDef newState)
{
  /* Print state transition to debug output */
  // emberSerialPrintf(SER232,"%s => %s\r\n", USBD_GetUsbStateName(oldState), USBD_GetUsbStateName(newState));
  
  if (newState == USBD_STATE_CONFIGURED)
  {
    /* Start waiting for the 'tick' messages */
    // emberSerialPrintf(SER232,"starting read\r\n");
    int8u t='\r';

    USBD_Read(EP_DATA_OUT, &t, 1, NULL);
    
  }
}

#endif //CORTEXM3_EM35X_USB && EM_SERIAL3_ENABLED