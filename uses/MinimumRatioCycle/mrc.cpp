//
// Created by zeb on 03/06/2022.
//


#include "CoTask.hpp"
#include "Scheduler.hpp"
#include "ComparisonResolver.hpp"
#include <optional>
#include "LinearFunction.hpp"
#include "GraphGenerators.hpp"
#include "MultiTScheduler.hpp"
#include "MultiTComparisonResolver.hpp"
#include <mutex>
#include <condition_variable>

auto inf = std::numeric_limits<double>::infinity();
std::mutex setMutex;
std::condition_variable setCV;


// This should amount to a negative cycle detection algorithm
class SeqAlgoMinimumMeanCycle : public ps_framework::ISeqAlgo{
public:
    SeqAlgoMinimumMeanCycle(std::vector<std::vector<std::optional<ps_framework::LinearFunction>>> *fs){
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

std::vector<std::vector<std::optional<ps_framework::LinearFunction>>> * generateGraphOpt(){
    auto vec = new std::vector<std::vector<std::optional<ps_framework::LinearFunction>>>(
            5,
            std::vector<std::optional<ps_framework::LinearFunction>>(5,std::nullopt));

    // add edges with defined length and time
    (*vec)[0][1].emplace(ps_framework::LinearFunction(-1,1));

    (*vec)[1][2].emplace(ps_framework::LinearFunction(-1,1));

    (*vec)[1][4].emplace(ps_framework::LinearFunction(-1,2));

    (*vec)[2][3].emplace(ps_framework::LinearFunction(-2,1));

    (*vec)[3][4].emplace(ps_framework::LinearFunction(-1,1));

    (*vec)[4][0].emplace(ps_framework::LinearFunction(-2,1));

    return vec;
}


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
//    std::cout<<"Set Min resumed"<<std::endl;
    if (res == ps_framework::GreaterThan) {
//        (*u)[i][j] = {(*u)[i][m].value() + (*u)[m][j].value()};
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

double approach () {
    auto mat = generateGraphOpt();
//    auto mat = generateBigGraphOpt();
    auto n = mat->size();
    auto u = generateGraphOpt();
//    auto u = generateBigGraphOpt();
    auto seqAlgo = SeqAlgoMinimumMeanCycle(mat);

    auto psCore = ps_framework::PSCore(&seqAlgo);
    auto linComparer = ps_framework::LinearFunctionComparer();
//    ps_framework::Scheduler scheduler = ps_framework::Scheduler();
    ps_framework::MultiTScheduler scheduler = ps_framework::MultiTScheduler();
//    ps_framework::ComparisonResolver comparisonResolver = ps_framework::ComparisonResolver<ps_framework::LinearFunction>(
//            &scheduler,
//            &psCore,
//            &linComparer
//    );
    ps_framework::MultiTComparisonResolver comparisonResolver = ps_framework::MultiTComparisonResolver<ps_framework::LinearFunction>(
            &scheduler,
            &psCore,
            &linComparer
    );

    auto task = new ps_framework::coroTaskVoid(psFloyd(n, u, &comparisonResolver, &scheduler));
    scheduler.spawnIndependent(task);
    scheduler.run();

// Destroy things
    delete mat;
    delete u;
    // Return end of interval from pscore even though this should be
    // start of interval, but because of loss in precision lambda* is in end
    return psCore.end;
}

int main(){
    auto lambda = approach();
    std::cout << "approach lambda* = " << lambda << std::endl;
    std::cout << "5/7 = " << 5.0/7.0 << std::endl;
    exit(0);
}

