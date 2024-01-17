//
// Created by alxdsptr on 2024/1/17.
//

#ifndef UPLOAD_BOARD_H
#define UPLOAD_BOARD_H

#define CORNER 0x8100000000000081
#define SIDE_ 0x3C0081818181003C
#define SEC 0x3C424242423C00
#define LEFT_MASK 0xFEFEFEFEFEFEFEFE
#define RIGHT_MASK 0x7F7F7F7F7F7F7F7F
#define MID_MASK 0x7E7E7E7E7E7E7E7E
#define EDGE_MASK 0xFF818181818181FF

typedef unsigned long long ull;

class chess_board{

public:
    ull board[2]; //0是黑 1是白
    int side = 0;
    chess_board(){
        board[0] = 0x810000000;
        board[1] = 0x1008000000;
    }
    void clear(){
        board[0] = 0x810000000;
        board[1] = 0x1008000000;
        side = 0;
    }
#define FIND_HELPER(orien, d, opp) \
            flip = (me orien d) & opp; \
            flip |= (flip orien d) & opp; \
            adjacent = opp & (opp orien d); \
            flip |= (flip orien (d+d)) & adjacent; \
            flip |= (flip orien (d+d)) & adjacent; \
            moves |= flip orien d
    ull find_available(){
        ull me = board[side], opp = board[!side];
        ull flip, adjacent, inner, moves = 0; // adjacent存储了相邻位置有棋子的位置
        inner = opp & MID_MASK; // inner存储了中间6列的棋子 因为在计算左右的flip的时候，
        // 显然最边上的不可能被翻转，所以这些位置一定是false 如果这些位置是true的话，
        // 因为最后计算可能的move是通过将flip左移或者右移，那么就会跑到上一行去，这是不可接受的
        FIND_HELPER(>>, 1, inner); FIND_HELPER(<<, 1, inner); //右， 左
        FIND_HELPER(>>, 8, opp); FIND_HELPER(<<, 8, opp); //下， 上
        FIND_HELPER(>>, 7, inner); FIND_HELPER(<<, 7, inner);// 左下 右上
        FIND_HELPER(>>, 9, inner); FIND_HELPER(<<, 9, inner);// 右下 左上
        return moves & ~(me | opp); //已经有子的地方不能落子
    }

    //潜在行动力 即对方棋子旁边空白格子的数量
    //当然也有其他算法 比如说对方紧邻空白的棋子个数 这个就看自己觉得哪个好用了
    int potential_mobility(int color){
        // finish it yourself
    }

    //这个函数的思路和find_available极其类似 不再赘述
#define PLAY_HELPER(orien, d, opp) \
            temp = (pos orien d) & opp; \
            temp |= (temp orien d) & opp; \
            adjacent = opp & (opp orien d); \
            temp |= (temp orien (d+d)) & adjacent; \
            temp |= (temp orien (d+d)) & adjacent; \
            if(temp orien d & me) flip |= temp
    void play(ull pos){
        ull me = board[side], opp = board[!side];
        ull flip = 0, adjacent, inner, temp;
        inner = opp & MID_MASK;
        PLAY_HELPER(>>, 1, inner); PLAY_HELPER(<<, 1, inner); //右， 左
        PLAY_HELPER(>>, 8, opp); PLAY_HELPER(<<, 8, opp); //下， 上
        PLAY_HELPER(>>, 7, inner); PLAY_HELPER(<<, 7, inner);// 左下 右上
        PLAY_HELPER(>>, 9, inner); PLAY_HELPER(<<, 9, inner);// 右下 左上
        board[side] ^= pos;
        board[side] ^= flip, board[!side] ^= flip;
        side = !side;
    }

    //半稳定子 我的算法是在边上且两边都是对方棋子的子（楔入） 或者四周都是对方棋子的子
    int semi_stable_cnt(int color){
        ull me = board[color], opp = board[!color];
        ull edge = me & EDGE_MASK;
        ull left = opp & LEFT_MASK;
        ull right = opp & LEFT_MASK;
        ull ss = ((left >> 1) & edge & (right <<1)) | ((opp >> 8) & edge & (opp << 8));
        ss |= (opp >> 8) & me & (opp << 8) & (left >> 1) & (right << 1);
        return __builtin_popcountll(ss);
    }

//稳定子 计算的是角落旁边的一坨 这个函数应该不太严谨 但是也够用了
    int stable_cnt(int color){
        //finish it yourself
    }
    void back(ull &b, ull &w){
        board[0] = b, board[1] = w;
        side = !side;
    }
    void skip_round(){
        side = !side;
    }

    //角落情况的计算 包括占角、星位、C位，以及一些奇怪的情况，比如说角旁边插入了一颗对方棋子或者一个空白 当然或许也没必要考虑 这个也见仁见智了
    int compute_corner(int color, bool end){
        // finish it yourself
    }

    //这是一个用于计算不稳定子的宏 这里的不稳定子指的是下一个回合就会被对方翻转的子
    //由于在计算不稳定子的过程中顺便也会得到可行位置 所以出于性能考虑我把计算不稳定子的部分放在了compute_score函数里面 当然这是一种很不好的抽象 但是性能和易读、清晰之间总要有取舍
#define UNSTABLE_HELPER(d, me, opp, ava, us) \
            adj1 = opp & (opp >> d); \
            adj2 = opp & (opp << d); \
            flip = (me >> d) & opp; \
            flip |= (flip >> d) & opp; \
            flip |= (flip >> (d+d)) & adj1; \
            flip |= (flip >> (d+d)) & adj1; \
            temp = (flip >> d) & empty; \
            ava |= temp;         \
            flip = (temp << d) & opp; \
            flip |= (flip << d) & opp; \
            flip |= (flip << (d+d)) & adj2; \
            flip |= (flip << (d+d)) & adj2; \
            us |= flip;         \
            flip = (me << d) & opp; \
            flip |= (flip << d) & opp; \
            flip |= (flip << (d+d)) & adj2; \
            flip |= (flip << (d+d)) & adj2; \
            temp = (flip << d) & empty; \
            ava |= temp;         \
            flip = (temp >> d) & opp; \
            flip |= (flip >> d) & opp; \
            flip |= (flip >> (d+d)) & adj1; \
            flip |= (flip >> (d+d)) & adj1; \
            us |= flip

    //这个函数里把之前计算出来的角落情况、行动力、潜在行动力、稳定子等等根据某个权重加在一起
    //权重应该根据局面的进行有所变化，比如说一开始棋子数量完全不应该作为判断依据，而在终局的时候棋子数量逐渐变得重要起来（当然不到最后一步仍然不起决定性的作用）
    //一开始行动力或许比较重要，中局之后稳定子逐渐重要起来（在此之前也基本不会出现稳定子）
    //具体的权重需要你自己去设计
    int compute_score(int color, int step){
        ull cur = board[color], opo = board[!color], empty = ~(cur | opo);
        int disc, mobility, a, b, potential_mob, cor, pos, stability;
        int res;
        // finish it yourself~
        if(step < 30){
            res = 1 * cor + mobility * 1 + potential_mob * 1 + stability * 1 + pos * 1;
        }else if(step < 60){
            res = 1 * cor + mobility * 1 + potential_mob * 1 + stability * 1 + pos * 1;
        }
        else{
            res = 1 * cor + mobility * 1 + potential_mob * 1 + stability * 1 + pos * 1;
        } //权重的选择是很玄学的事情 或许可以使用一些统计学手段来拟合 但是我没有想到很好的办法
        return res;
    }
    int cnt(int s){
        return __builtin_popcountll(board[s]);
    }
};

#endif //UPLOAD_BOARD_H
