//
// Created by alxdsptr on 2024/1/17.
//

#ifndef UPLOAD_AI_H
#define UPLOAD_AI_H
#include <algorithm>
#include <cstring>
#include <ctime>
#include "board.h"

struct node{
    ull pos;
    int index, score;
};
bool cmp(node a, node b){
    return a.score > b.score;
}
int convert(ull pos){
    //clz是一个返回前导0的数量的函数
    int index = __builtin_clzll(pos);
    return index;
}

class ai_player {
public:
    int start_time, step;
    int depth_cnt[30];
    bool time_out = false;
    bool no_avai = false;
    ull ans;
    node s[30];
    ai_player() {
        memset(depth_cnt, 0, sizeof(depth_cnt));
    }
    int depth;
    int negamax(chess_board &b, int h, int alpha, int beta) {
        depth_cnt[h]++;
        if (h == depth) {
            int temp = b.compute_score(b.side, step + h);
            return temp;
        }
        ull avai = b.find_available(), pos;
        ull black = b.board[0], white = b.board[1];
        if (!avai) {
            if(no_avai){
                //如果双方都无子可下，那么游戏已经结束，返回我方比对方多的子数*1000
                no_avai = false;
                return (b.cnt(b.side) - b.cnt(!b.side)) * 1000;
            }
            no_avai = true;
            b.skip_round();
            int value;
            //如果我方无子可下，那么就跳过我方回合，直接返回负的对方得分
            value = -negamax(b, h, -beta, -alpha);
            b.skip_round();
            return value;
        }
        no_avai = false;
        //我的find_available()函数返回的是一个64位整数，某个bit是1表示那个位置可以下棋
        //这样的话avai & (-avai) （对unsigned取负也相当于按位取反再+1） 就能够得到最小的那个是1的bit 也就是最靠右下角的可行位置
        //之后再avai ^= pos就是把这个bit设为0 然后再取avai & (-avai)就是取下一个最小的为1的bit
        for (pos = avai & (-avai); avai; avai ^= pos, pos = avai & (-avai)) {
            int temp;
            b.play(pos);
            temp = -negamax(b, h + 1, -beta, -alpha);
            b.back(black, white);
            if(time_out){
                return 0;
            }
            if (temp > alpha) {
                alpha = temp;
                if (temp >= beta) {
                    break;
                }
            }
        }
        return alpha;
    }
    int cnt = 0;
    //终局搜索就是完全不剪枝，直接搜到底
    ull end_game(chess_board &b) {
        ull pos, available;
        int max_score = -1e9 - 1;
        start_time = clock();
        ull black = b.board[0], white = b.board[1], res;
        depth = __builtin_popcountll(~(black | white));
        available = b.find_available();
        if (!available) {
            return 0;
        }
        for (pos = available & (~available + 1), res = pos; available; available ^= pos, pos = available &
                                                                                               (-available)) {
            b.play(pos);
            int temp = -negamax(b, 1, -1e9, -max_score + 1);
            b.back(black, white);
            if (temp > max_score) {
                max_score = temp, res = pos;
            }
            cnt++;
        }
        return res;
    }
    //迭代加深搜索
    ull iterative_deepening(chess_board &b) {
        ull pos, available;
        start_time = clock();
        available = b.find_available();
        if (!available) {
            return 0;
        }
        depth = 6;
        int max_score = -1e9 - 1, max_depth;
        ull black = b.board[0], white = b.board[1], res;
        max_depth = __builtin_popcountll(~(black | white));
        cnt = 0;
        //先使用6层进行浅层试探搜索 把结果存在结构体数组s里面 包括这一步的位置、得分以及位置转换成一个1-64之间的整数（index） index主要是输出debug用 删掉也没关系
        for (pos = available & (~available + 1), res = pos; available; available ^= pos, pos = available &
                                                                                               (-available)) {
            b.play(pos);
            int temp = -negamax(b, 1, -INT_MAX, INT_MAX);
            b.back(black, white);
            int index = convert(pos);
            s[cnt].index = index;
            s[cnt].score = temp;
            s[cnt].pos = pos;
            if (temp > max_score) {
                max_score = temp, res = pos;
            }
            cnt++;
        }
        //对试探搜索的结果进行排序 排序的目的是为了能剪更多枝
        std::sort(s, s + cnt, cmp);
        ull temp_res;
        //然后从7层开始进行迭代加深搜索
        for(depth = 7; depth <= max_depth; depth++) {
            pos = s[0].pos;
            temp_res = pos;
            b.play(pos);
            max_score = -negamax(b, 1, -1e9, 1e9);
            b.back(black, white);
            s[0].score = max_score;
            //首先搜索排名第一的步 得到一个临时的max_score
            for(int i = 1; i < cnt; i++){
                pos = s[i].pos;
                b.play(pos);
                int temp;
                //之后的步先进行一个零宽窗口搜索
                temp = -negamax(b, 1, -max_score - 1, -max_score);
                //如果零宽搜索的结果说明这一步有>max_score的潜力再进行正式搜索
                if(temp > max_score){
                    temp = -negamax(b, 1, -1e9, -max_score);
                }
                b.back(black, white);
                if(time_out){
                    break;
                }
                s[i].score = temp;
                if(temp > max_score){
                    max_score = temp, temp_res = pos;
                }
            }
            if(!time_out){
                res = temp_res;
            }else{
                break;
            }
            if(clock() - start_time >= CLOCKS_PER_SEC / 1000 * 500){
                break;
            }
        }
        ans = res;
        return res;
    }
};
#endif //UPLOAD_AI_H
