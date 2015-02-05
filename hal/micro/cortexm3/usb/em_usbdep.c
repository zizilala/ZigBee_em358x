/**************************************************************************//**
 * @file
 * @brief USB protocol stack library, USB device endpoint handlers.
 * @author Nathaniel Ting
 * @version 3.20.3
 * <!-- Copyright 2013 by Silicon Laboratories. All rights reserved.     *80*-->
 *****************************************************************************/

#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "hal/hal.h"

#if defined(CORTEXM3_EM35X_USB)
#include "em_usb.h"
#include "em_usbhal.h"
#include "em_usbtypes.h"
#include "em_usbd.h"


/*
 * USBDEP_Ep0Handler() is called each time a packet has been transmitted
 * or recieved on the default endpoint.
 * A state machine navigate us through the phases of a control transfer
 * according to "chapter 9" in the USB spec.
 */
void USBDEP_Ep0Handler( USBD_Device_TypeDef *device )
{
  // halToggleLed(BOARDLED0);
  int status;
  USBD_Ep_TypeDef *ep;
  static bool statusIn;
  static uint32_t xferred;
  static USB_XferCompleteCb_TypeDef callback;
  ep = &device->ep[ 0 ];
  
  #ifdef USB_DEBUG_EP0
  DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "ep->state = ");
  #endif
 

  switch ( ep->state )
  {
    case D_EP_IDLE:
      ep->remaining = 0;
      ep->zlp = 0;
      callback = NULL;
      statusIn = false;
      #ifdef USB_DEBUG_EP0
      DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "IDLE\r\n");
      #endif

      if (*EPOUTBUFSIZE[0] == 0)
      {

        #ifdef USB_DEBUG_EP0
        DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "0 byte ack\r\n");
        #endif
        break;
      }
      status = USBDCH9_SetupCmd( device );

      if ( status == USB_STATUS_REQ_ERR )
      {
        // ignore error for now since this can be triggered by requests we don't
        // need to handle
        // halSetLed(BOARDLED1);
        ep->in = true;
        USBDHAL_StallEp( ep );              /* Stall Ep0 IN                 */
        ep->in = false;                     /* OUT for next SETUP           */
        // USBDHAL_StallEp( ep );              /* Stall Ep0 OUT                */
        USBDHAL_ReenableEp0Setup( device ); /* Prepare for next SETUP packet*/
      }
      else /* ( Status == USB_STATUS_OK ) */
      {
        if ( (ep->state == D_EP_RECEIVING) || (ep->state == D_EP_TRANSMITTING) )
        {
          callback = ep->xferCompleteCb;
        }

        if ( ep->state != D_EP_RECEIVING )
        {
          // enable with care. since setup involves lots of transfers, the debug
          // buffer will fill up very quickly.
          // #ifdef USB_DEBUG_EP0
          // DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "ep->remaining = %d\r\n",ep->remaining);
          // #endif

          if ( ep->remaining )
          {
            /* Data will be sent to host, check if a ZLP must be appended */
            if ( ( ep->remaining < device->setup->wLength ) &&
                 ( ep->remaining % ep->packetSize == 0    )    )
            {
              ep->zlp = 1;
            }
          }
          else
          {
            /* Prepare for next SETUP packet*/
            USBDHAL_ReenableEp0Setup( device );

            /* No data stage, a ZLP may have been sent. If not, send one */
            if ( ep->zlp == 0 )
            {
              #ifdef USB_DEBUG_EP0
              DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "no ack\r\n");
              #endif
              ep->state = D_EP_IDLE;
              // ack is already handled by hardware
              // USBD_Write( 0, NULL, 0, NULL );             /* ACK to host */
              ep->zlp = 2;
              ep->state = D_EP_STATUS;
            }
            else
            {
              ep->state = D_EP_IDLE;
              ep->in = false;                      /* OUT for next SETUP */
            }
          }
        }
      }
      break; //D_EP_IDLE
      case D_EP_TRANSMITTING:
        #ifdef USB_DEBUG_EP0
        DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "TX\r\n");
        #endif
        if ( ep->remaining )
        {
          /* There is more data to transmit */
          USBD_ReArmEp0( ep );


          // enable with care. since setup involves lots of transfers, the debug
          // buffer will fill up very quickly.
          // #ifdef USB_DEBUG_EP0
          // DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "ep->remaining = %d\r\n",ep->remaining);
          // #endif
        }
        else
        {
          #ifdef USB_DEBUG_EP0
          DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "TX done\r\n");
          #endif
          /* All data transferred, is a ZLP packet needed ? */
          if ( ep->zlp == 1 )
          {
            #ifdef USB_DEBUG_EP0
            DEBUG_BUFFER += sprintf(DEBUG_BUFFER,"ack\r\n");
            #endif
            xferred   = ep->xferred;
            ep->state = D_EP_IDLE;          /* USBD_Write() sets state back */
                                            /* to EP_TRANSMITTING           */
            USBD_Write( 0, NULL, 0, NULL ); /* Send ZLP                     */
            ep->zlp = 2;
          }
          else
          {
            if ( ep->zlp == 0 )
            {
              xferred = ep->xferred;
              ep->zlp = 1;
            }


            // host will send ack, which will be handled by hardware
            // ep->state = D_EP_IDLE;
            // USBD_Read( 0, NULL, 0, NULL );  /* Get ZLP packet (ACK) from host */
            statusIn = true;
            ep->state = D_EP_STATUS;
          }
        }
        break;
      case D_EP_STATUS:
        #ifdef USB_DEBUG_EP0
        DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "STATUS\r\n");
        #endif
        if ( statusIn )
        {
          USBDHAL_ReenableEp0Setup( device );
        }

        if ( callback != NULL )
        {
          callback( USB_STATUS_OK, xferred, 0 );
        }

        ep->state = D_EP_IDLE;
        ep->in = false;                     /* OUT for next SETUP */
        break;
    }
}

void USBDEP_EpHandler( uint8_t epAddr )
{
  USB_XferCompleteCb_TypeDef callback;
  
  USBD_Ep_TypeDef *ep = USBD_GetEpFromAddr( epAddr );

  #if defined(USB_DEBUG_READ) || defined(USB_DEBUG_WRITE) || defined(USB_DEBUG_EP)
  DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "EP%dHandler\tep->state = %d\r\n",ep->num,ep->state);
  #endif


  if ( ep->state == D_EP_TRANSMITTING ) 
  {
    #ifdef USB_DEBUG_EP
    DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "ep->xferred=%d \tep->remaining = %d\r\n",ep->xferred, ep->remaining);
    #endif
    if ( ep->remaining )
    {
      // int8u txSize = (ep->remaining > ep->packetSize) ? ep->packetSize : ep->remaining;
      /* There is more data to transmit */
      USBD_ArmEp( ep );
      // ep->buf += ep->xferred;
      // ep->buf += txSize;
    }
    else // Done transmitting
    {
      // Put ep back into idle state
      ep->state = D_EP_IDLE;

      // If there's a callback, call it
      if ( ep->xferCompleteCb )
      {
        callback = ep->xferCompleteCb;
        ep->xferCompleteCb = NULL;
        callback( USB_STATUS_OK, ep->xferred, ep->remaining );
      }
      #if EM_SERIAL3_ENABLED
        usbTxData();
      #endif
    }
  }
  else if ( ep->state == D_EP_RECEIVING )
  {
    #ifdef USB_DEBUG_EP
    DEBUG_BUFFER += sprintf(DEBUG_BUFFER, "ep->remaining = %d\r\n",ep->remaining);
    #endif

    if ( ep->remaining )
    {
      /* Short Packet */
      if (ep->packetSize>(ep->remaining + *EPOUTBUFSIZE[ep->num]))
      {
        
        
        if ( ep->xferCompleteCb )
        {
          callback = ep->xferCompleteCb;
          ep->xferCompleteCb = NULL;
          callback( USB_STATUS_TIMEOUT, ep->xferred, ep->remaining );
        }
      }
      else
      {
        /* There is more data to transmit */
        USBD_ArmEp( ep );
        ep->buf += *EPOUTBUFSIZE[ep->num];  // move buffer pointer
      }
      
    }
    
    else
    {
      // Put ep back into idle state
      ep->state = D_EP_IDLE;
      // If there's a callback, call it
      if ( ep->xferCompleteCb )
      {
        callback = ep->xferCompleteCb;
        ep->xferCompleteCb = NULL;
        callback( USB_STATUS_OK, ep->xferred, ep->remaining );
      }
    }
  }
  else
  {
    // EpHandler should only be called if the endpoint is in use
    // ep->state set in read/write
    // assert( 0 );
  }
}

#endif //CORTEXM3_EM35X_USB