//
//  AppleMultitouchTrackpadHIDEventDriver.hpp
//  Apple Multitouch Reversing
//
//  Created by Alexandre on 12/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef AppleMultitouchTrackpadHIDEventDriver_hpp
#define AppleMultitouchTrackpadHIDEventDriver_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#include "AppleMultitouchInputHIDEventDriver.hpp"

typedef struct {
    bool button_swap;
    bool disabled_by_external_mouse;
    bool disabled_by_mouse_keys;
    bool mouse_keys_enabled;
} TrackpadPreferences;


class AppleMultitouchTrackpadHIDEventDriver : public AppleMultitouchInputHIDEventDriver {
  OSDeclareDefaultStructors(AppleMultitouchTrackpadHIDEventDriver);

 public:
    // offset 0x170
    TrackpadPreferences* trackpad_preferences;
    
    // offset 0x174
    bool mouse_keys_stops_trackpad;
    
    // offset 0x178
    IONotifier* usb_mouse_first_match_notifier;
    
    // offset 0x180
    IONotifier* usb_mouse_terminated_notifier;
    
    // offset 0x188
    bool usb_mouse_stops_trackpad;
    
    bool attach(IOService* provider);
    void detach(IOService* provider);
    bool init(OSDictionary* properties);
    void free();
    bool start(IOService* provider);
    void stop(IOService* provider);
    
    int CheckOtherMouseEntry(OSObject* target, void* refCon, IOService* newService, IONotifier *notifier);
    
    // offset 0xc38
    int checkOtherMouse();
    
    int attachToChild(IORegistryEntry* child, IORegistryPlane* plane);
    
    int setSystemProperties(OSDictionary* properties);
    
    // offset 0xc40
    int setBuiltinTrackpadDisablingMouseAttached(bool enabled);

    // offset 0xc48
    int setBuiltinTrackpadDisablingMouseKeysEnabled(bool enabled);
 protected:
 private:
};


#endif /* AppleMultitouchTrackpadHIDEventDriver_hpp */
