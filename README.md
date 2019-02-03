# This is the project to do the AI of Threes!

The game link:
http://threesjs.com/

![alt text](https://imgur.com/ThmKXas.png)

AI training Simulation:
![alt text](https://imgur.com/bFCAcbw.png)

---------------------------------------------------
### Project1 - The environment of Threes!

* To Build the Threes Environment to simulate the game.
* With heuristic Player agent and Random evil environment.
---------------------------------------------------
### Project2 - TD Learning player

* Player is trained by TD Learning (backward update), updating the after state.
---------------------------------------------------
### Project3 - Minimax Search
* To simulate the minimax Search.
* Because the original board game-space complexity is too big, so the board is simplified to only six tile.
* The result shows up less than 1 sec.
(0) (1) (2)
(3) (4) (5)
---------------------------------------------------
### Project4 - The bonus tile added to the rule (the true rule)

* There is the bonus tile rule in the actual Threes! rule.
* Also, tell the player the next hint (1~11)
* Be careful, the weight table including the hint tile is very large ~ 2GB, so the I/O of weights is slow
---------------------------------------------------
### Project5 - The evil environment and the arena API

* The environment will find the worst step to let the player die asap.
* With 2-layer minimax tree search.

* Modify the next hint only 1~4 (4 is bonus tile)
* Also, build the arena provided by TA, so students can pk on the arena with their agents

===================================================

# 這是一個玩Threes!遊戲的AI程序

遊戲連結:
http://threesjs.com/

![alt text](https://imgur.com/ThmKXas.png)

AI訓練中的模擬結果:
![alt text](https://imgur.com/bFCAcbw.png)

---------------------------------------------------
### 專案一 - 建立Threes!的遊戲環境

* 建立模擬Threes!遊戲的環境
* 玩家用了一點小聰明讓成績好一點，不過跟隨機亂下差不了多少
* 環境(產生棋子)則是隨便亂放
---------------------------------------------------
### 專案二 - 玩家用TD Learning學習，成為了一個厲害的玩家

* 用TD Learning訓練玩家，用backward的方式更新after state
---------------------------------------------------
### 專案三 - Minimax Search

* 模擬Miminax 搜尋
* 因為原本棋盤有16格，空間時間複雜度太大，因此將棋盤縮小成6格
* 不到1秒就可跑出來結果
(0) (1) (2)
(3) (4) (5)
---------------------------------------------------
### 專案四 - 將bonus 棋子加進規則中(符合真實情況)

* 真實的Threes!有一項規則是當你棋子最大超過48時，有機會產生超過3的棋子
* 這使得玩家更難玩遊戲了
* 此專案同時讓玩家能夠知道下一個棋子(1~11)會是多少
* 注意！weight table因為包含了hint，所以檔案變很大將近2GB，所以I/O很慢
---------------------------------------------------
### 專案五 - 惡意環境加上競技場接口

* 現在環境變得很惡意，它會讓玩家盡可能的死掉
* 用兩層的minimax tree search來實現

* 修改對玩家的提示1~4(4是bonus棋子)
* 同時，實現了助教所提供的競技場的接口，使程序能與其他同學的程序比賽對打
