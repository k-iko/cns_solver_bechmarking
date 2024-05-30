#include "compare_time.hpp"

/**
 * @brief 時間の大小を比較する関数
 * @param[in] a 比較する変数1
 * @param[in] b 比較する変数2
 * @param[in] eps 許容誤差
 * @return int a > bなら1、a < bなら-1、a = bなら0
 * @details 許容誤差込みで2つの時間の大小を比較し結果に応じて1、-1、0のいずれかを返す関数。
 */
int compare_time(const double a, const double b, const double eps){
    int rc;
    
    if(a-b > eps){ //a > bの場合
        rc = 1;
    }
    else if(b-a > eps){ //b > aの場合
        rc=-1;
    }
    else{ //a = bの場合
        rc=0;
    }

    return rc;
}