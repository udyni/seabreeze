/***************************************************//**
 * @file    NativeUSBLinux.c
 * @date    June 2009
 * @author  Ocean Optics, Inc.
 *
 * This is an implementation of the USB interface using
 * the libusb API.  This should provide support for
 * Linux 2.4.20 and beyond, both 32-bit and 64-bit.
 *
 * LICENSE:
 *
 * SeaBreeze Copyright (C) 2014, Ocean Optics Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *******************************************************/

#include "common/globals.h"
#include <libusb-1.0/libusb.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>  // for perror()
#include "native/usb/NativeUSB.h"
#include "api/seabreezeapi/SeaBreezeAPIConstants.h"

/* Definitions and macros */
#define MAX_USB_DEVICES             127
#define DEFAULT_TIMEOUT               0 /* milliseconds */
/* Tell gcc not to warn about a particular
 * variable being unused.  This is useful for function
 * parameters that are required by an interface prototype, but not
 * for the actual implementation.  This also mangles
 * the name so that an error will be generated if anything
 * tries to use an UNUSED variable.
 */
#define UNUSED(x)  UNUSED_ ## x __attribute__((unused))

/* struct definitions */
typedef struct {
    long deviceID;  /* Unique ID for device.  Assigned by this driver */
    int interface;  /* Interface number, needed to release it on close */
    libusb_device_handle *dev;
} __usb_interface_t;

typedef struct {
    long deviceID;                /* Unique ID for device.  Assigned by this driver. */
    __usb_interface_t *handle;    /* Pointer to USB interface instance */
    libusb_device *device;        /* Device */
    unsigned short vendorID;
    unsigned short productID;
    unsigned char valid;          /* Whether this struct is valid */
    unsigned char mark;           /* Used to determine if device is still present */
} __device_instance_t;


/* Global variables (mostly static lookup tables) */
static __device_instance_t __enumerated_devices[MAX_USB_DEVICES] = { { 0 } };
static int __enumerated_device_count = 0;   /* To keep linear searches short */
static long __last_assigned_deviceID = 0;   /* To keep device IDs unique */

/**
 * Global USB context (should be initialized before any call is done)
 */
static libusb_context *__libusb_ctx = NULL;
static int __libusb_ctx_usage = 0;

/* Function prototypes */
static __device_instance_t *__lookup_device_instance_by_ID(long deviceID);
static __device_instance_t *__lookup_device_instance_by_location(libusb_device* device);
static __device_instance_t *__add_device_instance(libusb_device *device, int vendorID, int productID);
static void __purge_unmarked_device_instances(int vendorID, int productID);
static void __close_and_dealloc_usb_interface(__usb_interface_t *usb);
static int __probe_devices();

/* This function will iterate over the known devices and attempt to match
 * the given ID.  It might be more efficient for the sake of this search
 * to store the device instances in a linked list so that no invalid elements
 * need to be searched through, but that just has the effect of shifting the
 * time burden to when a device must be removed from the list.  This lookup
 * is most likely to be called when the calling code is trying to open a device,
 * at which point they more or less expect to have to wait a little while.
 * Not that searching over this array will take very long...
 */
static __device_instance_t *__lookup_device_instance_by_ID(long deviceID) {
    int i;
    int valid;
    /* The __enumerated_device_count is used to end the search once it is
     * known that there are no more devices to be found.
     */
    for(i = 0, valid = 0; i < MAX_USB_DEVICES && valid < __enumerated_device_count; i++) {
        if(0 != __enumerated_devices[i].valid) {
            if(__enumerated_devices[i].deviceID == deviceID) {
                return &(__enumerated_devices[i]);
            }
            valid++;
        }
    }
    return NULL;
}

static __device_instance_t *__lookup_device_instance_by_location(libusb_device *device) {
    int i;
    int valid;
    /* The __enumerated_device_count is used to end the search once it is
     * known that there are no more devices to be found.
     */
    for(i = 0, valid = 0; i < MAX_USB_DEVICES && valid < __enumerated_device_count; i++) {
        if(0 != __enumerated_devices[i].valid) {
            if(libusb_get_bus_number(__enumerated_devices[i].device) == libusb_get_bus_number(device) &&
               libusb_get_port_number(__enumerated_devices[i].device) == libusb_get_port_number(device)) {
                return &(__enumerated_devices[i]);
            }
            valid++;
        }
    }
    return NULL;
}

static __device_instance_t *__add_device_instance(libusb_device* device, int vendorID, int productID) {
    int i;

    /* First need to find an empty slot to store this device descriptor */
    for(i = 0; i < MAX_USB_DEVICES; i++) {
        if(0 == __enumerated_devices[i].valid) {
            /* Found an empty slot */
            __enumerated_devices[i].valid = 1;
            /* Increse reference count on device */
            libusb_ref_device(device);
            __enumerated_devices[i].device = device;
            __enumerated_devices[i].deviceID = __last_assigned_deviceID++;
            __enumerated_devices[i].vendorID = vendorID;
            __enumerated_devices[i].productID = productID;
            __enumerated_device_count++;
            return &(__enumerated_devices[i]);
        }
    }
    return NULL;
}

static void __purge_unmarked_device_instances(int vendorID, int productID) {
    int new_count = 0;
    int valid = 0;
    int i;
    __device_instance_t *device;

    for(i = 0; i < MAX_USB_DEVICES && valid < __enumerated_device_count; i++) {
        device = &(__enumerated_devices[i]);
        if(0 == device->valid) {
            continue;
        }
        /* From here down, the device entry was thought to be valid */
        valid++;

        /* Now check whether it was marked as still being present.  Note that
         * this search is limited just to the type of device that was being
         * probed when this call was made -- otherwise, any devices that were
         * not being probed for just now would be unmarked and would be
         * discarded.
         */
        if(0 == device->mark
                && (vendorID == device->vendorID)
                && (productID == device->productID)) {
            /* Not marked, so it needs to be purged */
            if(NULL != device->handle) {
                /* Clean up the device since it seems to have been disconnected */
                __close_and_dealloc_usb_interface(device->handle);
            }
            /* Unreference device */
            libusb_unref_device(device->device);
            /* Wipe the structure completely */
            memset(&__enumerated_devices[i], (int)0, sizeof(__device_instance_t));
        } else {
            /* Device was valid or a different type so it survived this pass */
            __enumerated_devices[i].mark = 0;
            /* Keep track of how many survived */
            new_count++;
        }
    }
    __enumerated_device_count = new_count;
}


/* This will attempt to free up all resources associated with an open
 * USB descriptor.  This also deallocates the provided pointer.
 */
static void __close_and_dealloc_usb_interface(__usb_interface_t *usb) {
    if(NULL == usb) {
        return;
    }

    if(NULL != usb->dev) {
        /* MZ: This call to usb_reset() resolves a reported issue in which Linux apps 
         * would run correctly once, then require spectrometer to be un/replugged to 
         * run a second time.
         */
        libusb_reset_device(usb->dev);
        /* Release interface */
        libusb_release_interface(usb->dev, usb->interface);
        /* Close device */
        libusb_close(usb->dev);
    }

    free(usb);
}


int
USBProbeDevices(int vendorID, int productID, unsigned long *output, int max_devices) {

    /* Local variables */
    libusb_device **dev_list;
    struct libusb_device_descriptor desc;
    __device_instance_t *instance;
    int i;
    int matched = 0;
    int valid = 0;
    int r = 0;
    int num_dev = 0;

    /* Check if libusb_init() has been called since it must be called before
     * anything else happens.  This will be checked here, but not in any of
     * the other functions because it only needs to happen once and this function
     * is the entry point into the API.
     */
    if(NULL == __libusb_ctx) {
        r = libusb_init(&__libusb_ctx);
        if(r < 0) { // Init failed
            fprintf(stderr, "libusb_init() failed (Error: %s)", libusb_strerror(r));
            return -1;
        }
    }

    /* Get device list */
    num_dev = libusb_get_device_list(__libusb_ctx, &dev_list);
    if(num_dev < 0) {
        fprintf(stderr, "libusb_get_device_list() failed (Error: %s)", libusb_strerror(num_dev));
        return -1;
    }

    /* Traverse the tree and update the local cached device table as needed. */
    for(i = 0; i < num_dev; i++) {

        // Clear struct
        memset(&desc, 0, sizeof(desc));

        // Get device descriptor
        r = libusb_get_device_descriptor(dev_list[i], &desc);
        if(r < 0) {
            /* Error. Skip device */
            continue;
        }

        if(desc.idVendor == vendorID && desc.idProduct == productID) {
            /* Got a matching device node.  Determine if this is
             * already in the cache.
             */
            instance = __lookup_device_instance_by_location(dev_list[i]);
            if(NULL != instance) {
                /* Device is already known, so mark it and keep going */
                instance->mark = 1;
                continue;
            }

            /* At this point, we must be dealing with a newly discovered USB
             * device that matches the given VID and PID.  It must now be
             * cached for use with the open function.  Note that instance was
             * checked above so it must be NULL here.
             */
            libusb_ref_device(dev_list[i]);
            instance = __add_device_instance(dev_list[i], vendorID, productID);
            if(NULL == instance) {
                /* Could not add the device -- this should not be possible, so bail out. */
                return -1;
            }
            instance->mark = 1;     /* Preserve this since it was just seen */
        }
    }
    /* free the list, unref the devices in it */
    libusb_free_device_list(dev_list, 1);

    /* Purge any devices that are cached but that no longer exist. */
    __purge_unmarked_device_instances(vendorID, productID);

    /* Count up how many of this type of device are known */
    for(i = 0, matched = 0, valid = 0; i < MAX_USB_DEVICES && valid < __enumerated_device_count; i++) {
        if(0 != __enumerated_devices[i].valid) {
            valid++;
            if(__enumerated_devices[i].vendorID == vendorID &&
               __enumerated_devices[i].productID == productID) {
                matched++;
            }
        }
    }

    for(i = 0, valid = 0; i < MAX_USB_DEVICES && valid < matched && valid < max_devices; i++) {
        if(0 != __enumerated_devices[i].valid
                && __enumerated_devices[i].vendorID == vendorID
                && __enumerated_devices[i].productID == productID) {
            output[valid] = __enumerated_devices[i].deviceID;
            valid++;
        }
    }

    return valid;
}

void *
USBOpen(unsigned long deviceID, int *errorCode) {
    // Local variables
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config = NULL;
    libusb_device_handle *deviceHandle = NULL;
    __usb_interface_t *retval;
    __device_instance_t *instance;
    int interface = 0;
    int r = 0;

    /* Set a default error code in case a premature return is required */
    SET_ERROR_CODE(NO_DEVICE_FOUND);

    /* Try to look up the deviceID provided by the caller.  If this does
     * not exist, then there is nothing else to do.
     */
    instance = __lookup_device_instance_by_ID(deviceID);
    if(NULL == instance) {
        /* The device ID was not found.  The caller must only provide IDs that
         * have previously been provided by the USBProbeDevices() function.
         */
        return NULL;
    }

    if(NULL != instance->handle) {
        /* If there is already a device handle then the device has been opened.
         * It is illegal to try to open a device twice without first closing it.
         */
        return NULL;
    }

    r = libusb_open(instance->device, &deviceHandle);
    if(r < 0) {
        /* Could not open device */
        fprintf(stderr, "libusb_open() failed (Error: %s)", libusb_strerror(r));
        return NULL;
    }

    /* Get device descriptor */
    r = libusb_get_device_descriptor(instance->device, &desc);
    if(r < 0) {
        fprintf(stderr, "libusb_get_device_descriptor() failed (Error: %s)", libusb_strerror(r));
        return NULL;
    }

    /* Get configuration descritor */
    if(desc.bNumConfigurations < 1) {
        fprintf(stderr, "USB device has no configurations available. Cannot claim interface");
        return NULL;
    }
    if(desc.bNumConfigurations > 1) {
        fprintf(stderr, "Warning: USB device has more than one configuration available. Claim interface of the first one.");
    }
    r = libusb_get_config_descriptor(instance->device, 0, &config);
    if(r < 0) {
        fprintf(stderr, "libusb_get_config_descriptor() failed (Error: %s)", libusb_strerror(r));
        return NULL;
    }
    if(config->bNumInterfaces < 1) {
        fprintf(stderr, "Configuration has no interfaces. Cannot claim.");
        libusb_free_config_descriptor(config);
        return NULL;
    }
    interface = config->interface[0].altsetting[0].bInterfaceNumber;
    libusb_free_config_descriptor(config);

    /* Claim interface */
    r = libusb_claim_interface(deviceHandle, interface);
    if(r < 0) {
         /* Could not claim interface */
        fprintf(stderr, "usb_claim_interface() failed (Error: %s)\nDid you copy os-support/linux/10-oceanoptics.rules to /etc/udev/rules.d?\n", libusb_strerror(r));
        return NULL;
    }

    retval = (__usb_interface_t *)calloc(sizeof(__usb_interface_t), 1);
    if(NULL == retval) {
        libusb_release_interface(deviceHandle, interface);
        libusb_close(deviceHandle);
        /* Could not allocate memory */
        SET_ERROR_CODE(CLAIM_INTERFACE_FAILED);
        return 0;
    }
    retval->interface = interface;
    retval->dev = deviceHandle;
    retval->deviceID = instance->deviceID;
    instance->handle = retval;

    SET_ERROR_CODE(OPEN_OK);
    return (void *)retval;
}

int
USBWrite(void *deviceHandle, unsigned char endpoint, char *data, int numberOfBytes) {
    /* Local variables */
    int retval;
    int bytesWritten;
    __usb_interface_t *usb;

    if(0 == deviceHandle) {
        return WRITE_FAILED;
    }

    usb = (__usb_interface_t *)deviceHandle;

    /*
     * Perform the write.  This is effectively blocking (the timeout is large)
     */
    retval = libusb_bulk_transfer(usb->dev, endpoint, data, numberOfBytes, &bytesWritten, DEFAULT_TIMEOUT);
    if(retval < 0 || (0 == bytesWritten && 0 != numberOfBytes)) {
        /* Transfer error */
        retval = WRITE_FAILED;
    } else {
        retval = bytesWritten;
    }
    return retval;
}

int
USBRead(void *deviceHandle, unsigned char endpoint, char * data, int numberOfBytes) {
    /* Local variables */
    int retval;
    int bytesRead;
    __usb_interface_t *usb;

    if(0 == deviceHandle) {
        return READ_FAILED;
    }

    usb = (__usb_interface_t *)deviceHandle;

    /*
     * Perform the read.  This is effectively blocking (the timeout is large)
     */
    retval = libusb_bulk_transfer(usb->dev, endpoint, data, numberOfBytes, &bytesRead, DEFAULT_TIMEOUT);

    if(retval < 0 || (0 == bytesRead && 0 != numberOfBytes)) {
        retval = READ_FAILED;
    } else {
        retval = bytesRead;
    }
    return retval;
}

int
USBClose(void *deviceHandle) {
    /* Local variables */
    __usb_interface_t *usb;
    __device_instance_t *device;

    if(NULL == deviceHandle) {
        return CLOSE_ERROR;
    }

    usb = (__usb_interface_t *)deviceHandle;

    device = __lookup_device_instance_by_ID(usb->deviceID);
    if(NULL != device) {
        /* This had an extra reference to the handle so free it up */
        device->handle = NULL;
    }

    __close_and_dealloc_usb_interface(usb);
    return CLOSE_OK;
}

void USBClearStall(void *deviceHandle, unsigned char endpoint) {
    __usb_interface_t *usb;

    if(0 == deviceHandle) {
        return;
    }

    usb = (__usb_interface_t *)deviceHandle;

    libusb_clear_halt(usb->dev, endpoint);
}

int
USBGetDeviceDescriptor(void *deviceHandle, struct USBDeviceDescriptor *desc) {
    struct libusb_device_descriptor dd;
    __usb_interface_t *usb;
    __device_instance_t *device;
    int r = 0;

    if(0 == desc) {
        return -1;
    }

    if(0 == deviceHandle) {
        return -2;
    }

    usb = (__usb_interface_t *)deviceHandle;
    device = __lookup_device_instance_by_ID(usb->deviceID);
    if(0 == device) {
        return -2;
    }

    /* Get device descriptor */
    r = libusb_get_device_descriptor(device->device, &dd);
    if(r < 0) {
        fprintf(stderr, "libusb_get_device_descriptor() failed (Error: %s)", libusb_strerror(r));
        return -3;
    }

    desc->bLength = dd.bLength;
    desc->bDescriptorType = dd.bDescriptorType;
    desc->bcdUSB = dd.bcdUSB;
    desc->bDeviceClass = dd.bDeviceClass;
    desc->bDeviceSubClass = dd.bDeviceSubClass;
    desc->bDeviceProtocol = dd.bDeviceProtocol;
    desc->bMaxPacketSize0 = dd.bMaxPacketSize0;
    desc->idVendor = dd.idVendor;
    desc->idProduct = dd.idProduct;
    desc->bcdDevice = dd.bcdDevice;
    desc->iManufacturer = dd.iManufacturer;
    desc->iProduct = dd.iProduct;
    desc->iSerialNumber = dd.iSerialNumber;
    desc->bNumConfigurations = dd.bNumConfigurations;

    return 0;
}

int
USBGetInterfaceDescriptor(void *deviceHandle, struct USBInterfaceDescriptor *desc) {
    struct libusb_device_descriptor dd;
    struct libusb_config_descriptor *config = NULL;
    const struct libusb_interface_descriptor *id = NULL;
    __usb_interface_t *usb;
    __device_instance_t *device;
    int r = 0;

    if(0 == desc) {
        return -1;
    }

    if(0 == deviceHandle) {
        return -2;
    }

    usb = (__usb_interface_t *)deviceHandle;
    device = __lookup_device_instance_by_ID(usb->deviceID);
    if(0 == device) {
        return -2;
    }

    /* Get device descriptor */
    r = libusb_get_device_descriptor(device->device, &dd);
    if(r < 0) {
        fprintf(stderr, "libusb_get_device_descriptor() failed (Error: %s)", libusb_strerror(r));
        return -3;
    }

    /* Get configuration descritor */
    if(dd.bNumConfigurations < 1) {
        fprintf(stderr, "USB device has no configurations available. Cannot claim interface");
        return -3;
    }
    if(dd.bNumConfigurations > 1) {
        fprintf(stderr, "Warning: USB device has more than one configuration available. Get descriptor of the first one.");
    }
    r = libusb_get_config_descriptor(device->device, 0, &config);
    if(r < 0) {
        fprintf(stderr, "libusb_get_config_descriptor() failed (Error: %s)", libusb_strerror(r));
        return -3;
    }
    if(config->bNumInterfaces < 1) {
        fprintf(stderr, "Configuration has no interfaces. Cannot claim.");
        libusb_free_config_descriptor(config);
        return -3;
    }
    id = &(config->interface[0].altsetting[0]);

    /* FIXME: are there more than one altsetting that should be reachable? */
    desc->bLength = id->bLength;
    desc->bDescriptorType = id->bDescriptorType;
    desc->bInterfaceNumber = id->bInterfaceNumber;
    desc->bAlternateSetting = id->bAlternateSetting;
    desc->bNumEndpoints = id->bNumEndpoints;
    desc->bInterfaceClass = id->bInterfaceClass;
    desc->bInterfaceSubClass = id->bInterfaceSubClass;
    desc->bInterfaceProtocol = id->bInterfaceProtocol;
    desc->iInterface = id->iInterface;

    libusb_free_config_descriptor(config);

    return 0;
}


int
USBGetEndpointDescriptor(void *deviceHandle, int endpoint_index,
        struct USBEndpointDescriptor *desc) {
    struct libusb_device_descriptor dd;
    struct libusb_config_descriptor *config = NULL;
    const struct libusb_endpoint_descriptor* ed = NULL;
    __usb_interface_t *usb;
    __device_instance_t *device;
    int r = 0;

    if(0 == desc) {
        return -1;
    }

    if(0 == deviceHandle) {
        return -2;
    }

    usb = (__usb_interface_t *)deviceHandle;
    device = __lookup_device_instance_by_ID(usb->deviceID);
    if(0 == device) {
        return -2;
    }

    /* Get device descriptor */
    r = libusb_get_device_descriptor(device->device, &dd);
    if(r < 0) {
        fprintf(stderr, "libusb_get_device_descriptor() failed (Error: %s)", libusb_strerror(r));
        return -3;
    }

    /* Get configuration descritor */
    if(dd.bNumConfigurations < 1) {
        fprintf(stderr, "USB device has no configurations available. Cannot claim interface");
        return -3;
    }
    if(dd.bNumConfigurations > 1) {
        fprintf(stderr, "Warning: USB device has more than one configuration available. Get descriptor of the first one.");
    }
    r = libusb_get_config_descriptor(device->device, 0, &config);
    if(r < 0) {
        fprintf(stderr, "libusb_get_config_descriptor() failed (Error: %s)", libusb_strerror(r));
        return -3;
    }
    if(config->bNumInterfaces < 1) {
        fprintf(stderr, "Configuration has no interfaces. Cannot claim.");
        libusb_free_config_descriptor(config);
        return -3;
    }

    /* FIXME: Deal with alternate endpoints or interfaces? */
    ed = &(config->interface[0].altsetting[0].endpoint[endpoint_index]);

    desc->bLength = ed->bLength;
    desc->bDescriptorType = ed->bDescriptorType;
    desc->bEndpointAddress = ed->bEndpointAddress;
    desc->bmAttributes = ed->bmAttributes;
    desc->wMaxPacketSize = ed->wMaxPacketSize;
    desc->bInterval = ed->bInterval;

    libusb_free_config_descriptor(config);

    return 0;
}


int
USBGetStringDescriptor(void *deviceHandle, unsigned int string_index,
        char *buffer, int maxLength) {
    /* Local variables */
    int length = 0;
    __usb_interface_t *usb;

    if(0 == deviceHandle || 0 == buffer) {
        /* Invalid device handle or buffer */
        return 0;
    }

    usb = (__usb_interface_t *)deviceHandle;

    /* Obtain the string and return it */
    length = libusb_get_string_descriptor_ascii(usb->dev, string_index, buffer, maxLength);
    if(length <= 0) {
        buffer[0] = '\0';
    }

    return length;
}
