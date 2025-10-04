#include "RunAcq.h"

// run acq method
// Run method (thread entry)
/*
Accept: a stop request token(or equivalent), a reference to your ring buffer of “Chunk”, and a shared atomic “active frequency” that indicates which flicker the user is “looking at”.

Document that this method loops until a stop is requested, produces exactly one fixed - size chunk per loop, pushes it into the ring buffer, and paces itself based on the chunk period.
*/