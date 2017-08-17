//
//  AppleMultitouchDriver.hpp
//  Apple Multitouch Reversing
//
//  Created by Alexandre on 17/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef AppleMultitouchDriver_hpp
#define AppleMultitouchDriver_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

class AppleMultitouchDriver : public IOService {
  OSDeclareDefaultStructors(AppleMultitouchDriver);

 public:
    // data members

    // function members
    bool attach(IOService* provider);
    void detach(IOService* provider);
    bool init(OSDictionary* properties);
    void free();
    bool start(IOService* provider);
    void stop(IOService* provider);
    
    // offset 0xa00
    bool setDisablerPresent(bool arg0);

 protected:
 private:
};


#endif /* AppleMultitouchDriver_hpp */
