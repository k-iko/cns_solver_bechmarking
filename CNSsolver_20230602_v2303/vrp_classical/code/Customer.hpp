#ifndef INCLUDED_CUSTOMER
#define INCLUDED_CUSTOMER

#include <vector>
#include <string>
using namespace std;

class Customer{
    public:
        vector<int> tieup_customer; //tie-upされた配送先
		vector<int> reje_ve; //立寄不可車両番号
		string custno; //顧客番号
		string spotid; //スポット番号
		int spotno; //テーブル参照時のインデックス番号
		double y; //y座標
		double x; //x座標
		double dem; //荷物量
		double dem2; //第2荷物量
		double servt; //荷作業時間
		double e_rate; //充電レート
		double parkt_arrive; //駐車時間
        double parkt_open; //駐車時間
		double depotservt; //拠点作業時間
		double ready_fs; //受け入れ開始時刻
		double due_fs; //受け入れ終了時刻
        string requestType; //荷物の種類
        bool deny_unass; //未割り当て禁止フラグ
        string changecust; //割当済荷物(＝初期解入力荷物)の変更条件

    Customer(const vector<int> &tieup_customer,
            const vector<int> &reje_ve,
            const string &custno,
            const string &spotid,
            const int &spotno,
            const double &y,
            const double &x,
            const double &dem,
            const double &dem2,
            const double &servt,
            const double &e_rate,
            const double &parkt_arrive,
            const double &parkt_open,
            const double &depotservt,
            const double &ready_fs,
            const double &due_fs,
            const string &requestType,
            const bool   &deny_unass,
            const string &changecust):
    // Customer(vector<int> &tieup_customer,
    //         vector<int> &reje_ve,
    //         string &custno,
    //         string &spotid,
    //         int &spotno,
    //         double &y,
    //         double &x,
    //         double &dem,
    //         double &dem2,
    //         double &servt,
    //         double &e_rate,
    //         double &parkt_arrive,
    //         double &parkt_open,
    //         double &depotservt,
    //         double &ready_fs,
    //         double &due_fs,
    //         string &requestType,
    //         string &changecust):
                tieup_customer(tieup_customer),
                reje_ve(reje_ve),
                custno(custno),
                spotid(spotid),
                spotno(spotno),
                y(y),
                x(x),
                dem(dem),
                dem2(dem2),
                servt(servt),
                e_rate(e_rate),
                parkt_arrive(parkt_arrive),
                parkt_open(parkt_open),
                depotservt(depotservt),
                ready_fs(ready_fs),
                due_fs(due_fs),
                requestType(requestType),
                deny_unass(deny_unass),
                changecust(changecust)
                {}
};

#endif