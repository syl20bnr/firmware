#include "usb_composite_device.h"

uint32_t UsbMouseActionCounter;
static usb_mouse_report_t usbMouseReports[2];
usb_mouse_report_t* ActiveUsbMouseReport = usbMouseReports;

static usb_mouse_report_t* getInactiveUsbMouseReport(void)
{
    return ActiveUsbMouseReport == usbMouseReports ? usbMouseReports+1 : usbMouseReports;
}

void SwitchActiveUsbMouseReport(void)
{
    ActiveUsbMouseReport = getInactiveUsbMouseReport();
}

void ResetActiveUsbMouseReport(void)
{
    bzero(ActiveUsbMouseReport, USB_MOUSE_REPORT_LENGTH);
}

usb_status_t usbMouseAction(void)
{
    UsbMouseActionCounter++;
    return USB_DeviceHidSend(
            UsbCompositeDevice.mouseHandle, USB_MOUSE_ENDPOINT_INDEX,
            (uint8_t*)getInactiveUsbMouseReport(), USB_MOUSE_REPORT_LENGTH);
}

usb_status_t UsbMouseCallback(class_handle_t handle, uint32_t event, void *param)
{
    static bool usbMouseActionActive = false;
    usb_status_t error = kStatus_USB_Error;

    switch (event) {
        case kUSB_DeviceHidEventSendResponse:
            if (UsbCompositeDevice.attach) {
                // Send out the report continuously if the report is not zeros
                usb_mouse_report_t *report = getInactiveUsbMouseReport();
                uint8_t zeroBuf[sizeof(usb_mouse_report_t)] = { 0 };
                bool reportChanged = memcmp(report, zeroBuf, sizeof(usb_mouse_report_t)) != 0;
                if (usbMouseActionActive || reportChanged) {
                    usbMouseActionActive = reportChanged; // Used to send out all zeros once after a report has been sent
                    return usbMouseAction();
                }
            }
        case kUSB_DeviceHidEventGetReport:
        case kUSB_DeviceHidEventSetReport:
        case kUSB_DeviceHidEventRequestReportBuffer:
            error = kStatus_USB_InvalidRequest;
            break;
        case kUSB_DeviceHidEventGetIdle:
        case kUSB_DeviceHidEventGetProtocol:
        case kUSB_DeviceHidEventSetIdle:
        case kUSB_DeviceHidEventSetProtocol:
            break;
        default:
            break;
    }

    return error;
}

usb_status_t UsbMouseSetConfiguration(class_handle_t handle, uint8_t configuration)
{
    if (USB_COMPOSITE_CONFIGURATION_INDEX == configuration) {
        //return usbMouseAction();
    }
    return kStatus_USB_Error;
}

usb_status_t UsbMouseSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    if (USB_MOUSE_INTERFACE_INDEX == interface) {
        //return usbMouseAction();
    }
    return kStatus_USB_Error;
}
