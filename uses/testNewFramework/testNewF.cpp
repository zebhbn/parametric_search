//
// Created by zeb on 01/06/2022.
//

#include "co_task.hpp"
#include "Scheduler.hpp"
#include "ComparisonResolver.hpp"
#include <optional>
#include "LinearFunction.hpp"
auto inf = std::numeric_limits<double>::infinity();

// This should amount to a negative cycle detection algorithm
class SeqAlgoMinimumMeanCycle : public ps_framework::ISeqAlgo{
public:
    SeqAlgoMinimumMeanCycle(std::vector<std::vector<std::optional<ps_framework::LinearFunction>>> *fs){
        p_funcs = fs;
    }

    ps_framework::cmp_res compare(double lambda){
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
                    dist[i][j] = ((*p_funcs)[i])[j].value().compute(-lambda);
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
                return ps_framework::LessThan;
            else if (dist[i][i] == 0)
                zerocycle = true;
        }
        // Megiddo '79 states:
        // If there is a zero cycle and no negative cycles then terminate
        if (zerocycle)
            return ps_framework::EqualTo;
        return ps_framework::GreaterThan;
    }

private:
    std::vector<std::vector<std::optional<ps_framework::LinearFunction>>> *p_funcs;
};

std::vector<std::vector<std::optional<ps_framework::LinearFunction>>> * generateGraphOpt(){
    auto vec = new std::vector<std::vector<std::optional<ps_framework::LinearFunction>>>(
            5,
            std::vector<std::optional<ps_framework::LinearFunction>>(5,std::nullopt));

    // add edges with defined length and time
    (*vec)[0][1].emplace(ps_framework::LinearFunction(1,1));

    (*vec)[1][2].emplace(ps_framework::LinearFunction(1,1));

    (*vec)[1][4].emplace(ps_framework::LinearFunction(1,2));

    (*vec)[2][3].emplace(ps_framework::LinearFunction(2,1));

    (*vec)[3][4].emplace(ps_framework::LinearFunction(1,1));

    (*vec)[4][0].emplace(ps_framework::LinearFunction(2,1));

    return vec;
}


// Naive approach as described by Megiddo '79
// has time complexity O(n⁶)
//double naiveApproach(){
//    auto mat = generateGraphOpt();
//    auto n = mat->size();
//    auto u = generateGraphOpt();
//    auto seqAlgo = SeqAlgoMinimumMeanCycle(mat);
//
//    auto psCore = ps_framework::PSCore(&seqAlgo);
//    auto linComparer = ps_framework::LinearFunctionComparer();
//    auto schedular = ps_framework::Schedular<ps_framework::LinearFunction>(&psCore, &linComparer);
//
//    auto cmp_res = ps_framework::Unresolved;
//
//    for (int m = 0; m<n; m++){
//        for (int i = 0; i<n; i++){
//            for (int j = 0; j<n; j++){
//                auto uij = (*u)[i][j];
//                auto uim = (*u)[i][m];
//                auto umj = (*u)[m][j];
//                if((!uim)||(!umj)){
//                    continue;
//                }
//                else if (!uij){
//                    (*u)[i][j] = {uim.value()+umj.value()};
//                }
//                else {
//                    auto uplus = (uim.value()+umj.value());
//                    schedular.addComparison(&uij.value(), &uplus, &cmp_res);
//                    schedular.resolveComparisons();
//                    if (cmp_res!=ps_framework::Unresolved){
//                        if (cmp_res == ps_framework::GreaterThan){
//                            (*u)[i][j] = {(*u)[i][m].value()+(*u)[m][j].value()};
//                        }
//                        else if (cmp_res == ps_framework::EqualTo){
//                        }
//                        // Reset cmp value
//                        cmp_res = ps_framework::Unresolved;
//                    }
//                }
//            }
//        }
//    }
//    // Destroy things
//    delete mat;
//    delete u;
//    // Return e from pscore ;)
//    return psCore.end;
//}

// A bit better approach also described in Megiddo '79
// has time complexity O(n⁴log(n))
//double betterApproach(){
//    auto mat = generateGraphOpt();
//    auto n = mat->size();
//    auto u = generateGraphOpt();
//    auto seqAlgo = SeqAlgoMinimumMeanCycle(mat);
//
//    auto psCore = ps_framework::PSCore(&seqAlgo);
//    auto linComparer = ps_framework::LinearFunctionComparer();
//    auto schedular = ps_framework::Schedular<ps_framework::LinearFunction>(&psCore, &linComparer);
//
//    auto cmp_res = std::vector<std::vector<ps_framework::cmp_res>> (
//            n,
//            std::vector<ps_framework::cmp_res>(
//                    n,
//                    ps_framework::Unresolved
//            )
//    );
//
//    for (int m = 0; m<n; m++){
//        for (int i = 0; i<n; i++){
//            for (int j = 0; j<n; j++){
//                auto uij = (*u)[i][j];
//                auto uim = (*u)[i][m];
//                auto umj = (*u)[m][j];
//                if((!uim)||(!umj)){
//                    continue;
//                }
//                else if (!uij){
//                    (*u)[i][j] = {uim.value()+umj.value()};
//                }
//                else {
//                    auto uplus = (uim.value()+umj.value());
//                    schedular.addComparison(&uij.value(), &uplus, &cmp_res[i][j]);
//                }
//            }
//        }
//        schedular.resolveComparisons();
//        for (int i = 0; i<n; i++){
//            for (int j = 0; j<n; j++){
//                if (cmp_res[i][j]!=ps_framework::Unresolved){
//                    if (cmp_res[i][j] == ps_framework::GreaterThan){
//                        (*u)[i][j] = {(*u)[i][m].value()+(*u)[m][j].value()};
//                    }
//                    else if (cmp_res[i][j] == ps_framework::EqualTo){
//                    }
//                    // Reset cmp value
//                    cmp_res[i][j] = ps_framework::Unresolved;
//                }
//            }
//        }
//    }
//    // Destroy things
//    delete mat;
//    delete u;
//    // Return e from pscore ;)
//    return psCore.end;
//}


double betterApproach(){
    auto mat = generateGraphOpt();
    auto n = mat->size();
    auto u = generateGraphOpt();
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

//    auto cmp_res = std::vector<std::vector<ps_framework::cmp_res>> (
//            n,
//            std::vector<ps_framework::cmp_res>(
//                    n,
//                    ps_framework::Unresolved
//            )
//    );

    ps_framework::coro_task_void task = [] (
            auto n,
            auto u,
            ps_framework::ComparisonResolver<ps_framework::LinearFunction> *cR,
            ps_framework::Scheduler *scheduler1
            ) -> ps_framework::coro_task_void{
        std::cout<<"1 lambda running"<<std::endl;
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
                        auto task = [](
                                ps_framework::ComparisonResolver<ps_framework::LinearFunction> *cr,
                                auto u,
                                auto i,
                                auto j,
                                auto m,
                                auto uplus,
                                auto uij
                                ) -> ps_framework::coro_task_void {
                            std::cout<<"2 lambda running"<<std::endl;
                            auto res = co_await cr->compare(uij.value(),uplus);
                            std::cout<<"2 lambda running again"<<std::endl;
                            if (res == ps_framework::GreaterThan){
                                std::cout<<"inside if"<<std::endl;
                                (*u)[i][j] = {(*u)[i][m].value()+(*u)[m][j].value()};
                            }
                            std::cout<<"ending lambda 2"<<std::endl;
                            co_return;
                        }(cR, u,i,j,m,uplus,uij);
                        std::cout<<"1 lambda running again"<<std::endl;
                        scheduler1->spawnDependent(&task);
                        std::cout<<"1 lambda running again again"<<std::endl;
//                        imp_scheduler.addComparison(&uij.value(),
//                                                    &uplus,
//                                                    [i,j,m,u](ps_framework::cmp_res res){
//                                                        if (res == ps_framework::GreaterThan){
//                                                            (*u)[i][j] = {(*u)[i][m].value()+(*u)[m][j].value()};
//                                                        }
//                                                    });
                    }
                }
            }
            co_await std::suspend_always();
//        imp_scheduler.resolveCallbackComparisons();
        }
        co_return;
    }(n, u, &comparisonResolver, &scheduler);
//}(n,u);
    scheduler.spawnIndependent(&task);
    auto cmpResolverTask = comparisonResolver.resolveComparisons();
    scheduler.spawnIndependentIntermediate(&cmpResolverTask);
    scheduler.run();

// Destroy things
    delete mat;
    delete u;
    // Return e from pscore ;)
    return psCore.end;
}


ps_framework::coro_task_void setMin(
        ps_framework::ComparisonResolver<ps_framework::LinearFunction> *cr,
        auto u,
        auto i,
        auto j,
        auto m,
        auto uplus,
        auto uij
) {
    std::cout << "2 lambda running" << std::endl;
    auto res = co_await cr->compare(uij.value(), uplus);
    std::cout << "2 lambda running again" << std::endl;
    if (res == ps_framework::GreaterThan) {
        std::cout << "inside if" << std::endl;
        (*u)[i][j] = {(*u)[i][m].value() + (*u)[m][j].value()};
    }
}

ps_framework::coro_task_void ps_floyd(
        auto n,
        auto u,
        ps_framework::ComparisonResolver<ps_framework::LinearFunction> *cR,
        ps_framework::Scheduler *scheduler1
) {
    std::cout<<"1 lambda running"<<std::endl;
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
                    auto task = new ps_framework::coro_task_void(setMin(cR, u,i,j,m,uplus,uij));
                    std::cout<<"1 lambda running again"<<std::endl;
                    co_await scheduler1->spawnDependent(task);
                    std::cout<<"1 lambda running again again"<<std::endl;
                }
            }
        }
        co_await std::suspend_always();
        //        imp_scheduler.resolveCallbackComparisons();
    }
    co_return;
}

double testApproach () {
    auto mat = generateGraphOpt();
    auto n = mat->size();
    auto u = generateGraphOpt();
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
    auto task = new ps_framework::coro_task_void(ps_floyd(n, u, &comparisonResolver, &scheduler));
    scheduler.spawnIndependent(task);
    auto cmpResolverTask = comparisonResolver.resolveComparisons();
    scheduler.spawnIndependentIntermediate(&cmpResolverTask);
    scheduler.run();

// Destroy things
    delete mat;
    delete u;
    // Return e from pscore ;)
    return psCore.end;
}




ps_framework::coro_task_void bobCoro(){
    co_return;
}
void bobTest() {
    ps_framework::coro_task_void t1 = bobCoro();
    ps_framework::coro_task_void t2 = bobCoro();
    ps_framework::coro_task_void t3 = bobCoro();
    ps_framework::coro_task_void t4 = bobCoro();
    t1.handle_.promise().id = 1;
    t2.handle_.promise().id = 2;
    t3.handle_.promise().id = 3;
    t4.handle_.promise().id = 4;
    std::cout<<t1.handle_.promise().id<<std::endl;
    std::cout<<t2.handle_.promise().id<<std::endl;
    std::cout<<t3.handle_.promise().id<<std::endl;
    std::cout<<t4.handle_.promise().id<<std::endl;
    return;
}

int main(){
    bobTest();
//    auto lambdaNA = naiveApproach();
    auto lambdaBA = testApproach();
//    std::cout << "naive approach lambda* = " << lambdaNA << std::endl;
    std::cout << "better approach lambda* = " << lambdaBA << std::endl;
}

