#ifndef BASE_DMRG_OPTS_H
#define BASE_DMRG_OPTS_H
#include "svdworker.h"

// virtual base class

class BaseDMRGOpts 
{
public:
    
    virtual bool quiet() const = 0;
    virtual void quiet(bool val) = 0;

    virtual void measure(int sw, int ha, int b, const SVDWorker& svd, Real energy) = 0;
    
    virtual bool checkDone(int sw, Real energy) = 0;

    virtual ~BaseDMRGOpts() { }
};

#endif // BASE_DMRG_OPTS_H
