//
// Created by zeb on 15/04/2022.
//

#include <cassert>
#include "ps_interfaces.hpp"
#include "LinearFunction.hpp"
#include "PSCore.hpp"
#include "improved_scheduler.hpp"
#include "schedular.hpp"
#include <optional>
#include <functional>


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
double naiveApproach(){
    auto mat = generateGraphOpt();
    auto n = mat->size();
    auto u = generateGraphOpt();
    auto seqAlgo = SeqAlgoMinimumMeanCycle(mat);

    auto psCore = ps_framework::PSCore(&seqAlgo);
    auto linComparer = ps_framework::LinearFunctionComparer();
    auto schedular = ps_framework::Schedular<ps_framework::LinearFunction>(&psCore, &linComparer);

    auto cmp_res = ps_framework::Unresolved;

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
                    schedular.addComparison(&uij.value(), &uplus, &cmp_res);
                    schedular.resolveComparisons();
                    if (cmp_res!=ps_framework::Unresolved){
                        if (cmp_res == ps_framework::GreaterThan){
                            (*u)[i][j] = {(*u)[i][m].value()+(*u)[m][j].value()};
                        }
                        else if (cmp_res == ps_framework::EqualTo){
                        }
                        // Reset cmp value
                        cmp_res = ps_framework::Unresolved;
                    }
                }
            }
        }
    }
    // Destroy things
    delete mat;
    delete u;
    // Return e from pscore ;)
    return psCore.end;
}

// A bit better approach also described in Megiddo '79
// has time complexity O(n⁴log(n))
double betterApproach(){
    auto mat = generateGraphOpt();
    auto n = mat->size();
    auto u = generateGraphOpt();
    auto seqAlgo = SeqAlgoMinimumMeanCycle(mat);

    auto psCore = ps_framework::PSCore(&seqAlgo);
    auto linComparer = ps_framework::LinearFunctionComparer();
    auto schedular = ps_framework::Schedular<ps_framework::LinearFunction>(&psCore, &linComparer);

    auto cmp_res = std::vector<std::vector<ps_framework::cmp_res>> (
            n,
            std::vector<ps_framework::cmp_res>(
                    n,
                    ps_framework::Unresolved
                    )
            );

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
                    schedular.addComparison(&uij.value(), &uplus, &cmp_res[i][j]);
                }
            }
        }
        schedular.resolveComparisons();
        for (int i = 0; i<n; i++){
            for (int j = 0; j<n; j++){
                if (cmp_res[i][j]!=ps_framework::Unresolved){
                    if (cmp_res[i][j] == ps_framework::GreaterThan){
                        (*u)[i][j] = {(*u)[i][m].value()+(*u)[m][j].value()};
                    }
                    else if (cmp_res[i][j] == ps_framework::EqualTo){
                    }
                    // Reset cmp value
                    cmp_res[i][j] = ps_framework::Unresolved;
                }
            }
        }
    }
    // Destroy things
    delete mat;
    delete u;
    // Return e from pscore ;)
    return psCore.end;
}


double betterApproachImproved(){
    auto mat = generateGraphOpt();
    auto n = mat->size();
    auto u = generateGraphOpt();
    auto seqAlgo = SeqAlgoMinimumMeanCycle(mat);

    auto psCore = ps_framework::PSCore(&seqAlgo);
    auto linComparer = ps_framework::LinearFunctionComparer();
    auto imp_scheduler = ps_framework::ImprovedScheduler<ps_framework::LinearFunction>(&psCore, &linComparer);

//    auto cmp_res = std::vector<std::vector<ps_framework::cmp_res>> (
//            n,
//            std::vector<ps_framework::cmp_res>(
//                    n,
//                    ps_framework::Unresolved
//            )
//    );

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
                    imp_scheduler.addComparison(&uij.value(),
                                                &uplus,
                                                [i,j,m,u](ps_framework::cmp_res res){
                        if (res == ps_framework::GreaterThan){
                            (*u)[i][j] = {(*u)[i][m].value()+(*u)[m][j].value()};
                        }
                    });
                }
            }
        }
        imp_scheduler.resolveCallbackComparisons();
    }
    // Destroy things
    delete mat;
    delete u;
    // Return e from pscore ;)
    return psCore.end;
}

int main(){
    auto lambdaNA = naiveApproach();
    auto lambdaBA = betterApproach();
    std::cout << "naive approach lambda* = " << lambdaNA << std::endl;
    std::cout << "better approach lambda* = " << lambdaBA << std::endl;
}
