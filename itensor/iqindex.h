#ifndef __IQINDEX_H
#define __IQINDEX_H
#include "index.h"

/*
* Conventions regarding arrows:
*
* * Arrows point In or Out, never right/left/up/down.
*
* * The Site indices of a ket point out.
*
* * Conjugation switches arrow directions.
*
* * All arrows flow out from the ortho center of an MPS (ket - in if it's a bra).
*
* * IQMPOs are created with the same arrow structure as if they are orthogonalized
*   to site 1, but this is just a default since they aren't actually ortho. If position 
*   is called on an IQMPO it follows the same convention as for an MPS except Site 
*   indices point In and Site' indices point Out.
*
*/

class QN
{
    int _sz, _Nf, _Nfp; //_Nfp stands for fermion number parity, and tracks whether Nf is even or odd
public:
    QN(int sz=0,int Nf=0) : _sz(sz), _Nf(Nf), _Nfp(abs(Nf%2)) { }
    QN(int sz,int Nf,int Nfp) : _sz(sz), _Nf(Nf), _Nfp(abs(Nfp%2))
    { assert(_Nf==0 || abs(_Nf%2) == _Nfp); }

    QN(std::istream& s) { read(s); }

    int sz() const { return _sz; }
    int Nf() const { return _Nf; }
    int Nfp() const { assert(_Nfp == 0 || _Nfp == 1); return _Nfp; }
    int fp() const { return (_Nfp == 0 ? +1 : -1); }
    //int& sz() { return _sz; }
    //int& Nf() { return _Nf; }
    //int& Nfp() { assert(_Nfp == 0 || _Nfp == 1); return _Nfp; }

    void write(std::ostream& s) const 
    { 
        s.write((char*)&_sz,sizeof(_sz)); 
        s.write((char*)&_Nf,sizeof(_Nf)); 
        s.write((char*)&_Nfp,sizeof(_Nfp)); 
    }
    void read(std::istream& s) 
    { 
        s.read((char*)&_sz,sizeof(_sz)); 
        s.read((char*)&_Nf,sizeof(_Nf)); 
        s.read((char*)&_Nfp,sizeof(_Nfp)); 
    }

    QN operator+(const QN &other) const
	{ QN res(*this); res+=other; return res; }
    QN operator-(const QN &other) const
	{ QN res(*this); res-=other; return res; }
    QN& operator+=(const QN &other)
	{
        _sz+=other._sz; _Nf+=other._Nf; _Nfp = abs(_Nfp+other._Nfp)%2;
        return *this;
	}
    QN& operator-=(const QN &other)
	{
        _sz-=other._sz; _Nf-=other._Nf; _Nfp = abs(_Nfp-other._Nfp)%2;
        return *this;
	}
    QN operator-() const  
	{ return QN(-_sz,-_Nf,_Nfp); }
    
    QN negated() const { return QN(-_sz,-_Nf,_Nfp); }

    //Multiplication and division should only be used to change the sign
    QN& operator*=(int i) { assert(i*i == 1); _sz*=i; _Nf*=i; return *this; }
    QN operator*(int i) const { QN res(*this); res*=i; return res; }
    QN operator/(int i) const { QN res(*this); res*=i; return res; }

    inline std::string toString() const
    {  return (boost::format("(%+d:%d:%s)")%_sz%_Nf%(_Nfp==1 ? "-" : "+")).str(); }

    inline friend std::ostream& operator<<(std::ostream &o, const QN &q)
    { return o<< boost::format("sz = %d, Nf = %d, fp = %s") % q.sz() % q.Nf() % (q.fp() < 0 ? "-" : "+"); }

    void print(std::string name = "") const
    { std::cerr << "\n" << name << " =\n" << *this << "\n"; }
};
inline bool operator==(const QN &a,const QN &b)
    { return a.sz() == b.sz() && a.Nf() == b.Nf() && a.Nfp() == b.Nfp(); }
inline bool operator!=(const QN &a,const QN &b)
    { return a.sz() != b.sz() || a.Nf() != b.Nf() || a.Nfp() != b.Nfp(); }
inline bool operator<(const QN &a,const QN &b)
    { return a.sz() < b.sz() || (a.sz() == b.sz() && a.Nf() < b.Nf()) 
             || (a.sz() == b.sz() && a.Nf() == b.Nf() && a.Nfp() < b.Nfp()); }
inline QN operator*(int i,const QN& a)
    { return a*i; }


struct inqn
{
    Index index;
    QN qn;
    inqn() { }
    inqn(const Index& i, QN q) : index(i), qn(q) { }

    void write(std::ostream& s) const { index.write(s); qn.write(s); }
    void read(std::istream& s) { index.read(s); qn.read(s); }
    inline friend std::ostream& operator<<(std::ostream &o, const inqn& x)
    { o << "inqn: " << x.index << " (" << x.qn << ")\n"; return o; }
};

class IQIndex;
class DoPrimer		// Functor which applies doprime within STL's for_each, etc
{
public:
    PrimeType pt; int inc;
    DoPrimer (PrimeType _pt, int _inc = 1) : pt(_pt), inc(_inc) {}
    void operator()(inqn& iq) const { iq.index.doprime(pt,inc); }
    void operator()(Index& i) const { i.doprime(pt,inc); }
    void operator()(ITensor& it) const { it.doprime(pt,inc); }
    void operator()(IQIndex& iqi) const;
};
class MapPrimer // Functor which applies mapprime within STL's for_each, etc
{
public:
    PrimeType pt;
    int plevold, plevnew;
    MapPrimer (int _plevold,int _plevnew,PrimeType _pt = primeBoth) 
		: pt(_pt), plevold(_plevold), plevnew(_plevnew) {}
    void operator()(inqn& iq) const { iq.index.mapprime(plevold,plevnew,pt); }
    void operator()(Index& i) const { i.mapprime(plevold,plevnew,pt); }
    void operator()(ITensor& it) const { it.mapprime(plevold,plevnew,pt); }
    void operator()(IQIndex& iqi) const;
};
class IndEq // Functor which checks if the index is equal to a specified value within STL's for_each, etc
{
public:
    Index i;
    IndEq(Index _i) : i(_i) {}
    bool operator()(const inqn &j) const { return i == j.index; }
};

class IQIndexDat;
namespace boost
{
    inline void intrusive_ptr_add_ref(IQIndexDat* p);
    inline void intrusive_ptr_release(IQIndexDat* p);
}

class IQIndexDat
{
    mutable unsigned int numref;
    const bool is_static_;
public:
    typedef std::vector<inqn>::iterator iq_it;
    typedef std::vector<inqn>::const_iterator const_iq_it;

    std::vector<inqn> iq_;

    IQIndexDat() : numref(0), is_static_(false) { }

    IQIndexDat(const Index& i1, const QN& q1)
    : numref(0), is_static_(false)
    {
        iq_.push_back(inqn(i1,q1));
    }

    IQIndexDat(const Index& i1, const QN& q1,
               const Index& i2, const QN& q2)
    : numref(0), is_static_(false)
    {
        iq_.push_back(inqn(i1,q1));
        iq_.push_back(inqn(i2,q2));
    }

    IQIndexDat(const Index& i1, const QN& q1,
               const Index& i2, const QN& q2,
               const Index& i3, const QN& q3)
    : numref(0), is_static_(false)
    {
        iq_.push_back(inqn(i1,q1));
        iq_.push_back(inqn(i2,q2));
        iq_.push_back(inqn(i3,q3));
    }

    IQIndexDat(const Index& i1, const QN& q1,
               const Index& i2, const QN& q2,
               const Index& i3, const QN& q3,
               const Index& i4, const QN& q4)
    : numref(0), is_static_(false)
    {
        iq_.push_back(inqn(i1,q1));
        iq_.push_back(inqn(i2,q2));
        iq_.push_back(inqn(i3,q3));
        iq_.push_back(inqn(i4,q4));
    }

    IQIndexDat(const Index& i1, const QN& q1,
               const Index& i2, const QN& q2,
               const Index& i3, const QN& q3,
               const Index& i4, const QN& q4,
               const Index& i5, const QN& q5)
    : numref(0), is_static_(false)
    {
        iq_.push_back(inqn(i1,q1));
        iq_.push_back(inqn(i2,q2));
        iq_.push_back(inqn(i3,q3));
        iq_.push_back(inqn(i4,q4));
        iq_.push_back(inqn(i5,q5));
    }

    IQIndexDat(const Index& i1, const QN& q1,
               const Index& i2, const QN& q2,
               const Index& i3, const QN& q3,
               const Index& i4, const QN& q4,
               const Index& i5, const QN& q5,
               const Index& i6, const QN& q6)
    : numref(0), is_static_(false)
    {
        iq_.push_back(inqn(i1,q1));
        iq_.push_back(inqn(i2,q2));
        iq_.push_back(inqn(i3,q3));
        iq_.push_back(inqn(i4,q4));
        iq_.push_back(inqn(i5,q5));
        iq_.push_back(inqn(i6,q6));
    }

    IQIndexDat(const Index& i1, const QN& q1,
               const Index& i2, const QN& q2,
               const Index& i3, const QN& q3,
               const Index& i4, const QN& q4,
               const Index& i5, const QN& q5,
               const Index& i6, const QN& q6,
               const Index& i7, const QN& q7)
    : numref(0), is_static_(false)
    {
        iq_.push_back(inqn(i1,q1));
        iq_.push_back(inqn(i2,q2));
        iq_.push_back(inqn(i3,q3));
        iq_.push_back(inqn(i4,q4));
        iq_.push_back(inqn(i5,q5));
        iq_.push_back(inqn(i6,q6));
        iq_.push_back(inqn(i7,q7));
    }

    IQIndexDat(const Index& i1, const QN& q1,
               const Index& i2, const QN& q2,
               const Index& i3, const QN& q3,
               const Index& i4, const QN& q4,
               const Index& i5, const QN& q5,
               const Index& i6, const QN& q6,
               const Index& i7, const QN& q7,
               const Index& i8, const QN& q8)
    : numref(0), is_static_(false)
    {
        iq_.push_back(inqn(i1,q1));
        iq_.push_back(inqn(i2,q2));
        iq_.push_back(inqn(i3,q3));
        iq_.push_back(inqn(i4,q4));
        iq_.push_back(inqn(i5,q5));
        iq_.push_back(inqn(i6,q6));
        iq_.push_back(inqn(i7,q7));
        iq_.push_back(inqn(i8,q8));
    }

    IQIndexDat(std::vector<inqn>& ind_qn)
    : numref(0), is_static_(false)
    { iq_.swap(ind_qn); }

    explicit IQIndexDat(const IQIndexDat& other) : numref(0), is_static_(false), iq_(other.iq_)
    { }

    explicit IQIndexDat(std::istream& s) : numref(0), is_static_(false) { read(s); }

    void write(std::ostream& s) const
    {
        size_t size = iq_.size();
        s.write((char*)&size,sizeof(size));
        for(const_iq_it x = iq_.begin(); x != iq_.end(); ++x)
            { x->write(s); }
    }

    void read(std::istream& s)
    {
        size_t size; s.read((char*)&size,sizeof(size));
        iq_.resize(size);
        for(iq_it x = iq_.begin(); x != iq_.end(); ++x)
            { x->read(s); }
    }

    explicit IQIndexDat(Imaker im)
    : numref(0), is_static_(true)
    { 
        if(im == makeNull)
            iq_.push_back(inqn(Index::Null(),QN())); 
        else if(im == makeReIm)
            iq_.push_back(inqn(Index::IndReIm(),QN())); 
        else if(im == makeReImP)
            iq_.push_back(inqn(Index::IndReImP(),QN())); 
        else if(im == makeReImPP)
            iq_.push_back(inqn(Index::IndReImPP(),QN())); 
    }

    static IQIndexDat* Null()
    {
        static IQIndexDat Null_(makeNull);
        return &Null_;
    }

    static IQIndexDat* ReImDat()
    {
        static IQIndexDat ReImDat_(makeReIm);
        return &ReImDat_;
    }

    static IQIndexDat* ReImDatP()
    {
        static IQIndexDat ReImDatP_(makeReImP);
        return &ReImDatP_;
    }

    static IQIndexDat* ReImDatPP()
    {
        static IQIndexDat ReImDatPP_(makeReImPP);
        return &ReImDatPP_;
    }

    friend inline void boost::intrusive_ptr_add_ref(IQIndexDat* p);
    friend inline void boost::intrusive_ptr_release(IQIndexDat* p);
    int count() const { return numref; }
private:
    void operator=(const IQIndexDat&);
};

namespace boost
{
    inline void intrusive_ptr_add_ref(IQIndexDat* p) { ++(p->numref); }
    inline void intrusive_ptr_release(IQIndexDat* p) { if(!p->is_static_ && --(p->numref) == 0){ delete p; } }
}

struct IQIndexVal;

#ifdef DEBUG
#define IQINDEX_CHECK_NULL if(pd == 0) Error("IQIndex is null");
#else
#define IQINDEX_CHECK_NULL
#endif

class IQIndex : public Index
    {
    Arrow _dir;
    boost::intrusive_ptr<IQIndexDat> pd;

    void solo()
        {
        IQINDEX_CHECK_NULL
        if(pd->count() != 1)
            {
            boost::intrusive_ptr<IQIndexDat> new_pd(new IQIndexDat(*pd));
            //new_pd->iq_ = pd->iq_;
            pd.swap(new_pd);
            }
        }

    public:

    typedef IQIndexDat::iq_it iq_it;
    typedef IQIndexDat::const_iq_it const_iq_it;

    const std::vector<inqn>& iq() const 
        { 
        IQINDEX_CHECK_NULL
        return pd->iq_;
        }
    int nindex() const 
        { 
        IQINDEX_CHECK_NULL
        return (int) pd->iq_.size(); 
        }
    const Index& index(int i) const 
        {
        IQINDEX_CHECK_NULL
        return GET(pd->iq_,i-1).index; 
        }
    const QN& qn(int i) const 
        {
        IQINDEX_CHECK_NULL
        return GET(pd->iq_,i-1).qn; 
        }

    //------------------------------------------
    //IQIndex: Constructors

    IQIndex() : _dir(Out), pd(0) { }

    explicit IQIndex(const Index& other, Arrow dir = Out) 
        : Index(other), _dir(dir), pd(0)
        { }

    explicit IQIndex(const std::string& name,
                     IndexType it = Link, 
                     Arrow dir = Out, 
                     int plev = 0) 
        : Index(name,1,it,plev), _dir(dir), pd(0) 
        { }

    IQIndex(const std::string& name, 
            const Index& i1, const QN& q1, 
            Arrow dir = Out) 
        : Index(name,i1.m(),i1.type()), 
          _dir(dir), pd(new IQIndexDat(i1,q1))
        {
        primeLevel(i1.primeLevel());
        }

    IQIndex(const std::string& name, 
            const Index& i1, const QN& q1, 
            const Index& i2, const QN& q2,
            Arrow dir = Out) 
        : Index(name,i1.m()+i2.m(),i1.type()), _dir(dir), 
          pd(new IQIndexDat(i1,q1,i2,q2))
        {
        primeLevel(i1.primeLevel());
        if(i2.type() != i1.type())
            Error("Indices must have the same type");
        }

    IQIndex(const std::string& name, 
            const Index& i1, const QN& q1, 
            const Index& i2, const QN& q2,
            const Index& i3, const QN& q3,
            Arrow dir = Out) 
        : Index(name,i1.m()+i2.m()+i3.m(),i1.type()), _dir(dir),
          pd(new IQIndexDat(i1,q1,i2,q2,i3,q3))
        {
        primeLevel(i1.primeLevel());
        if(i2.type() != i1.type() 
        || i3.type() != i1.type())
            Error("Indices must have the same type");
        }

    IQIndex(const std::string& name, 
            const Index& i1, const QN& q1, 
            const Index& i2, const QN& q2,
            const Index& i3, const QN& q3,
            const Index& i4, const QN& q4,
            Arrow dir = Out) 
        : Index(name,i1.m()+i2.m()+i3.m()+i4.m(),i1.type()), _dir(dir),
          pd(new IQIndexDat(i1,q1,i2,q2,i3,q3,i4,q4))
        {
        primeLevel(i1.primeLevel());
        if(i2.type() != i1.type() 
        || i3.type() != i1.type()
        || i4.type() != i1.type())
            Error("Indices must have the same type");
        }

    IQIndex(const std::string& name, 
            const Index& i1, const QN& q1, 
            const Index& i2, const QN& q2,
            const Index& i3, const QN& q3,
            const Index& i4, const QN& q4,
            const Index& i5, const QN& q5,
            Arrow dir = Out) 
        : Index(name,i1.m()+i2.m()+i3.m()+i4.m()+i5.m(),i1.type()), _dir(dir),
          pd(new IQIndexDat(i1,q1,i2,q2,i3,q3,i4,q4,i5,q5))
        {
        primeLevel(i1.primeLevel());
        if(i2.type() != i1.type() 
        || i3.type() != i1.type()
        || i4.type() != i1.type()
        || i5.type() != i1.type())
            Error("Indices must have the same type");
        }

    IQIndex(const std::string& name, 
            std::vector<inqn>& ind_qn, 
            Arrow dir = Out, int plev = 0) 
        : Index(name,0,ind_qn.back().index.type(),plev), 
          _dir(dir), pd(new IQIndexDat(ind_qn))
        { 
        int mm = 0;
        for(const_iq_it x = pd->iq_.begin(); x != pd->iq_.end(); ++x)
            {
            mm += x->index.m();
            if(x->index.type() != this->type())
                Error("Indices must have the same type");
            }
        set_m(mm);
        primeLevel(pd->iq_.back().index.primeLevel());
        }

    IQIndex(const IQIndex& other, 
            std::vector<inqn>& ind_qn)
        : Index(other.name(),0,other.type()), 
          _dir(other._dir), pd(new IQIndexDat(ind_qn))
        { 
        int mm = 0;
        for(const_iq_it x = pd->iq_.begin(); x != pd->iq_.end(); ++x)
            {
            mm += x->index.m();
            if(x->index.type() != this->type())
                Error("Indices must have the same type");
            }
        set_m(mm);
        primeLevel(pd->iq_.back().index.primeLevel());
        }

    IQIndex(const Index& other, 
            const Index& i1, const QN& q1, 
            Arrow dir = Out) 
        : Index(other), _dir(dir), pd(new IQIndexDat(i1,q1))
        {
        primeLevel(i1.primeLevel());
        }

    IQIndex(PrimeType pt, const IQIndex& other, int inc = 1) 
        : Index(other), _dir(other._dir), pd(other.pd)  
        { doprime(pt,inc); }

    explicit IQIndex(std::istream& s) 
        { read(s); }

    void write(std::ostream& s) const
        {
        IQINDEX_CHECK_NULL
        Index::write(s);
        s.write((char*)&_dir,sizeof(_dir));
        pd->write(s);
        }

    void read(std::istream& s)
        {
        Index::read(s);
        s.read((char*)&_dir,sizeof(_dir));
        pd = new IQIndexDat(s);
        }

    IQIndex(Imaker im)
        : Index(im), _dir(In)
        {
        if(im == makeNull)
            { pd = IQIndexDat::Null(); }
        else if(im == makeReIm)
            { pd = IQIndexDat::ReImDat(); }
        else if(im == makeReImP)
            { pd = IQIndexDat::ReImDatP(); }
        else if(im == makeReImPP)
            { pd = IQIndexDat::ReImDatPP(); }
        else Error("IQIndex: Unrecognized Imaker type.");
        }

    static const IQIndex& Null()
        {
        static const IQIndex Null_(makeNull);
        return Null_;
        }

    static const IQIndex& IndReIm()
        {
        static const IQIndex IndReIm_(makeReIm);
        return IndReIm_;
        }

    static const IQIndex& IndReImP()
        {
        static const IQIndex IndReImP_(makeReImP);
        return IndReImP_;
        }

    static const IQIndex& IndReImPP()
        {
        static const IQIndex IndReImPP_(makeReImPP);
        return IndReImPP_;
        }

    IQIndexVal operator()(int n) const;

    //------------------------------------------
    //IQIndex: methods for querying m's

    int biggestm() const
        {
        IQINDEX_CHECK_NULL
        int mm = 0;
        for(const_iq_it x = pd->iq_.begin(); x != pd->iq_.end(); ++x)
            { mm = max(mm,x->index.m()); }
        return mm;
        }
    std::string showm() const
        {
        IQINDEX_CHECK_NULL
        std::string res = " ";
        std::ostringstream oh; 
        oh << this->m() << " | ";
        for(const_iq_it x = pd->iq_.begin(); x != pd->iq_.end(); ++x)
            {
            QN q = x->qn;
            oh << boost::format("[%d,%d,%s]:%d ") % q.sz() % q.Nf() % (q.fp()==1?"+":"-") % x->index.m(); 
            }
        return oh.str();
        }

    //------------------------------------------
    //IQIndex: quantum number methods

    void negate()
        {
        IQINDEX_CHECK_NULL
        for(iq_it x = pd->iq_.begin(); x != pd->iq_.end(); ++x)
            { x->qn *= -1; }
        }

    friend inline IQIndex negate(IQIndex I) // Quantum numbers negated
        { 
        I.negate();
        return I;
        }
     
    QN qn(const Index& i) const
        { 
        IQINDEX_CHECK_NULL
        for(const_iq_it x = pd->iq_.begin(); x != pd->iq_.end(); ++x)
            { if(x->index == i) return x->qn; }
        std::cerr << *this << "\n";
        std::cerr << "i = " << i << "\n";
        Error("IQIndex::qn(Index): IQIndex does not contain given index.");
        return QN();
        }

    Arrow dir() const { return _dir; }
    void conj() { _dir = _dir*Switch; }

    //------------------------------------------
    //IQIndex: index container methods


    const Index& findbyqn(QN q) const
        { 
        IQINDEX_CHECK_NULL
        for(size_t i = 0; i < pd->iq_.size(); ++i)
            if(pd->iq_[i].qn == q) return pd->iq_[i].index;
        Error("IQIndex::findbyqn: no Index had a matching QN.");
        return pd->iq_[0].index;
        }

    bool hasindex(const Index& i) const
        { 
        IQINDEX_CHECK_NULL
        for(const_iq_it x = pd->iq_.begin(); x != pd->iq_.end(); ++x)
            if(x->index == i) return true;
        return false;
        }
    bool hasindex_noprime(const Index& i) const
        { 
        IQINDEX_CHECK_NULL
        for(const_iq_it x = pd->iq_.begin(); x != pd->iq_.end(); ++x)
            if(x->index.noprime_equals(i)) return true;
        return false;
        }

    int offset(const Index& I) const
        {
        int os = 0;
        for(size_t j = 0; j < pd->iq_.size(); ++j)
            {
            const Index& J = pd->iq_[j].index;
            if(J == I) return os;
            os += J.m();
            }
        Print(*this);
        Print(I);
        Error("Index not contained in IQIndex");
        return 0;
        }

    //------------------------------------------
    //IQIndex: prime methods

    void doprime(PrimeType pt, int inc = 1)
        {
        solo();
        Index::doprime(pt,inc);
        DoPrimer dp(pt,inc);
        for_each(pd->iq_.begin(),pd->iq_.end(),dp);
        }
    void mapprime(int plevold, int plevnew, PrimeType pt = primeBoth)
        {
        solo();
        Index::mapprime(plevold,plevnew,pt);
        for_each(pd->iq_.begin(),pd->iq_.end(),MapPrimer(plevold,plevnew,pt));
        }
    void noprime(PrimeType pt = primeBoth)
        {
        solo();
        Index::noprime(pt);
        for(size_t j = 0; j < pd->iq_.size(); ++j)
        { pd->iq_[j].index.noprime(pt); }
        }
    IQIndex primed(int inc = 1) const
        {
        return IQIndex(primeBoth,*this,inc);
        }

    friend inline IQIndex
    primed(const IQIndex& I, int inc = 1)
        { return IQIndex(primeBoth,I,inc); }

    inline friend std::ostream& operator<<(std::ostream &o, const IQIndex &I)
        {
        if(I.is_null()) 
            { 
            o << "IQIndex: (null)"; 
            return o;
            }
        o << "IQIndex: " << (const Index&) I << " <" << I.dir() << ">" << std::endl;
        for(int j = 1; j <= I.nindex(); ++j) 
            o << " " << I.index(j) SP I.qn(j) << "\n";
        return o;
        }

    void print(std::string name = "") const
        { std::cerr << "\n" << name << " =\n" << *this << "\n"; }

    }; //class IQIndex

#undef IQINDEX_CHECK_NULL

enum IQmaker {makeSing};

struct IQIndexVal
{
    IQIndex iqind; 
    int i;

    IQIndexVal();

    IQIndexVal(const IQIndex& iqindex, int i_);

    Index index() const;

    QN qn() const;

    IQIndexVal primed() const 
        { return IQIndexVal(iqind.primed(),i); }

    void conj() 
        { iqind.conj(); }

    IndexVal toIndexVal() const;

    operator ITensor() const;

    ITensor operator*(const IndexVal& iv) const 
        { 
        return IndexVal(Index(iqind),i) * iv; 
        }

    void print(std::string name = "") const
        { std::cerr << "\n" << name << " =\n" << *this << "\n"; }
    inline friend std::ostream& operator<<(std::ostream& s, const IQIndexVal& iv)
        { return s << "IQIndexVal: i = " << iv.i << ", iqind = " << iv.iqind << "\n"; }

    static const IQIndexVal& Null()
        {
        static const IQIndexVal Null_(IQIndex::Null(),1);
        return Null_;
        }
private:
    void calc_ind_ii(int& j, int& ii) const;

};

inline
IQIndexVal::
IQIndexVal()
    : iqind(IQIndex::Null()), i(1) 
    { }

inline
IQIndexVal::
IQIndexVal(const IQIndex& iqindex, int i_) 
    : iqind(iqindex),i(i_) 
    { 
    if(i > iqind.m() || i < 1) 
        {
        Print(iqindex);
        Print(i);
        Error("IQIndexVal: i out of range");
        }
    }

inline
Index IQIndexVal::
index() const 
    { 
    int j,ii;
    calc_ind_ii(j,ii);
    return iqind.index(j);
    }

inline
QN IQIndexVal::
qn() const 
    { 
    int j,ii;
    calc_ind_ii(j,ii);
    return iqind.qn(j);
    }

inline
IndexVal IQIndexVal::
toIndexVal() const 
    { 
    int j,ii;
    calc_ind_ii(j,ii);
    return IndexVal(iqind.index(j),ii); 
    }

inline
IQIndexVal::
operator ITensor() const 
    { 
    return ITensor(IndexVal(iqind,i)); 
    }

inline
void IQIndexVal::
calc_ind_ii(int& j, int& ii) const
    {
    j = 1;
    ii = i;
    while(ii > iqind.index(j).m())
        {
        ii -= iqind.index(j).m();
        ++j;
        }
    }

inline IQIndexVal IQIndex::operator()(int n) const 
    { return IQIndexVal(*this,n); }

#endif
