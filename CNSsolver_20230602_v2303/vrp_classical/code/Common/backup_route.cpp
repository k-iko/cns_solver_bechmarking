#include "backup_route.hpp"

/**
 * @brief 違反なしのルートを保存する関数
 * @param[in] vehicles 車両リスト
 * @param[in] argvs グローバル変数格納用
 * @details 全車両で違反がない場合は各車両のルートをargvs.non_violation_vehicles_routeに保存する関数。
 */
void backup_route(const vector<Vehicle> &vehicles, const vector<Customer> &customers, Cppargs &argvs){
    bool can_backup =true;

    for(auto v : vehicles){
        if(((argvs.loadPenaltyType == "hard" || argvs.loadPenaltyWeight > 0) && v.load_pen > 0) || ((argvs.load2PenaltyType == "hard" || argvs.load2PenaltyWeight > 0) && v.load_pen2 > 0) || ((argvs.timePenaltyType == "hard" || argvs.timePenaltyWeight > 0) && compare_time(v.time_pen) > 0)){
            can_backup = false; //違反がある場合はバックアップしない
        }
    }

    if(((argvs.timeLevelingPenaltyType == "hard" || argvs.timeLevelingPenaltyWeight > 0) && argvs.avetime_ape_flag == true && argvs.time_error > 0)
    || ((argvs.timeLevelingPenaltyType == "hard" || argvs.timeLevelingPenaltyWeight > 0) && argvs.avetime_ae_flag == true && argvs.time_error > 0)
    || ((argvs.visitsLevelingPenaltyType == "hard" || argvs.visitsLevelingPenaltyWeight > 0) && argvs.avevisits_ape_flag == true && argvs.visits_error > 0)
    || ((argvs.visitsLevelingPenaltyType == "hard" || argvs.visitsLevelingPenaltyWeight > 0) && argvs.avevisits_ae_flag == true && argvs.visits_error > 0)
    || ((argvs.distLevelingPenaltyType == "hard" || argvs.distLevelingPenaltyWeight > 0) && argvs.avedist_ape_flag == true && argvs.dist_error > 0)
    || ((argvs.distLevelingPenaltyType == "hard" || argvs.distLevelingPenaltyWeight > 0) && argvs.avedist_ae_flag == true && argvs.dist_error > 0)){
        can_backup = false; //違反がある場合はバックアップしない
    }

    //違反なしの場合、解保存用Vehicle.routeを保持
    if(can_backup){
        for(auto v : vehicles){
            argvs.non_violation_vehicles_route[v.vehno] = v.route;
        }

		if(argvs.break_flag){
            bool no_spotid = false;
            for(auto v : vehicles){
                int takebreak_size = v.takebreak.size();
                for(int t=0; t<takebreak_size; t++){
                    if(customers.at(v.takebreak.at(t)).spotid ==""){
                        no_spotid = true;
                        break;
                    }
                }
            }

            if(get_breakspot_pattern(vehicles, argvs)==3 || no_spotid==true){
                // 違反なしの場合、ttable,dtable,etable,xtableを保持
                argvs.non_violation_management_dtable = argvs.management_dtable;
                argvs.non_violation_management_ttable = argvs.management_ttable;
                argvs.non_violation_management_etable = argvs.management_etable;
                argvs.non_violation_management_xtable = argvs.management_xtable;
            }
        }
    }
}