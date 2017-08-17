//
//  AppleMultitouchDriver.cpp
//  Apple Multitouch Reversing
//
//  Created by Alexandre on 17/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "AppleMultitouchDriver.hpp"

#define super IOService
OSDefineMetaClassAndStructors(AppleMultitouchDriver, IOService);

/**
 Attaches an IOService object to `this`
 
 @param provider IOService* representing the provider
 
 @return returns true on succesful attach, else returns false
 */

bool AppleMultitouchDriver::attach(IOService* provider) {
    if (!super::attach(provider))
        return false;

    return true;
}

/**
 Deattaches an IOService object from `this`
 
 @param provider IOService* representing the provider
 */

void AppleMultitouchDriver::detach(IOService* provider) {
    super::detach(provider);
}

/**
 Initialises class

 @param properties OSDictionary* representing the matched personality

 @return returns true on successful initialisation, else returns false
 */

bool AppleMultitouchDriver::init(OSDictionary* properties) {
    if (!super::init(properties))
        return false;

    return true;
}

/**
 Frees class - releases objects instantiated in `init`
 */

void AppleMultitouchDriver::free() {
    super::free();
}

/**
 Starts the class
 
 @param provider IOService* representing the matched entry in the IORegistry
 
 @return returns true on succesful start, else returns false
 */

bool AppleMultitouchDriver::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    return true;
}

/**
 Stops the class and undoes the effects of `start` and `probe`

 @param provider IOService* representing the matched entry in the IORegistry
 */

void AppleMultitouchDriver::stop(IOService* provider) {
    super::stop(provider);
}
