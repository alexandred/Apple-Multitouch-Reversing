//
//  AppleMultitouchTrackpadHIDEventDriver.cpp
//  Apple Multitouch Reversing
//
//  Created by Alexandre on 12/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "AppleMultitouchTrackpadHIDEventDriver.hpp"

#define super AppleMultitouchInputHIDEventDriver

OSDefineMetaClassAndStructors(AppleMultitouchTrackpadHIDEventDriver, AppleMultitouchInputHIDEventDriver);

bool AppleMultitouchTrackpadHIDEventDriver::attach(IOService* provider) {
    if (!super::attach(provider))
        return false;

    return true;
}

void AppleMultitouchTrackpadHIDEventDriver::detach(IOService* provider) {
    super::detach(provider);
}

bool AppleMultitouchTrackpadHIDEventDriver::init(OSDictionary* properties) {
    if (!super::init(properties)) {
        IOLog("[HID] [%s] [Error] %s::%s super::init() failed\n", "ATC", "AppleMultitouchTrackpadHIDEventDriver", "init");
        return false;
    }
    
    trackpad_preferences = 0x0;
    mouse_keys_stops_trackpad = false;
    *(int8_t *)(rbx + 0x189) = 0x0;

    return true;
}

void AppleMultitouchTrackpadHIDEventDriver::free() {
    if (usb_mouse_first_match_notifier) {
        usb_mouse_first_match_notifier->remove();
        usb_mouse_first_match_notifier = NULL;
    }

    if (usb_mouse_terminated_notifier) {
        usb_mouse_terminated_notifier->remove();
        usb_mouse_terminated_notifier = NULL;
    }
    super::free();
}

bool AppleMultitouchTrackpadHIDEventDriver::start(IOService* provider) {
    if (!super::start(provider)) {
        IOLog("[HID] [%s] [Error] %s::%s super::start failed\n", "ATC", "AppleMultitouchTrackpadHIDEventDriver", "start");
        super::stop(provider);
        return false;
    }

    return true;
}

void AppleMultitouchTrackpadHIDEventDriver::stop(IOService* provider) {
    super::stop(provider);
}

int AppleMultitouchTrackpadHIDEventDriver::CheckOtherMouseEntry(OSObject* target, void* refCon, IOService* newService, IONotifier *notifier) {
    AppleMultitouchTrackpadHIDEventDriver* us = OSDynamicCast(AppleMultitouchTrackpadHIDEventDriver, target);
    if (us) {
        us->checkOtherMouse();
    }
    return 0x1;
}

int AppleMultitouchTrackpadHIDEventDriver::checkOtherMouse() {
    OSBoolean* is_embedded = OSDynamicCast(OSBoolean, getProperty("TrackpadEmbedded"));
    if (is_embedded != kOSBooleanTrue)
        return 0;
    
    OSDictionary* matched_IOHIPointing = IOService::serviceMatching("IOHIPointing", 0x0);
    if (!matched_IOHIPointing)
        return 0;

    OSDictionary* temp = OSDictionary::withCapacity(0x1);
    if (temp) {
        temp->setObject("HIDVirtualDevice", kOSBooleanFalse);
        matched_IOHIPointing->setObject("IOPropertyMatch", temp);
        temp->release();
    }
    
    OSIterator* matching_services = IOService::getMatchingServices(matched_IOHIPointing);
    if (!matching_services) {
        matched_IOHIPointing->release();
        return 0;
    }

    IOService* current_matched = OSDynamicCast(IOService, matching_services->getNextObject());
    if (!current_matched) {
        if (trackpad_preferences->disabled_by_external_mouse) {
            setBuiltinTrackpadDisablingMouseAttached(true);
            matching_services->release();
            return 0;
        }
    }

    int match_count = 0;
    do {
        if (current_matched->getProperty("TrackpadEmbedded"))
            ++match_count;
        current_matched = OSDynamicCast(IOService, matching_services->getNextObject());
    } while (current_matched);
            
    if (match_count < 0) {
        matching_services->release();
        matched_IOHIPointing->release();
        return 0;
    }
    
    
    if (match_count == 0) {
        if (!trackpad_preferences->disabled_by_external_mouse) {
            matching_services->release();
            matched_IOHIPointing->release();
            return 0;
        } else {
            setBuiltinTrackpadDisablingMouseAttached(true);
            matching_services->release();
            matched_IOHIPointing->release();
            return 0;
        }
    }
    
    if (!trackpad_preferences->disabled_by_external_mouse) {
        setBuiltinTrackpadDisablingMouseAttached(true);
    }
    
    matching_services->release();
    matched_IOHIPointing->release();
    return 0;

}

int AppleMultitouchTrackpadHIDEventDriver::attachToChild(IORegistryEntry* child, IORegistryPlane* plane) {
    bool successful_attach = IORegistryEntry::attachToChild(child, plane);
    IOHIPointing* child_IOHIPointing = OSDynamicCast(IOHIPointing, child);
    if ((successful_attach && child_IOHIPointing) && (IORegistryEntry::getPlane("IOService") == plane)) {
        if (OSDynamicCast(OSBoolean, getProperty("TrackpadEmbedded")) == kOSBooleanTrue) {
            child->setProperty("TrackpadEmbedded", 0x1);
        }
    }
    return successful_attach;
}


int AppleMultitouchTrackpadHIDEventDriver::setSystemProperties(OSDictionary* properties) {
    int ret;
    if (OSDynamicCast(OSBoolean, getProperty("TrackpadEmbedded")) == kOSBooleanTrue) {
        OSNumber* num = OSDynamicCast(OSNumber, properties->getObject("USBMouseStopsTrackpad"));
        if (num) {
            ret = num->unsigned8BitValue();
            if (ret != usb_mouse_stops_trackpad) {
                usb_mouse_stops_trackpad = COND_BYTE_SET(NE); // i.e set it to ret
                if (!ret) {
                    setBuiltinTrackpadDisablingMouseAttached(false);
                    if (usb_mouse_first_match_notifier) {
                        usb_mouse_first_match_notifier->remove();
                        usb_mouse_first_match_notifier = 0x0;
                    }
                    if (usb_mouse_terminated_notifier) {
                        usb_mouse_terminated_notifier->remove();
                        usb_mouse_terminated_notifier = 0x0;
                    }
                }
            }
        }
        num = OSDynamicCast(OSNumber, properties->getObject("MouseKeysStopsTrackpad"));
        ret = num->unsigned8BitValue();
        if (num) {
            mouse_keys_stops_trackpad = (ret ? true : false);
        }
        num = OSDynamicCast(OSNumber, properties->getObject("HIDMouseKeysOn"));
        if (ret) {
            ret = num->unsigned8BitValue();
            trackpad_preferences->mouse_keys_enabled = (ret ? true : false);
            ret = COND_BYTE_SET(NE); // ie set it to ret
        } else {
            ret = trackpad_preferences->mouse_keys_enabled;
        }
        setBuiltinTrackpadDisablingMouseKeysEnabled(mouse_keys_stops_trackpad);
    }
    ret = IOHIDEventService::setSystemProperties(properties);
    return ret;
}

int AppleMultitouchTrackpadHIDEventDriver::handleInterruptReportEyP18IOMemoryDescriptor15IOHIDReportTypej(AbsoluteTime timeStamp, IOMemoryDescriptor* report, IOHIDReportType reportType, UInt32 reportID) {
    r13 = r8;
    r14 = arg3;
    r12 = arg2;
    rbx = arg1;
    r15 = arg0;
    
    if (report) {
        if (_inputReportBuffer) {
            IOByteCount report_length = report->getLength();
            if (report_length) {
                var_38 = rbx; //timestamp
                var_44 = r14; //reportID
                r14 = (*(**(r15 + 0x138) + 0x2e0))(*(r15 + 0x138));
                var_50 = r14;
                rbx = (*(*r12 + 0x1c8))(r12);
                var_40 = rbx;
                (*(*r12 + 0x1e0))(r12, 0x0, r14, rbx, *r12);
                _memset(&var_30, 0x0, 0x8);
                if (*(int8_t *)(r15 + 0x188) != 0x0) {
                    if (*(r15 + 0x178) == 0x0) {
                        *(r15 + 0x178) = IOService::addMatchingNotification(*_gIOFirstMatchNotification, IOService::serviceMatching("IOHIPointing", 0x0), AppleMultitouchTrackpadHIDEventDriver::CheckOtherMouseEntry(OSObject*, void*, IOService*, IONotifier*), r15, 0x0, 0x0);
                    }
                    if (*(r15 + 0x180) == 0x0) {
                        *(r15 + 0x180) = IOService::addMatchingNotification(*_gIOTerminatedNotification, IOService::serviceMatching("IOHIPointing", 0x0), AppleMultitouchTrackpadHIDEventDriver::CheckOtherMouseEntry(OSObject*, void*, IOService*, IONotifier*), r15, 0x0, 0x0);
                    }
                }
                (*(**(r15 + 0x138) + 0x2c8))(*(r15 + 0x138), var_40);
                r14 = var_44;
                if (r13 == 0x2) {
                    r12 = 0x2;
                    if (rbx > 0x8) {
                        if ((*(*r15 + 0xb08))(r15) != 0x0) {
                            rax = __atc_extractTrackingAndButtonState();
                            if (rax != 0x0) {
                                rcx = var_2F;
                                if ((*(int8_t *)(r15 + 0x170) & rcx) != 0x0) {
                                    rcx = rcx & 0xfc | 0x2;
                                }
                                var_2F = rcx;
                                if (((*(int8_t *)(r15 + 0x171) == 0x0) && (*(int8_t *)(r15 + 0x172) == 0x0)) && ((var_2A & 0x2) == 0x0)) {
                                    r12 = rax;
                                    rax = (*(*r15 + 0xb08))(r15);
                                    (*(*rax + 0x8e0))(rax, sign_extend_64(0x0), sign_extend_64(var_2D), var_2F & 0xff, 0x0, *rax);
                                    rax = r12;
                                }
                                if ((*(int8_t *)(r15 + 0x189) == 0x0) && ((var_2A & 0x2) != 0x0)) {
                                    r12 = rax;
                                    rax = (*(*r15 + 0xb08))(r15);
                                    (*(*rax + 0x8e0))(rax, 0x0, 0x0, 0x0, 0x0, *rax);
                                    rax = r12;
                                }
                                *(int8_t *)(r15 + 0x189) = var_2A >> 0x1 & 0x1;
                            }
                        }
                        else {
                            rax = 0x0;
                        }
                        r12 = r13;
                        if (r13 == 0x2) {
                            if (rax == 0x0) {
                                (*(**(r15 + 0x138) + 0x2c8))(*(r15 + 0x138), 0x8);
                                AppleMultitouchHIDEventDriverV2::handleInterruptReport(r15, var_38, *(r15 + 0x138), r14, 0x2);
                                (*(**(r15 + 0x138) + 0x2c8))(*(r15 + 0x138), var_40);
                            }
                            r12 = *(int8_t *)(var_50 + 0x8) & 0xff;
                            _memmove(var_50, var_50 + 0x8, 0xfffffff8 + rbx & 0xffffffff);
                            (*(**(r15 + 0x138) + 0x2c8))(*(r15 + 0x138), 0xfffffff8 + rbx & 0xffffffff);
                        }
                    }
                }
                else {
                    if ((r13 != 0x31) && (r13 != 0x75)) {
                        r12 = r13;
                    }
                    else {
                        if ((*(*r15 + 0xb08))(r15) != 0x0) {
                            rax = __atc_extractTrackingAndButtonState();
                            if (rax != 0x0) {
                                rcx = var_2F;
                                if ((*(int8_t *)(r15 + 0x170) & rcx) != 0x0) {
                                    rcx = rcx & 0xfc | 0x2;
                                }
                                var_2F = rcx;
                                if (((*(int8_t *)(r15 + 0x171) == 0x0) && (*(int8_t *)(r15 + 0x172) == 0x0)) && ((var_2A & 0x2) == 0x0)) {
                                    r12 = rax;
                                    rax = (*(*r15 + 0xb08))(r15);
                                    (*(*rax + 0x8e0))(rax, sign_extend_64(0x0), sign_extend_64(var_2D), var_2F & 0xff, 0x0, *rax);
                                    rax = r12;
                                }
                                if ((*(int8_t *)(r15 + 0x189) == 0x0) && ((var_2A & 0x2) != 0x0)) {
                                    r12 = rax;
                                    rax = (*(*r15 + 0xb08))(r15);
                                    (*(*rax + 0x8e0))(rax, 0x0, 0x0, 0x0, 0x0, *rax);
                                    rax = r12;
                                }
                                *(int8_t *)(r15 + 0x189) = var_2A >> 0x1 & 0x1;
                            }
                        }
                        else {
                            rax = 0x0;
                        }
                        r12 = r13;
                        if (r13 == 0x2) {
                            if (rax == 0x0) {
                                (*(**(r15 + 0x138) + 0x2c8))(*(r15 + 0x138), 0x8);
                                AppleMultitouchHIDEventDriverV2::handleInterruptReport(r15, var_38, *(r15 + 0x138), r14, 0x2);
                                (*(**(r15 + 0x138) + 0x2c8))(*(r15 + 0x138), var_40);
                            }
                            r12 = *(int8_t *)(var_50 + 0x8) & 0xff;
                            _memmove(var_50, var_50 + 0x8, 0xfffffff8 + rbx & 0xffffffff);
                            (*(**(r15 + 0x138) + 0x2c8))(*(r15 + 0x138), 0xfffffff8 + rbx & 0xffffffff);
                        }
                    }
                }
                rax = AppleMultitouchHIDEventDriverV2::handleInterruptReport(r15, var_38, *(r15 + 0x138), r14, r12);
            }
        }
    }
    return rax;
}

int AppleMultitouchTrackpadHIDEventDriver::setBuiltinTrackpadDisablingMouseAttached(bool enabled_state) {
    int ret;
    bool trackpad_is_disabled = true;
    if (!trackpad_preferences->disabled_by_external_mouse) {
        trackpad_is_disabled = trackpad_preferences->disabled_by_mouse_keys;
    }
    trackpad_preferences->disabled_by_external_mouse = enabled_state;
    if (enabled_state) {
        if (!trackpad_is_disabled) {
            AppleMultitouchDevice* multitouch_device = getMultitouchDevice();
            if (multitouch_device)
                ret = multitouch_device->setDisablerPresent(true);
        }
    } else {
        trackpad_is_disabled = (trackpad_is_disabled ^ trackpad_preferences->disabled_by_mouse_keys); // i.e true if exactly one is true
        if (trackpad_is_disabled) {
            AppleMultitouchDevice* multitouch_device = getMultitouchDevice();
            if (multitouch_device) {
                ret = multitouch_device->setDisablerPresent(trackpad_is_disabled);
            }
        }
    }
    return ret;
}

int AppleMultitouchTrackpadHIDEventDriver::setBuiltinTrackpadDisablingMouseKeysEnabled(bool enabled_state) {
    int ret;
    if (trackpad_preferences->disabled_by_external_mouse) {
        trackpad_preferences->disabled_by_mouse_keys = enabled_state;
    } else {
        bool should_we_disable = trackpad_preferences->disabled_by_mouse_keys ^ enabled_state; // i.e true if exactly one is true
        trackpad_preferences->disabled_by_mouse_keys = enabled_state;
        if (should_we_disable) {
            AppleMultitouchDevice* multitouch_device = getMultitouchDevice();
            if (multitouch_device) {
                ret = multitouch_device->setDisablerPresent(should_we_disable);
            }
        }
    }
    return ret;
}
