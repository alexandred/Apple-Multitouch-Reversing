//
//  AppleMultitouchInputHIDEventDriver.hpp
//  Apple Multitouch Reversing
//
//  Created by Alexandre on 12/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef AppleMultitouchInputHIDEventDriver_hpp
#define AppleMultitouchInputHIDEventDriver_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/hidevent/IOHIDEventDriver.h>
#include <kern/thread_call.h>
#include <IOKit/IOBufferMemoryDescriptor.h>

class AppleMultitouchInputHIDEventDriver : public AppleMultitouchHIDEventDriverV2 {
  OSDeclareDefaultStructors(AppleMultitouchInputHIDEventDriver);

 public:
    // offset 0x138
    IOBufferMemoryDescriptor* _inputReportBuffer;
    
    // offset 0x148
    OSDictionary* _mtPreferences;
    
    //ofset 0x150
    IORecursiveLock* _mtPrefsLock;
    
    //ofset 0x158
    IONotifier* _notifier;
    
    //ofset 0x160
    thread_call_t _unleashThread;
    
    bool attach(IOService* provider);
    void detach(IOService* provider);
    bool init(OSDictionary* properties);
    void free();
    bool start(IOService* provider);
    void stop(IOService* provider);
    
    // offset 0xc18
    int systemPowerChangeHandler(void * target, void * refCon, UInt32 messageType, IOService * provider, void * messageArgument, vm_size_t argSize);
    
    //offset 0xc20
    int scheduleUnleash()
    
    // offset 0xc28
    int unleashThreadEnter();
    
    // offset 0xc30
    int unleashThreadGated()
 protected:
 private:
};


#endif /* AppleMultitouchInputHIDEventDriver_hpp */
