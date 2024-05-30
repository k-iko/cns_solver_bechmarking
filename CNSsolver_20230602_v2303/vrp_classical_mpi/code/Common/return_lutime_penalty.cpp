#include "return_lutime_penalty.hpp"

/**
 * @brief 営業時間の上下限違反量を計算する関数
 * @param[in] aservice_time 改善後の作業時間
 * @param[in] bservice_time 改善前の作業時間
 * @param[in] lowertime 下限時間
 * @param[in] uppertime 上限時間
 * @param[in] atpen 改善後の上下限違反量
 * @param[in] btpen 改善前の上下限違反量
 * @details 改善前後の営業時間の上下限違反量を計算しbtpen、atpenにそれぞれ加算する関数。
 */
void return_lutime_penalty(const double aservice_time, const double bservice_time, const double lowertime, const double uppertime, double &atpen, double &btpen){
    if(compare_time(lowertime) > 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, lowertime) < 0
    && compare_time(aservice_time, bservice_time) < 0){ 
        //下限以下だった作業時間が改善によって更に減少した場合
        atpen += bservice_time-aservice_time;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, lowertime) < 0
    && compare_time(aservice_time, uppertime) > 0){
        //下限以下だった作業時間が改善によって上限以上になった場合
        atpen += aservice_time-uppertime;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, lowertime) < 0
    && compare_time(aservice_time, lowertime) > 0
    && compare_time(aservice_time, uppertime) < 0){
        //下限以下だった作業時間が改善によって下限以上上限以下になった場合
        btpen += lowertime-bservice_time;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, lowertime) < 0
    && compare_time(aservice_time, lowertime) < 0
    && compare_time(aservice_time, bservice_time) > 0
    ){
        //下限以下だった作業時間が改善によって違反量が減少した場合
        btpen += aservice_time-bservice_time;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, lowertime) >= 0
    && compare_time(bservice_time, uppertime) <= 0
    && compare_time(aservice_time, lowertime) < 0
    ){
        //下限以上上限以下だった作業時間が改善によって下限以下になった場合
        atpen += lowertime-aservice_time;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, lowertime) >= 0
    && compare_time(bservice_time, uppertime) <= 0
    && compare_time(aservice_time, uppertime) > 0
    ){
        //下限以上上限以下だった作業時間が改善によって上限以上になった場合
        atpen += aservice_time-uppertime;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, uppertime) > 0
    && compare_time(aservice_time, lowertime) < 0
    ){
        //上限以上だった作業時間が改善によって下限以下になった場合
        atpen += lowertime-aservice_time;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, uppertime) > 0
    && compare_time(bservice_time, aservice_time) < 0
    ){
        //上限以上だった作業時間が改善によって更に増加した場合
        atpen += aservice_time-bservice_time;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, uppertime) > 0
    && compare_time(aservice_time, uppertime) <= 0
    && compare_time(aservice_time, lowertime) >= 0
    ){
        //上限以上だった作業時間が改善によって下限以上上限以下になった場合
        btpen += bservice_time-uppertime;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, uppertime) > 0
    && compare_time(aservice_time, uppertime) > 0
    && compare_time(aservice_time, bservice_time) < 0
    ){
        //上限以上だった作業時間が改善によって違反量が減少した場合
        btpen += bservice_time-aservice_time;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) == 0 
    && compare_time(bservice_time, lowertime) < 0
    && compare_time(aservice_time, bservice_time) < 0
    ){
        //下限以下だった作業時間が改善によって更に悪化した場合(上限なし)
        atpen += bservice_time-aservice_time;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) == 0 
    && compare_time(bservice_time, lowertime) >= 0
    && compare_time(aservice_time, lowertime) < 0
    ){
        //下限以上だった作業時間が改善によって下限以下になった場合(上限なし)
        atpen += lowertime-aservice_time;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) == 0 
    && compare_time(bservice_time, lowertime) < 0
    && compare_time(aservice_time, lowertime) < 0
    && compare_time(aservice_time, bservice_time) > 0
    ){
        //下限以下だった作業時間が改善によって違反量が減少した場合(上限なし)
        btpen += aservice_time-bservice_time;
    }
    else if(compare_time(lowertime) > 0 
    && compare_time(uppertime) == 0 
    && compare_time(bservice_time, lowertime) < 0
    && compare_time(aservice_time, lowertime) >= 0
    ){
        //下限以下だった作業時間が改善によって下限以上になった場合(上限なし)
        btpen += lowertime-bservice_time;
    }
    else if(compare_time(lowertime) == 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, uppertime) > 0
    && compare_time(aservice_time, bservice_time) > 0
    ){
        //上限以上だった作業時間が改善によって更に悪化した場合(下限なし)
        atpen += aservice_time-bservice_time;
    }
    else if(compare_time(lowertime) == 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, uppertime) <= 0
    && compare_time(aservice_time, uppertime) > 0
    ){
        //上限以下だった作業時間が改善によって上限以上になった場合(下限なし)
        atpen += aservice_time-uppertime;
    }
    else if(compare_time(lowertime) == 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, uppertime) > 0
    && compare_time(aservice_time, uppertime) > 0
    && compare_time(aservice_time, bservice_time) < 0
    ){
        //上限以上だった作業時間が改善によって違反量が減少した場合(下限なし)
        btpen += bservice_time-aservice_time;
    }
    else if(compare_time(lowertime) == 0 
    && compare_time(uppertime) > 0 
    && compare_time(bservice_time, uppertime) > 0
    && compare_time(aservice_time, uppertime) <= 0
    ){
        //上限以上だった作業時間が改善によって上限以下になった場合(下限なし)
        btpen += bservice_time-uppertime;
    }
}