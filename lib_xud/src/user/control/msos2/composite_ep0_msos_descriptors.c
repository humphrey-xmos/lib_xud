// Copyright 2024-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xud.h"

#if (XUD_ENABLE_DFU) || (XUD_ENABLE_VENDOR_CONTROL)

#include "composite_ep0_msos_descriptors.h"

#include <stddef.h>

#include "xud_device.h"
#include "msos_descriptors.h"
#include "msos_helpers.h"
#include "simple_ep0_msos_descriptors.h"

/* Device Interface GUID*/
char g_device_interface_guid_dfu_str[DEVICE_INTERFACE_GUID_MAX_STRLEN+1] = XUD_WINUSB_DEVICE_INTERFACE_GUID_DFU;
char g_device_interface_guid_control_str[DEVICE_INTERFACE_GUID_MAX_STRLEN+1] = XUD_WINUSB_DEVICE_INTERFACE_GUID_CONTROL;

typedef struct {
  MSOS_desc_header_t              msos_desc_header;
  MSOS_desc_cfg_subset_header_t   msos_desc_cfg_subset_header;

#if (XUD_ENABLE_VENDOR_CONTROL)
  MSOS_desc_fn_subset_header_t    msos_fn_subset_header_control;
  MSOS_desc_compat_id_t           msos_desc_compat_id_control;
  MSOS_desc_registry_property_t   msos_desc_registry_property_control;
#endif

#if XUD_ENABLE_DFU
  MSOS_desc_fn_subset_header_t    msos_fn_subset_header_dfu;
  MSOS_desc_compat_id_t           msos_desc_compat_id_dfu;
  MSOS_desc_registry_property_t   msos_desc_registry_property_dfu;
#endif
} __attribute__((packed)) MSOS_desc_composite_t;

/* USB Binary Device Object Store (BOS) descriptor */
// TODO - replace with simple MSOS Bos descriptor, but needs FW to change length based on request.
USB_Descriptor_BOS_t desc_bos_xud =
{
    .usb_desc_bos_standard = {
        .bLength = sizeof(USB_Descriptor_BOS_standard_t),
        .bDescriptorType = USB_DESCTYPE_BOS,
        .wTotalLength = sizeof(USB_Descriptor_BOS_standard_t) + sizeof(USB_Descriptor_BOS_platform_t),
        .bNumDeviceCaps = 1
    },
    .usb_desc_bos_platform = {
        .bLength = sizeof(USB_Descriptor_BOS_platform_t),
        .bDescriptorType = USB_DESCTYPE_DEVICE_CAPABILITY,
        .bDevCapabilityType = DEVICE_CAPABILITY_PLATFORM,
        .bReserved = 0,
        .PlatformCapabilityUUID = {USB_BOS_MS_OS_20_UUID},
        .CapabilityData = {U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(sizeof(MSOS_desc_composite_t)), XUD_REQUEST_GET_MSOS_DESCRIPTOR, 0}
    }
};

MSOS_desc_composite_t desc_ms_os_20_composite =
{
    .msos_desc_header =
    {
        .wLength = sizeof(MSOS_desc_header_t),
        .wDescriptorType = MS_OS_20_SET_HEADER_DESCRIPTOR,
        .dwWindowsVersion = 0x06030000,
        .wTotalLength = sizeof(MSOS_desc_composite_t)
    },
    .msos_desc_cfg_subset_header =
    {
        .wLength = sizeof(MSOS_desc_cfg_subset_header_t),
        .wDescriptorType = MS_OS_20_SUBSET_HEADER_CONFIGURATION,
        .bConfigurationValue = 0,
        .bReserved = 0,
        .wTotalLength = sizeof(MSOS_desc_composite_t) - sizeof(MSOS_desc_header_t)
    },
  #if (XUD_ENABLE_VENDOR_CONTROL)
    .msos_fn_subset_header_control =
    {
        .wLength = sizeof(MSOS_desc_fn_subset_header_t),
        .wDescriptorType = MS_OS_20_SUBSET_HEADER_FUNCTION,
        .bFirstInterface = INTERFACE_NUMBER_MISC_CONTROL,
        .bReserved = 0,
        .wSubsetLength = sizeof(MSOS_desc_fn_subset_header_t) + sizeof(MSOS_desc_compat_id_t) + sizeof(MSOS_desc_registry_property_t)
    },
    .msos_desc_compat_id_control =
    {
        .wLength = sizeof(MSOS_desc_compat_id_t),
        .wDescriptorType = MS_OS_20_FEATURE_COMPATIBLE_ID,
        .CompatibleID = {'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00}, // "WINUSB\0\0"
        .SubCompatibleID = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    },
    .msos_desc_registry_property_control =
    {
        .wLength = sizeof(MSOS_desc_registry_property_t),
        .wDescriptorType = MS_OS_20_FEATURE_REG_PROPERTY,
        .wPropertyDataType = 0x0007, // REG_MULTI_SZ as defined in Table 15 of Microsoft OS 2.0 Descriptors Specification. Check IMPORTANT NOTE 2 in https://github.com/pbatard/libwdi/wiki/WCID-Devices
        // for why we need REG_MULTI_SZ and not REG_SZ
        .wPropertyNameLength = MSOS_PROPERTY_NAME_LEN,
        .PropertyName = { 'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
                          'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00}, //"DeviceInterfaceGUIDs\0" in UTF-16
        .wPropertyDataLength = MSOS_INTERFACE_GUID_LEN,
        // "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}\0\0" generate from https://guidgenerator.com/ and stored as Unicode
        .PropertyData = { 0 } // defined in the XUA_WINUSB_DEVICE_INTERFACE_GUID_CONTROL define and updated in the descriptor at runtime with update_guid_in_msos_desc()
    },
#endif

#if XUD_ENABLE_DFU
    .msos_fn_subset_header_dfu =
    {
        .wLength = sizeof(MSOS_desc_fn_subset_header_t),
        .wDescriptorType = MS_OS_20_SUBSET_HEADER_FUNCTION,
        .bFirstInterface = XUD_INTERFACE_NUMBER_DFU,
        .bReserved = 0,
        .wSubsetLength = sizeof(MSOS_desc_fn_subset_header_t) + sizeof(MSOS_desc_compat_id_t) + sizeof(MSOS_desc_registry_property_t)
    },
    .msos_desc_compat_id_dfu =
    {
        .wLength = sizeof(MSOS_desc_compat_id_t),
        .wDescriptorType = MS_OS_20_FEATURE_COMPATIBLE_ID,
        .CompatibleID = {'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00}, // "WINUSB\0\0"
        .SubCompatibleID = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    },
    .msos_desc_registry_property_dfu =
    {
        .wLength = sizeof(MSOS_desc_registry_property_t),
        .wDescriptorType = MS_OS_20_FEATURE_REG_PROPERTY,
        .wPropertyDataType = 0x0007, // REG_MULTI_SZ as defined in Table 15 of Microsoft OS 2.0 Descriptors Specification. Check IMPORTANT NOTE 2 in https://github.com/pbatard/libwdi/wiki/WCID-Devices
        // for why we need REG_MULTI_SZ and not REG_SZ
        .wPropertyNameLength = MSOS_PROPERTY_NAME_LEN,
        .PropertyName = { 'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
                          'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00}, //"DeviceInterfaceGUIDs\0" in UTF-16
        .wPropertyDataLength = MSOS_INTERFACE_GUID_LEN,
        // "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}\0\0" generate from https://guidgenerator.com/ and stored as Unicode
        .PropertyData = { 0 } // defined in the XUA_WINUSB_DEVICE_INTERFACE_GUID_CONTROL define and updated in the descriptor at runtime with update_guid_in_msos_desc()
    }
#endif
};

static desc_handle_t xud_bos_handle = {
    (unsigned char*)&desc_bos_xud, sizeof(USB_Descriptor_BOS_t),
};

static desc_handle_t xud_msos_handle = {
    (unsigned char*)&desc_ms_os_20_composite, sizeof(MSOS_desc_composite_t),
};

void XUD_Init_Composite_Ep0_Msos_Descriptors(void)
{
#if XUD_ENABLE_DFU
    XUD_Update_Guid_In_Msos_Desc(&desc_ms_os_20_composite.msos_desc_registry_property_dfu, g_device_interface_guid_dfu_str);
#endif

#if (XUD_ENABLE_VENDOR_CONTROL)
    XUD_Update_Guid_In_Msos_Desc(&desc_ms_os_20_composite.msos_desc_registry_property_control, g_device_interface_guid_control_str);
#endif

    // Initialise simple MSOS descriptors to register with msos_helpers
    // The simple GUID is configured through xud_conf.h macros
    XUD_Init_Simple_Ep0_Msos_Descriptors();
}

XUD_Result_t XUD_GetCompositeBosDescriptor(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t *sp)
{
    // Use composite MSOS descriptor
    if ((xud_bos_handle.desc_ptr != NULL) && ((sp->wValue & 0xff00) == (USB_DESCTYPE_BOS << 8))) {
        return XUD_DoGetRequest(ep0_out, ep0_in, xud_bos_handle.desc_ptr, xud_bos_handle.desc_size, sp->wLength);
    } else {
        return XUD_RES_ERR;
    }
}

XUD_Result_t XUD_GetCompositeMsosDescriptor(XUD_ep ep0_out, XUD_ep ep0_in, USB_SetupPacket_t *sp)
{
    // Use composite MSOS descriptor
    if ((xud_msos_handle.desc_ptr != NULL) && (sp->wIndex == MS_OS_20_DESCRIPTOR_INDEX)) {
        return XUD_DoGetRequest(ep0_out, ep0_in, xud_msos_handle.desc_ptr, xud_msos_handle.desc_size, sp->wLength);
    } else {
        return XUD_RES_ERR;
    }
}

#endif
