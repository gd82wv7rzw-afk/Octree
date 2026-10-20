#include "SimulatorV0.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <simd/simd.h>
#include <cmath>
#include <simd/vector_types.h>
#include <utility>
#include <omp.h>

using namespace simd;

namespace 
{
    void insertAtom(SimulatorV0::Tree& tree, const Atom& atom, float length)
    {
        float3 center{0.5f*length, 0.5f*length, 0.5f*length};
        float3 offset{0.f, 0.f, 0.f}; 
        float  with = 0.5f * length; 
        char   oct  = (atom.pos.x >= center.x) | (atom.pos.y >= center.y) << 1 | (atom.pos.z >= center.z) << 2;

        auto sub = tree.root();

        // only the first time
        if (sub == nullptr) 
        {
            auto leaf = sub.newLeaf(oct);
            leaf->value() = atom;

            sub.self().value().com  = atom.pos;
            sub.self().value().with = with;

            return;
        }

        // update the center of mass
        {
            auto sz = sub.size();
            sub.self().value().com = (sub.self().value().com * sz + atom.pos) / (sz + 1);
        }

        if (sub.isEmpty(oct)) 
        {
            auto leaf = sub.newLeaf(oct);
            leaf->value() = atom;
            
            return;
        }
        else 
        {
            while (sub.isBranch(oct)) 
            {
                //sub.goIn(oct);
                auto tmp = sub.branch(oct).value();

                offset.x = oct & 0b001 ? offset.x + with: offset.x;
                offset.y = oct & 0b010 ? offset.y + with: offset.y;
                offset.z = oct & 0b100 ? offset.z + with: offset.z;

                with /= 2;

                center = offset + float3{with, with, with};

                auto sz = sub.size(oct);

                sub.goIn(oct);
                sub.self().value().com  = (tmp.com * sz + atom.pos) / (sz + 1);
                sub.self().value().with = with;

                oct = (atom.pos.x >= center.x) | (atom.pos.y >= center.y) << 1 | (atom.pos.z >= center.z) << 2;
            }

            if (sub.isLeaf(oct)) 
            {
                auto leaf = sub.leaf(oct);
                sub.deleate(oct);
                sub.newSubTree(oct);

                sub.goIn(oct);
                sub.self().value().com  = (leaf.value().pos + atom.pos) * 0.5f;

                offset.x = oct & 0b001 ? offset.x + with: offset.x;
                offset.y = oct & 0b010 ? offset.y + with: offset.y;
                offset.z = oct & 0b100 ? offset.z + with: offset.z;

                with /= 2;
                sub.self().value().with = with;

                center = offset + float3{with, with, with};
               
                oct = oct = (atom.pos.x >= center.x) | (atom.pos.y >= atom.pos.y) << 1 | (atom.pos.z >= center.z) << 2;
 
                sub.newLeaf(oct);
                sub.leaf(oct).value() = atom;

                oct = ~oct & 7;
 
                sub.newLeaf(oct);
                sub.leaf(oct).value() = leaf.value();

                return;
            }           
            else
            {
                auto leaf = sub.newLeaf(oct);
                leaf->value() = atom;
                
                return;
            }
        }
    }
    //------------------------------------------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------------------------------------------

SimulatorV0::SimulatorV0() : mRng(std::random_device{}())
{
    mR0   = mLength / cbrt(1.f*mNAtoms) / mFreeLength; 
    mR02  = mR0 * mR0;
    mR013 = mR0 * mR0 * mR0; mR013 *= mR013; mR013 *= mR013; mR013 *= mR0;
    mRoh  = 1.f / mFreeLength / mFreeLength / mFreeLength;

    mTmpAtoms.insert(mTmpAtoms.end(), mNAtoms, Atom{float3{0.f,0.f,0.f}, float3{0.f,0.f,0.f}, float3{0.f,0.f,0.f}, float3{0.f,0.f,0.f}, -1});
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::init()
{
    float3 center{0.5f*mLength,0.5f*mLength,0.5f*mLength};
    float3 offset{0.f, 0.f, 0.f}; 
    char   oct  = 0;
    float  with = 0.5f; 

    for (int i = 0; i < mNAtoms; ++i) 
    {
        float3 point;
        while (true)
        {
            float x = std::normal_distribution<float>(0.f, mLength/10)(mRng);
            float y = std::normal_distribution<float>(0.f, mLength/10)(mRng);
            float z = std::normal_distribution<float>(0.f, mLength/10)(mRng);

            float3 p{x, y, z};

            if (length(p) < 0.1f * mLength) 
            {
                point = p + float3{0.5f, 0.5f, 0.5f};
                break;
            }
            //std::uniform_real_distribution<float>(0.0f, mLength)(mRng),
            //std::uniform_real_distribution<float>(0.0f, mLength)(mRng),
            //std::uniform_real_distribution<float>(0.0f, mLength)(mRng)
        };       
        
        float3 v1
        {
            std::normal_distribution<float>(0.0f, 1.f)(mRng),
            std::normal_distribution<float>(0.0f, 1.f)(mRng),
            std::normal_distribution<float>(0.0f, 1.f)(mRng)
        }; 

        v1 /= mNAtoms;

        float3 v2
        {
            -point.y + 0.5f,
            point.x  - 0.5f,
            0.f
        };

        v2 /= mNAtoms;

        Atom atom{point, v1 + v2, {0.f,0.f,0.f}, {0.f,0.f,0.f}, -1};

        insertAtom(mTree, atom, mLength);
    }
}
//----------------------------------------------------------------------------------------------------------------------

float3 SimulatorV0::grad(const float3& x)
{
    auto r2 = dot(x,x)/mR02;
    auto r  = sqrt(r2);

    //auto r14r = r2 * r2 * r2; r14r *= r14r; r14r *= r2; r14r = 1.f / r14r;
    auto r3r  = r2 * r; r3r = 1.f / r3r; 
    auto r4r  = r2 * r2; r4r = 1.f / r4r; 

    //auto factor = -12.f * r14r + r3r;
    auto factor2= -2.f * r4r + r3r;

    // TODO
    //return x / r2;
    //return x * r3r;

    //assert(maxDim({x * factor2}) < 1.f);
    return x * factor2;
}
//----------------------------------------------------------------------------------------------------------------------

float3 SimulatorV0::calculateDistance(const float3& a, const float3& b)
{
    float3 x = b - a;

    //TODO
    // erst mal nicht periodisch
    //return x;

    if (x.x < -0.5) x.x += mLength;
    if (x.y < -0.5) x.y += mLength;
    if (x.z < -0.5) x.z += mLength;

    if (x.x >= 0.5) x.x -= mLength;
    if (x.y >= 0.5) x.y -= mLength;
    if (x.z >= 0.5) x.z -= mLength;

    return x;
}
//----------------------------------------------------------------------------------------------------------------------

float SimulatorV0::maxDim(const float3& x)
{
    float max = 0;

    if (fabs(x.x) > max) max = fabs(x.x);
    if (fabs(x.y) > max) max = fabs(x.y);
    if (fabs(x.z) > max) max = fabs(x.z);

    return max;
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::calculateAcceleration(Tree::reference::iterator it, Tree& tree)
{
    it->value().a = float3{0.f,0.f,0.f};
    auto sub = *(Tree::const_iterator(&tree, it->parentIndex()));
    auto wsub = sub;

    auto targetOct = it->possitonInParent();

    int oct = 0;
    double3 a{0,0,0};

    while (wsub)
    {
        for (; oct < 8; ++oct)
        {
            // Das eigene Blatt nicht berücksichtigen.
            if (wsub == sub && oct == targetOct) continue;

            if (wsub.isEmpty(oct)) continue;

            const auto rb    = (wsub.isLeaf(oct) ? wsub.leaf(oct).value().pos : wsub.branch(oct).value().com);
            const auto r     = calculateDistance(it->value().pos, rb);

            const auto maxR  = maxDim(r);
            auto theta = 0;
            if (wsub.isBranch(oct)) 
            {
                auto a = wsub.branch(oct).value().with;
                //a = a * a * 3;
                theta = a / maxR; //dot(r,r);
            }

            const uint wait  = wsub.isBranch(oct) ? wsub.size(oct) : 1;

            if (theta > mTheta)
            {
                wsub.goIn(oct);
                oct = 0;
                break;
            }

            //it->value().a += grad(r);
            const auto la = grad(r) * wait;
            a.x += la.x;
            a.y += la.y;
            a.z += la.z;

            //printf("atom a %7u wait %7u ax=%12.5e ay=%12.5e az=%12.5e\n", it.index(), wait, la.x, la.y, la.z);
            //printf("waits %5u\n", wait);
        }

        // Wenn wir in einen Kindknoten gegangen sind,
        // läuft die äußere while-Schleife dort weiter.
        if (oct < 8) continue;

        // Aktuellen Subtree verlassen und beim nächsten
        // Geschwister-Oktanten weitermachen.
        if (wsub == sub)
        {
            // im neuen root bei 0 anfangen
            oct = 0;
            targetOct = wsub.self().possitonInParent();
            sub.goUp();
        }
        else 
        {
            // Aktuellen Subtree verlassen und beim nächsten
            // Geschwister-Oktanten weitermachen.
            oct = wsub.self().possitonInParent() + 1;
        }

        wsub.goUp();
    }

    it->value().a = float3{float(a.x), float(a.y), float(a.z)};
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::calculateAcceleration(Tree& tree)
{
    auto begin = tree.root().begin();

    const int nThreads = omp_get_max_threads();

    #pragma omp parallel num_threads(nThreads)
    {
        const int tid = omp_get_thread_num();

        const int first = mNAtoms * tid       / nThreads;
        const int last  = mNAtoms * (tid + 1) / nThreads;

        auto it = begin;
        std::advance(it, first);

        for (int i = first; i < last; ++i, ++it)
        {
            calculateAcceleration(it, tree);
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::calculateAccelerationNaive(Tree::reference::iterator it)
{
    //TODO only testing
    auto b = mTree.root().begin();
    auto end = mTree.root().end();

    it->value().aNaiv = float3{0.f,0.f,0.f};

    double3 a{0,0,0};

    for (; b != end; ++b) 
    {
        if (b == it) continue;
        const auto r = calculateDistance(it->value().pos, b->value().pos);

        auto la = grad(r);

        a.x += la.x;
        a.y += la.y;
        a.z += la.z;
    }

    it->value().aNaiv = float3{ float(a.x), float(a.y), float(a.z)};
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::calculateAccelerationNaive()
{
    auto it = mTree.root().begin();
    auto end = mTree.root().end();

    while (it != end) 
    {
        calculateAccelerationNaive(it);
        for (int i = 0; i < 10; ++i) { ++it;}
    }
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::printAndCalcError() 
{
    auto it = mTree.root().begin();
    auto end = mTree.root().end();

    double rms  = 0;
    double qer  = 0;
    double max  = 0;
    int count   = 0;

    while (it != end) 
    {
        auto   aNaiv = it->value().aNaiv;
        auto   aBran = it->value().a; 
        auto   err   = aBran - aNaiv;

        double sErr = length(err);// / length(aNaiv);
        rms += sErr * sErr;
        qer += sErr;

        max = sErr > max ? sErr : max;
        ++count;

        //printf("naive:  %15.7e\t%15.7e\t%15.7e\n", aNaiv.x, aNaiv.y, aNaiv.z);
        //printf("barnes: %15.7e\t%15.7e\t%15.7e\n", aBran.x, aBran.y, aBran.z);
        //printf("error:  %15.7e\t%15.7e\t%15.7e\n", err.x,   err.y,   err.z);
        //printf("relerr: %15.7lf%%\n", length(err) * 100 / length(aNaiv));
        //printf("-----------------------------------------------------------\n\n");

        for (int i = 0; i < 10; ++i) { ++it;}
        //++it;
    }

    rms /= count;
    rms  = sqrt(rms);

    qer /= count;

    printf("rms: %13.6lf%% quer: %13.6lf%% max: %13.6lf%%\n", rms*100, qer*100, max*100);
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::printSnapShot()
{
    printf("%u\n\n", mNAtoms);
    for (auto a : mTree.root()) 
    {
        auto r = (a.value().pos - float3{0.5, 0.5, 0.5} * mLength) / mR0;
        auto v = a.value().vel;
        printf("Xx\t%13.6e %13.6e %13.6e\t%13.6e %13.6e %13.6e\n", r.x, r.y, r.z, v.x, v.y, v.z);
    }       
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::firstIntegrate(Tree::reference::iterator it)
{
    auto   a = it->value().a;
    auto   v = it->value().vel;
    auto   r = it->value().pos;
    int  idx = it.index();

    auto dr = a * 0.5 * mDt * mDt + v * mDt;
    dr /= mR0;

    r += dr;

    if (r.x < 0) r.x += mLength;
    if (r.y < 0) r.y += mLength;
    if (r.z < 0) r.z += mLength;

    if (r.x >= mLength) r.x -= mLength;
    if (r.y >= mLength) r.y -= mLength;
    if (r.z >= mLength) r.z -= mLength;

    Atom atom{r, v, a, {0.f, 0.f, 0.f}, idx};

    mTmpAtoms[idx] = atom;

    //insertAtom(mNextTree, atom, mLength);
    //#pragma omp critical
    //{
    //    insertAtom(mNextTree, atom, mLength);
    //}
}
//----------------------------------------------------------------------------------------------------------------------

float SimulatorV0::secondIntegrateAndError(Tree::reference::iterator it)
{
    auto idx   = it->value().oldIdx;
    auto atom0 = mTree.leafs()->getPointer(idx);

    auto   a = (atom0->value().a * 2 + it->value().a) / 3;
    auto   v = atom0->value().vel;
    auto   r = atom0->value().pos;

    auto dr = a * 0.5 * mDt * mDt + v * mDt;
    dr /= mR0;

    r += dr;

    if (r.x < 0) r.x += mLength;
    if (r.y < 0) r.y += mLength;
    if (r.z < 0) r.z += mLength;

    if (r.x >= mLength) r.x -= mLength;
    if (r.y >= mLength) r.y -= mLength;
    if (r.z >= mLength) r.z -= mLength;

    //it->value().vel  = v;
    auto  err = maxDim(calculateDistance(r, it->value().pos));// / maxDim(dr);
    //auto maxD = maxDim(dr);

    it->value().pos = r;

    //printf("err: %e\t relerr: %e\n", err, err/maxD);

    return err;
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::integrateVeloseties(Tree::reference::iterator it)
{
    auto idx   = it->value().oldIdx;
    auto atom0 = mTree.leafs()->getPointer(idx);

    auto   a = (atom0->value().a + it->value().a) / 2;
    auto   v = atom0->value().vel + a * mDt;

    it->value().vel = v * mT;
    it->value().oldIdx = -1;
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::step()
{

    auto begin = mTree.root().begin();
    const int nThreads = 1;//omp_get_max_threads();

    #pragma omp parallel num_threads(nThreads)
    {
        const int tid = 0;//omp_get_thread_num();

        const int first = mNAtoms * tid       / nThreads;
        const int last  = mNAtoms * (tid + 1) / nThreads;

        auto it = begin;
        std::advance(it, first);

        for (int i = first; i < last; ++i, ++it)
        {
            firstIntegrate(it);
        }
    }

    for (auto& atom : mTmpAtoms) 
    {
        insertAtom(mNextTree, atom, mLength);
    }

    float maxErrOld = 0;
    for(int i = 0; i < 100; ++ i) 
    {
        float maxErr = 0;
        calculateAcceleration(mNextTree);

        begin = mNextTree.root().begin();

        #pragma omp parallel num_threads(nThreads)
        {
            const int tid = omp_get_thread_num();

            const int first = mNAtoms * tid       / nThreads;
            const int last  = mNAtoms * (tid + 1) / nThreads;

            auto it = begin;
            std::advance(it, first);
            auto threadMaxErr = 0.0f;

            for (int j = first; j < last; ++j, ++it)
            {
                auto err = secondIntegrateAndError(it);
                if (err > threadMaxErr)
                threadMaxErr = err;
            }

            #pragma omp critical
            {
                if (threadMaxErr > maxErr) maxErr = threadMaxErr;
            }      
        }

        updateTree(mNextTree);

        float dErr = fabs(maxErr - maxErrOld);

        //printf("iteration: %i error: %e derror: %e\n",i, maxErr, dErr);
        if (maxErr < std::numeric_limits<float>::epsilon()*2 || dErr < std::numeric_limits<float>::epsilon()*2) break;

        maxErrOld = maxErr;
    };

    begin  = mNextTree.root().begin();

    #pragma omp parallel num_threads(nThreads)
    {
        const int tid = omp_get_thread_num();

        const int first = mNAtoms * tid       / nThreads;
        const int last  = mNAtoms * (tid + 1) / nThreads;

        auto it = begin;
        std::advance(it, first);
        auto threadMaxErr = 0.0f;

        for (int i = first; i < last; ++i, ++it)
        {
            integrateVeloseties(it);
        }
    }

    swap(mTree, mNextTree);  

    mNextTree = Tree{};
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::aniling(Tree::reference::iterator it)
{
    auto  a  = it->value().a;
    auto  v  = it->value().vel;
    auto  r  = it->value().pos;
    auto dt  = 0.2 * mDt;

    auto dr = a * 0.5 * dt * dt + v * dt;
    dr /= mR0;

    auto dv = a * dt;
    //dv /= mR0;

    r += dr;
    v += dv;

    if (r.x < 0) r.x += mLength;
    if (r.y < 0) r.y += mLength;
    if (r.z < 0) r.z += mLength;

    if (r.x >= mLength) r.x -= mLength;
    if (r.y >= mLength) r.y -= mLength;
    if (r.z >= mLength) r.z -= mLength;

    it->value().vel  = v * 0.7;
    it->value().pos  = r;
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::integrate()
{
    auto it  = mTree.root().begin();
    auto end = mTree.root().end();

    while (it != end) 
    {
        firstIntegrate(it);
        ++it;
    }
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::aniling()
{
    auto begin  = mTree.root().begin();
    const int nThreads = omp_get_max_threads();
    #pragma omp parallel num_threads(nThreads)
    {
        const int tid = omp_get_thread_num();

        const int first = mNAtoms * tid       / nThreads;
        const int last  = mNAtoms * (tid + 1) / nThreads;

        auto it = begin;
        std::advance(it, first);
        auto threadMaxErr = 0.0f;

        for (int i = first; i < last; ++i, ++it)
        {
            aniling(it);
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

void SimulatorV0::updateTree(Tree& tree)
{
    Tree oldTree;
    std::swap(tree, oldTree);

    auto it  = oldTree.root().begin();
    auto end = oldTree.root().end();

    for (; it != end; ++it) 
    {
        insertAtom(tree, it->value(), mLength);
    }
}
//----------------------------------------------------------------------------------------------------------------------