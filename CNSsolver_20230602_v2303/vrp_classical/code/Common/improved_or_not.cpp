#include "improved_or_not.hpp"

/**
 * @brief 改善の有無を判定する関数
 * @param[in] atpen 改善後の時間違反量
 * @param[in] btpen 改善前の時間違反量
 * @param[in] adist 改善後の総走行距離
 * @param[in] bdist 改善前の総走行距離
 * @param[in] aload 改善後の積載量違反量
 * @param[in] bload 改善前の積載量違反量
 * @param[in] aload2 改善後の第2積載量違反量
 * @param[in] bload2 改善前の第2積載量違反量
 * @param[in] atime 改善後の時間平準化違反量
 * @param[in] btime 改善前の時間平準化違反量
 * @param[in] avisits 改善後の訪問件数平準化違反量
 * @param[in] bvisits 改善前の訪問件数平準化違反量
 * @param[in] adist_error 改善後の距離平準化違反量
 * @param[in] bdist_error 改善前の距離平準化違反量
 * @param[in] aservice_time 改善後の作業時間
 * @param[in] bservice_time 改善前の作業時間
 * @param[in] apower 改善後の総消費電力
 * @param[in] bpower 改善前の総消費電力
 * @param[in] aexp 改善後の総経験コスト
 * @param[in] bexp 改善前の総経験コスト
 * @param[in] aunass 改善後の未割り当て荷物配送先数
 * @param[in] bunass 改善前の未割り当て荷物配送先数
 * @param[in] cdiff トータルペナルティ
 * @param[in] argvs グローバル変数格納用
 * @return bool true : 改善あり、false : 改善なし
 * @details 各種ペナルティやコストからトータルペナルティを計算し、改善の有無を判定する関数。
 */
bool improved_or_not(const double atpen, const double btpen, const double adist, const double bdist, const double aload, const double bload, const double aload2, const double bload2, const double atime, const double btime, const double avisits, const double bvisits, const double adist_error, const double bdist_error, const double aservice_time, const double bservice_time, const double apower, const double bpower, const double aexp, const double bexp, const double aunass, const double bunass, double &cdiff, const Cppargs &argvs){
    double tp_weight = argvs.timePenaltyWeight; //時間違反量の重み
    double lp_weight = argvs.loadPenaltyWeight; //積載量違反量の重み
    double l2p_weight = argvs.load2PenaltyWeight; //第2積載量違反量の重み
    double tlp_weight = argvs.timeLevelingPenaltyWeight; //時間平準化違反量の重み
    double vlp_weight = argvs.visitsLevelingPenaltyWeight; //時間平準化違反量の重み
    double dlp_weight = argvs.distLevelingPenaltyWeight; //距離平準化違反量の重み
	  double sp_weight = argvs.sparePenaltyWeight; //未割り当て荷物最適化重み
    double max_weight = max({tp_weight, lp_weight, l2p_weight, tlp_weight, vlp_weight, dlp_weight, sp_weight});
    if(argvs.timePenaltyType == "hard"){
		tp_weight = max(max_weight, 1e12);
    }
    if(argvs.loadPenaltyType == "hard"){
      lp_weight = max(max_weight, 1e12);
    }
    if(argvs.load2PenaltyType == "hard"){
      l2p_weight = max(max_weight, 1e12);
    }
    if(argvs.timeLevelingPenaltyType == "hard"){
      tlp_weight = max(max_weight, 1e12);
    }
    if(argvs.visitsLevelingPenaltyType == "hard"){
      vlp_weight = max(max_weight, 1e12);
    }
    if(argvs.distLevelingPenaltyType == "hard"){
      dlp_weight = max(max_weight, 1e12);
    }
    bool improved = false;

    cdiff = tp_weight*round((atpen-btpen)*100000)/100000
            +lp_weight*round((aload-bload)*1000000000)/1000000000
            +l2p_weight*round((aload2-bload2)*1000000000)/1000000000
            +argvs.opt_t_weight*round((aservice_time-bservice_time)*100000)/100000
            +argvs.opt_d_weight*(adist-bdist)
            +argvs.opt_e_weight*(apower-bpower)
            +argvs.opt_x_weight*(aexp-bexp)
            +tlp_weight*round((atime-btime)*100000)/100000
            +vlp_weight*(avisits-bvisits)
            +dlp_weight*(adist_error-bdist_error)
            +sp_weight*(aunass-bunass);
    
    if(cdiff < -1e-3){
        improved = true;
    }

    return improved;
}