//
// Created by alxdsptr on 2024/1/17.
//
#include "ai.h"
#include "board.h"
#include <iostream>
#include <thread>

using namespace std;

int main(){
    int n, i, x, y, cnt = 4;
    chess_board b;
    ai_player ai;
    cin >> n;
    cin >> y >> x;
    if(x != -1){
        ull mask = 1ull << (63 - x * 8 - y);
        b.play(mask);
        cnt++;
    }
    for(i = 1; i < n; i++){
        cin >> y >> x;
        if(x != -1){
            ull mask = 1ull << (63 - x * 8 - y);
            b.play(mask);
            cnt++;
        }else{
            b.skip_round();
        }
        cin >> y >> x;
        if(x != -1){
            ull mask = 1ull << (63 - x * 8 - y);
            b.play(mask);
            cnt++;
        }else{
            b.skip_round();
        }
    }
    ai.step = cnt;
    ull pos;
    if(cnt >= 54){
        //如果剩余少于8步就直接搜到底不剪枝
        pos = ai.end_game(b);
    }else{
        //这句话的意思就是单开一个线程来运行iterative_deepening这个函数 但是由于要用到对象的引用 所以要用一个Lambda函数封装一下
        thread search([&ai, &b]() {
            ai.iterative_deepening(b);
        });
        //进行一个时的卡
        this_thread::sleep_for(chrono::milliseconds(950));
        //time_out被设置成true之后自然就会停下了
        ai.time_out = true;
        //终止这个进程
        search.join();
        pos = ai.ans;
    }
    if(!pos){
        cout << "-1 -1" << endl;
    }
    else{
        int index = __builtin_clzll(pos);
        x = index % 8;
        y = index / 8;
        cout << x << ' ' << y << endl;
//        cout << "depth: " << ai.depth << endl;
        for(i = 0; i < ai.cnt; i++){
            index = ai.s[i].index;
            x = index % 8;
            y = index / 8;
            char x_ = x + 'a', y_ = y + '1';
            printf("%c%c: %d ", x_, y_, ai.s[i].score);
        }
//        cout << endl;
        cout << clock() - ai.start_time << ' ' << ai.depth << ' ' << ai.time_out << ' ' << endl;
    }
}
