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
        IORecursiveLockFree();
        _mtPrefsLock = 0x0;
    }

    if (_mtPreferences {
        _mtPreferences->release();
        _mtPreferences = 0x0;
    }

    if (_inputReportBuffer) {
        _inputReportBuffer->release();
        _inputReportBuffer = 0x0;
    }
        rax = super::free();
    return rax;
}


bool AppleMultitouchInputHIDEventDriver::start(IOService* provider) {
    if (!super::start(provider))
        return false;
    int ret = getMultitouchReport(0x0, &(0x0), &(0x1), 0x3);

    if (ret) {
        IOLog("[HID] ?? [Error] %s::%s Failed to get device initialized state. Result = 0x%08x\n", "ATC", "AppleMultitouchInputHIDEventDriver", "start", ret);
    } else if (0 != 0) {
            (*(*rbx + 0xb40))(rbx);
    }
    
    _notifier = _registerPrioritySleepWakeInterest(OSMemberFunctionCast(IOServiceInterestHandler, this, AppleMultitouchInputHIDEventDriver::systemPowerChangeHandler), this, 0x0);

    if (!notifier)
        return 0x1;

    IOLog("[HID] ?? [Error] AppleMultitouchInputHIDEventDriver::start registerPrioritySleepWakeInterest failed\n");
    
    (*(*rbx + 0x5c8))(rbx, r14);
    rax = 0x0;
    return rax;

    return true;
}

void AppleMultitouchInputHIDEventDriver::stop(IOService* provider) {
    r14 = rsi;
    rbx = arg0;
    rdi = *(rbx + 0x158);
    if (rdi != 0x0) {
        (*(*rdi + 0x118))();
        *(rbx + 0x158) = 0x0;
    }
    rax = (*0x1c3b8)(rbx, r14);
    return rax;
}

int AppleMultitouchInputHIDEventDriver::systemPowerChangeHandler(void * target, void * refCon, UInt32 messageType, IOService * provider, void * messageArgument, vm_size_t argSize) {
    r8 = arg4;
    rdi = arg0;
    if (((messageType == 0xe0000340) && (*(int32_t *)(argSize + 0x8) == 0x2)) && ((*(int32_t *)(argSize + 0x10) & 0x1) == 0x0)) {
        rax = *(int32_t *)(argSize + 0x14) & 0x1;
        if (rax != 0x0) {
            (*(*rdi + 0xc20))();
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

    work_loop->run(OSMemberFunctionCast(IOWorkLoop::Action, this, &AppleMultitouchInputHIDEventDriver::unleashThreadGated), this);
    
    return work_loop->release();
}
        
int AppleMultitouchInputHIDEventDriver::unleashThreadGated() {
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
