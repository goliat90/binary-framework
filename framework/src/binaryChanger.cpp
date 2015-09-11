/*  Handles the growth or shrinkage of a binary.
    Rewrites addresses.
    Corrects branch addresses.
    Symbol table entries fixed.
    Segment sizes changed.
*/

/*  Steps that need to be done.
    1. change the addresses for the subsequent instructions.
    2. fix up target addresses for branch instructions
    3. fix symbol tables to point to new addresses
    4. extend an existing ELF segment or add a new one
    5. move later segments to higher addresses
    6. fix the ELF segment table
*/

/*  Include. */
#include "binaryChanger.hpp"


/*  Constructor. */
binaryChanger::binaryChanger(CFGhandler* cfgH) {
    /*  Save the cfg handler pointer. */
    cfgObject = cfgH;

}

