//
//  AppleMultitouchInputHIDEventDriver.cpp
//  Apple Multitouch Reversing
//
//  Created by Alexandre on 12/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

//sizeof(OSClassMetaBase) = 0x8
//sizeof(OSObject) = 0x10
//sizeof(IORegistryEntry) = 0x28
//sizeof(IOService) = 0x88
//sizeof(IOHIDEventService) = 0xc8
//sizeof(IOHIDEventDriver) = 0x108

#include "AppleMultitouchInputHIDEventDriver.hpp"

#define super AppleMultitouchHIDEventDriverV2
OSDefineMetaClassAndStructors(AppleMultitouchInputHIDEventDriver, AppleMultitouchHIDEventDriverV2);


bool AppleMultitouchInputHIDEventDriver::attach(IOService* provider) {
    if (!super::attach(provider))
        return false;

    return true;
}

void AppleMultitouchInputHIDEventDriver::detach(IOService* provider) {
    super::detach(provider);
}


bool AppleMultitouchInputHIDEventDriver::init(OSDictionary* properties) {
    if (!super::init(properties)) {
        IOLog("[HID] [??] [Error] AppleMultitouchInputHIDEventDriver::init super::init() failed\n");
        return false;
    }
    
    if (!(_inputReportBuffer = IOBufferMemoryDescriptor::withCapacity(0x1000, kIODirectionOutIn, 0x0))) {
        IOLog("[HID] [??] [Error] AppleMultitouchInputHIDEventDriver::init Failed to allocate _inputReportBuffer\n");
        return false;
    }
    
    if (!(_mtPreferences = OSDictionary::withCapacity(0x1))) {
        IOLog("[HID] ?? [Error] AppleMultitouchInputHIDEventDriver::init Failed to allocate _mtPreferences\n");
        return false;
    }
        
    if (!(_mtPrefsLock = IORecursiveLockAlloc())) {
        IOLog("[HID] ?? [Error] AppleMultitouchInputHIDEventDriver::init Failed to allocate _mtPrefsLock\n");
        return false;
    }
        
    if (!(_unleashThread = thread_call_allocate((thread_call_func_t)AppleMultitouchInputHIDEventDriver::unleashThreadEnter(), (thread_call_param_t)this))) {
        IOLog("[HID] ?? [Error] AppleMultitouchInputHIDEventDriver::init Failed to allocate _unleashThread\n");
        return false;
    }

    return true;
}

void AppleMultitouchInputHIDEventDriver::free() {
    if (_unleashThread) {
        thread_call_free();
        _unleashThread = 0x0;
    }

    if (_mtPrefsLock) {
        IORecursiveLockFree(_mtPrefsLock);
        _mtPrefsLock = 0x0;
    }

    if (_mtPreferences) {
        _mtPreferences->release();
        _mtPreferences = 0x0;
    }

    if (_inputReportBuffer) {
        _inputReportBuffer->release();
        _inputReportBuffer = 0x0;
    }
    super::free();
}


bool AppleMultitouchInputHIDEventDriver::start(IOService* provider) {
    if (!super::start(provider))
        return false;
    int ret = getMultitouchReport(0x0, &(0x0), &(0x1), 0x3);

    if (ret) {
        IOLog("[HID] [%s] [Error] %s::%s Failed to get device initialized state. Result = 0x%08x\n", "ATC", "AppleMultitouchInputHIDEventDriver", "start", ret);
    } else if (0 != 0) {
            // (*(*rbx + 0xb40))(rbx); // not defined it seems
    }
    
    _notifier = _registerPrioritySleepWakeInterest(OSMemberFunctionCast(IOServiceInterestHandler, this, AppleMultitouchInputHIDEventDriver::systemPowerChangeHandler), this, 0x0);

    if (!_notifier) {
        detach(provider); // or maybe stop
        IOLog("[HID] ?? [Error] AppleMultitouchInputHIDEventDriver::start registerPrioritySleepWakeInterest failed\n");
        return false;
    }

    return true;
}

void AppleMultitouchInputHIDEventDriver::stop(IOService* provider) {
    if (_notifier) {
        _notifier->remove(); // or disable() but less likely
        _notifier = 0x0;
    }
    super::stop(provider);
}

int AppleMultitouchInputHIDEventDriver::setProperties(OSObject* properties) {
    int ret;
    bool user_preferences_set;

    OSDictionary* dictionary = OSDynamicCast(OSDictionary, properties);
    
    OSBoolean* user_preferences_bool;
    OSNumber* user_preferences_num;

    if (dictionary) {
        user_preferences_bool = OSDynamicCast(OSBoolean, dictionary->getObject("UserPreferences"));
        if (user_preferences_bool) {
            user_preferences_set = (user_preferences_bool == kOSBooleanTrue) ? true : false;
        }
        else {
            user_preferences_set = false;
        }
                   
        user_preferences_num = OSDynamicCast(OSNumber, dictionary->getObject("UserPreferences"));
        if (user_preferences_num) {
            user_preferences_set = (user_preferences_num->unsigned8BitValue() == 1) ? true : false;
        }
        if (!user_preferences_set) {
            ret = setProperties(properties);
        } else {
            IOWorkLoop* work_loop = getWorkLoop();
            if (work_loop) {
                ret = work_loop->runAction(OSMemberFunctionCast(IOWorkLoop::Action, this, &AppleMultitouchInputHIDEventDriver::setMultitouchPreferences), this, dictionary);
            }
            ret = 0;
        }
    }
    else {
        return super::setProperties(properties);
    }
    return ret;
}
                                    
int AppleMultitouchInputHIDEventDriver::message(unsigned int arg0, void * arg1, void * arg2) {
    return 0x0;
}

IOReturn AppleMultitouchInputHIDEventDriver::multitouchDeviceDidStart() {
    AppleMultitouchDevice* multitouch_device = getMultitouchDevice();
    IOReturn ret;
    ret = kIOReturnNoDevice;
    if (multitouch_device) {
        IORecursiveLockLock(_mtPrefsLock);
        multitouch_device->setPreferences(_mtPreferences);
        _mtPreferences->flushCollection(); // not 100% sure, need to see vftables for OSDictionary
        IORecursiveLockUnlock(_mtPrefsLock);
        ret = 0;
    }
    return ret;
}

IOReturn AppleMultitouchInputHIDEventDriver::setMultitouchPreferences(OSDictionary* properties) {
    IOReturn ret = kIOReturnOffline;
    if (!IOService::isInactive()) {
        ret = kIOReturnBadArgument;
        if (properties) {
            IORecursiveLockLock(_mtPrefsLock);
            AppleMultitouchDevice* multitouch_device = getMultitouchDevice();
            if (multitouch_device)
                multitouch_device->setPreferences(properties);
            else
                _mtPreferences->removeObject(properties);
            IORecursiveLockUnlock(_mtPrefsLock);
            ret = kIOReturnSuccess;
        }
    }
    return rax;
}

int AppleMultitouchInputHIDEventDriver::enableMultitouchEvents(bool enabled) {
    int ret;
    IOWorkLoop* work_loop = getWorkLoop();
    if (work_loop)
        ret = work_loop->runAction(OSMemberFunctionCast(IOWorkLoop::Action, this, &AppleMultitouchInputHIDEventDriver::enableMultitouchEventsGated), enabled);
    else
        ret = kIOReturnNotReady;
    }
    return ret;
}

int AppleMultitouchInputHIDEventDriver::enableMultitouchEventsGated(bool enabled) {
    r15 = rsi;
    rbx = arg0;
    IOReturn ret = kIOReturnNotReady;
    if (!IOService::isInactive(this)) {
        ret = kIOReturnSuccess;
        if (*_inputReportBuffer->getBytesNoCopy() & 0xff != (enabled & 0xff)) {
            *_inputReportBuffer->getBytesNoCopy() = enabled;
            /* not important, just some internal logging in `AppleMultitouchDevice' context
             AppleMultitouchDevice* multitouch_device;
            if (multitouch_device) {
                rsi = (*(*rbx + 0xb08))(rbx);
                //(*(r8 + 0x8f0))(rsi, rbx, "AppleMultitouchInputHIDEventDriver::enableMultitouchEvents(%s)");
                IOLog("AppleMultitouchInputHIDEventDriver::enableMultitouchEvents(0x%x)\n", enabled);
            }
             */
            ret = super::enabledMultitouchEvents(*_inputReportBuffer->getBytesNoCopy() & 0xff);
            if (ret) {
                IOLog("[HID] [%s] [Error] %s::%s Failed to set device leash state. Result = 0x%08x\n", "ATC", "AppleMultitouchInputHIDEventDriver", "enableMultitouchEventsGated", ret);
            }
            if (*_inputReportBuffer->getBytesNoCopy()) {
                scheduleUnleash(); // offset 0xc10 doesnt match but its the only one that makes sense to me...
            }
        }
    }
    return ret;
}

int AppleMultitouchInputHIDEventDriver::systemPowerChangeHandler(void * target, void * refCon, UInt32 messageType, IOService * provider, void * messageArgument, vm_size_t argSize) {
    // 0xe0000340 seems to be an IOMessage that is library defined. Likely something to do with the driver shutting down
    if (((messageType == 0xe0000340) && (*(int32_t *)(argSize + 0x8) == 0x2)) && ((*(int32_t *)(argSize + 0x10) & 0x1) == 0x0)) {
        int32_t ret = *(int32_t *)(argSize + 0x14) & 0x1;
        if (ret) {
            scheduleUnleash();
        }
    }
    return 0x0;
}
        
int AppleMultitouchInputHIDEventDriver::scheduleUnleash() {
    if (_unleashThread)) {
        this->retain()
        
        if (thread_call_enter(_unleashThread);) {
            IOLog("[HID] [%s] %s::%s Schedule unleash thread call is already pending\n", "ATC", "AppleMultitouchInputHIDEventDriver", "scheduleUnleash");
            this->release()
        }
    } else {
        IOLog("[HID] [%s] [Error] %s::%s _unleashThread is NULL\n", "ATC", "AppleMultitouchInputHIDEventDriver", "scheduleUnleash");
    }
    return rax;
}

int AppleMultitouchInputHIDEventDriver::unleashThreadEnter() {
    IOWorkLoop* work_loop = getWorkLoop();

    work_loop->runAction(OSMemberFunctionCast(IOWorkLoop::Action, this, &AppleMultitouchInputHIDEventDriver::unleashThreadGated), this);
    
    return work_loop->release();
}
        
int AppleMultitouchInputHIDEventDriver::unleashDeviceGated() {
    if (IOService::isInactive(this))
        return true;
    int ret;

    ret = getMultitouchReport(0x2, &(0x0), &(0x1), 0x3);

    if (ret) {
        IOLog("[HID] ?? [Error] AppleMultitouchInputHIDEventDriver::unleashThreadGated Failed to get device leash state. Error = 0x%08x\n", ret);
        return ret;
    }

    AppleMultitouchDevice* multitouch_device = getMultitouchDevice();
    
    if (!multitouch_device)
        return 0;

    if (!multitouch_device->doesClientWantFrames())
        return 0
    
    ret = multitouch_device->setMultitouchReport(0x0, &(0x01), 0x1, 0x3);
    if (ret) {
        IOLog("[HID] [%s] [Error] AppleMultitouchInputHIDEventDriver::unleashThreadGated() Failed to set device leash state. Error = 0x%08x\n", ret);
    }
    
    return ret;
    
}
