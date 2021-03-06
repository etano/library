#ifndef __COMBINER_H
#define __COMBINER_H

#include "itensor.h"

/*
Combine several indices into one, use * to convert tensors efficiently
   \
    \
  ---C====
    /
   /

*/
class Combiner
{
    typedef boost::array<Index,NMAX+1>::const_iterator left_it;
public:

    //Accessor Methods ----------------------------------------------

    const Index& 
    right() const 
        { 
        if(!initted) Error("Combiner: right ind requested prior to init");
        return right_; 
        }

    int 
    rl() const { return rl_; }

    const Index& 
    left(int j) const { return GET(left_,j); }

    const std::pair<left_it,left_it> 
    left() const 
        { 
        return std::make_pair(left_.begin()+1,left_.begin()+rl_+1); 
        }

    //Constructors --------------------------------------------------

    Combiner() : rl_(0), initted(false) {}

    Combiner(const Index& l1, const Index& l2 = Index::Null(), 
             const Index& l3 = Index::Null(), const Index& l4 = Index::Null(), 
             const Index& l5 = Index::Null(), const Index& l6 = Index::Null(), 
             const Index& l7 = Index::Null(), const Index& l8 = Index::Null() );
    
    //Operators -----------------------------------------------------

    ITensor 
    operator*(const ITensor& t) const 
        { ITensor res; product(t,res); return res; }

    friend inline ITensor 
    operator*(const ITensor& t, const Combiner& c) 
        { return c.operator*(t); }

    //Index Methods -------------------------------------------------

    inline bool 
    isInit() const { return initted; }

    void 
    reset();

    void 
    addleft(const Index& l);// Include another left index

    //Initialize after all lefts are added and before being used
    void 
    init(std::string rname = "combined", 
         IndexType type = Link, 
         Arrow dir = Switch,
         int primelevel = 0) const;

    int 
    findindex(const Index& I) const;

    bool 
    hasindex(const Index& I) const;


    //Other Methods -------------------------------------------------

    operator ITensor() const;

    friend inline std::ostream& 
    operator<<(std::ostream & s, const Combiner & c);

    void 
    conj() { init(); }

    void 
    product(const ITensor& t, ITensor& res) const;

    //For interface compatibility with IQCombiner
    void 
    doCondense(bool) { } 

private:

    boost::array<Index,NMAX+1> left_; // max dim is 8
    mutable Index right_;
    int rl_; //Number of m>1 'left' indices (indices to be combined into one)
    mutable bool initted;

}; //class Combiner



inline
Combiner::
Combiner(const Index& l1, const Index& l2,
         const Index& l3, const Index& l4, 
         const Index& l5, const Index& l6, 
         const Index& l7, const Index& l8)
    : rl_(0), initted(false)
	{
        boost::array<const Index*,NMAX+1> ll 
        = {{ &Index::Null(), &l1, &l2, &l3, &l4, &l5, &l6, &l7, &l8 }};

        do { ++rl_; left_[rl_] = *ll[rl_]; } 
        while(rl_ < NMAX && *ll[rl_+1] != Index::Null());

        assert(rl_ == NMAX || left_[rl_+1] == Index::Null());
        assert(left_[rl_] != Index::Null());
	}

inline
void Combiner::
reset()
    {
        rl_ = 0;
        initted = false;
    }

inline
void Combiner::
addleft(const Index& l)// Include another left index
    { 
    initted = false;
    if(rl_ == NMAX) 
        Error("Combiner: already reached max number of left indices.");
    left_[++rl_] = l; 
    }

inline
void Combiner::
init(std::string rname, IndexType type, Arrow dir, int primelevel) const
    {
    if(initted) return;
    int m = 1; 
    for(int i = 1; i <= rl_; ++i) 
        { m *= left_[i].m(); }
    right_ = Index(rname,m,type,primelevel); 
    initted = true;
    }

inline
int Combiner::
findindex(const Index& I) const
    {
    for(int j = 1; j <= rl_; ++j)
        { if(left_[j] == I) return j; }
    return 0;
    }

inline
bool Combiner::
hasindex(const Index& I) const
    {
    for(int j = 1; j <= rl_; ++j) if(left_[j] == I) return true;
    return false;
    }

inline
Combiner::
operator ITensor() const
    {
    /*
    if(right_.m() > 16) 
    { 
        std::cerr << "\n\n" 
        << "WARNING: too large of an m in Combiner to ITensor!\n\n"; 
    }
    */

    //Use a kronecker delta tensor to convert this Combiner into an Tensor
    ITensor res = operator*(ITensor(right_,right_.primed(5),1));
    res.primeind(right_.primed(5),-5);
    return res;
    }

inline
void Combiner::
product(const ITensor& t, ITensor& res) const
    {
    init();

    int j;
    if((j = t.findindex(right_)) != 0)
    {
        std::vector<Index> nindices; 
        nindices.reserve(t.r()+rl_-1);
        for(int i = 1; i < j; ++i)
            nindices.push_back(t.index(i));
        for(int i = 1; i <= rl_; ++i)
            nindices.push_back(left_[i]);
        for(int i = j+1; i <= t.r(); ++i)
            nindices.push_back(t.index(i));
        res = ITensor(nindices,t);
        return;
    }

    t.groupIndices(left_,rl_,right_,res);
    }

inline 
std::ostream& 
operator<<(std::ostream & s, const Combiner & c)
    {
    if(c.isInit())
        s << "\nRight index: " << c.right() << "\n";
    else
        s << "\nRight index not initialized" << "\n";
    s << "Left indices:\n";
    foreach(const Index& l, c.left()) s << " " << l << "\n";
    return s;
    }

#endif
