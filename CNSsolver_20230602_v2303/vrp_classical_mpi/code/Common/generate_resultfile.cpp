#include "generate_resultfile.hpp"

/**
 * @brief 結果の詳細ルートをCSVファイルに出力する関数
 * @param[in] filename 出力ファイル名
 * @param[in] vehicles 車両リスト
 * @param[in] customers 配送先リスト
 * @param[in] argvs グローバル変数格納用
 * @details 結果の各車両のルート等の詳細情報をCSVファイルに出力する関数。
 */
void generate_resultfile(const string filename, const vector<Vehicle> &vehicles, const vector<Customer> &customers, const Cppargs &argvs){
    ofstream writecsv(filename);
    
    for (auto v : vehicles) {
        int non_depo_num = 0;
        int route_size = (int) v.route.size();

        for(int i = 1; i < route_size-1; i++){
            if((customers.at(v.route.at(i)).custno != "0")&&(customers.at(v.route.at(i)).custno.at(0) != 'b')){
                non_depo_num++;
            }
        }
        if(non_depo_num == 0){
            continue; //空車両ならスキップ 
        }       
        
        writecsv << "VEHICLE,";
        writecsv << v.vehno;
        writecsv << ",ROUTE,";
        int tieup_cust_num = 0;
        if(argvs.bulkShipping_flag == true){
            for(auto c : v.route){
                tieup_cust_num += customers.at(c).tieup_customer.size();
            }

            writecsv << tieup_cust_num;
        }
        else{
            writecsv << route_size;
        }


        //ルート出力
        if(argvs.bulkShipping_flag == true){
            for (int i = 0; i < route_size; i++) {
                int tied_num = customers.at(v.route.at(i)).tieup_customer.size();

                for(int j = 0; j < tied_num; j++){
                    writecsv << ",";
                    writecsv << customers.at(customers.at(v.route.at(i)).tieup_customer.at(j)).custno;
                }
            }  
        }
        else{
            for (int i = 0; i < route_size; i++) {
                writecsv << ",";
                writecsv << customers.at(v.route.at(i)).custno;
            }  
        }  


        //コスト出力
        if(argvs.lastc_flag == true){
            writecsv << ",COST_TO_LAST,";

            if(v.delay_arr.empty() || argvs.break_flag==true){
                writecsv << v.totalcost_wolast;
            }
            else{
                writecsv << v.delay_totalcost_wolast;
            }
        }
        else{
            writecsv << ",COST_TO_DEPO,";

            if(v.delay_arr.empty() || argvs.break_flag==true){
                writecsv << v.totalcost;
            }
            else{
                writecsv << v.delay_totalcost;
            }
        }


        //車両情報出力
        writecsv << ",OPSKILL,";
        writecsv << v.opskill;
        writecsv << ",DRSKILL,";
        writecsv << v.drskill;
        writecsv << ",DEMAND+,";
        writecsv << v.load_onbrd;
        writecsv << ",DEMAND-,";
        writecsv << v.load_pickup;
        writecsv << ",LOADLIMIT,";
        writecsv << v.cap;
        writecsv << ",DEMAND2+,";
        writecsv << v.load_onbrd2;
        writecsv << ",DEMAND2-,";
        writecsv << v.load_pickup2;
        writecsv << ",LOADLIMIT2,";
        writecsv << v.cap2;

        writecsv << "\n";

        writecsv << "ARRIVAL,";
        writecsv << v.vehno;
        writecsv << ",TIME,";
        if(argvs.bulkShipping_flag == true){
            writecsv << tieup_cust_num;
        }
        else{
            writecsv << route_size;
        }

        if(argvs.bulkShipping_flag == true){
            for (int i = 0; i < route_size; i++) {
                int tied_num = customers.at(v.route.at(i)).tieup_customer.size();

                for(int j = 0; j < tied_num; j++){
                    writecsv << ",";
                    if(v.delay_arr.empty() || argvs.break_flag==true){
                        writecsv << round(v.arr.at(i)*100000)/100000;
                    }
                    else{
                        writecsv << round(v.delay_arr.at(i)*100000)/100000;
                    }
                }
            }  
        }
        else{
            for (int i = 0; i < route_size; i++) {
                writecsv << ",";
                if(v.delay_arr.empty() || argvs.break_flag==true){
                        writecsv << round(v.arr.at(i)*100000)/100000;
                }
                else{
                    writecsv << round(v.delay_arr.at(i)*100000)/100000;
                }
            }  
        }  

        writecsv << "\n";
    }


    //トータルコスト出力
    double totalcost = 0, totalcost_wolast = 0;
    for(auto v : vehicles){
        if(v.delay_arr.empty() || argvs.break_flag==true){
            totalcost += v.totalcost;
            totalcost_wolast += v.totalcost_wolast;
        }
        else{
            totalcost += v.delay_totalcost;
            totalcost_wolast += v.delay_totalcost_wolast;
        }
    }

    if(argvs.lastc_flag == true){
        writecsv << "TOTALCOST,";
        writecsv << totalcost_wolast;
        writecsv << "\n";
    }
    else{
        writecsv << "TOTALCOST,";
        writecsv << totalcost;
        writecsv << "\n";
    }

    writecsv << "TOTALCOST_TO_LAST,";
    writecsv << totalcost_wolast;
    writecsv << "\n";
    writecsv << "TOTALCOST_TO_DEPO,";
    writecsv << totalcost;
    writecsv << "\n";

    double totaltime = 0, totaltime_wolast = 0;
    for(auto v : vehicles){
        if(v.delay_arr.empty() || argvs.break_flag==true){
            int arr_size = (int) v.arr.size();
            totaltime += v.arr.at(arr_size-1)-v.arr.at(0);
            if(customers.at(v.route.at(arr_size-2)).custno != "0"){
                totaltime_wolast += v.arr.at(arr_size-2)-v.arr.at(0);
            }
            else{
                totaltime_wolast += v.arr.at(arr_size-3)-v.arr.at(0);
            }
        }
        else{
            int arr_size = (int) v.delay_arr.size();
            totaltime += v.delay_arr.at(arr_size-1)-v.delay_arr.at(0);
            if(customers.at(v.route.at(arr_size-2)).custno != "0"){
                totaltime_wolast += v.delay_arr.at(arr_size-2)-v.delay_arr.at(0);
            }
            else{
                totaltime_wolast += v.delay_arr.at(arr_size-3)-v.delay_arr.at(0);
            }
        }
    }

    writecsv << "TOTALTIME_TO_LAST,";
    writecsv << totaltime_wolast;
    writecsv << "\n";
    writecsv << "TOTALTIME_TO_DEPO,";
    writecsv << totaltime;
    writecsv << "\n";

    if(argvs.evplan == true){
        double totalerec = 0, totalerec_wolast = 0;
        if(!argvs.management_etable.empty()){
            for(auto v : vehicles){
                if(v.delay_arr.empty() || argvs.break_flag==true){
                    int route_size = (int) v.route.size();
                    for(int i = 0; i < route_size-2; i++){
                        if(i+1 != route_size-2 || customers.at(v.route.at(i+1)).custno != "0"){
                            totalerec_wolast += get_etable(v.vtype, v.etype, v.lv.at(i), argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
                        }
                        totalerec += get_etable(v.vtype, v.etype, v.lv.at(i), argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
                    }

                    totalerec += get_etable(v.vtype, v.etype, v.lv.at(route_size-2), argvs.management_etable, customers.at(v.route.at(route_size-2)).spotno, customers.at(v.route.at(route_size-1)).spotno);
                }
                else{
                    int route_size = (int) v.route.size();
                    for(int i = 0; i < route_size-2; i++){
                        if(i+1 != route_size-2 || customers.at(v.route.at(i+1)).custno != "0"){
                            totalerec_wolast += get_etable(v.vtype, v.etype, v.delay_lv.at(i), argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
                        }
                        totalerec += get_etable(v.vtype, v.etype, v.delay_lv.at(i), argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
                    }

                    totalerec += get_etable(v.vtype, v.etype, v.delay_lv.at(route_size-2), argvs.management_etable, customers.at(v.route.at(route_size-2)).spotno, customers.at(v.route.at(route_size-1)).spotno);
                }
            }
        }
        else{
            for(auto v : vehicles){
                if(v.delay_arr.empty() || argvs.break_flag==true){
                    int route_size = (int) v.route.size();
                    for(int i = 0; i < route_size-2; i++){
                        if(i+1 != route_size-2 || customers.at(v.route.at(i+1)).custno != "0"){
                            totalerec_wolast += (get_dtable(v.vtype, v.lv.at(i), argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno)/1000)/v.e_cost;
                        }
                        totalerec += (get_dtable(v.vtype, v.lv.at(i), argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno)/1000)/v.e_cost;
                    }

                    totalerec += (get_dtable(v.vtype, v.lv.at(route_size-2), argvs.management_dtable, customers.at(v.route.at(route_size-2)).spotno, customers.at(v.route.at(route_size-1)).spotno)/1000)/v.e_cost;
                }
                else{
                    int route_size = (int) v.route.size();
                    for(int i = 0; i < route_size-2; i++){
                        if(i+1 != route_size-2 || customers.at(v.route.at(i+1)).custno != "0"){
                            totalerec_wolast += (get_dtable(v.vtype, v.delay_lv.at(i), argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno)/1000)/v.e_cost;
                        }
                        totalerec += (get_dtable(v.vtype, v.delay_lv.at(i), argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno)/1000)/v.e_cost;
                    }

                    totalerec += (get_dtable(v.vtype, v.delay_lv.at(route_size-2), argvs.management_dtable, customers.at(v.route.at(route_size-2)).spotno, customers.at(v.route.at(route_size-1)).spotno)/1000)/v.e_cost;
                }
            }
        }

        writecsv << "TOTALEREC_TO_LAST,";
        writecsv << totalerec_wolast;
        writecsv << "\n";
        writecsv << "TOTALEREC_TO_DEPO,";
        writecsv << totalerec;
        writecsv << "\n";
    }

    double totalxcost = 0, totalxcost_wolast = 0;
    if(!argvs.management_xtable.empty()){
        for(auto v : vehicles){
            if(v.delay_arr.empty() || argvs.break_flag==true){
                int route_size = (int) v.route.size();
                for(int i = 0; i < route_size-2; i++){
                    if(i+1 != route_size-2 || customers.at(v.route.at(i+1)).custno != "0"){
                        totalxcost_wolast += get_xtable(v.vtype, v.lv.at(i), argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
                    }
                    totalxcost += get_xtable(v.vtype, v.lv.at(i), argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
                }

                totalxcost += get_xtable(v.vtype, v.lv.at(route_size-2), argvs.management_xtable, customers.at(v.route.at(route_size-2)).spotno, customers.at(v.route.at(route_size-1)).spotno);
            }
            else{
                int route_size = (int) v.route.size();
                for(int i = 0; i < route_size-2; i++){
                    if(i+1 != route_size-2 || customers.at(v.route.at(i+1)).custno != "0"){
                        totalxcost_wolast += get_xtable(v.vtype, v.delay_lv.at(i), argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
                    }
                    totalxcost += get_xtable(v.vtype, v.delay_lv.at(i), argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
                }

                totalxcost += get_xtable(v.vtype, v.delay_lv.at(route_size-2), argvs.management_xtable, customers.at(v.route.at(route_size-2)).spotno, customers.at(v.route.at(route_size-1)).spotno);
            }
        }

        writecsv << "TOTALXCOST_TO_LAST,";
        writecsv << totalxcost_wolast;
        writecsv << "\n";
        writecsv << "TOTALXCOST_TO_DEPO,";
        writecsv << totalxcost;
        writecsv << "\n";
    }


    //コスト計算用オプション出力
    writecsv << "LAST_FLAG,";
    if(argvs.lastc_flag == true){
        writecsv << "True";
    }
    else{
        writecsv << "False";
    }
    writecsv << "\n";

    writecsv << "OPT_D,OPT_T,OPT_E,OPT_X,";
    writecsv << argvs.opt_d_weight;
    writecsv << ",";
    writecsv << argvs.opt_t_weight;
    writecsv << ",";
    writecsv << argvs.opt_e_weight;
    writecsv << ",";
    writecsv << argvs.opt_x_weight;
    writecsv << "\n";


    //配送先情報出力
    if(argvs.bulkShipping_flag == true){
        for(auto c : customers){
            for(auto tc : c.tieup_customer){
                bool is_enddepot = false;
                for(auto v : vehicles){
                    int route_size = (int) v.route.size();
                    if(customers.at(v.route.at(route_size-1)).tieup_customer.at(0) == tc){
                        is_enddepot =true; //最終訪問先は出力しない
                        break;
                    }
                }

                if(is_enddepot){
                    continue; //最終訪問先は出力しない
                }

                writecsv << "LOC,";
                writecsv << customers.at(tc).custno;
                writecsv << ",";
                writecsv << customers.at(tc).y;
                writecsv << ",";
                writecsv << customers.at(tc).x;
                writecsv << "\n";
            }
        }
    }
    else{
        int cust_num = customers.size();
        for(int i = 0; i < cust_num; i++){
            bool is_enddepot = false;
            for(auto v : vehicles){
                int route_size = (int) v.route.size();
                if(v.route.at(route_size-1) == i){
                    is_enddepot =true; //最終訪問先は出力しない
                    break;
                }
            }

            if(is_enddepot){
                continue; //最終訪問先は出力しない
            }

            writecsv << "LOC,";
            writecsv << customers.at(i).custno;
            writecsv << ",";
            writecsv << customers.at(i).y;
            writecsv << ",";
            writecsv << customers.at(i).x;
            writecsv << "\n";
        }
    }

    writecsv.close();
}