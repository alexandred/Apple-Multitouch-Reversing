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
    if (!super::init(properties))
        return false;
    
    

    return true;
}

void AppleMultitouchTrackpadHIDEventDriver::free() {
    super::free();
}

bool AppleMultitouchTrackpadHIDEventDriver::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    return true;
}

void AppleMultitouchTrackpadHIDEventDriver::stop(IOService* provider) {
    super::stop(provider);
}
