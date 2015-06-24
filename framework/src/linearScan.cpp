/* Source code for the framework linear scan */

/*  Algorithms for linear scan. */

/*  Linear scan register allocation
active list <- empty
foreach live interval i, in order of increasing start point
    EXPIREOLDINTERVAL(i)
    if length(active) = R then
        SPILLATINTERVAL(i)
    else
        register[i] <- a register removed from pool of free registers
        add i to active, sorted by inreasing end point
*/

/*  ExpireOldInterval(i)
foreach interval j in active, in order of increasing end point
    if endpoint[i] >= startpoint[i] then
        return
    remove j from active
    add register[j] to pool of free registers
*/

/*  SpillAtInterval(i)
spill <- last interval in active
if endpoint[spill] > endpoint[i] then
    register[i] <- register[spill]
    location[spill] <- new stack location
    remove spill from active
    add i to active, sorted by increasing end point
else
    location[i] <- new stack location
*/


/* includes */
#include "linearScan.hpp"


/*  Constructor */
linearScanHandler::linearScanHandler(){

}

