/**
 ******************************************************************************
 * @file    can.h
 * @brief   Aggregator for the CAN driver
 * @details Single include that pulls the entire CAN driver. Include this
 *          header to get all CAN types, lifecycle, transfer, filter, events
 *          and the backend-neutral bus facade.
 *
 * Usage:
 * - #include "can.h" — everything (types, core, baudrate, filter, transfer,
 *   events, bus)
 * - Or include individual headers (can_types.h, can_core.h, etc.) for
 *   smaller dependencies
 ******************************************************************************
 */

#ifndef CAN_H
#define CAN_H

#include "can_types.h"
#include "can_core.h"
#include "can_baudrate.h"
#include "can_filter.h"
#include "can_transfer.h"
#include "can_events.h"
#include "can_bus.h"

#endif /* CAN_H */
