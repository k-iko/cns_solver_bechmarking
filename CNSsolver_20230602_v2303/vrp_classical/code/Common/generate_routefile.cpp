#include "generate_routefile.hpp"

/**
 * @brief 結果のルートをCSVファイルに出力する関数
 * @param[in] filename 出力ファイル名
 * @param[in] vehicles 車両リスト
 * @param[in] customers 配送先リスト
 * @param[in] bulkShipping_flag 大量輸送モード判定フラグ(true : 大量輸送モード)
 * @details 結果の各車両のルートをCSVファイルに出力する関数。
 */
void generate_routefile(const string filename, const vector<Vehicle> &vehicles, const vector<Customer> &customers, const bool bulkShipping_flag){
    ofstream writecsv(filename);
    writecsv << "route#,Id\n";
    
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
        
        writecsv << "route";
        writecsv << v.vehno;
        if(bulkShipping_flag == true){
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

        writecsv << "\n";
    }
    writecsv.close();
}