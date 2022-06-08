//
// Created by zeb on 08/06/2022.
//

#include "CoTask.hpp"
#include "Scheduler.hpp"
#include "ComparisonResolver.hpp"
#include <optional>
#include "LinearFunction.hpp"
#include "MultiTScheduler.hpp"
#include "MultiTComparisonResolver.hpp"
#include <mutex>
#include <condition_variable>
#include <chrono>

#ifndef MINIMUMMEANCYCLE_PSFLOYD_HPP
#define MINIMUMMEANCYCLE_PSFLOYD_HPP


auto inf = std::numeric_limits<double>::infinity();
class SeqAlgoMinimumRatioCycle : public ps_framework::ISeqAlgo{
public:
    SeqAlgoMinimumRatioCycle(std::vector<std::vector<std::optional<ps_framework::LinearFunction>>> *fs){
        p_funcs = fs;
    }

    ps_framework::CmpRes compare(double lambda){
        int n = p_funcs->size();
        std::vector<std::vector<double>> dist(n, std::vector<double> (n, std::numeric_limits<double>::infinity()));
        // Set the edge values in the dist matrix
        for (int i = 0; i<n; i++){
            for (int j = 0; j<n; j++){
                // Check if a value is set in p_funcs_ij
                if (!((*p_funcs)[i][j])) {
                    continue;
                }
                    // We do not add 0 self pointing cycles to vertexes as normal FW
                else{
                    // Set a real value in dist_ij
                    // we have to compute with the negative value of lambda
                    // to get the f(t)=a - tb
                    dist[i][j] = ((*p_funcs)[i])[j].value().compute(lambda);
                }
            }
        }
        // Run Floyd-Warshall
        for (int m = 0; m<n; m++){
            for (int i = 0; i<n; i++){
                for (int j = 0; j<n; j++){
                    // If D_im or Dmj then just continue to the next itr
                    if((dist[i][m]==inf)||(dist[m][j]==inf)){
                        continue;
                    }
                        // If D_ij is infinity then just set it equal to
                        // D_im + D_mj as non of them are infinity (above)
                    else if (dist[i][j]==inf){
                        dist[i][j] = dist[i][m]+dist[m][j];
                    }
                        // Else check if D_im + D_mj < then D_ij
                        // and set the value accordingly
                    else {
                        if((dist[i][m]+dist[m][j]) < dist[i][j]){
                            dist[i][j] = dist[i][m]+dist[m][j];
                        }
                    }
                }
            }
        }

        bool zerocycle = false;
        for (int i = 0; i<n; i++){
            if (dist[i][i] < 0)
                // If there is a negative cycle then
                // lambda is too big
                return ps_framework::GreaterThan;
            else if (dist[i][i] == 0)
                zerocycle = true;
        }
        // Megiddo '79 states:
        // If there is a zero cycle and no negative cycles then terminate
        if (zerocycle)
            return ps_framework::EqualTo;
        // If there is no negative cycle then
        // lambda is too small
        return ps_framework::LessThan;
    }

private:
    std::vector<std::vector<std::optional<ps_framework::LinearFunction>>> *p_funcs;
};

class PSFloyd {

public:
    PSFloyd(auto matIn, auto uIn) {
//        takeTime = shouldTakeTime;
        mat = matIn;
        u = uIn;
    }
    void run(){
        auto seqAlgo = SeqAlgoMinimumRatioCycle(mat);
        auto psCore = ps_framework::PSCore(&seqAlgo);
        auto linComparer = ps_framework::LinearFunctionComparer();
        ps_framework::Scheduler scheduler = ps_framework::Scheduler();
        ps_framework::ComparisonResolver comparisonResolver = ps_framework::ComparisonResolver<ps_framework::LinearFunction>(
                &scheduler,
                &psCore,
                &linComparer
        );
        auto n = mat->size();
        auto task = new ps_framework::coroTaskVoid(psFloyd(n, u, &comparisonResolver, &scheduler));
        auto start = std::chrono::high_resolution_clock::now();
        scheduler.spawnIndependent(task);
        scheduler.run();
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "    lambda* = "
                  << psCore.end
                  << std::endl
                  << "    time    = "
                  << duration
                  << std::endl;
    }
    std::vector<std::vector<std::optional<ps_framework::LinearFunction>>> * mat;
    std::vector<std::vector<std::optional<ps_framework::LinearFunction>>> * u;
    std::mutex setMutex;
    std::condition_variable setCV;
    void setValue(auto u, auto i, auto j, std::optional<ps_framework::LinearFunction> elm) {
        (*u)[i][j] = elm;
    }


    ps_framework::coroTaskVoid setMin(
            ps_framework::ComparisonResolver<ps_framework::LinearFunction> *cr,
            auto u,
            auto i,
            auto j,
            auto m,
            auto uplus,
            auto uij
    ) {
        auto res = co_await cr->compare(uij.value(), uplus);
        if (res == ps_framework::GreaterThan) {
            setValue(u, i, j, {(*u)[i][m].value() + (*u)[m][j].value()});
        }
    }

    ps_framework::coroTaskVoid psFloyd(
            auto n,
            auto u,
            ps_framework::ComparisonResolver<ps_framework::LinearFunction> *cR,
            ps_framework::Scheduler *scheduler1
    ) {
        for (int m = 0; m<n; m++){
            for (int i = 0; i<n; i++){
                for (int j = 0; j<n; j++){
                    auto uij = (*u)[i][j];
                    auto uim = (*u)[i][m];
                    auto umj = (*u)[m][j];
                    if((!uim)||(!umj)){
                        continue;
                    }
                    else if (!uij){
//                    (*u)[i][j] = {uim.value()+umj.value()};
                        setValue(u,i,j,{uim.value()+umj.value()});
                    }
                    else {
                        auto uplus = (uim.value()+umj.value());
                        //                    imp_scheduler.addComparison(&uij.value(), &uplus, &cmp_res[i][j]);
                        auto task = new ps_framework::coroTaskVoid(setMin(cR, u, i, j, m, uplus, uij));
//                    std::cout<<"Spawning task"<<std::endl;
                        co_await scheduler1->spawnDependent(task);
                    }
                }
            }
            co_await std::suspend_always();
        }
        co_return;
    }
};

class MultiPSFloyd : public PSFloyd {
public:
    MultiPSFloyd(auto mat, auto u) : PSFloyd(mat,u) {}
    int numThreads = 0;

    void setValue(auto u, auto i, auto j, std::optional<ps_framework::LinearFunction> elm) {
        // Lock or wait
        std::unique_lock lock(setMutex);
        setCV.wait(lock, []{return true;});
        // Set value
        (*u)[i][j] = elm;
        // Unlock
        lock.unlock();
        setCV.notify_one();
    }

    void run(){
        auto seqAlgo = SeqAlgoMinimumRatioCycle(mat);
        auto psCore = ps_framework::PSCore(&seqAlgo);
        auto linComparer = ps_framework::LinearFunctionComparer();
        ps_framework::MultiTScheduler scheduler;
        if (numThreads)
            ps_framework::MultiTScheduler scheduler = ps_framework::MultiTScheduler(numThreads);
        else
            ps_framework::MultiTScheduler scheduler = ps_framework::MultiTScheduler();

        auto comparisonResolver = ps_framework::MultiTComparisonResolver<ps_framework::LinearFunction>(
                &scheduler,
                &psCore,
                &linComparer
        );
        auto n = mat->size();
        auto task = new ps_framework::coroTaskVoid(psFloyd(n, u, &comparisonResolver, &scheduler));
        auto start = std::chrono::high_resolution_clock::now();
        scheduler.spawnIndependent(task);
        scheduler.run();
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "    lambda* = "
                  << psCore.end
                  << std::endl
                  << "    time    = "
                  << duration
                  << std::endl;
    }
};

class ImprovedMultiPSFloyd : public MultiPSFloyd {
public:
    ImprovedMultiPSFloyd(auto mat, auto u) : MultiPSFloyd(mat,u) {}
protected:
    ps_framework::coroTaskVoid psFloyd(
            auto n,
            auto u,
            ps_framework::ComparisonResolver<ps_framework::LinearFunction> *cR,
            ps_framework::Scheduler *scheduler1
    ) {
        for (int m = 0; m<n; m++){
            for (int i = 0; i<n; i++) {
                scheduler1->spawnDependent(
                        new ps_framework::coroTaskVoid(
                                [](auto u, auto i, auto n, auto m, auto me, auto cR, auto scheduler1) -> auto {
                                    for (int j = 0; j < n; j++) {
                                        auto uij = (*u)[i][j];
                                        auto uim = (*u)[i][m];
                                        auto umj = (*u)[m][j];
                                        if ((!uim) || (!umj)) {
                                            continue;
                                        } else if (!uij) {
//                    (*u)[i][j] = {uim.value()+umj.value()};
                                            me->setValue(u, i, j, {uim.value() + umj.value()});
                                        } else {
                                            auto uplus = (uim.value() + umj.value());
                                            //                    imp_scheduler.addComparison(&uij.value(), &uplus, &cmp_res[i][j]);
                                            auto task = new ps_framework::coroTaskVoid(
                                                    me->setMin(cR, u, i, j, m, uplus, uij));
//                    std::cout<<"Spawning task"<<std::endl;
                                            co_await scheduler1->spawnDependent(task);
                                        }
                                    }
                                }(u, i, n, m, this, cR, scheduler1)
                        )
                );
            }
            co_await std::suspend_always();
        }
        co_return;
    }
};


#endif //MINIMUMMEANCYCLE_PSFLOYD_HPP
