/**
 * @file GoWired.h
 * @author Jakub Kazibudzki (hello@gowired.dev)
 * @brief This file combines headers used in GoWired in-wall (Software/Modules) and GoWired Touch (Software/Touch)
 * @version 1.0
 * @date 2022-04-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef GoWired_h
#define GoWired_h

// Internal library files
#include "core/AnalogTemp.h"
#include "core/CommonIO.h"
#include "core/Dimmer.h"
#include "core/PowerSensor.h"
#include "core/Shutters.h"

// GoWired dependencies
#include <MySensors.h>
#include <Wire.h>
#include <avr/wdt.h>

#endif