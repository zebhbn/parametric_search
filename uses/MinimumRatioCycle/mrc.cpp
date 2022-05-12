//
// Created by zeb on 15/04/2022.
//

#include <cassert>
#include "ps_interfaces.hpp"
#include "LinearFunction.hpp"
#include "PSCore.hpp"
#include "schedular.hpp"
#include <optional>


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
        // Find out if there is a negative cycle
//        bool zerocycle = false;
//        for (int i = 0; i<n; i++){
//            for (int j = 0; j<n; j++){
////                std::cout<<dist[i][j]<<std::endl;
//                if (dist[i][j] < 0)
//                    return ps_framework::LessThan;
//                else if (dist[i][j] == 0)
//                    zerocycle = true;
//            }
//        }
        bool zerocycle = false;
        for (int i = 0; i<n; i++){
//                std::cout<<dist[i][j]<<std::endl;
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

double psVersion(){
    assert(false);
    // Following approach 1 in megiddo 83'

    // 1. Start by creating comparison between each ordered pair
    // (i,j)
    // 2. Then using a new comparerer and object definition generate
    // O(V³) breakpoints. This will probably require a modification
    // to the current framework...
    // 3. Use binary search/PSCore with negative cycle detection
    // algorithm used as sequential algorithm ISeqAlgo.
}

//std::vector<std::vector<ps_framework::LinearFunction>> * generateGraph(){
//    auto vec = new std::vector<std::vector<ps_framework::LinearFunction>>(
//            5,
//            std::vector(5,ps_framework::LinearFunction(
//                    std::numeric_limits<double>::infinity(),
//                    0.0)));
//    for (int i = 0; i<5;i++) {
//        (((*vec)[i])[i]).a = 0;
//        (((*vec)[i])[i]).b = 0;
//    }
//    // add edges with defined length and time
//
//    (*vec)[0][1].a = 1;
//    (*vec)[0][1].b = 1;
//
//    (*vec)[1][2].a = 1;
//    (*vec)[1][2].b = 1;
//
//    (*vec)[1][4].a = 2;
//    (*vec)[1][4].b = 1;
//
//    (*vec)[2][3].a = 1;
//    (*vec)[2][3].b = 2;
//
//    (*vec)[3][4].a = 1;
//    (*vec)[3][4].b = 1;
//
//    (*vec)[2][3].a = 1;
//    (*vec)[2][3].b = 2;
//
//    return vec;
//}

std::vector<std::vector<std::optional<ps_framework::LinearFunction>>> * generateGraphOpt(){
//    auto vec = new std::vector<std::vector<ps_framework::LinearFunction>>(
//            5,
//            std::vector(5,std::nullopt));
    auto vec = new std::vector<std::vector<std::optional<ps_framework::LinearFunction>>>(
            5,
            std::vector<std::optional<ps_framework::LinearFunction>>(5,std::nullopt));
//    for (int i = 0; i<5;i++) {
//        (((*vec)[i])[i]).a = 0;
//        (((*vec)[i])[i]).b = 0;
//    }
    // add edges with defined length and time
//    (*vec)[0][1].emplace(ps_framework::LinearFunction(1,1));
    (*vec)[0][1].emplace(ps_framework::LinearFunction(1,1));

//    (*vec)[1][2].emplace(ps_framework::LinearFunction(1,1));
    (*vec)[1][2].emplace(ps_framework::LinearFunction(1,1));

//    (*vec)[1][4].emplace(ps_framework::LinearFunction(2,1));
    (*vec)[1][4].emplace(ps_framework::LinearFunction(1,2));

//    (*vec)[2][3].emplace(ps_framework::LinearFunction(1,2));
    (*vec)[2][3].emplace(ps_framework::LinearFunction(2,1));

//    (*vec)[3][4].emplace(ps_framework::LinearFunction(1,1));
    (*vec)[3][4].emplace(ps_framework::LinearFunction(1,1));

//    (*vec)[4][0].emplace(ps_framework::LinearFunction(1,2));
    (*vec)[4][0].emplace(ps_framework::LinearFunction(2,1));

    return vec;
}

// Solves uij = uim + umj
//double solve_t(ps_framework::LinearFunction ij,
//               ps_framework::LinearFunction im,
//               ps_framework::LinearFunction mj){
//    return (im.a + mj.a - ij.a) / (im.b - mj.b - ij.b);
//}




// Following the very naive approach as explain in Megiddo '79
double nonPsVersion(){
//    double e = - std::numeric_limits<double>::infinity();
//    double f = std::numeric_limits<double>::infinity();
//    int i,j,m = 0;
//
//    auto mat = generateGraph();
//    auto u = generateGraph();
//    auto seqAlgo = SeqAlgoMinimumMeanCycle(mat);
//
//    while(1){
//        auto tp = solve_t((*mat)[i][j],
//                          (*mat)[i][m],
//                          (*mat)[j][m]);
//        // The unique solution is inside the interval
//        if (e<tp && tp<f){
//            auto res = seqAlgo.compare(tp);
//            if (res == ps_framework::EqualTo){
//                return tp;
//            } else if (res == ps_framework::LessThan) {
//                f = tp;
//            } else {
//                e = tp;
//            }
//        }
//
//
//    }
//
//    assert(false);
}

double naiveApproach(){
    auto mat = generateGraphOpt();
    auto n = mat->size();
    auto u = generateGraphOpt();
    auto seqAlgo = SeqAlgoMinimumMeanCycle(mat);

    auto psCore = ps_framework::PSCore(&seqAlgo);
    auto linComparer = ps_framework::LinearFunctionComparer();
    auto schedular = ps_framework::Schedular<ps_framework::LinearFunction>(&psCore, &linComparer);

    auto inf = std::numeric_limits<double>::infinity();
//    auto cmp_res = std::vector<std::vector<ps_framework::cmp_res>> (
//            n,
//            std::vector<ps_framework::cmp_res>(
//                    n,
//                    ps_framework::Unresolved
//            )
//    );
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
//                    auto up = new ps_framework::LinearFunction(uplus.a, )
                    schedular.addComparison(&uij.value(), &uplus, &cmp_res);
                    schedular.resolveComparisons();
                    if (cmp_res!=ps_framework::Unresolved){
                        if (cmp_res == ps_framework::GreaterThan){
//                            std::cout<<"comparison resolved"<<std::endl;
                            (*u)[i][j] = {(*u)[i][m].value()+(*u)[m][j].value()};
                        }
                        else if (cmp_res == ps_framework::EqualTo){
//                            std::cout<<"Found lambda star by accident:)"<<std::endl;
                        }
                        // Reset cmp value
                        cmp_res = ps_framework::Unresolved;
                    }
                }
            }
        }
//        schedular.resolveComparisons();
//        for (int i = 0; i<n; i++){
//            for (int j = 0; j<n; j++){
//                if (cmp_res[i][j]!=ps_framework::Unresolved){
//                    if (cmp_res[i][j] == ps_framework::GreaterThan){
//                        std::cout<<"comparison resolved"<<std::endl;
//                        (*u)[i][j] = {(*u)[i][m].value()+(*u)[m][j].value()};
//                    }
//                    else if (cmp_res[i][j] == ps_framework::EqualTo){
//                        std::cout<<"Found lambda star by accident:)"<<std::endl;
//                    }
//                    // Reset cmp value
//                    cmp_res[i][j] = ps_framework::Unresolved;
//                }
//            }
//        }
    }
    // Return e from pscore ;)
    return psCore.end;
}

double betterApproach(){
    auto mat = generateGraphOpt();
    auto n = mat->size();
    auto u = generateGraphOpt();
    auto seqAlgo = SeqAlgoMinimumMeanCycle(mat);

    auto psCore = ps_framework::PSCore(&seqAlgo);
    auto linComparer = ps_framework::LinearFunctionComparer();
    auto schedular = ps_framework::Schedular<ps_framework::LinearFunction>(&psCore, &linComparer);

    auto inf = std::numeric_limits<double>::infinity();
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
//                    auto up = new ps_framework::LinearFunction(uplus.a, )
                    schedular.addComparison(&uij.value(), &uplus, &cmp_res[i][j]);
//                    std::cout<<"adding comparison to scheduler"<<std::endl;
                }
            }
        }
        schedular.resolveComparisons();
        for (int i = 0; i<n; i++){
            for (int j = 0; j<n; j++){
                if (cmp_res[i][j]!=ps_framework::Unresolved){
                    if (cmp_res[i][j] == ps_framework::GreaterThan){
//                        std::cout<<"comparison resolved"<<std::endl;
                        (*u)[i][j] = {(*u)[i][m].value()+(*u)[m][j].value()};
                    }
                    else if (cmp_res[i][j] == ps_framework::EqualTo){
//                        std::cout<<"Found lambda star by accident:)"<<std::endl;
                    }
                    // Reset cmp value
                    cmp_res[i][j] = ps_framework::Unresolved;
                }
            }
        }
    }
    // Return e from pscore ;)
    return psCore.end;
}

//void test(){
//    char *ResTypes[] =
//            {
//                    "LessThan",
//                    "EqualTo",
//                    "GreaterThan",
//                    "Unresolved"
//            };
//    auto mat = generateGraphOpt();
//    auto bob = (*mat)[0][1];
//    std::cout<<"A is: "<<bob.value().a<<std::endl;
//    auto n = mat->size();
//    auto u = generateGraphOpt();
//    u->push_back(std::vector<std::optional<ps_framework::LinearFunction>>(n,std::nullopt));
//    auto seqAlgo = SeqAlgoMinimumMeanCycle(mat);
//    double l = 0.5;
//    double e = 5.0/7.0;
//    double g = 2;
//
//    std::cout<<"number is: "<<l<<std::endl;
//    auto res = seqAlgo.compare(l);
//    std::cout<<"Result: "<<ResTypes[res]<<" "<<res<<std::endl;
//
//    std::cout<<"number is: "<<e<<std::endl;
//    res = seqAlgo.compare(e);
//    std::cout<<"Result: "<<ResTypes[res]<<" "<<res<<std::endl;
//
//    std::cout<<"number is: "<<g<<std::endl;
//    res = seqAlgo.compare(g);
//    std::cout<<"Result: "<<ResTypes[res]<<" "<<res<<std::endl;
//}

int main(){
//    test();
    auto lambdaNA = naiveApproach();
    auto lambdaBA = betterApproach();
    std::cout << "naive approach lambda* = " << lambdaNA << std::endl;
    std::cout << "better approach lambda* = " << lambdaBA << std::endl;
}
