#ifndef INCLUDED_VEHICLE
#define INCLUDED_VEHICLE

#include <vector>
#include <string>
#include "Customer.hpp"
#include "Cppargs.hpp"
using namespace std;

class Vehicle{
    public:
        vector<int> route; //車両ルート
        vector<double> arr; //到着時刻
        vector<double> rem_batt; //残電力
        vector<double> chg_batt; //充電量
        vector<double> slack_t; //余裕時間
        vector<double> st; //作業開始時刻
        vector<double> lv; //出発時刻
        vector<double> remdist; //残距離
        int vehno; //車両番号
        double cap; //積載量上限
        double cap2; //第2積載量上限
        double totalcost; //総走行距離
        double totalcost_wolast; //総走行距離(最終配送先まで)
        double load_onbrd; //配送荷物量
        double load_onbrd2; //第2配送荷物量 
        double load_pickup; //集荷荷物量
        double load_pickup2; //第2集荷荷物量
        double opskill; //作業スキル
        double drskill; //運転スキル
        double e_capacity; //充電容量
        double e_initial; //初期充電量
        double e_cost; //電費
        double e_margin; //残電力マージン
        double time_pen; //時間ペナルティ
        double load_pen; //積載量ペナルティ
        double load_pen2; //第2積載量ペナルティ
        string vtype; //車両タイプ
        string etype; //電動車両タイプ
        bool direct_flag; //直送便判定フラグ(true : 直送便)
        double starthour; //営業開始時刻
        double endhour; // 営業終了時刻
        int maxvisit; //訪問数上限
        int maxrotate; //回転数上限
        double depotservt; //拠点作業時間
        int rmpriority; //台数削減優先度
        map<int, double> chg_batt_dict; //充電用
        vector<double> delay_arr; //遅延後到着時刻
        double delay_totalcost; //遅延後総走行距離
        double delay_totalcost_wolast; //遅延後総走行距離(最終配送先まで)
        vector<double> delay_rem_batt; //遅延後残電力
        vector<double> delay_chg_batt; //遅延後充電量
        map<int, double> delay_chg_batt_dict; //遅延後
        vector<double> delay_slack_t;// 充電用(遅延後)
        vector<int> takebreak; //休憩情報
        bool reserved_v_flag;
        vector<double> delay_remdist; //遅延後距離
        vector<double> delay_lv; //遅延後出発時刻
        vector<double> delay_st; //遅延後作業開始時刻

        Vehicle(const vector<int> &route,
                const vector<double> &arr,
                const vector<double> &rem_batt,
                const vector<double> &chg_batt,
                const vector<double> &slack_t,
                const vector<double> &st,
                const vector<double> &lv,
                const vector<double> &remdist,
                const int &vehno,
                const double &cap,
                const double &cap2,
                const double &totalcost,
                const double &totalcost_wolast,
                const double &load_onbrd,
                const double &load_onbrd2,
                const double &load_pickup,
                const double &load_pickup2,
                const double &opskill,
                const double &drskill,
                const double &e_capacity,
                const double &e_initial,
                const double &e_cost,
                const double &e_margin,
                const double &time_pen,
                const double &load_pen,
                const double &load_pen2,
                const string &vtype,
                const string &etype,
                const bool &direct_flag,
                const double &starthour,
                const double &endhour,
                const int &maxvisit,
                const int &maxrotate,
                const double &depotservt,
                const int &rmpriority,
                const map<int, double> &chg_batt_dict,
                const vector<double> &delay_arr,
		const double &delay_totalcost,
		const double &delay_totalcost_wolast,
		const vector<double> &delay_rem_batt,
		const vector<double> &delay_chg_batt,
		const map<int, double> &delay_chg_batt_dict,
		const vector<double> &delay_slack_t,
                const vector<int> &takebreak,
                const bool &reserved_v_flag,
                const vector<double> &delay_remdist,
                const vector<double> &delay_lv,
                const vector<double> &delay_st);

        tuple<double, double> find_power_to_charge(const vector<Customer> &customers, const int start_idx, const Cppargs &argvs);

        void update_remdist(const vector<Customer> &customers, const Cppargs &argvs);

        void update_load_onbrd(const vector<Customer> &customers);

        void update_arrtime_tw(const vector<Customer> &customers, const map<tuple<string, double>, vector<vector<double>>> &management_ttable, const bool break_flag, const bool delayst2_flag = false);

        void update_rem_battery(const vector<Customer> &customers, const Cppargs &argvs, const int after_idx = 0);

        void update_totalcost_dtable(const vector<Customer> &customers, const Cppargs &argvs, const int start_idx = 0, const bool delayst2_flag = false);
};

#endif