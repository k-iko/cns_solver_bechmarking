#include "outputs_middle.hpp"

/**
 * @brief タイムアウト発生時に途中結果を出力する関数
 * @param[in] vehicles 車両リスト
 * @param[in] customers 配送先リスト
 * @param[in] argvs グローバル変数格納用
 * @param[in] sLogBuf ログメッセージ
 * @details タイムアウト発生時に途中結果をCSVファイルおよびコンソールログに出力する関数。
 */
void outputs_middle(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const Cppargs &argvs, string &sLogBuf){
    sLogBuf += "\n";
    sLogBuf += "output_middle:time,count ";
    sLogBuf += to_string(argvs.outputstime);
    sLogBuf += " ";
    sLogBuf += to_string(argvs.outputs_count);
    sLogBuf += "\n";
    sLogBuf += "output_middle nowtime: ";

    time_t rawtime;
    tm* timeinfo;
    char buffer [80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    string tod = buffer;
    sLogBuf += tod;

    //ファイル出力
    replace(tod.begin(), tod.end(), ':', '-');
    generate_routefile(argvs.outputfile+"0_"+tod+"(noskill).csv", vehicles, customers, argvs.bulkShipping_flag);
    generate_resultfile(argvs.outputfile+"0_"+tod+"(noskill).detail.csv", vehicles, customers, argvs);

    //コスト計算
    int vidx = 0;
    double totalcost = 0, totalcost_wolast = 0;
    double totaltime = 0, totaltime_wolast = 0;

    for(auto v : vehicles){
        int non_depo_num = 0;
        int route_size = (int) v.route.size();

        for(int i = 1; i < route_size-1; i++){
            if(customers.at(v.route.at(i)).custno != "0" && customers.at(v.route.at(i)).custno.at(0) != 'b' && customers.at(v.route.at(i)).custno.at(0) != 'e'){
                non_depo_num++;
            }
        }
        if(non_depo_num == 0){
            continue; //空車両ならスキップ 
        }  

        totalcost += v.totalcost;
        totalcost_wolast += v.totalcost_wolast;
        vidx++;
    }

    sLogBuf += "===== output middle(skills not supported): TOTAL_DISTANCE:  ";
    char *totalcost_str;
    sprintf(totalcost_str, "%.2f", round(totalcost*100)/100);
    sLogBuf += totalcost_str;
    sLogBuf += "  ===== 0\n";
    sLogBuf += "===== output middle(skills not supported): TOTAL_DISTANCE_TO_LASTCUST:  ";
    char *totalcost_wolast_str;
    sprintf(totalcost_wolast_str, "%.2f", round(totalcost_wolast*100)/100);
    sLogBuf += totalcost_wolast_str;
    sLogBuf += "  =====\n";

    sLogBuf += "\n";
    for(auto v : vehicles){
        int non_depo_num = 0;
        int route_size = (int) v.route.size();

        for(int i = 1; i < route_size-1; i++){
            if(customers.at(v.route.at(i)).custno != "0" && customers.at(v.route.at(i)).custno.at(0) != 'b' && customers.at(v.route.at(i)).custno.at(0) != 'e'){
                non_depo_num++;
            }
        }
        if(non_depo_num == 0){
            continue; //空車両ならスキップ 
        }  

        totaltime += v.lv.at(route_size-1)-v.arr.at(0);
        for(int i = route_size-2; i > 0; i--){
            if(v.route.at(i) != 0 && customers.at(v.route.at(i)).custno.at(0) != 'b' && customers.at(v.route.at(i)).custno.at(0) != 'e'){
                totaltime_wolast += v.lv.at(i)-v.arr.at(0);
                break;
            }
        }
    }
    sLogBuf += "===== output middle(skills not supported): TOTAL_DELIVERY_TIME:  ";
    char *totaltime_str;
    sprintf(totaltime_str, "%.2f", round(totaltime*100)/100);
    sLogBuf += totaltime_str;
    sLogBuf += "  ===== 0\n";
    sLogBuf += "===== output middle(skills not supported): TOTAL_DELIVERY_TIME_TO_LASTCUST:  ";
    char *totaltime_wolast_str;
    sprintf(totaltime_wolast_str, "%.2f", round(totaltime_wolast*100)/100);
    sLogBuf += totaltime_wolast_str;
    sLogBuf += "  =====\n";

    if(argvs.decr_vnum == true){
        sLogBuf += "===== output middle(skills not supported): TOTAL_NUMBER_OF_VEHICLES:  ";
        sLogBuf += to_string(vidx);
        sLogBuf += "  =====\n";
    }
}