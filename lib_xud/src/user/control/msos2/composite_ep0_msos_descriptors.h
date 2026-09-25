// Copyright 2024-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef COMPOSITE_EP0_MSOS_DESCRIPTORS
#define COMPOSITE_EP0_MSOS_DESCRIPTORS

#include "xud.h"

#if (XUD_ENABLE_DFU) || (XUD_ENABLE_VENDOR_CONTROL)

#include <stddef.h>

#include "xud_device.h"
#include "msos_descriptors.h"


/** Initialise the Composite Ep0 MSOS Descriptors before enumeration of the device */
void XUD_Init_Composite_Ep0_Msos_Descriptors(void);

/** Function to send the BOS descriptor when prompted via a Standard Get request
 * 
 * Request will be Standard Get request (USB_GET_DESCRIPTOR) with wValue high byte == USB_DESCTYPE_BOS
 * 
 * \param ep0_out   Endpoint 0 OUT endpoint
 * \param ep0_in    Endpoint 0 IN endpoint
 * \param sp        Pointer to the setup packet of the request
 * 
 * \retval          XUD_RES_ERR if request not handled
 * \retval          XUD_RES_OKAY if successful
 * \retval          XUD_RES_WAIT if transfer in progress
 */
XUD_Result_t XUD_GetCompositeBosDescriptor(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t *sp);

/** Function to send the MSOS descriptor when prompted via a Vendor Get request
 * 
 * Request will be a Vendor Get request with bRequest == XUA_REQUEST_GET_MSOS_DESCRIPTOR.
 * This is defined in xua_conf_default.h
 * 
 * \param ep0_out   Endpoint 0 OUT endpoint
 * \param ep0_in    Endpoint 0 IN endpoint
 * \param sp        Pointer to the setup packet of the request
 * 
 * \retval          XUD_RES_ERR if request not handled
 * \retval          XUD_RES_OKAY if successful
 * \retval          XUD_RES_WAIT if transfer in progress
 */
XUD_Result_t XUD_GetCompositeMsosDescriptor(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t *sp);

#endif

#endif // COMPOSITE_EP0_MSOS_DESCRIPTORS
