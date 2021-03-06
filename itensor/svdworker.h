#ifndef __ITENSOR_SVDWORKER_H
#define __ITENSOR_SVDWORKER_H
#include "iqcombiner.h"

enum Direction { Fromright, Fromleft, Both, None };

class SVDWorker
{
public:
    //SVDWorker Accessors ---------------

    int NN() const { return N; }

    Real cutoff() const { return cutoff_; }
    void cutoff(Real val) { cutoff_ = val; }

    Real truncerr(int b = 1) const 
        { return truncerr_.at(b); }

    int minm() const { return minm_; }
    void minm(int val) { minm_ = val; }

    int maxm() const { return maxm_; }
    void maxm(int val) { maxm_ = val; }

    bool truncate() const { return truncate_; }
    void truncate(bool val) { truncate_ = val; }

    bool showeigs() const { return showeigs_; }
    void showeigs(bool val) { showeigs_ = val; }

    // If doRelCutoff_ is false,
    // refNorm_ defines an overall scale factored
    // out of the denmat before truncating.
    //
    // If doRelCutoff_ is true, refNorm_
    // is determined automatically.
    //
    bool doRelCutoff() const { return doRelCutoff_; }
    void doRelCutoff(bool val) { doRelCutoff_ = val; }

    // If absoluteCutoff_ == true, do a strict limit
    // on the eigenvalues of the density matrix, 
    // not trying for a truncation error
    bool absoluteCutoff() const { return absoluteCutoff_; }
    void absoluteCutoff(bool val) { absoluteCutoff_ = val; }

    LogNumber refNorm() const { return refNorm_; }
    void refNorm(const LogNumber& val) 
        { 
        if(val.sign() == 0) Error("zero refNorm");
        refNorm_ = val; 
        }

    const Vector& eigsKept(int b = 1) const 
        { return eigsKept_.at(b); }

    int maxEigsKept() const;

    Real maxTruncerr() const;

    //Constructors ---------------

    SVDWorker();

    SVDWorker(int N_);

    SVDWorker(int N_, Real cutoff, int minm, int maxm, 
              bool doRelCutoff, const LogNumber& refNorm);

    SVDWorker(std::istream& s) { read(s); }

    void read(std::istream& s);
    void write(std::ostream& s) const;

    Real diag_denmat(const ITensor& rho, Vector& D, ITensor& U);
    Real diag_denmat(const IQTensor& rho, Vector& D, IQTensor& U);
    Real diag_denmat_complex(const IQTensor& rho, Vector& D, IQTensor& U);
    Real diag_denmat_complex(const ITensor& rho, Vector& D, ITensor& U);

    template <class Tensor>
    void operator()(int b, const Tensor& AA, Tensor& A, Tensor& B, Direction dir);

    template <class Tensor>
    inline void operator()(const Tensor& AA, Tensor& A, Tensor& B, Direction dir)
        { operator()<Tensor>(1,AA,A,B,dir); }

private:

    int N;
    std::vector<Real> truncerr_;
    Real cutoff_;
    int minm_;
    int maxm_;
    bool truncate_; 
    bool showeigs_;
    bool doRelCutoff_;
    bool absoluteCutoff_;
    LogNumber refNorm_;
    std::vector<Vector> eigsKept_;

}; //class SVDWorker

inline
int SVDWorker::
maxEigsKept() const
    {
    int res = -1;
    foreach(const Vector& eigs,eigsKept_)
        res = max(res,eigs.Length());
    return res;
    }

inline
Real SVDWorker::
maxTruncerr() const
    {
    Real res = -1;
    foreach(const Real& te,truncerr_)
        res = max(res,te);
    return res;
    }

inline
SVDWorker::
SVDWorker() 
    : N(1), truncerr_(N+1), cutoff_(MIN_CUT), minm_(1), maxm_(MAX_M),
      truncate_(true), showeigs_(false), doRelCutoff_(false),
      absoluteCutoff_(false), refNorm_(1), eigsKept_(N+1)
    { }

inline
SVDWorker::
SVDWorker(int N_)
    : N(N_), truncerr_(N+1), cutoff_(MIN_CUT), minm_(1), maxm_(MAX_M),
      truncate_(true), showeigs_(false), doRelCutoff_(false),
      absoluteCutoff_(false), refNorm_(1), eigsKept_(N+1)
    { }

inline
SVDWorker::
SVDWorker(int N_, Real cutoff, int minm, int maxm, 
          bool doRelCutoff, const LogNumber& refNorm)
    : N(N_), truncerr_(N+1), cutoff_(cutoff), minm_(minm), maxm_(maxm),
      truncate_(true), showeigs_(false), doRelCutoff_(doRelCutoff),
      absoluteCutoff_(false), refNorm_(refNorm), eigsKept_(N+1)
    { }

inline
void SVDWorker::
read(std::istream& s)
    {
    s.read((char*) &N,sizeof(N));
    truncerr_.resize(N+1);
    for(int j = 1; j <= N; ++j)
        s.read((char*)&truncerr_[j],sizeof(truncerr_[j]));
    s.read((char*)&cutoff_,sizeof(cutoff_));
    s.read((char*)&minm_,sizeof(minm_));
    s.read((char*)&maxm_,sizeof(maxm_));
    s.read((char*)&truncate_,sizeof(truncate_));
    s.read((char*)&showeigs_,sizeof(showeigs_));
    s.read((char*)&doRelCutoff_,sizeof(doRelCutoff_));
    s.read((char*)&absoluteCutoff_,sizeof(absoluteCutoff_));
    s.read((char*)&refNorm_,sizeof(refNorm_));
    for(int j = 1; j <= N; ++j)
        readVec(s,eigsKept_[j]);
    }

inline
void SVDWorker::
write(std::ostream& s) const
    {
    s.write((char*) &N,sizeof(N));
    for(int j = 1; j <= N; ++j)
        s.write((char*)&truncerr_[j],sizeof(truncerr_[j]));
    s.write((char*)&cutoff_,sizeof(cutoff_));
    s.write((char*)&minm_,sizeof(minm_));
    s.write((char*)&maxm_,sizeof(maxm_));
    s.write((char*)&truncate_,sizeof(truncate_));
    s.write((char*)&showeigs_,sizeof(showeigs_));
    s.write((char*)&doRelCutoff_,sizeof(doRelCutoff_));
    s.write((char*)&absoluteCutoff_,sizeof(absoluteCutoff_));
    s.write((char*)&refNorm_,sizeof(refNorm_));
    for(int j = 1; j <= N; ++j)
        writeVec(s,eigsKept_[j]);
    }

template<class Tensor>
void SVDWorker::operator()(int b, const Tensor& AA, 
                          Tensor& A, Tensor& B, Direction dir)
    {
    typedef typename Tensor::IndexT IndexT;
    typedef typename Tensor::CombinerT CombinerT;

    if(AA.vec_size() == 0) 
        {
        A *= 0;
        B *= 0;
        eigsKept_.at(b).ReDimension(1);
        eigsKept_.at(b) = 1;
        return;
        }

    IndexT mid = index_in_common(A,B,Link);
    if(mid.is_null()) mid = IndexT("mid");

    //If dir==None, put the O.C. on the side
    //that keeps mid's arrow the same
    bool do_edge_case = true;
    if(dir == None)
        {
        //std::cerr << boost::format("Arrow before = %s\n")%(mid.dir() == Out ? "Out" : "In");
        dir = (mid.dir() == Out ? Fromright : Fromleft);
        do_edge_case = false;
        }

    Tensor& to_orth = (dir==Fromleft ? A : B);
    Tensor& newoc   = (dir==Fromleft ? B : A);

    CombinerT comb;

    int unique_link = 0; //number of Links unique to to_orth
    for(int j = 1; j <= to_orth.r(); ++j) 
        { 
        const IndexT& I = to_orth.index(j);
        if(!(newoc.hasindex(I) || I == Tensor::ReImIndex() ))
            {
            if(I.type() == Link) ++unique_link;
            comb.addleft(I);
            }
        }

    //Check if we're at the edge
    if(unique_link == 0 && do_edge_case)
        {
        comb.init(mid.rawname());
        comb.product(AA,newoc);
        to_orth = comb; to_orth.conj();
        eigsKept_.at(b) = Vector(comb.right().m()); 
        eigsKept_.at(b) = 1.0/comb.right().m();
        return;
        }

    //Apply combiner
    comb.doCondense(true);
    comb.init(mid.rawname());
    Tensor AAc; comb.product(AA,AAc);

    const IndexT& active = comb.right();

    Tensor rho;
    if(0 && AAc.is_complex())
	{
	Tensor re,im;
	AAc.SplitReIm(re,im);
	rho = re; rho.conj(); rho.primeind(active);
	rho *= re;
	im *= conj(primeind(im,active));
	rho += im;
	}
    else 
	{ 
	Tensor AAcc = conj(AAc); 
	AAcc.primeind(active); 
	rho = AAc*AAcc; 
	}

    Real saved_cutoff = cutoff_; 
    int saved_minm = minm_; 
    int saved_maxm = maxm_; 
    if(!truncate_)
        {
        cutoff_ = -1;
        minm_ = mid.m();
        maxm_ = mid.m();
        }

    Tensor U;
    if(AAc.is_complex())
        truncerr_.at(b) = diag_denmat_complex(rho,eigsKept_.at(b),U);
    else
        truncerr_.at(b) = diag_denmat(rho,eigsKept_.at(b),U);

    cutoff_ = saved_cutoff; 
    minm_ = saved_minm; 
    maxm_ = saved_maxm; 

    comb.conj();
    comb.product(U,to_orth);
    newoc = conj(U) * AAc;

    //PrintDat(U);

    IndexT nmid = index_in_common(A,B,Link);
    //std::cerr << boost::format("Arrow after = %s\n")%(nmid.dir() == Out ? "Out" : "In");

    } //void SVDWorker::operator()

#endif
