//
// Created by zeb on 03/06/2022.
//


#include "CoTask.hpp"
#include "Scheduler.hpp"
#include "ComparisonResolver.hpp"
#include <optional>
#include "LinearFunction.hpp"
#include "GraphGenerators.hpp"

auto inf = std::numeric_limits<double>::infinity();

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
        // If there is a negative cycle then
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
        (*u)[i][j] = {(*u)[i][m].value() + (*u)[m][j].value()};
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
                    (*u)[i][j] = {uim.value()+umj.value()};
                }
                else {
                    auto uplus = (uim.value()+umj.value());
                    //                    imp_scheduler.addComparison(&uij.value(), &uplus, &cmp_res[i][j]);
                    auto task = new ps_framework::coroTaskVoid(setMin(cR, u, i, j, m, uplus, uij));
                    co_await scheduler1->spawnDependent(task);
                    co_await std::suspend_always();
                }
//                co_await std::suspend_always();
            }
//            co_await std::suspend_always();
        }
//        co_await std::suspend_always();
//        co_await std::suspend_always();
        //        imp_scheduler.resolveCallbackComparisons();
    }
    co_return;
}

double approach () {
    auto mat = generateBigGraphOpt();
//    auto mat = generateBigGraphOpt();
    auto n = mat->size();
    auto u = generateBigGraphOpt();
//    auto u = generateBigGraphOpt();
    auto seqAlgo = SeqAlgoMinimumMeanCycle(mat);

    auto psCore = ps_framework::PSCore(&seqAlgo);
    auto linComparer = ps_framework::LinearFunctionComparer();
//    auto imp_scheduler = ps_framework::ImprovedScheduler<ps_framework::LinearFunction>(&psCore, &linComparer);
    ps_framework::Scheduler scheduler = ps_framework::Scheduler();
    ps_framework::ComparisonResolver comparisonResolver = ps_framework::ComparisonResolver<ps_framework::LinearFunction>(
            &scheduler,
            &psCore,
            &linComparer
    );
    auto task = new ps_framework::coroTaskVoid(psFloyd(n, u, &comparisonResolver, &scheduler));
    scheduler.spawnIndependent(task);
    auto cmpResolverTask = comparisonResolver.resolveComparisons();
//    scheduler.spawnIndependentIntermediate(&cmpResolverTask);
    scheduler.run();

    // Test
    auto test = seqAlgo.compare(0.25263157895);
    std::cout<<"Test result: "<<test<<std::endl;


// Destroy things
    delete mat;
    delete u;
    // Return e from pscore ;)
    std::cout<<"["<<psCore.start<<",  "<<psCore.end<<"]"<<std::endl;
    return psCore.start;
}

int main(){
    auto lambda = approach();
    std::cout << "approach lambda* = " << lambda << std::endl;
}

