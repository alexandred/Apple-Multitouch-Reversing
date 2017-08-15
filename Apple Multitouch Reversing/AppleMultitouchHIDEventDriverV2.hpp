//
//  AppleMultitouchHIDEventDriverV2.hpp
//  Apple Multitouch Reversing
//
//  Created by Alexandre on 15/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef AppleMultitouchHIDEventDriverV2_hpp
#define AppleMultitouchHIDEventDriverV2_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/hidevent/IOHIDEventDriver.h>

class AppleMultitouchHIDEventDriverV2 : public IOHIDEventDriver {
  OSDeclareDefaultStructors(AppleMultitouchHIDEventDriverV2);

 public:
    // data members

    // function members
    bool attach(IOService* provider);
    void detach(IOService* provider);
    bool init(OSDictionary* properties);
    void free();
    bool start(IOService* provider);
    void stop(IOService* provider);

 protected:
 private:
};


#endif /* AppleMultitouchHIDEventDriverV2_hpp */
