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

class AppleMultitouchTrackpadHIDEventDriver : public AppleMultitouchInputHIDEventDriver {
  OSDeclareDefaultStructors(AppleMultitouchTrackpadHIDEventDriver);

 public:
    bool attach(IOService* provider);
    void detach(IOService* provider);
    bool init(OSDictionary* properties);
    void free();
    bool start(IOService* provider);
    void stop(IOService* provider);

 protected:
 private:
};


#endif /* AppleMultitouchTrackpadHIDEventDriver_hpp */
