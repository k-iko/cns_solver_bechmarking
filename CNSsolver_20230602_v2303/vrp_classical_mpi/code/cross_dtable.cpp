#include <map>
#include <algorithm>
#include <limits.h>
#include <cassert>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <omp.h>
#ifdef MPI_ENABLED 
    #include <mpi.h>
#endif

#define STRING2CHAR_LENGTH  128

namespace py = pybind11;
using namespace std;
#include "cross_dtable.hpp"

namespace{
    bool vector_finder(const vector<int> &vec, const int number);
}

int str2char(string str, vector<char> &ch)
{
    int str_size = str.size();
    for(int i=0; i<STRING2CHAR_LENGTH; i++){
        if(i < str_size){
            ch.push_back(str.at(i));
        }
        else{
            ch.push_back(NULL);
        }
    }
    return str_size;
}

void char2str(vector<char> ch, string &str, int chidx=0)
{
    str = "";
    for(int i=STRING2CHAR_LENGTH*chidx; i<STRING2CHAR_LENGTH*chidx+STRING2CHAR_LENGTH; i++){
        if(ch.at(i) != NULL){
            str.push_back(ch.at(i));
        }
    }
}

/**
 * @brief cross処理を実施する関数
 * @param[in] vehicles 車両リスト
 * @param[in] customers 配送先リスト
 * @param[in] argvs グローバル変数格納用
 * @param[in] no_depo_parkt_flag 簡易アルゴリズム使用フラグ(true : 使用可能)
 * @param[in] one_table_flag 単一テーブル判定フラグ(true : 単一テーブル)
 * @return tuple<int, string, vector<Vehicle>> 改善回数、メッセージ、改善後の車両リスト
 * @details cross処理後の改善有無を判定し改善する場合はその解を採用し、改善がなくなるまで繰り返す関数。
 */
tuple<int, string, vector<Vehicle>, vector<Customer>> cross_dtable(vector<Vehicle> &vehicles, vector<Customer> &customers, Cppargs &argvs, bool no_depo_parkt_flag, bool one_table_flag){
    int rank, size;
    #ifdef MPI_ENABLED 
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
    #else
        rank = 0;
        size = 1;
    #endif

    string sLogBuf="\n";
    int improved_cnt = 0;
    long long int vehnum = vehicles.size();
    vector<int> exclude_list;

    map<tuple<string, double>, vector<vector<double>>>  t_ttable;
    map<tuple<string, double>, vector<vector<double>>>  t_dtable;
    map<tuple<string, string, double>, vector<vector<double>>>  t_etable;
    map<tuple<string, double>, vector<vector<double>>>  t_xtable;
    timespec ts;
    double dtable_endtime;
    if(argvs.timeout > 0){
        timespec_get(&ts, TIME_UTC);
        dtable_endtime = ts.tv_sec + ts.tv_nsec * 1e-9;

        if(dtable_endtime-argvs.ls_d_starttime > argvs.timeout){ //タイムアウト処理
            sLogBuf += "cross timeout!\n";
            return make_tuple(improved_cnt, sLogBuf, vehicles, customers);
        }
    }
    
    //並列数
    int para_num;
    if(size == 1){ //OpenMP適用
        if(argvs.multithread_num == 0 || argvs.multithread_num > omp_get_num_procs()){
            para_num = omp_get_num_procs();
        }
        else{
            para_num = argvs.multithread_num;
        }
    }
    else{ //MPI適用
        para_num = 1;
    }

    omp_set_num_threads(para_num);
    
    while(1){
        bool my_improved = false, improved = false;
        long long int max_route_size = 0;

        struct{
            double val = 1e20;
            long long int idx;
        } my_min_pair, min_pair;

        //ルートの最大サイズ計算
        for(auto v : vehicles){
            long long int route_size = v.route.size();
            max_route_size = max(max_route_size, route_size);
        }

        assert(vehnum < LLONG_MAX/vehnum && 
                vehnum*vehnum < LLONG_MAX/max_route_size && 
                vehnum*vehnum*max_route_size < LLONG_MAX/max_route_size && 
                vehnum*vehnum*max_route_size*max_route_size < LLONG_MAX/max_route_size && 
                vehnum*vehnum*max_route_size*max_route_size*max_route_size < LLONG_MAX/max_route_size); //最大値を超えないかチェック

        long long int loop_size = vehnum*vehnum*max_route_size*max_route_size*max_route_size*max_route_size; //最大ループ数

        vector<int> custs1_best, custs2_best;
        bool pick_opt_flag1_best, pick_opt_flag2_best;
        vector<int> route1_best, route2_best;
        map<tuple<string, double>, vector<vector<double>>>  management_ttable_best;
        map<tuple<string, double>, vector<vector<double>>>  management_dtable_best;
        map<tuple<string, string, double>, vector<vector<double>>>  management_etable_best;
        map<tuple<string, double>, vector<vector<double>>>  management_xtable_best;
        vector<vector<int>> t_route_best;
        vector<Customer> customer_best;
        if(argvs.break_flag){
            vector<int> route1_best, route2_best;
            management_ttable_best = argvs.management_ttable;
            management_dtable_best = argvs.management_dtable;
            management_etable_best = argvs.management_etable;
            management_xtable_best = argvs.management_xtable;
            t_route_best = r_backup(vehicles);
            customer_best = customers;
        }
        //OpenMP用
        vector<double> min_vec(para_num, 1e20);
        vector<long long int> min_idx_vec(para_num);
        vector<vector<int>> min_custs1_best(para_num), min_custs2_best(para_num);
        vector<bool> min_pick_opt_flag1_best(para_num), min_pick_opt_flag2_best(para_num);
        string thrown_str = "";

        vector<vector<int>> min_route1_best(para_num), min_route2_best(para_num);
        vector<map<tuple<string, double>, vector<vector<double>>>>  min_management_ttable_best(para_num);
        vector<map<tuple<string, double>, vector<vector<double>>>>  min_management_dtable_best(para_num);
        vector<map<tuple<string, string, double>, vector<vector<double>>>>  min_management_etable_best(para_num);
        vector<map<tuple<string, double>, vector<vector<double>>>>  min_management_xtable_best(para_num);
        vector<vector<vector<int>>> min_t_route_best(para_num);
        vector<vector<Customer>> min_customer_best(para_num);
        vector<int> route1, route2;
        map<tuple<string, double>, vector<vector<double>>>  ttable;
        map<tuple<string, double>, vector<vector<double>>>  dtable;
        map<tuple<string, string, double>, vector<vector<double>>>  etable;
        map<tuple<string, double>, vector<vector<double>>>  xtable;
        vector<vector<int>> t_route;
        vector<Customer> cust;
        // // 平準化高速化の為のソート
        // int vehicles_size = vehicles.size();
        // vector<int> sorted_vehicles;
        // for(int vi=0; vi<vehicles_size; vi++){
        //     sorted_vehicles.push_back(vi);
        // }
        // if(argvs.avedist_ape_flag || argvs.avedist_ae_flag){
        //     vector<double> derrs;
        //     if(argvs.avedist_ape_flag){
        //         derrs = calc_avedist_APE_list(vehicles, customers, argvs.avedist_ape, argvs.lastc_flag);
        //     }
        //     else{
        //         derrs = calc_avedist_AE_list(vehicles, customers, argvs.avedist_ae, argvs.lastc_flag);
        //     }
        //     for(int c1=0; c1<vehicles_size-1; c1++){
        //         for(int c2=c1+1; c2<vehicles_size; c2++){
        //             if(derrs.at(sorted_vehicles.at(c1)) < derrs.at(sorted_vehicles.at(c2))){
        //                 swap(sorted_vehicles.at(c1), sorted_vehicles.at(c2));
        //             }
        //         }
        //     }
        // }
        // if(argvs.avedist_ape_flag || argvs.avedist_ae_flag){
        //     vector<double> verrs;
        //     if(argvs.avedist_ape_flag){
        //         verrs = calc_avevisits_APE_list(vehicles, customers, argvs.avevisits_ape, argvs.bulkShipping_flag);
        //     }
        //     else{
        //         verrs = calc_avevisits_AE_list(vehicles, customers, argvs.avevisits_ae, argvs.bulkShipping_flag);
        //     }
        //     for(int c1=0; c1<vehicles_size-1; c1++){
        //         for(int c2=c1+1; c2<vehicles_size; c2++){
        //             if(verrs.at(sorted_vehicles.at(c1)) < verrs.at(sorted_vehicles.at(c2))){
        //                 swap(sorted_vehicles.at(c1), sorted_vehicles.at(c2));
        //             }
        //         }
        //     }
        // }
        // if(argvs.avedist_ape_flag || argvs.avedist_ae_flag){
        //     vector<double> terrs;
        //     if(argvs.avedist_ape_flag){
        //         terrs = calc_avetime_APE_list(vehicles, customers, argvs.avetime_ape, argvs.lastc_flag);
        //     }
        //     else{
        //         terrs = calc_avetime_AE_list(vehicles, customers, argvs.avetime_ae, argvs.lastc_flag);
        //     }
        //     for(int c1=0; c1<vehicles_size-1; c1++){
        //         for(int c2=c1+1; c2<vehicles_size; c2++){
        //             if(terrs.at(sorted_vehicles.at(c1)) < terrs.at(sorted_vehicles.at(c2))){
        //                 swap(sorted_vehicles.at(c1), sorted_vehicles.at(c2));
        //             }
        //         }
        //     }
        // }

        Cppargs mt_argvs = argvs;
        vector<Vehicle> mt_vehicles = vehicles;
        vector<Customer> mt_customers = customers;
        #pragma omp parallel firstprivate(my_min_pair, custs1_best, custs2_best, pick_opt_flag1_best, pick_opt_flag2_best, route1_best, route2_best, management_ttable_best, management_dtable_best, management_etable_best, management_xtable_best, customer_best, t_route_best, route1, route2, ttable, dtable, etable, xtable, t_route, cust, mt_argvs, mt_vehicles, mt_customers)
        {
        #pragma omp for schedule(dynamic, max_route_size*max_route_size)
        for(long long int cnt = rank; cnt < loop_size; cnt += size){
            try{
                bool exists = false;
                double cdiff = 0;
                bool pick_opt_flag1 = false, pick_opt_flag2 = false;

                int vehno1 = cnt/(vehnum*max_route_size*max_route_size*max_route_size*max_route_size);
                int vehno2 = (cnt/(max_route_size*max_route_size*max_route_size*max_route_size))%vehnum;
                // if(vehno2 <= vehno1){
                //     continue; //条件外の場合スキップ
                // }
                // vehno1 = sorted_vehicles.at(vehno1);    // 平準化のソートの結果を反映
                // vehno2 = sorted_vehicles.at(vehno2);
                int p1 = (cnt/(max_route_size*max_route_size*max_route_size))%max_route_size;
                int p2 = (cnt/(max_route_size*max_route_size))%max_route_size;
                int p3 = (cnt/max_route_size)%max_route_size;
                int p4 = cnt%max_route_size;
                int route_size1 = vehicles.at(vehno1).route.size();
                int route_size2 = vehicles.at(vehno2).route.size();
                bool all_depo1 = true, all_depo2 =true;




                // if(route_size1 < p2 || route_size2 < p4
                if(vehno2 <= vehno1
                || route_size1 < p2 || route_size2 < p4
                || p1 < 1 || p3 < 1 || p2 <= p1 || p4 <= p3
                || vehicles.at(vehno1).direct_flag == true || vehicles.at(vehno2).direct_flag == true
                || vector_finder(exclude_list, cnt) == true){
                    continue; //条件外の場合スキップ
                }

                for(int i = 1; i < route_size1-1; i++){
                    if(customers.at(vehicles.at(vehno1).route.at(i)).custno != "0"){
                        all_depo1 = false;
                        break; 
                    }
                }
                for(int i = 1; i < route_size2-1; i++){
                    if(customers.at(vehicles.at(vehno2).route.at(i)).custno != "0"){
                        all_depo2 = false;
                        break; 
                    }
                }

                if(argvs.break_flag){
                    // 休憩スポットが含まれていたらfalseを返す
                    for(int i = 1; i < route_size1-1; i++){
                        if(customers.at(vehicles.at(vehno1).route.at(i)).custno[0] != 'b'){
                            all_depo1 = false;
                            break; 
                        }
                    }
                    for(int i = 1; i < route_size2-1; i++){
                        if(customers.at(vehicles.at(vehno2).route.at(i)).custno[0] != 'b'){
                            all_depo2 = false;
                            break; 
                        }
                    }
                }

                if(all_depo1 == true || all_depo2 == true){
                    continue; //空の場合はスキップ
                }

                if(argvs.timeout > 0){
                    timespec_get(&ts, TIME_UTC);
                    dtable_endtime = ts.tv_sec + ts.tv_nsec * 1e-9;
                    if(dtable_endtime-argvs.ls_d_starttime > argvs.timeout){ //途中結果を出力
                        continue;
                    }
                }

                // dtable_endtime = time(NULL);
                // if(argvs.outputstime > 0 && dtable_endtime-argvs.ls_d_starttime > argvs.outputstime*argvs.outputs_count){ //途中結果を出力
                //     sLogBuf += "output_middle cross\n";
                //     outputs_middle(vehicles, customers, argvs, sLogBuf);
                //     argvs.outputs_count++;
                // }
                // while(1){
                //     if(argvs.outputstime > 0 && dtable_endtime-argvs.ls_d_starttime > argvs.outputstime*argvs.outputs_count){ //途中結果を出力
                //         argvs.outputs_count++;
                //     }
                //     else{
                //         break;
                //     }
                // }

                // if(argvs.timeout > 0 && dtable_endtime-argvs.ls_d_starttime > argvs.timeout){ //途中結果を出力
                //     sLogBuf += "cross timeout!\n";
                //     break;
                // }


                //改善判定
                vector<int> custs1, custs2;
                if(argvs.lastc_flag == true){
                    tie(exists, custs1, custs2, pick_opt_flag1, pick_opt_flag2, route1, route2, ttable, dtable, etable, xtable, t_route, cust) = cross_improved_lastc_dtable(mt_vehicles.at(vehno1), mt_vehicles.at(vehno2), mt_customers, p1, p2, p3, p4, mt_vehicles, mt_argvs, cdiff, no_depo_parkt_flag, one_table_flag);
                }
                else{
                    tie(exists, custs1, custs2, pick_opt_flag1, pick_opt_flag2, route1, route2, ttable, dtable, etable, xtable, t_route, cust) = cross_improved_todepot_dtable(mt_vehicles.at(vehno1), mt_vehicles.at(vehno2), mt_customers, p1, p2, p3, p4, mt_vehicles, mt_argvs, cdiff, no_depo_parkt_flag, one_table_flag);
                }
                if(exists && cdiff < my_min_pair.val){ //改善があった場合
                    my_improved = true;
                    my_min_pair.val = cdiff;
                    my_min_pair.idx = cnt;
                    
                    custs1_best = custs1;
                    custs2_best = custs2;

                    pick_opt_flag1_best = pick_opt_flag1;
                    pick_opt_flag2_best = pick_opt_flag2;
                    if(argvs.break_flag){
                        route1_best = route1;
                        route2_best = route2;

                        management_ttable_best = ttable;
                        management_dtable_best = dtable;
                        management_etable_best = etable;
                        management_xtable_best = xtable;
                        t_route_best = t_route;
                        customer_best = cust;
                    }
                }
            }
            catch(string& thrown_str){
                continue;
            }
        }

        
        min_vec.at(omp_get_thread_num()) = my_min_pair.val;
        min_idx_vec.at(omp_get_thread_num()) = my_min_pair.idx;
        min_custs1_best.at(omp_get_thread_num()) = custs1_best;
        min_custs2_best.at(omp_get_thread_num()) = custs2_best;
        min_pick_opt_flag1_best.at(omp_get_thread_num()) = pick_opt_flag1_best;
        min_pick_opt_flag2_best.at(omp_get_thread_num()) = pick_opt_flag2_best;
        if(argvs.break_flag){
            min_route1_best.at(omp_get_thread_num()) = route1_best;
            min_route2_best.at(omp_get_thread_num()) = route2_best;
            min_management_ttable_best.at(omp_get_thread_num()) = management_ttable_best;
            min_management_dtable_best.at(omp_get_thread_num()) = management_dtable_best;
            min_management_etable_best.at(omp_get_thread_num()) = management_etable_best;
            min_management_xtable_best.at(omp_get_thread_num()) = management_xtable_best;
            min_customer_best.at(omp_get_thread_num()) = customer_best;
            min_t_route_best.at(omp_get_thread_num()) = t_route_best;
        }
        }

        if(thrown_str != ""){
            throw invalid_argument(thrown_str);
        }

        if(size == 1){ //OpenMP用
            improved = my_improved;
        }
    #ifdef MPI_ENABLED 
        else{ //MPI用
            MPI_Allreduce(&my_improved, &improved, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);
        }
    #endif

        if(!improved){
            break; //改善がなかった場合終了
        }
        else{
            if(size == 1){//OpenMP用
                double min_val = 1e20;
                min_pair.idx = -1;
                for(int i = 0; i < para_num; i++){
                    if(min_vec.at(i) < min_val){
                        min_val = min_vec.at(i);
                        min_pair.idx = min_idx_vec.at(i);
                        custs1_best = min_custs1_best.at(i);
                        custs2_best = min_custs2_best.at(i);
                        pick_opt_flag1_best = min_pick_opt_flag1_best.at(i);
                        pick_opt_flag2_best = min_pick_opt_flag2_best.at(i);
                        if(argvs.break_flag){
                            route1_best = min_route1_best.at(i);
                            route2_best = min_route2_best.at(i);
                            management_ttable_best = min_management_ttable_best.at(i);
                            management_dtable_best = min_management_dtable_best.at(i);
                            management_etable_best = min_management_etable_best.at(i);
                            management_xtable_best = min_management_xtable_best.at(i);
                            t_route_best = min_t_route_best.at(i);
                            customer_best = min_customer_best.at(i);
                        }
                    }
                    else if(min_vec.at(i) == min_val && min_idx_vec.at(i) < min_pair.idx){
                        min_pair.idx = min_idx_vec.at(i);
                        custs1_best = min_custs1_best.at(i);
                        custs2_best = min_custs2_best.at(i);
                        pick_opt_flag1_best = min_pick_opt_flag1_best.at(i);
                        pick_opt_flag2_best = min_pick_opt_flag2_best.at(i);
                        if(argvs.break_flag){
                            route1_best = min_route1_best.at(i);
                            route2_best = min_route2_best.at(i);
                            management_ttable_best = min_management_ttable_best.at(i);
                            management_dtable_best = min_management_dtable_best.at(i);
                            management_etable_best = min_management_etable_best.at(i);
                            management_xtable_best = min_management_xtable_best.at(i);
                            t_route_best = min_t_route_best.at(i);
                            customer_best = min_customer_best.at(i);
                        }
                    }
                }
            }
        #ifdef MPI_ENABLED 
            else{
                int size1, size2;
                int route_size1, route_size2;
                int t_route_size;
                int cust_size;
                int ttable_size1;
                int ttable_size2;
                int dtable_size1;
                int dtable_size2;
                int xtable_size1;
                int xtable_size2;
                int etable_size1;
                int etable_size2;
                vector<char> tcustno;
                vector<int> tspotno;
                vector<vector<double>> tvalues;
                vector<vector<double>> dvalues;
                vector<vector<double>> xvalues;
                vector<vector<double>> evalues;

                vector<double> min_val(size);
                vector<long long int> min_idx(size);
                
                my_min_pair.idx = min_idx_vec.at(0);
                my_min_pair.val = min_vec.at(0);
                MPI_Allgather(&my_min_pair.val, 1, MPI_DOUBLE, min_val.data(), 1, MPI_DOUBLE, MPI_COMM_WORLD);
                MPI_Allgather(&my_min_pair.idx, 1, MPI_LONG_LONG_INT, min_idx.data(), 1, MPI_LONG_LONG_INT, MPI_COMM_WORLD);

                min_pair.val = 1e20;
                min_pair.idx = -1;

                for(int i = 0; i < size; i++){
                    if(min_val.at(i) < min_pair.val){
                        min_pair.val = min_val.at(i);
                        min_pair.idx = min_idx.at(i);
                    }
                    else if(min_val.at(i) == min_pair.val && min_idx.at(i) < min_pair.idx){
                        min_pair.idx = min_idx.at(i);
                    }
                }

                custs1_best = min_custs1_best.at(0);
                custs2_best = min_custs2_best.at(0);
                pick_opt_flag1_best = min_pick_opt_flag1_best.at(0);
                pick_opt_flag2_best = min_pick_opt_flag2_best.at(0);
                size1 = custs1_best.size();
                size2 = custs2_best.size();
                MPI_Bcast(&size1, 1, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                MPI_Bcast(&size2, 1, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);

                custs1_best.resize(size1);
                custs2_best.resize(size2);

                MPI_Bcast(custs1_best.data(), size1, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                MPI_Bcast(custs2_best.data(), size2, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                MPI_Bcast(&pick_opt_flag1_best, 1, MPI_C_BOOL, min_pair.idx%size, MPI_COMM_WORLD);
                MPI_Bcast(&pick_opt_flag2_best, 1, MPI_C_BOOL, min_pair.idx%size, MPI_COMM_WORLD);
                if(argvs.break_flag){
                    route1_best = min_route1_best.at(0);
                    route2_best = min_route2_best.at(0);
                    t_route_best = min_t_route_best.at(0);

                    // customers の処理
                    // 1 固定のcustomer以外(spotid =="") をコピー cust の中のspot=="" のcustidをchar[[128]]に変えたものと spotno を作る char[]はMIPCHAR (型)
                    customer_best = min_customer_best.at(0);
                    int customer_size = customer_best.size();
                    for(int i=0; i < customer_size; i++){
                        Customer c = customer_best.at(i);
                        if(c.spotid == ""){
                            str2char(c.custno, tcustno);
                            tspotno.push_back(c.spotno);
                        }
                    }
                    cust_size = tspotno.size();

                    // ttable の処理
                    management_ttable_best = min_management_ttable_best.at(0);
                    ttable_size1 = management_ttable_best.size();
                    ttable_size2 = management_ttable_best.rbegin()->second.size(); // テーブル内の要素数
                    for(auto table = management_ttable_best.rbegin(); table != management_ttable_best.rend(); table++){
                        vector<double> tvalue;
                        for(int j=0; j<ttable_size2; j++){
                            for(int k=0; k<ttable_size2; k++){
                                tvalue.insert(tvalue.end(), table->second.at(j).at(k));
                            }
                        }
                        tvalues.push_back(tvalue);
                    }

                    // dtable の処理
                    management_dtable_best = min_management_dtable_best.at(0);
                    dtable_size1 = management_dtable_best.size();
                    dtable_size2 = management_dtable_best.rbegin()->second.size(); // テーブル内の要素数
                    for(auto table = management_dtable_best.rbegin(); table != management_dtable_best.rend(); table++){
                        vector<double> dvalue;
                        for(int j=0; j<dtable_size2; j++){
                            for(int k=0; k<dtable_size2; k++){
                                dvalue.insert(dvalue.end(), table->second.at(j).at(k));
                            }
                        }
                        dvalues.push_back(dvalue);
                    }

                    // xtable の処理
                    if(min_management_xtable_best.at(0).size() > 0){
                        management_xtable_best = min_management_xtable_best.at(0);
                        xtable_size1 = management_xtable_best.size();
                        xtable_size2 = management_xtable_best.rbegin()->second.size(); // テーブル内の要素数
                        for(auto table = management_xtable_best.rbegin(); table != management_xtable_best.rend(); table++){
                            vector<double> xvalue;
                            for(int j=0; j<xtable_size2; j++){
                                for(int k=0; k<xtable_size2; k++){
                                    xvalue.insert(xvalue.end(), table->second.at(j).at(k));
                                }
                            }
                            xvalues.push_back(xvalue);
                        }
                    }
                    else{
                        xtable_size1 = 0;
                        xtable_size2 = 0;
                    }

                    // etable の処理
                    if(min_management_etable_best.at(0).size() > 0){
                        management_etable_best = min_management_etable_best.at(0);
                        etable_size1 = management_etable_best.size();
                        etable_size2 = management_etable_best.rbegin()->second.size(); // テーブル内の要素数
                        for(auto table = management_etable_best.rbegin(); table != management_etable_best.rend(); table++){
                            vector<double> evalue;
                            for(int j=0; j<etable_size2; j++){
                                for(int k=0; k<etable_size2; k++){
                                    evalue.insert(evalue.end(), table->second.at(j).at(k));
                                }
                            }
                            evalues.push_back(evalue);
                        }
                    }
                    else{
                        etable_size1 = 0;
                        etable_size2 = 0;
                    }
                    route_size1 = route1_best.size();
                    route_size2 = route2_best.size();
                    t_route_size = t_route_best.size();
                    vector<int> t_route_size2;
                    for(int r=0; r<t_route_size; r++){
                        t_route_size2.push_back(t_route_best.at(r).size());
                    }
                    
                    MPI_Bcast(&route_size1, 1, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                    MPI_Bcast(&route_size2, 1, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                    MPI_Bcast(t_route_size2.data(), t_route_size, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                    MPI_Bcast(&cust_size, 1, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                    MPI_Bcast(&ttable_size2, 1, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                    MPI_Bcast(&dtable_size2, 1, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                    MPI_Bcast(&xtable_size2, 1, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                    MPI_Bcast(&etable_size2, 1, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);

                    // custとspotnoのサイズの共有
                    route1_best.resize(route_size1);
                    route2_best.resize(route_size2);
                    for(int r=0; r<t_route_size; r++){
                        t_route_best.at(r).resize(t_route_size2.at(r));
                    }
                    tcustno.resize(cust_size * STRING2CHAR_LENGTH);
                    tspotno.resize(cust_size);
                    for(int j=0; j<ttable_size1; j++){
                        tvalues.at(j).resize(ttable_size2 * ttable_size2);
                    }
                    for(int j=0; j<dtable_size1; j++){
                        dvalues.at(j).resize(dtable_size2 * dtable_size2);
                    }
                    for(int j=0; j<xtable_size1; j++){
                        xvalues.at(j).resize(xtable_size2 * xtable_size2);
                    }
                    for(int j=0; j<etable_size1; j++){
                        evalues.at(j).resize(etable_size2 * etable_size2);
                    }

                    MPI_Bcast(route1_best.data(), route_size1, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                    MPI_Bcast(route2_best.data(), route_size2, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                    for(int r=0; r<t_route_size; r++){
                        MPI_Bcast(t_route_best.at(r).data(), t_route_size2.at(r), MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                    }
                    MPI_Bcast(tcustno.data(), cust_size*STRING2CHAR_LENGTH , MPI_CHAR, min_pair.idx%size, MPI_COMM_WORLD);
                    MPI_Bcast(tspotno.data(), cust_size, MPI_INT, min_pair.idx%size, MPI_COMM_WORLD);
                    for(int j=0; j<ttable_size1; j++){
                        MPI_Bcast(tvalues.at(j).data(), ttable_size2*ttable_size2, MPI_DOUBLE, min_pair.idx%size, MPI_COMM_WORLD);
                    }
                    for(int j=0; j<dtable_size1; j++){
                        MPI_Bcast(dvalues.at(j).data(), dtable_size2*dtable_size2, MPI_DOUBLE, min_pair.idx%size, MPI_COMM_WORLD);
                    }
                    for(int j=0; j<xtable_size1; j++){
                        MPI_Bcast(xvalues.at(j).data(), xtable_size2*xtable_size2, MPI_DOUBLE, min_pair.idx%size, MPI_COMM_WORLD);
                    }
                    for(int j=0; j<etable_size1; j++){
                        MPI_Bcast(evalues.at(j).data(), etable_size2*etable_size2, MPI_DOUBLE, min_pair.idx%size, MPI_COMM_WORLD);
                    }

                    int br_pattern = get_breakspot_pattern(vehicles, argvs);
                    if(br_pattern==1 || br_pattern==2){
                        // パターン1,2
                        //custnoを使って、元のリストのどこに有るか(index) そのインデックスを使ってspotnoだけ更新する
                        int customer_size = customer_best.size();
                        for(int j=0; j<cust_size; j++){
                            string tmp_custno;
                            char2str(tcustno, tmp_custno, j);
                            for(int k=0; k<customer_size; k++){
                                if(customer_best.at(k).custno == tmp_custno){
                                    customer_best.at(k).spotno = tspotno.at(j);
                                    break;
                                }
                            }
                        }
                    }
                    else{
                        // パターン3
                        // custオブジェクトを再構成(後ろの部分を削って足す)
                        int customer_size = customer_best.size();
                        for(int j=customer_size-1; j>=0; j--){
                            if(customer_best.at(j).custno.at(0) == 'b'){
                                customer_best.erase(customer_best.begin() + j);
                            }
                            else{
                                break;
                            }
                        }
                        for(int j=0; j<cust_size; j++){
                            string tmp_custno;
                            char2str(tcustno, tmp_custno, j);
                            Customer br_spot = create_customer(tmp_custno, argvs);
                            br_spot.spotno = tspotno.at(j);
                            int br_spot_idx = customer_best.size();
                            if(argvs.bulkShipping_flag == false){
                                customer_best.push_back(br_spot);
                            }
                            else{
                                int tcnum = 0;
                                vector<int> _tc;
                                _tc.push_back(br_spot_idx);
                                Customer tied_customers(
                                    _tc,
                                    br_spot.reje_ve,
                                    br_spot.custno,
                                    br_spot.spotid,
                                    br_spot.spotno,
                                    br_spot.y,
                                    br_spot.x,
                                    br_spot.dem,
                                    br_spot.dem2,
                                    br_spot.servt,
                                    br_spot.e_rate,
                                    br_spot.parkt_arrive,
                                    br_spot.parkt_open,
                                    br_spot.depotservt,
                                    br_spot.ready_fs,
                                    br_spot.due_fs,
                                    br_spot.requestType,
                                    br_spot.deny_unass,
                                    br_spot.changecust
                                );
                                customer_best.push_back(tied_customers);
                            }
                        }
                    }

                    // ttable の処理
                    int i1 = 0;
                    for(auto table = management_ttable_best.rbegin(); table != management_ttable_best.rend(); table++){
                        int i2 = 0;
                        vector<vector<double>> t;
                        for(int k=0; k<ttable_size2; k++){
                            vector<double> tt;
                            for(int l=0; l<ttable_size2; l++){
                                tt.push_back(tvalues.at(i1).at(i2));
                                i2++;
                            }
                            t.push_back(tt);
                        }
                        management_ttable_best[table->first] = t;
                        i1++;
                    }

                    // dtable の処理
                    i1 = 0;
                    for(auto table = management_dtable_best.rbegin(); table != management_dtable_best.rend(); table++){
                        int i2 = 0;
                        vector<vector<double>> t;
                        for(int k=0; k<dtable_size2; k++){
                            vector<double> tt;
                            for(int l=0; l<dtable_size2; l++){
                                tt.push_back(dvalues.at(i1).at(i2));
                                i2++;
                            }
                            t.push_back(tt);
                        }
                        management_dtable_best[table->first] = t;
                        i1++;
                    }

                    // xtable の処理
                    if(management_xtable_best.size() > 0){
                        i1 = 0;
                        for(auto table = management_xtable_best.rbegin(); table != management_xtable_best.rend(); table++){
                            int i2 = 0;
                            vector<vector<double>> t;
                            for(int k=0; k<xtable_size2; k++){
                                vector<double> tt;
                                for(int l=0; l<xtable_size2; l++){
                                    tt.push_back(xvalues.at(i1).at(i2));
                                    i2++;
                                }
                                t.push_back(tt);
                            }
                            management_xtable_best[table->first] = t;
                            i1++;
                        }
                    }

                    // etable の処理
                    if(management_etable_best.size() > 0){
                        i1 = 0;
                        for(auto table = management_etable_best.rbegin(); table != management_etable_best.rend(); table++){
                            int i2 = 0;
                            vector<vector<double>> t;
                            for(int k=0; k<etable_size2; k++){
                                vector<double> tt;
                                for(int l=0; l<etable_size2; l++){
                                    tt.push_back(evalues.at(i1).at(i2));
                                    i2++;
                                }
                                t.push_back(tt);
                            }
                            management_etable_best[table->first] = t;
                            i1++;
                        }
                    }

                    // if(rank==2){
                    //     printf("ttable\n");
                    //     for(auto table = management_ttable_best.rbegin(); table != management_ttable_best.rend(); table++){
                    //         for(int k=0; k<ttable_size2; k++){
                    //             for(int l=0; l<ttable_size2; l++){
                    //                 printf("%f ", table->second.at(k).at(l));
                    //             }
                    //             printf("\n");
                    //         }
                    //     }
                    //     printf("dtable\n");
                    //     for(auto table = management_dtable_best.rbegin(); table != management_dtable_best.rend(); table++){
                    //         for(int k=0; k<dtable_size2; k++){
                    //             for(int l=0; l<dtable_size2; l++){
                    //                 printf("%f ", table->second.at(k).at(l));
                    //             }
                    //             printf("\n");
                    //         }
                    //     }
                    //     printf("xtable\n");
                    //     for(auto table = management_xtable_best.rbegin(); table != management_xtable_best.rend(); table++){
                    //         for(int k=0; k<xtable_size2; k++){
                    //             for(int l=0; l<xtable_size2; l++){
                    //                 printf("%f ", table->second.at(k).at(l));
                    //             }
                    //             printf("\n");
                    //         }
                    //     }
                    //     printf("etable\n");
                    //     for(auto table = management_etable_best.rbegin(); table != management_etable_best.rend(); table++){
                    //         for(int k=0; k<etable_size2; k++){
                    //             for(int l=0; l<etable_size2; l++){
                    //                 printf("%f ", table->second.at(k).at(l));
                    //             }
                    //             printf("\n");
                    //         }
                    //     }
                    //     printf("customer\n");
                    //     customer_size = customer_best.size();
                    //     for(int j=0; j<customer_size; j++){
                    //         printf("%s %d\n", customer_best.at(j).custno.c_str(), customer_best.at(j).spotno);
                    //     }
                    //     printf("route\n");
                    //     for(int r=0; r<t_route_size; r++){
                    //         for(int j=0; j<t_route_best.at(r).size(); j++){
                    //             printf("%d ", t_route_best.at(r).at(j));
                    //         }
                    //         printf("\n");
                    //     }
                    // }
                    //fflush(NULL);
                }
            }
        #endif
            try{
                int vehno1_min = min_pair.idx/(vehnum*max_route_size*max_route_size*max_route_size*max_route_size);
                int vehno2_min = (min_pair.idx/(max_route_size*max_route_size*max_route_size*max_route_size))%vehnum;
                // vehno1_min = sorted_vehicles.at(vehno1_min);    // 平準化高速化の為のソートの結果を反映
                // vehno2_min = sorted_vehicles.at(vehno2_min);
                int p1_min = (min_pair.idx/(max_route_size*max_route_size*max_route_size))%max_route_size;
                int p2_min = (min_pair.idx/(max_route_size*max_route_size))%max_route_size;
                int p3_min = (min_pair.idx/max_route_size)%max_route_size;
                int p4_min = min_pair.idx%max_route_size; 

                vector<int> tv1_route = vehicles.at(vehno1_min).route;
                vector<int> tv2_route = vehicles.at(vehno2_min).route;
                
                if(argvs.break_flag){
                    t_ttable = argvs.management_ttable;
                    t_dtable = argvs.management_dtable;
                    t_etable = argvs.management_etable;
                    t_xtable = argvs.management_xtable;
                    argvs.management_ttable = management_ttable_best;
                    argvs.management_dtable = management_dtable_best;
                    argvs.management_etable = management_etable_best;
                    argvs.management_xtable = management_xtable_best;
                    customers = customer_best;
                    vehicles = r_restore(vehicles, t_route_best, argvs, customers);
                }
                //交換処理
                cross_cust_dtable(vehicles.at(vehno1_min), vehicles.at(vehno2_min), customers, p1_min, p2_min, p3_min, p4_min, custs1_best, custs2_best, pick_opt_flag1_best, pick_opt_flag2_best, route1_best, route2_best, argvs);
            
                if(argvs.delaystart2_flag == true && argvs.break_flag==false){
                    delay_arr_time2(customers, vehicles, argvs);
                }

                double dist, power, exp;
                int v1_route_size = vehicles.at(vehno1_min).route.size();
                
                vector<int> tdepo_idx1;
                for(int i = 0; i < v1_route_size-1; i++){
                    if(customers.at(vehicles.at(vehno1_min).route.at(i)).custno == "0"){
                        tdepo_idx1.emplace_back(i); //routeにおけるデポのインデックス
                    }
                }
                tdepo_idx1.emplace_back(v1_route_size-1); //最終訪問先を追加

                int depo_idx1_size = (int) tdepo_idx1.size();

                map<int, double> servt_dict1;
                //配送時間
                for(int i = 0; i < depo_idx1_size-1; i++){
                    servt_dict1.emplace(tdepo_idx1.at(i), 0);
                    double servt = 0;

                    for(int j = tdepo_idx1.at(i)+1; j < tdepo_idx1.at(i+1); j++){
                        if(customers.at(vehicles.at(vehno1_min).route.at(j)).requestType == "DELIVERY" 
                        && customers.at(vehicles.at(vehno1_min).route.at(j)).custno != customers.at(vehicles.at(vehno1_min).route.at(j-1)).custno
                        && customers.at(vehicles.at(vehno1_min).route.at(j)).custno.at(0) != 'b'){
                            servt += customers.at(vehicles.at(vehno1_min).route.at(j)).depotservt;
                        }
                    }

                    if(customers.at(vehicles.at(vehno1_min).route.at(tdepo_idx1.at(i)+1)).custno != "0"){
                        servt_dict1.at(tdepo_idx1.at(i)) += servt+vehicles.at(vehno1_min).depotservt; //デポが連続していない場合
                    }
                    else{
                        servt_dict1.at(tdepo_idx1.at(i)) += servt; //デポが連続している場合
                    }
                }

                //集荷時間
                servt_dict1.emplace(v1_route_size-1, 0);
                for(int i = 1; i < depo_idx1_size; i++){
                    double servt = 0;

                    for(int j = tdepo_idx1.at(i-1)+1; j < tdepo_idx1.at(i); j++){
                        if(customers.at(vehicles.at(vehno1_min).route.at(j)).requestType == "PICKUP"
                        && customers.at(vehicles.at(vehno1_min).route.at(j)).custno != customers.at(vehicles.at(vehno1_min).route.at(j-1)).custno){
                            servt += customers.at(vehicles.at(vehno1_min).route.at(j)).depotservt;
                        }
                    }

                    servt_dict1.at(tdepo_idx1.at(i)) += servt;
                }

                vector<double> load11(v1_route_size, 0), load12(v1_route_size, 0);

                for(int i = 0; i < v1_route_size-1; i++){
                    if(vector_finder(tdepo_idx1, i) == true){
                        for(int j=i+1; j < tdepo_idx1.at(distance(tdepo_idx1.begin(), find(tdepo_idx1.begin(), tdepo_idx1.end(), i))+1); j++){
                            if(customers.at(vehicles.at(vehno1_min).route.at(j)).requestType == "DELIVERY"){
                                if(customers.at(vehicles.at(vehno1_min).route.at(j)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                                    load11.at(i) += customers.at(vehicles.at(vehno1_min).route.at(j)).dem;

                                    load12.at(i) += customers.at(vehicles.at(vehno1_min).route.at(j)).dem2;
                                }
                            }
                        }
                    }
                    else{
                        if(customers.at(vehicles.at(vehno1_min).route.at(i)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                            if (customers.at(vehicles.at(vehno1_min).route.at(i)).requestType == "DELIVERY"){
                                load11.at(i) = load11.at(i-1)-customers.at(vehicles.at(vehno1_min).route.at(i)).dem;
                                load12.at(i) = load12.at(i-1)-customers.at(vehicles.at(vehno1_min).route.at(i)).dem2;
                            }
                            else{
                                load11.at(i) = load11.at(i-1)+customers.at(vehicles.at(vehno1_min).route.at(i)).dem;
                                load12.at(i) = load12.at(i-1)+customers.at(vehicles.at(vehno1_min).route.at(i)).dem2;
                            }
                        }
                    }
                }

                for(int i = v1_route_size-1; i >= 0; i--){
                    if(vector_finder(tdepo_idx1, i) == true && vector_finder(tdepo_idx1, i+1) == true){
                        load11.at(i) = load11.at(i+1);
                        load12.at(i) = load12.at(i+1);
                    }
                }

                //誤差対策
                for(int i = 0; i < v1_route_size; i++){
                    if(load11.at(i) < 1.0e-9){
                        load11.at(i) = 0.0;
                    }
                }
                for(int i = 0; i < v1_route_size; i++){
                    if(load12.at(i) < 1.0e-9){
                        load12.at(i) = 0.0;
                    }
                }

                vector<int> no_park_idx1;
                bool no_park_flag1 = true;

                for(int i = tdepo_idx1.at(0)+1; i < tdepo_idx1.at(1); i++){
                    if(customers.at(vehicles.at(vehno1_min).route.at(i)).requestType == "DELIVERY"){
                        no_park_flag1 = false;
                        break;
                    }
                }
                if(no_park_flag1 == true && load11.at(0) == 0 && load12.at(0) == 0){
                    no_park_idx1.emplace_back(0);
                }

                for(int i = 1; i < depo_idx1_size-1; i++){
                    if(load11.at(tdepo_idx1.at(i)-1) == 0 && load12.at(tdepo_idx1.at(i)-1) == 0){
                        no_park_flag1 = true;

                        for(int j = tdepo_idx1.at(i)+1; j < tdepo_idx1.at(i+1); j++){
                            if(customers.at(vehicles.at(vehno1_min).route.at(j)).requestType == "DELIVERY"){
                                no_park_flag1 = false;
                                break;
                            }
                        }
                        if(no_park_flag1 == true && load11.at(tdepo_idx1.at(i)) == 0 && load12.at(tdepo_idx1.at(i)) == 0){
                            no_park_idx1.emplace_back(tdepo_idx1.at(i));
                        }
                    }
                }

                if(load11.at(tdepo_idx1.at(depo_idx1_size-1)-1) == 0 && load12.at(tdepo_idx1.at(depo_idx1_size-1)-1) == 0){
                    no_park_idx1.emplace_back(tdepo_idx1.at(depo_idx1_size-1));
                }

                bool pick_opt_flag1 = false;
                bool non_depo = false;
                if(argvs.pickup_flag == true){
                    for(int i = v1_route_size-2; i >= 1; i--){
                        if(customers.at(vehicles.at(vehno1_min).route.at(i)).custno != "0"){
                            non_depo = true;
                            if(customers.at(vehicles.at(vehno1_min).route.at(i)).requestType == "PICKUP"){
                                pick_opt_flag1 = true;
                                break;
                            }
                        }
                        else if(non_depo == true){
                            break;
                        }
                    }
                }

                bool v1_passable = check_impassability(vehicles.at(vehno1_min), customers, vehicles.at(vehno1_min).lv.at(0), 1, v1_route_size, servt_dict1, dist, power, exp, no_park_idx1, pick_opt_flag1, argvs);
                
                int v2_route_size = vehicles.at(vehno2_min).route.size();
                
                vector<int> tdepo_idx2;
                for(int i = 0; i < v2_route_size-1; i++){
                    if(customers.at(vehicles.at(vehno2_min).route.at(i)).custno == "0"){
                        tdepo_idx2.emplace_back(i); //routeにおけるデポのインデックス
                    }
                }
                tdepo_idx2.emplace_back(v2_route_size-1); //最終訪問先を追加

                int depo_idx2_size = (int) tdepo_idx2.size();

                map<int, double> servt_dict2;
                //配送時間
                for(int i = 0; i < depo_idx2_size-1; i++){
                    servt_dict2.emplace(tdepo_idx2.at(i), 0);
                    double servt = 0;

                    for(int j = tdepo_idx2.at(i)+1; j < tdepo_idx2.at(i+1); j++){
                        if(customers.at(vehicles.at(vehno2_min).route.at(j)).requestType == "DELIVERY"
                        && customers.at(vehicles.at(vehno2_min).route.at(j)).custno != customers.at(vehicles.at(vehno2_min).route.at(j-1)).custno
                        && customers.at(vehicles.at(vehno2_min).route.at(j)).custno.at(0) != 'b'){
                            servt += customers.at(vehicles.at(vehno2_min).route.at(j)).depotservt;
                        }
                    }

                    if(customers.at(vehicles.at(vehno2_min).route.at(tdepo_idx2.at(i)+1)).custno != "0"){
                        servt_dict2.at(tdepo_idx2.at(i)) += servt+vehicles.at(vehno2_min).depotservt; //デポが連続していない場合
                    }
                    else{
                        servt_dict2.at(tdepo_idx2.at(i)) += servt; //デポが連続している場合
                    }
                }

                //集荷時間
                servt_dict2.emplace(v2_route_size-1, 0);
                for(int i = 1; i < depo_idx2_size; i++){
                    double servt = 0;

                    for(int j = tdepo_idx2.at(i-1)+1; j < tdepo_idx2.at(i); j++){
                        if(customers.at(vehicles.at(vehno2_min).route.at(j)).requestType == "PICKUP"
                        && customers.at(vehicles.at(vehno2_min).route.at(j)).custno != customers.at(vehicles.at(vehno2_min).route.at(j-1)).custno){
                            servt += customers.at(vehicles.at(vehno2_min).route.at(j)).depotservt;
                        }
                    }

                    servt_dict2.at(tdepo_idx2.at(i)) += servt;
                }

                vector<double> load21(v2_route_size, 0), load22(v2_route_size, 0);

                for(int i=0; i < v2_route_size-1; i++){
                    if(vector_finder(tdepo_idx2, i) == true){
                        for(int j=i+1; j < tdepo_idx2.at(distance(tdepo_idx2.begin(), find(tdepo_idx2.begin(), tdepo_idx2.end(), i))+1); j++){
                            if(customers.at(vehicles.at(vehno2_min).route.at(j)).requestType == "DELIVERY"){
                                if(customers.at(vehicles.at(vehno2_min).route.at(j)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                                    load21.at(i) += customers.at(vehicles.at(vehno2_min).route.at(j)).dem;
                            
                                    load22.at(i) += customers.at(vehicles.at(vehno2_min).route.at(j)).dem2;
                                }
                            }
                        }
                    }
                    else{
                        if(customers.at(vehicles.at(vehno2_min).route.at(i)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                            if (customers.at(vehicles.at(vehno2_min).route.at(i)).requestType == "DELIVERY"){
                                load21.at(i) = load21.at(i-1)-customers.at(vehicles.at(vehno2_min).route.at(i)).dem;
                                load22.at(i) = load22.at(i-1)-customers.at(vehicles.at(vehno2_min).route.at(i)).dem2;
                            }
                            else{
                                load21.at(i) = load21.at(i-1)+customers.at(vehicles.at(vehno2_min).route.at(i)).dem;
                                load22.at(i) = load22.at(i-1)+customers.at(vehicles.at(vehno2_min).route.at(i)).dem2;
                            }
                        }
                    }
                }

                for(int i = v2_route_size-1; i >= 0; i--){
                    if(vector_finder(tdepo_idx2, i) == true && vector_finder(tdepo_idx2, i+1) == true){
                        load21.at(i) = load21.at(i+1);
                        load22.at(i) = load22.at(i+1);
                    }
                }

                //誤差対策
                for(int i = 0; i < v2_route_size; i++){
                    if(load21.at(i) < 1.0e-9){
                        load21.at(i) = 0.0;
                    }
                }
                for(int i = 0; i < v2_route_size; i++){
                    if(load22.at(i) < 1.0e-9){
                        load22.at(i) = 0.0;
                    }
                }

                vector<int> no_park_idx2;
                bool no_park_flag2 = true;

                for(int i = tdepo_idx2.at(0)+1; i < tdepo_idx2.at(1); i++){
                    if(customers.at(vehicles.at(vehno2_min).route.at(i)).requestType == "DELIVERY"){
                        no_park_flag1 = false;
                        break;
                    }
                }
                if(no_park_flag2 == true && load21.at(0) == 0 && load22.at(0) == 0){
                    no_park_idx2.emplace_back(0);
                }

                for(int i = 1; i < depo_idx2_size-1; i++){
                    if(load21.at(tdepo_idx2.at(i)-1) == 0 && load22.at(tdepo_idx2.at(i)-1) == 0){
                        no_park_flag2 = true;

                        for(int j = tdepo_idx2.at(i)+1; j < tdepo_idx2.at(i+1); j++){
                            if(customers.at(vehicles.at(vehno2_min).route.at(j)).requestType == "DELIVERY"){
                                no_park_flag2 = false;
                                break;
                            }
                        }
                        if(no_park_flag2 == true && load21.at(tdepo_idx2.at(i)) == 0 && load22.at(tdepo_idx2.at(i)) == 0){
                            no_park_idx2.emplace_back(tdepo_idx2.at(i));
                        }
                    }
                }

                if(load21.at(tdepo_idx2.at(depo_idx2_size-1)-1) == 0 && load22.at(tdepo_idx2.at(depo_idx2_size-1)-1) == 0){
                    no_park_idx2.emplace_back(tdepo_idx2.at(depo_idx2_size-1));
                }

                bool pick_opt_flag2 = false;
                non_depo = false;
                if(argvs.pickup_flag == true){
                    for(int i = v2_route_size-2; i >= 1; i--){
                        if(customers.at(vehicles.at(vehno2_min).route.at(i)).custno != "0"){
                            non_depo = true;
                            if(customers.at(vehicles.at(vehno2_min).route.at(i)).requestType == "PICKUP"){
                                pick_opt_flag2 = true;
                                break;
                            }
                        }
                        else if(non_depo == true){
                            break;
                        }
                    }
                }

                bool v2_passable = check_impassability(vehicles.at(vehno2_min), customers, vehicles.at(vehno2_min).lv.at(0), 1, v2_route_size, servt_dict2, dist, power, exp, no_park_idx2, pick_opt_flag2, argvs);

                if(!v1_passable || !v2_passable){ //通行不可ルートならもとに戻す
                    if(argvs.break_flag){
                        argvs.management_ttable = t_ttable;
                        argvs.management_dtable = t_dtable;
                        argvs.management_etable = t_etable;
                        argvs.management_xtable = t_xtable;
                    }
                    vehicles.at(vehno1_min).route = tv1_route;
                    vehicles.at(vehno1_min).update_totalcost_dtable(customers, argvs);
                    vehicles.at(vehno2_min).route = tv2_route;
                    vehicles.at(vehno2_min).update_totalcost_dtable(customers, argvs);

                    exclude_list.emplace_back(min_pair.idx);

                    continue;
                }

                improved_cnt++;

                exclude_list.clear();



                //現在(=最新)のatpenとaloadとAPEを保存する
                if(argvs.lastc_flag == true){
                    //最終訪問先インデックスを計算
                    int last_idx1 = v1_route_size-1;
                    for(int i = v1_route_size-2; i >=0; i--){
                        if(customers.at(vehicles.at(vehno1_min).route.at(i)).custno == "0" || customers.at(vehicles.at(vehno1_min).route.at(i)).custno.at(0) == 'b' || customers.at(vehicles.at(vehno1_min).route.at(i)).custno.at(0) == 'e'){
                            last_idx1 = i;
                        }
                        else{
                            break;
                        }
                    }

                    //時間違反量
                    vehicles.at(vehno1_min).time_pen = max(0.0, vehicles.at(vehno1_min).lv.at(last_idx1)-vehicles.at(vehno1_min).endhour); 
                    for(int i = 1; i < last_idx1; i++){
                        if(customers.at(vehicles.at(vehno1_min).route.at(i)).custno != customers.at(vehicles.at(vehno1_min).route.at(i-1)).custno){
                            vehicles.at(vehno1_min).time_pen += max(0.0, vehicles.at(vehno1_min).st.at(i)-customers.at(vehicles.at(vehno1_min).route.at(i)).due_fs);
                        }
                    }
                }
                else{
                    //時間違反量
                    vehicles.at(vehno1_min).time_pen = max(0.0, vehicles.at(vehno1_min).lv.at(v1_route_size-1)-vehicles.at(vehno1_min).endhour); 
                    for(int i = 1; i < v1_route_size; i++){
                        if(customers.at(vehicles.at(vehno1_min).route.at(i)).custno != customers.at(vehicles.at(vehno1_min).route.at(i-1)).custno){
                            vehicles.at(vehno1_min).time_pen += max(0.0, vehicles.at(vehno1_min).st.at(i)-customers.at(vehicles.at(vehno1_min).route.at(i)).due_fs);
                        }
                    }
                }


                //積載量違反量
                double max_load_pen1_1 = 0, max_load_pen1_2 = 0;
                if(argvs.pickup_flag == true){
                    tie(max_load_pen1_1, max_load_pen1_2) = calc_vehicle_loadpickup_penalty(vehicles.at(vehno1_min), customers, vehicles.at(vehno1_min).route);
                }

                vector<int> depo_idx1;
                for(int i = 0; i < v1_route_size-1; i++){
                    if(customers.at(vehicles.at(vehno1_min).route.at(i)).custno == "0"){
                        depo_idx1.emplace_back(i); //routeにおけるデポのインデックス
                    }
                }
                depo_idx1.emplace_back(v1_route_size-1); //最終訪問先を追加
                
                int depo_list1_size = depo_idx1.size();
                double load_pen = 0, load_pen2 = 0, load_onbrd = 0, load_onbrd2 = 0;
                for(int i = 0; i < depo_list1_size-1; i++){
                    load_onbrd = 0;
                    load_onbrd2 = 0;

                    for(int j = depo_idx1.at(i)+1; j < depo_idx1.at(i+1); j++){
                        if(customers.at(vehicles.at(vehno1_min).route.at(j)).requestType == "DELIVERY"){
                            if(customers.at(vehicles.at(vehno1_min).route.at(j)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                                load_onbrd += customers.at(vehicles.at(vehno1_min).route.at(j)).dem;
                        
                                load_onbrd2 += customers.at(vehicles.at(vehno1_min).route.at(j)).dem2;
                            }
                        }
                    }

                    load_pen += max(load_onbrd-vehicles.at(vehno1_min).cap,0.0);
                    load_pen2 += max(load_onbrd2-vehicles.at(vehno1_min).cap2,0.0);
                }

                vehicles.at(vehno1_min).load_pen = load_pen+max_load_pen1_1;
                vehicles.at(vehno1_min).load_pen2 = load_pen2+max_load_pen1_2;


                if(argvs.lastc_flag == true){
                    //最終訪問先インデックスを計算
                    int last_idx2 = v2_route_size-1;
                    for(int i = v2_route_size-2; i >=0; i--){
                        if(customers.at(vehicles.at(vehno2_min).route.at(i)).custno == "0" || customers.at(vehicles.at(vehno2_min).route.at(i)).custno.at(0) == 'b' || customers.at(vehicles.at(vehno2_min).route.at(i)).custno.at(0) == 'e'){
                            last_idx2 = i;
                        }
                        else{
                            break;
                        }
                    }

                    //時間違反量
                    vehicles.at(vehno2_min).time_pen = max(0.0, vehicles.at(vehno2_min).lv.at(last_idx2)-vehicles.at(vehno2_min).endhour); 
                    for(int i = 1; i < last_idx2; i++){
                        if(customers.at(vehicles.at(vehno2_min).route.at(i)).custno != customers.at(vehicles.at(vehno2_min).route.at(i-1)).custno){
                            vehicles.at(vehno2_min).time_pen += max(0.0, vehicles.at(vehno2_min).st.at(i)-customers.at(vehicles.at(vehno2_min).route.at(i)).due_fs);
                        }
                    }
                }
                else{
                    //時間違反量
                    vehicles.at(vehno2_min).time_pen = max(0.0, vehicles.at(vehno2_min).lv.at(v2_route_size-1)-vehicles.at(vehno2_min).endhour); 
                    for(int i = 1; i < v2_route_size; i++){
                        if(customers.at(vehicles.at(vehno2_min).route.at(i)).custno != customers.at(vehicles.at(vehno2_min).route.at(i-1)).custno){
                            vehicles.at(vehno2_min).time_pen += max(0.0, vehicles.at(vehno2_min).st.at(i)-customers.at(vehicles.at(vehno2_min).route.at(i)).due_fs);
                        }
                    }
                }


                //積載量違反量
                double max_load_pen2_1 = 0, max_load_pen2_2 = 0;
                if(argvs.pickup_flag == true){
                    tie(max_load_pen2_1, max_load_pen2_2) = calc_vehicle_loadpickup_penalty(vehicles.at(vehno2_min), customers, vehicles.at(vehno2_min).route);
                }

                vector<int> depo_idx2;
                for(int i = 0; i < v2_route_size-1; i++){
                    if(customers.at(vehicles.at(vehno2_min).route.at(i)).custno == "0"){
                        depo_idx2.emplace_back(i); //routeにおけるデポのインデックス
                    }
                }
                depo_idx2.emplace_back(v2_route_size-1); //最終訪問先を追加
                
                int depo_list2_size = depo_idx2.size();
                load_pen = 0; load_pen2 = 0; load_onbrd = 0; load_onbrd2 = 0;
                for(int i = 0; i < depo_list2_size-1; i++){
                    load_onbrd = 0;
                    load_onbrd2 = 0;

                    for(int j = depo_idx2.at(i)+1; j < depo_idx2.at(i+1); j++){
                        if(customers.at(vehicles.at(vehno2_min).route.at(j)).requestType == "DELIVERY"){
                            if(customers.at(vehicles.at(vehno2_min).route.at(j)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                                load_onbrd += customers.at(vehicles.at(vehno2_min).route.at(j)).dem;
                        
                                load_onbrd2 += customers.at(vehicles.at(vehno2_min).route.at(j)).dem2;
                            }
                        }
                    }

                    load_pen += max(load_onbrd-vehicles.at(vehno2_min).cap,0.0);
                    load_pen2 += max(load_onbrd2-vehicles.at(vehno2_min).cap2,0.0);
                }

                vehicles.at(vehno2_min).load_pen = load_pen+max_load_pen2_1;
                vehicles.at(vehno2_min).load_pen2 = load_pen2+max_load_pen2_2;


                argvs.atpen = 0;
                argvs.aload = 0;
                for(auto v : vehicles){
                    argvs.atpen += v.time_pen;
                    argvs.aload += v.load_pen+v.load_pen2;
                }


                //APEを計算
                if(argvs.avetime_ape_flag == true){
                    argvs.time_error = calc_avetime_APE(vehicles, customers, {}, argvs.avetime_ape, argvs.lastc_flag, argvs.break_flag);
                }
                else if(argvs.avetime_ae_flag == true){
                    argvs.time_error = calc_avetime_AE(vehicles, customers, {}, argvs.avetime_ae, argvs.lastc_flag);
                }

                if(argvs.avevisits_ape_flag == true){
                    argvs.visits_error = calc_avevisits_APE(vehicles, customers, {}, argvs.avevisits_ape, argvs.bulkShipping_flag, argvs.break_flag);
                }
                else if(argvs.avevisits_ae_flag == true){
                    argvs.visits_error = calc_avevisits_AE(vehicles, customers, {}, argvs.avevisits_ae, argvs.bulkShipping_flag);
                }

                if(argvs.avedist_ape_flag == true){
                    argvs.dist_error = calc_avedist_APE(vehicles, customers, {}, argvs.avedist_ape, argvs.lastc_flag, argvs.break_flag);
                }
                else if(argvs.avedist_ae_flag == true){
                    argvs.dist_error = calc_avedist_AE(vehicles, customers, {}, argvs.avedist_ae, argvs.lastc_flag);
                }


                //違反なしのrouteを保存する
                backup_route(vehicles, customers, argvs);
            }
            catch(string& thrown_str){
                throw invalid_argument(thrown_str);
            }
        }
        
        if(improved_cnt == 0){
            break; //改善がなかった場合終了
        }
    }

    if(argvs.timeout > 0){
        timespec_get(&ts, TIME_UTC);
        dtable_endtime = ts.tv_sec + ts.tv_nsec * 1e-9;
        if(dtable_endtime-argvs.ls_d_starttime > argvs.timeout){ //途中結果を出力
            sLogBuf += "cross timeout!\n";
        }
    }

    return make_tuple(improved_cnt, sLogBuf, vehicles, customers);
}

namespace{
    //vecにnumberが含まれているか調べる
    bool vector_finder(const vector<int> &vec, const int number){
        auto itr = find(vec.begin(), vec.end(), number);
        size_t index = distance(vec.begin(), itr);

        if(index != vec.size()){
            return true; //発見できたとき
        }
        else{
            return false; //発見できなかったとき
        }
    }
}


//バインディング
PYBIND11_MODULE(cross_dtable, m) {
    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------
        .. currentmodule:: cmake_example
        .. autosummary::
           :toctree: _generate
           add
           subtract
    )pbdoc";
    //Vehicles
    py::class_<Vehicle>(m, "Vehicle")
        .def(py::init<const vector<int>,
                        const vector<double>,
                        const vector<double>,
                        const vector<double>,
                        const vector<double>,
                        const vector<double>,
                        const vector<double>,
                        const vector<double>,
                        const int,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const string,
                        const string,
                        const bool,
                        const double,
                        const double,
                        const int,
                        const int,
                        const double,
                        const int,
                        map<int, double>,
                        const vector<double>,
                        const double,
                        const double,
                        const vector<double>,
                        const vector<double>,
                        map<int, double>,
                        const vector<double>,
                        const vector<int>,
                        const bool,
                        const vector<double>,
                        const vector<double>,
                        const vector<double>>())
        .def_readwrite("route", &Vehicle::route)
        .def_readwrite("arr", &Vehicle::arr)
        .def_readwrite("rem_batt", &Vehicle::rem_batt)
        .def_readwrite("chg_batt", &Vehicle::chg_batt)
        .def_readwrite("slack_t", &Vehicle::slack_t)
        .def_readwrite("st", &Vehicle::st)
        .def_readwrite("lv", &Vehicle::lv)
        .def_readwrite("remdist", &Vehicle::remdist)
        .def_readwrite("vehno", &Vehicle::vehno)
        .def_readwrite("cap", &Vehicle::cap)
        .def_readwrite("cap2", &Vehicle::cap2)
        .def_readwrite("totalcost", &Vehicle::totalcost)
        .def_readwrite("totalcost_wolast", &Vehicle::totalcost_wolast)
        .def_readwrite("load_onbrd", &Vehicle::load_onbrd)
        .def_readwrite("load_onbrd2", &Vehicle::load_onbrd2)
        .def_readwrite("load_pickup", &Vehicle::load_pickup)
        .def_readwrite("load_pickup2", &Vehicle::load_pickup2)
        .def_readwrite("opskill", &Vehicle::opskill)
        .def_readwrite("drskill", &Vehicle::drskill)
        .def_readwrite("e_capacity", &Vehicle::e_capacity)
        .def_readwrite("e_initial", &Vehicle::e_initial)
        .def_readwrite("e_cost", &Vehicle::e_cost)
        .def_readwrite("e_margin", &Vehicle::e_margin)
        .def_readwrite("time_pen", &Vehicle::time_pen)
        .def_readwrite("load_pen", &Vehicle::load_pen)
        .def_readwrite("load_pen2", &Vehicle::load_pen2)
        .def_readwrite("vtype", &Vehicle::vtype)
        .def_readwrite("etype", &Vehicle::etype)
        .def_readwrite("starthour", &Vehicle::starthour)
        .def_readwrite("endhour", &Vehicle::endhour)
        .def_readwrite("maxvisit", &Vehicle::maxvisit)
        .def_readwrite("maxrotate", &Vehicle::maxrotate)
        .def_readwrite("depotservt", &Vehicle::depotservt)
        .def_readwrite("rmpriority", &Vehicle::rmpriority)
        .def_readwrite("chg_batt_dict", &Vehicle::chg_batt_dict)
        .def_readwrite("delay_arr", &Vehicle::delay_arr)
		.def_readwrite("delay_totalcost", &Vehicle::delay_totalcost)
		.def_readwrite("delay_totalcost_wolast", &Vehicle::delay_totalcost_wolast)
		.def_readwrite("delay_rem_batt", &Vehicle::delay_rem_batt)
		.def_readwrite("delay_chg_batt", &Vehicle::delay_chg_batt)
		.def_readwrite("delay_chg_batt_dict", &Vehicle::delay_chg_batt_dict)
		.def_readwrite("delay_slack_t", &Vehicle::delay_slack_t)
        .def_readwrite("takebreak", &Vehicle::takebreak)
        .def_readwrite("reserved_v_flag", &Vehicle::reserved_v_flag)
        .def_readwrite("delay_remdist", &Vehicle::delay_remdist)
        .def_readwrite("delay_lv", &Vehicle::delay_lv)
        .def_readwrite("delay_st", &Vehicle::delay_st);

        //Customers
        py::class_<Customer>(m, "Customer")
        .def(py::init<vector<int>,
                        const vector<int>,
                        const string,
                        const string,
                        const int,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const string,
                        const bool,
                        const string>())
        .def_readwrite("tieup_customer", &Customer::tieup_customer)
        .def_readwrite("reje_ve", &Customer::reje_ve)
        .def_readwrite("custno", &Customer::custno)
        .def_readwrite("spotid", &Customer::spotid)
        .def_readwrite("spotno", &Customer::spotno)
        .def_readwrite("y", &Customer::y)
        .def_readwrite("x", &Customer::x)
        .def_readwrite("dem", &Customer::dem)
        .def_readwrite("dem2", &Customer::dem2)
        .def_readwrite("servt", &Customer::servt)
        .def_readwrite("e_rate", &Customer::e_rate)
        .def_readwrite("parkt_arrive", &Customer::parkt_arrive)
        .def_readwrite("parkt_open", &Customer::parkt_open)
        .def_readwrite("depotservt", &Customer::depotservt)
        .def_readwrite("ready_fs", &Customer::ready_fs)
        .def_readwrite("due_fs", &Customer::due_fs)
        .def_readwrite("requestType", &Customer::requestType)
        .def_readwrite("deny_unass", &Customer::deny_unass)
        .def_readwrite("changecust", &Customer::changecust);

        //Cppargs
        py::class_<Cppargs>(m, "Cppargs")
        .def(py::init<const string,
                        const bool,
                        const double,
                        const double,
                        const double,
                        const bool,
                        const bool,
                        const double,
                        const bool,
                        const double,
                        const bool,
                        const double,
                        const bool,
                        const double,
                        const bool,
                        const double,
                        const bool,
                        const double,
                        const double,
                        const int,
                        const bool,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const map<int, vector<int>>,
                        const map<tuple<string, double>, vector<vector<double>>>,
                        const map<tuple<string, double>, vector<vector<double>>>,
                        const map<tuple<string, string, double>, vector<vector<double>>>,
                        const map<tuple<string, double>, vector<vector<double>>>,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const bool,
                        const bool,
                        const bool,
                        const map<tuple<string, double>, vector<vector<double>>>,
                        const map<tuple<string, double>, vector<vector<double>>>,
                        const map<tuple<string, string, double>, vector<vector<double>>>,
                        const map<tuple<string, double>, vector<vector<double>>>,
                        const bool,
                        const bool,
                        const int,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const double,
                        const string,
                        const string,
                        const string,
                        const string,
                        const string,
                        const string,
                        const bool,
                        const string,
                        const double,
                        const double,
                        const string,
                        const bool,
                        const bool,
                        const string,
                        const string,
                        const map<int, vector<string>>>())
        .def_readwrite("outputfile", &Cppargs::outputfile)
        .def_readwrite("lastc_flag", &Cppargs::lastc_flag)
        .def_readwrite("lowertime", &Cppargs::lowertime)
        .def_readwrite("uppertime", &Cppargs::uppertime)
        .def_readwrite("timeout", &Cppargs::timeout)
        .def_readwrite("decr_vnum", &Cppargs::decr_vnum)
        .def_readwrite("avetime_ape_flag", &Cppargs::avetime_ape_flag)
        .def_readwrite("avetime_ape", &Cppargs::avetime_ape)
        .def_readwrite("avetime_ae_flag", &Cppargs::avetime_ae_flag)
        .def_readwrite("avetime_ae", &Cppargs::avetime_ae)
        .def_readwrite("avevisits_ape_flag", &Cppargs::avevisits_ape_flag)
        .def_readwrite("avevisits_ape", &Cppargs::avevisits_ape)
        .def_readwrite("avevisits_ae_flag", &Cppargs::avevisits_ae_flag)
        .def_readwrite("avevisits_ae", &Cppargs::avevisits_ae)
        .def_readwrite("avedist_ape_flag", &Cppargs::avedist_ape_flag)
        .def_readwrite("avedist_ape", &Cppargs::avedist_ape)
        .def_readwrite("avedist_ae_flag", &Cppargs::avedist_ae_flag)
        .def_readwrite("avedist_ae", &Cppargs::avedist_ae)
        .def_readwrite("outputstime", &Cppargs::outputstime)
        .def_readwrite("outputs_count", &Cppargs::outputs_count)
        .def_readwrite("evplan", &Cppargs::evplan)
        .def_readwrite("ls_d_starttime", &Cppargs::ls_d_starttime)
        .def_readwrite("opt_d_weight", &Cppargs::opt_d_weight)
        .def_readwrite("opt_t_weight", &Cppargs::opt_t_weight)
        .def_readwrite("opt_e_weight", &Cppargs::opt_e_weight)
        .def_readwrite("opt_x_weight", &Cppargs::opt_x_weight)
        .def_readwrite("non_violation_vehicles_route", &Cppargs::non_violation_vehicles_route)
        .def_readwrite("non_violation_management_dtable", &Cppargs::non_violation_management_dtable)
        .def_readwrite("non_violation_management_ttable", &Cppargs::non_violation_management_ttable)
        .def_readwrite("non_violation_management_etable", &Cppargs::non_violation_management_etable)
        .def_readwrite("non_violation_management_xtable", &Cppargs::non_violation_management_xtable)
        .def_readwrite("atpen", &Cppargs::atpen)
        .def_readwrite("aload", &Cppargs::aload)
        .def_readwrite("time_error", &Cppargs::time_error)
        .def_readwrite("visits_error", &Cppargs::visits_error)
        .def_readwrite("dist_error", &Cppargs::dist_error)
        .def_readwrite("pickup_flag", &Cppargs::pickup_flag)
        .def_readwrite("skillshuffle_flag", &Cppargs::skillshuffle_flag)
        .def_readwrite("bulkShipping_flag", &Cppargs::bulkShipping_flag)
        .def_readwrite("management_dtable", &Cppargs::management_dtable)
        .def_readwrite("management_ttable", &Cppargs::management_ttable)
        .def_readwrite("management_etable", &Cppargs::management_etable)
        .def_readwrite("management_xtable", &Cppargs::management_xtable)
        .def_readwrite("delaystart_flag", &Cppargs::delaystart_flag)
        .def_readwrite("multitrip_flag", &Cppargs::multitrip_flag)
        .def_readwrite("multithread_num", &Cppargs::multithread_num)
        .def_readwrite("timePenaltyWeight", &Cppargs::timePenaltyWeight)
        .def_readwrite("loadPenaltyWeight", &Cppargs::loadPenaltyWeight)
        .def_readwrite("load2PenaltyWeight", &Cppargs::load2PenaltyWeight)
        .def_readwrite("timeLevelingPenaltyWeight", &Cppargs::timeLevelingPenaltyWeight)
        .def_readwrite("visitsLevelingPenaltyWeight", &Cppargs::visitsLevelingPenaltyWeight)
        .def_readwrite("distLevelingPenaltyWeight", &Cppargs::distLevelingPenaltyWeight)
        .def_readwrite("sparePenaltyWeight", &Cppargs::sparePenaltyWeight)
        .def_readwrite("timePenaltyType", &Cppargs::timePenaltyType)
        .def_readwrite("loadPenaltyType", &Cppargs::loadPenaltyType)
        .def_readwrite("load2PenaltyType", &Cppargs::load2PenaltyType)
        .def_readwrite("timeLevelingPenaltyType", &Cppargs::timeLevelingPenaltyType)
        .def_readwrite("visitsLevelingPenaltyType", &Cppargs::visitsLevelingPenaltyType)
        .def_readwrite("distLevelingPenaltyType", &Cppargs::distLevelingPenaltyType)
        .def_readwrite("delaystart2_flag", &Cppargs::delaystart2_flag)
        .def_readwrite("init_changecust", &Cppargs::init_changecust)
        .def_readwrite("break_interval", &Cppargs::break_interval)
        .def_readwrite("break_time", &Cppargs::break_time)
        .def_readwrite("br_file", &Cppargs::br_file)
        .def_readwrite("break_flag", &Cppargs::break_flag)
        .def_readwrite("opt_unassigned_flag", &Cppargs::opt_unassigned_flag)
        .def_readwrite("unass_file", &Cppargs::unass_file)
        .def_readwrite("reservedv_file", &Cppargs::reservedv_file)
        .def_readwrite("initrout", &Cppargs::initroute);

        //cross_dtable
        m.def("cross_dtable", &cross_dtable);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}