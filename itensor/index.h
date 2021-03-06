#ifndef __ITENSOR_INDEX_H
#define __ITENSOR_INDEX_H
#include <string>
#include "types.h"
#include "boost/format.hpp"
#include "boost/intrusive_ptr.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/string_generator.hpp"

enum Arrow { In = -1, Out = 1 };

inline Arrow operator*(const Arrow& a, const Arrow& b)
{ return (int(a)*int(b) == In) ? In : Out; }

const Arrow Switch = In*Out;

inline std::ostream& operator<<(std::ostream& s, const Arrow& D)
{ if(D == In) s << "In"; else s << "Out"; return s; }

enum IndexType { Link, Site, ReIm };
static const char * indextypename[] = { "Link","Site","ReIm" };

enum PrimeType { primeLink, primeSite, primeBoth, primeNone };

inline std::ostream& operator<<(std::ostream& s, const IndexType& it)
{ 
    if(it == Link) s << "Link"; 
    else if(it == Site) s << "Site"; 
    else if(it == ReIm) s << "ReIm"; 
    return s; 
}

inline int IndexTypeToInt(IndexType it)
{
    if(it == Link) return 1;
    if(it == Site) return 2;
    if(it == ReIm) return 3;
    Error("No integer value defined for IndexType.");
    return -1;
}
inline IndexType IntToIndexType(int i)
{
    if(i == 1) return Link;
    if(i == 2) return Site;
    if(i == 3) return ReIm;
    std::cerr << boost::format("No IndexType value defined for i=%d\n")%i,Error("");
    return Link;
}

inline std::string putprimes(std::string s, int plev = 0)
{ for(int i = 1; i <= plev; ++i) s += "\'"; return s;}

inline std::string nameindex(IndexType it, int plev = 0)
{ return putprimes(std::string(indextypename[(int)it]),plev); }

inline std::string nameint(std::string f,int ix)
{ std::stringstream ss; ss << f << ix; return ss.str(); }

enum Imaker {makeReIm,makeReImP,makeReImPP,makeNull};

#define UID_NUM_PRINT 2
inline std::ostream& operator<<(std::ostream& s, const boost::uuids::uuid& id)
{ 
    s.width(2);
    for(boost::uuids::uuid::size_type i = id.size()-UID_NUM_PRINT; i < id.size(); ++i) 
    {
        s << static_cast<unsigned int>(id.data[i]);
    }
    s.width(0);
    return s; 
}

struct UniqueID
{
    boost::uuids::uuid id;

    UniqueID() : id(boost::uuids::random_generator()()) { }

    UniqueID& operator++()
    {
        int i = id.size(); 
        while(--i >= 0)
        { 
            if(++id.data[i] == 0) continue; 
            break;
        }
        return *this;
    }

    operator boost::uuids::uuid() const { return id; }

    friend inline std::ostream& operator<<(std::ostream& s, const UniqueID& uid) { s << uid.id; return s; }
};

inline int prime_number(int n)
{
    static const boost::array<int,54> plist = { { 
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 
    37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 
    79, 83, 89, 97, 101, 103, 107, 109, 113, 
    127, 131, 137, 139, 149, 151, 157, 163, 
    167, 173, 179, 181, 191, 193, 197, 199, 
    211, 223, 227, 229, 233, 239, 241, 251 
    } };
    return plist.at(n);
}

class IndexDat;
namespace boost
{
    inline void intrusive_ptr_add_ref(IndexDat* p);
    inline void intrusive_ptr_release(IndexDat* p);
}

//Storage for Index's
class IndexDat
{
    mutable unsigned int numref;
    const bool is_static_;
public:
    static const UniqueID& nextID()
    {
        static UniqueID lastID_;
        return ++lastID_;
    }

    IndexType _type;
    boost::uuids::uuid ind;
    int m_;
    Real ur;
    std::string sname;

    void set_unique_Real()
	{
        //ur = sin(ind * sqrt(1.0/7.0) + ((int)_type - (int)Site) * sqrt(1.0 / 13.0));
        Real arg = 0;
        int pn = 1;
        for(int i = int(ind.size()); i >= 0; --i)
        { arg += ind.data[i]*sqrt(1.0/(prime_number(++pn)*1.0)); }
        arg *= sqrt(1.0/(prime_number(++pn)*1.0));
        arg += ((int)_type - (int)Site) * sqrt(1.0/(prime_number(++pn)*1.0));
        ur = sin(arg);
	}

    IndexDat(const std::string& name="", int mm = 1,IndexType it=Link) :
    numref(0), is_static_(false),
    _type(it), 
    ind(nextID()),
    m_(mm), 
    sname(name)
	{ 
        if(it == ReIm) Error("bad call to create IndexDat with type ReIm");
        set_unique_Real();
	}

    //For use with read/write functionality of Index class
    IndexDat(const std::string& ss, int mm, IndexType it, const boost::uuids::uuid& ind_) :
    numref(0), is_static_(false), _type(it), ind(ind_), m_(mm), sname(ss)
	{ 
        if(it == ReIm) Error("bad call to create IndexDat with type ReIm");
        set_unique_Real();
	}

    //Don't actually use random uuid generator for these static IndexDats
    IndexDat(Imaker im) : 
    numref(1000000000), is_static_(true),
    _type(ReIm), 
    m_( (im==makeNull) ? 1 : 2)
	{ 
        boost::uuids::string_generator gen;
        if(im==makeNull)
        { ind = gen("{00000000-0000-0000-0000-000000000000}"); }
        else                               
        { ind = gen("{10000000-0000-0000-0000-000000000000}"); }

        if(im == makeNull)
        {
            sname = "Null";
            _type = Site;
            ur = 0.0;
            return;
        }
        else if(im == makeReIm) sname = "ReIm";
        else if(im == makeReImP) sname = "ReImP";
        else if(im == makeReImPP) sname = "ReImPP";
        set_unique_Real(); 
	}

    static IndexDat* Null()
    {
        static IndexDat Null_(makeNull);
        return &Null_;
    }

    static IndexDat* ReImDat()
    {
        static IndexDat ReImDat_(makeReIm);
        return &ReImDat_;
    }

    static IndexDat* ReImDatP()
    {
        static IndexDat ReImDatP_(makeReImP);
        return &ReImDatP_;
    }

    static IndexDat* ReImDatPP()
    {
        static IndexDat ReImDatPP_(makeReImPP);
        return &ReImDatPP_;
    }

    friend inline void ::boost::intrusive_ptr_add_ref(IndexDat* p);
    friend inline void ::boost::intrusive_ptr_release(IndexDat* p);
    int count() const { return numref; }
private:
    IndexDat(const IndexDat&);
    void operator=(const IndexDat&);
};

namespace boost
{
    inline void intrusive_ptr_add_ref(IndexDat* p) { ++(p->numref); }
    inline void intrusive_ptr_release(IndexDat* p) { if(!p->is_static_ && --(p->numref) == 0){ delete p; } }
}

//extern IndexDat IndexDatNull, IndReDat, IndReDatP, IndReDatPP;

struct IndexVal;

class Index
{
private:
    boost::intrusive_ptr<IndexDat> p;
    int primelevel_; 
protected:
    void set_m(int newm) { p->m_ = newm; }
public:

    inline int m() const { return p->m_; }
    boost::uuids::uuid Ind() const { return p->ind; }
    inline IndexType type() const { return p->_type; }

    std::string name() const  { return putprimes(rawname(),primelevel_); }
    std::string rawname() const { return p->sname; }
    void setname(std::string newname) { p->sname = newname; }

    inline std::string showm() const { return (boost::format("m=%d")%(p->m_)).str(); }
    Real unique_Real() const { return p->ur*(1+primelevel_); }
    //inline bool is_null() const { return (p == &IndexDatNull); }
    //inline bool is_not_null() const { return (p != &IndexDatNull); }
    inline bool is_null() const { return (p == IndexDat::Null()); }
    inline bool is_not_null() const { return (p != IndexDat::Null()); }
    int count() const { return p->count(); }

    inline int primeLevel() const { return primelevel_; }
    inline void primeLevel(int plev) { primelevel_ = plev; }

    Arrow dir() const { return Out; }

    //-----------------------------------------------
    //Index: Constructors

    Index() : p(IndexDat::Null()), primelevel_(0) { }

    Index(const std::string& name, int mm = 1, IndexType it=Link, int plev = 0) 
	: p(new IndexDat(name,mm,it)), primelevel_(plev) { }

    Index(std::istream& s) { read(s); }

    Index(Imaker im)
	{
        if(im == makeNull)
            p = IndexDat::Null(), primelevel_ = 0;
        else if(im == makeReIm)
            p = IndexDat::ReImDat(), primelevel_ = 0;
        else if(im == makeReImP)
            p = IndexDat::ReImDatP(),  primelevel_ = 1;
        else if(im == makeReImPP)
            p = IndexDat::ReImDatPP(),  primelevel_ = 2;
        else Error("Unrecognized Imaker type.");
	}

    Index(PrimeType pt,const Index& other, int primeinc = 1) 
	: p(other.p), primelevel_(other.primelevel_)
	{
        primelevel_ = other.primelevel_;
        doprime(pt,primeinc);
	}

    static const Index& Null()
    {
        static const Index Null_(makeNull);
        return Null_;
    }

    static const Index& IndReIm()
    {
        static const Index IndReIm_(makeReIm);
        return IndReIm_;
    }

    static const Index& IndReImP()
    {
        static const Index IndReImP_(makeReImP);
        return IndReImP_;
    }

    static const Index& IndReImPP()
    {
        static const Index IndReImPP_(makeReImPP);
        return IndReImPP_;
    }

    //-----------------------------------------------
    //Index: Operators

    // rel_ops defines the other comparisons based on == and <
    bool operator==(const Index& other) const 
    { return (p->ur == other.p->ur && primelevel_ == other.primelevel_); }

    bool noprime_equals(const Index& other) const
	{ return (p->ur == other.p->ur); }

    bool operator<(const Index& other) const 
	{ return (unique_Real() < other.unique_Real()); }

    IndexVal operator()(int i) const;


    //-----------------------------------------------
    //Index: Prime methods

    void mapprime(int plevold, int plevnew, PrimeType pr = primeBoth)
	{
        if(type() == ReIm) return;
        if(primelevel_ != plevold) return;
        else if( pr == primeBoth
        || (type() == Site && pr == primeSite) 
        || (type() == Link && pr == primeLink) )
        {
            primelevel_ = plevnew;
        }
	}
    void doprime(PrimeType pr, int inc = 1)
	{
        if(type() == ReIm) return;
        if( pr == primeBoth
        || (type() == Site && pr == primeSite) 
        || (type() == Link && pr == primeLink) )
        {
            primelevel_ += inc;
        }
	}
    Index primed(int inc = 1) const { return Index(primeBoth,*this,inc); }

    friend inline Index
    primed(const Index& I, int inc = 1) { return Index(primeBoth,I,inc); }

    Index deprimed() const { Index cp(*this); cp.primelevel_ = 0; return cp; }

    void noprime(PrimeType pt = primeBoth) { doprime(pt,-primelevel_); }

    friend inline std::ostream & operator<<(std::ostream & s, const Index & t)
    {
        if(t.name() != "" && t.name() != " ") s << t.name() << "/";
        return s << nameindex(t.type(),t.primelevel_) << "-" << t.Ind() << ":" << t.m();
    }

    //-----------------------------------------------
    //Index: Other methods

    void write(std::ostream& s) const 
    { 
        if(is_null()) Error("Index::write: Index is null");
        s.write((char*) &primelevel_,sizeof(primelevel_));
        const int t = IndexTypeToInt(p->_type);
        s.write((char*) &t,sizeof(t));
        for(int i = 0; i < int(p->ind.size()); ++i) 
        { const char c = p->ind.data[i] - '0'; s.write(&c,sizeof(c)); }
        s.write((char*) &(p->m_),sizeof(p->m_));
        const int nlength = p->sname.length();
        s.write((char*) &nlength,sizeof(nlength));
        s.write(p->sname.data(),nlength+1);
    }

    void read(std::istream& s)
        {
        s.read((char*) &primelevel_,sizeof(primelevel_));
        int t; s.read((char*) &t,sizeof(t));
        boost::uuids::uuid ind;
        for(int i = 0; i < int(ind.size()); ++i) 
            { char c; s.read(&c,sizeof(c)); ind.data[i] = '0'+c; }
        int mm; s.read((char*) &mm,sizeof(mm));
        int nlength; s.read((char*) &nlength,sizeof(nlength));
        char* newname = new char[nlength+1]; s.read(newname,nlength+1);
        std::string ss(newname); delete newname;
        if(IntToIndexType(t) == ReIm)
            {
            if(primelevel_ == 0) 
                p = IndexDat::ReImDat();
            else if(primelevel_ == 1) 
                p = IndexDat::ReImDatP();
            else if(primelevel_ == 2) 
                p = IndexDat::ReImDatPP();
            else
                Error("Illegal primelevel for Index of ReIm type");
            }
        else
            {
            p = new IndexDat(ss,mm,IntToIndexType(t),ind);
            }
        }

    void print(std::string name = "") const
    { std::cerr << "\n" << name << " =\n" << *this << "\n"; }

    void conj() { } //for forward compatibility with arrows

}; //class Index

template <class T> 
T conj(T res) { res.conj(); return res; }

struct IndexVal
{
    Index ind; 
    int i;
    IndexVal() : ind(Index::Null()),i(0) { }
    IndexVal(const Index& index, int i_) : ind(index),i(i_) { assert(i <= ind.m()); }
    bool operator==(const IndexVal& other) const { return (ind == other.ind && i == other.i); }
    inline friend std::ostream& operator<<(std::ostream& s, const IndexVal& iv)
    { return s << "IndexVal: i = " << iv.i << ", ind = " << iv.ind << "\n"; }
    IndexVal primed() const { return IndexVal(ind.primed(),i); }

    static const IndexVal& Null()
    {
        static const IndexVal Null_(Index::Null(),1);
        return Null_;
    }
};

inline IndexVal Index::operator()(int i) const 
{ return IndexVal(*this,i); }

#endif
