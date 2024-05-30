#ifndef INCLUDED_CPPARGS
#define INCLUDED_CPPARGS

#include <vector>
#include <string>
#include <map>
using namespace std;

class Cppargs{
    public:
        string outputfile; //出力ファイル名
        bool lastc_flag; //最終訪問先をペナルティ計算に含めるかどうかのフラグ(true : 含めない)
        double lowertime; //下限作業時間
        double uppertime; //上限作業時間
        double timeout; //タイムアウト時間
        bool decr_vnum; //台数削減用フラグ(true : 台数削減モード)
        bool avetime_ape_flag; //作業時間平準化(パーセンテージ)用フラグ(true : 平準化モード)
        double avetime_ape; //作業時間平準化(パーセンテージ)マージン
        bool avetime_ae_flag; //作業時間平準化(差)用フラグ(true : 平準化モード)
        double avetime_ae; //作業時間平準化(差)マージン
        bool avevisits_ape_flag; //配送件数平準化(パーセンテージ)用フラグ(true : 平準化モード)
        double avevisits_ape; //配送件数平準化(パーセンテージ)マージン
        bool avevisits_ae_flag; //配送件数平準化(差)用フラグ(true : 平準化モード)
        double avevisits_ae; //配送件数平準化(差)マージン
        bool avedist_ape_flag; //距離平準化(パーセンテージ)用フラグ(true : 平準化モード)
        double avedist_ape; //距離平準化(パーセンテージ)マージン
        bool avedist_ae_flag; //距離平準化(差)用フラグ(true : 平準化モード)
        double avedist_ae; //距離平準化(差)マージン
        double outputstime; //デバッグ用タイムアウト時間
        int outputs_count; //デバッグ用出力回数
        bool evplan; //電力解用フラグ(true : 電力解生成)
        double ls_d_starttime; //プログラム実行開始時刻
        double opt_d_weight; //距離最適化重み
        double opt_t_weight; //時間最適化重み
        double opt_e_weight; //電力最適化重み
        double opt_x_weight; //経験コスト最適化重み
        map<int, vector<int>> non_violation_vehicles_route; //違反なし解保存用
        map<tuple<string, double>, vector<vector<double>>> non_violation_management_dtable; //違反なし解距離テーブル
        map<tuple<string, double>, vector<vector<double>>> non_violation_management_ttable; //違反なし解時間テーブル
        map<tuple<string, string, double>, vector<vector<double>>> non_violation_management_etable; //違反なし解電力テーブル
        map<tuple<string, double>, vector<vector<double>>> non_violation_management_xtable; //違反なし解経験コストテーブル
        double atpen; //時間ペナルティ違反量
        double aload; //積載量ペナルティ違反量
        double time_error; //配送時間平準化ペナルティ違反量
        double visits_error; //配送件数平準化ペナルティ違反量
        double dist_error; //距離平準化ペナルティ違反量
        bool pickup_flag; //集荷があるかを判定するフラグ(true : 集荷あり)
        bool skillshuffle_flag; //スキルシャッフル用フラグ(true : スキルシャッフル可能)
        bool bulkShipping_flag; //荷物tie-up用フラグ(true : 荷物をtie-upする) 
        map<tuple<string, double>, vector<vector<double>>> management_dtable; //距離テーブル
        map<tuple<string, double>, vector<vector<double>>> management_ttable; //時間テーブル
        map<tuple<string, string, double>, vector<vector<double>>> management_etable; //電力テーブル
        map<tuple<string, double>, vector<vector<double>>> management_xtable; //経験コストテーブル
        bool delaystart_flag; //出発遅延用フラグ(true : 出発遅延モード)
        bool multitrip_flag; //回転モード用フラグ(tru : 回転モード)
        int multithread_num; //スレッド数
        double timePenaltyWeight; //時間ペナルティ重み
        double loadPenaltyWeight; //積載量ペナルティ重み
        double load2PenaltyWeight; //第2積載量ペナルティ重み
        double timeLevelingPenaltyWeight; //時間ばらつきペナルティ重み
        double visitsLevelingPenaltyWeight; //件数ばらつきペナルティ重み
        double distLevelingPenaltyWeight; //距離ばらつきペナルティ重み
        double sparePenaltyWeight; //未割り当て荷物最適化ペナルティ
        string timePenaltyType; //時間ペナルティタイプ
        string loadPenaltyType; //積載量ペナルティタイプ
        string load2PenaltyType; //第2積載量ペナルティタイプ
        string timeLevelingPenaltyType; //時間ばらつきペナルティタイプ
        string visitsLevelingPenaltyType; //件数ばらつきペナルティタイプ
        string distLevelingPenaltyType; //距離ばらつきペナルティタイプ
        bool delaystart2_flag; //出発遅延用フラグ(最適化考慮、true : 出発遅延モード)
        string init_changecust; //割当済荷物(＝初期解入力荷物)の変更条件
        double break_interval;  //休憩間隔
        double break_time;  //休憩時間
        string br_file; //休憩ファイル
        bool break_flag;    //休憩機能使用
        bool opt_unassigned_flag; //未割り当て荷物最適化フラグ
        string unass_file;  // 未割り当て荷物
        string reservedv_file;  // 予約車両
        map<int, vector<string>> initroute;

    Cppargs(const string &outputfile,
            const bool &lastc_flag,
            const double &lowertime,
            const double &uppertime,
            const double &timeout,
            const bool &decr_vnum,
            const bool &avetime_ape_flag,
            const double &avetime_ape,
            const bool &avetime_ae_flag,
            const double &avetime_ae,
            const bool &avevisits_ape_flag,
            const double &avevisits_ape,
            const bool &avevisits_ae_flag,
            const double &avevisits_ae,
            const bool &avedist_ape_flag,
            const double &avedist_ape,
            const bool &avedist_ae_flag,
            const double &avedist_ae,
            const double &outputstime,
            const int &outputs_count,
            const bool &evplan,
            const double &ls_d_starttime,
            const double &opt_d_weight,
            const double &opt_t_weight,
            const double &opt_e_weight,
            const double &opt_x_weight,
            const map<int, vector<int>> &non_violation_vehicles_route,
            const map<tuple<string, double>, vector<vector<double>>> &non_violation_management_dtable,
            const map<tuple<string, double>, vector<vector<double>>> &non_violation_management_ttable,
            const map<tuple<string, string, double>, vector<vector<double>>> &non_violation_management_etable,
            const map<tuple<string, double>, vector<vector<double>>> &non_violation_management_xtable,
            const double &atpen,
            const double &aload,
            const double &time_error,
            const double &visits_error,
            const double &dist_error,
            const bool &pickup_flag,
            const bool &skillshuffle_flag,
            const bool &bulkShipping_flag,
            const map<tuple<string, double>, vector<vector<double>>> &management_dtable,
            const map<tuple<string, double>, vector<vector<double>>> &management_ttable,
            const map<tuple<string, string, double>, vector<vector<double>>> &management_etable,
            const map<tuple<string, double>, vector<vector<double>>> &management_xtable,
            const bool &delaystart_flag,
            const bool &multitrip_flag,
            const int &multithread_num,
            const double &timePenaltyWeight,
            const double &loadPenaltyWeight,
            const double &load2PenaltyWeight,
            const double &timeLevelingPenaltyWeight,
            const double &visitsLevelingPenaltyWeight,
            const double &distLevelingPenaltyWeight,
            const double &sparePenaltyWeight,
            const string &timePenaltyType,
            const string &loadPenaltyType,
            const string &load2PenaltyType,
            const string &timeLevelingPenaltyType,
            const string &visitsLevelingPenaltyType,
            const string &distLevelingPenaltyType,
            const bool &delaystart2_flag,
            const string &init_changecust,
            const double &break_interval,
            const double &break_time,
            const string &br_file,
            const bool &break_flag,
            const bool &opt_unassigned_flag,
            const string &unass_file,
            const string &reservedv_file,
            const map<int, vector<string>> &initroute
            ):
                outputfile(outputfile),
                lastc_flag(lastc_flag),
                lowertime(lowertime),
                uppertime(uppertime),
                timeout(timeout),
                decr_vnum(decr_vnum),
                avetime_ape_flag(avetime_ape_flag),
                avetime_ape(avetime_ape),
                avetime_ae_flag(avetime_ae_flag),
                avetime_ae(avetime_ae),
                avevisits_ape_flag(avevisits_ape_flag),
                avevisits_ape(avevisits_ape),
                avevisits_ae_flag(avevisits_ae_flag),
                avevisits_ae(avevisits_ae),
                avedist_ape_flag(avedist_ape_flag),
                avedist_ape(avedist_ape),
                avedist_ae_flag(avedist_ae_flag),
                avedist_ae(avedist_ae),
                outputstime(outputstime),
                outputs_count(outputs_count),
                evplan(evplan),
                ls_d_starttime(ls_d_starttime),
                opt_d_weight(opt_d_weight),
                opt_t_weight(opt_t_weight),
                opt_e_weight(opt_e_weight),
                opt_x_weight(opt_x_weight),
                non_violation_vehicles_route(non_violation_vehicles_route),
                non_violation_management_dtable(non_violation_management_dtable),
                non_violation_management_ttable(non_violation_management_ttable),
                non_violation_management_etable(non_violation_management_etable),
                non_violation_management_xtable(non_violation_management_xtable),
                atpen(atpen),
                aload(aload),
                time_error(time_error),
                visits_error(visits_error),
                dist_error(dist_error),
                pickup_flag(pickup_flag),
                skillshuffle_flag(skillshuffle_flag),
                bulkShipping_flag(bulkShipping_flag),
                management_dtable(management_dtable),
                management_ttable(management_ttable),
                management_etable(management_etable),
                management_xtable(management_xtable),
                delaystart_flag(delaystart_flag),
                multitrip_flag(multitrip_flag),
                multithread_num(multithread_num),
                timePenaltyWeight(timePenaltyWeight),
                loadPenaltyWeight(loadPenaltyWeight),
                load2PenaltyWeight(load2PenaltyWeight),
                timeLevelingPenaltyWeight(timeLevelingPenaltyWeight),
                visitsLevelingPenaltyWeight(visitsLevelingPenaltyWeight),
                distLevelingPenaltyWeight(distLevelingPenaltyWeight),
                sparePenaltyWeight(sparePenaltyWeight),
                timePenaltyType(timePenaltyType),
                loadPenaltyType(loadPenaltyType),
                load2PenaltyType(load2PenaltyType),
                timeLevelingPenaltyType(timeLevelingPenaltyType),
                visitsLevelingPenaltyType(visitsLevelingPenaltyType),
                distLevelingPenaltyType(distLevelingPenaltyType),
                delaystart2_flag(delaystart2_flag),
                init_changecust(init_changecust),
                break_interval(break_interval),
                break_time(break_time),
                br_file(br_file),
                break_flag(break_flag),
                opt_unassigned_flag(opt_unassigned_flag),
                unass_file(unass_file),
                reservedv_file(reservedv_file),
                initroute(initroute)
                {}
};

#define debug_line {printf("%s %d\n", __FILE__, __LINE__); fflush(NULL);}

#endif