# include <Siv3D.hpp> // OpenSiv3D v0.6.6



// 固定値
int32 defaltLifespan = 60;
int32 defaltCooltime = 3;
int32 defaltX = 160;
int32 defaltY = 100;
int32 defaltW = 200;
int32 defaltH = 400;
int32 defaltR = 20;
int32 GravPoint = 0;		// GPが足りないとGravを使えない
int32 GravNeedPoint = 10;	// GPがこの値より大きいならGrav使用可能
Array<int32> order = { 0, 1, 2, 3, 4, 5, 6 };
/*
						,,,,	,,,,
	..#	#..	.#.	##.	.##	####	.##.
	###	###	###	.##	##. ....	.##.
	...	... ...	...	... ....	....

*/
// 橙 青 紫 緑 赤 青 黄色
// BlockColorにブロックの色をしまう
Array<Color> blockColor = {
	Palette::Blue,
	Palette::Orange,
	Palette::Darkviolet,
	Palette::Green,
	Palette::Red,
	Palette::Cyan,
	Palette::Yellow
};
Array<Grid<int32>> blockShape = {
	{{0, 0, 1}, {1, 1, 1}, {0, 0, 0}},							/* L */
	{{1, 0, 0}, {1, 1, 1}, {0, 0, 0}},							/* J */
	{{0, 1, 0}, {1, 1, 1}, {0, 0, 0}},							/* T */
	{{1, 1, 0}, {0, 1, 1}, {0, 0, 0}},							/* S */
	{{0, 1, 1}, {1, 1, 0}, {0, 0, 0}},							/* Z */
	{{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0} },	/* I */
	{{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0} }	/* O */
};
// Blocks に 配置済みのブロックの情報を入れる
Grid<int32> Blocks(22, 10, -1);
// NextQに次のミノを入れる
std::deque<int32> NextQ;
int32 score = 0;
int32 lines = 0;
double bgmVolume = 0.2;
double seVolume = 0.2;

int32 putBlock(int32 x, int32 y, Color c) // (x, y) に c 色のブロックを配置
{
	Rect{ defaltX + defaltR * x, defaltY + defaltR * y, defaltR }.draw(c);
	Rect{ defaltX + defaltR * x, defaltY + defaltR * y, defaltR }.drawFrame(1, 0, Palette::Gray);
	return 0;
}
int32 putFlame(int32 x, int32 y, Color c) // (x, y) に c 色のブロックの枠を配置
{
	Rect{ defaltX + defaltR * x, defaltY + defaltR * y, defaltR }.drawFrame(2, 0, c);
	return 0;
}
int32 nextReplesh() // Qを埋める
{
	order.shuffle();
	if (NextQ.size() <= 7)
	{
		for (int32 i = 0; i < 7; i++)
			NextQ.push_back(order[i]); // 乱数にするのを忘れない！
	}
	return 0;
}
Grid<int32> rotate(Grid<int32> G, int32 d) // d方向に |d|回 回転させる
{
	Grid<int32> newG = G;
	int32 n = G.height();
	while (d > 0)
	{
		for (int32 i = 0; i < n; i++)
		{
			for (int32 j = 0; j < n; j++)
			{
				newG[i][j] = G[n - 1 - j][i];
			}
		}
		d--;
		G = newG;
	}
	while (d < 0)
	{
		for (int32 i = 0; i < n; i++)
		{
			for (int32 j = 0; j < n; j++)
			{
				newG[i][j] = G[j][n - 1 - i];
			}
		}
		d++;
		G = newG;
	}
	return newG;
}
int32 putNext(int32 x, int32 y, int32 type) // nextを表示する
{
	Grid<int32> G = rotate(blockShape[type], -1);
	for (int32 i = 0; i < G.height(); i++)
	{
		for (int32 j = 0; j < G.width(); j++)
		{
			if (G[i][j] == 1)
				putBlock(x + i, y + j, blockColor[type]);
		}
	}
	return 0;
}
int32 eraseBlock(int32 n) // n列目を消す
{
	for (int32 i = 0; i < 10; i++)
	{
		Blocks[i][n] = -1;
	}
	lines++;		// 消去ライン数を増やす
	GravPoint++;	// GPを加算する
	return 0;
}
int32 swapBlock(int32 n) // n列目と n-1列目を入れ替える
{
	for (int32 i = 0; i < 10; i++)
	{
		int32 tmp = Blocks[i][n];
		Blocks[i][n] = Blocks[i][n - 1];
		Blocks[i][n - 1] = tmp;
	}
	return 0;
}
int32 findErase() // 削除可能な列を探す
{
	int32 n = 0;
	for (int32 i = 0; i < 20; i++)
	{
		bool flag = true;
		for (int32 j = 0; j < 10; j++)
		{
			if (Blocks[j][i] == -1)
				flag = false;
		}
		if (flag)
		{
			eraseBlock(i); // 削除を行う
			for (int32 k = i; k > 0; k--)
			{
				swapBlock(k);
			}
			n++;
		}
	}
	if (n != 0)
		return n; // 削除した列数
	return 0;
}
int32 Grav()
{
	// ブロックが落ちる
	// ポイントが溜まってないと呼び出せないようにしたい
	if (GravPoint >= GravNeedPoint)
	{
		for (int32 i = 20; i > 0; i--)
		{
			for (int32 j = 0; j < 10; j++)
			{
				if (Blocks[j][i] != -1)
				{
					int32 x = j;
					int32 y = i;
					while (y <= 18 && Blocks[x][y + 1] == -1) // 下側が空きマスか確認する
					{
						Blocks[x][y + 1] = Blocks[x][y];
						Blocks[x][y] = -1;
						y++;
						// 下にブロックを移動する
					}
				}
			}
		}
		return 0;
	}
	return 1;
}
class Mino {
private:
	Vec2 Pos;					// ミノの座標
	int32 dire;					// ミノの方向
	int32 type;					// ミノの種類
	int32 lifespan;				// 落下までの残り時間
	Array<int32> cooltime;		// 入力受付
	int32 cooltimeA;			// 入力受付
	int32 cooltimeD;			// 入力受付
	int32 cooltimeQ;			// 入力受付
	int32 cooltimeE;			// 入力受付
	int32 hold;					// holdしているミノの種類
	bool holdable;				// hold可能か
	bool gameover;				// gameoverフラグ
	Vec2 ghostPos;				// ghostの座標
public:
	int32 rRotate();								// ミノの右回転
	int32 lRotate();								// ミノの左回転
	int32 fallMino();								// ミノの落下
	int32 dx(int32 x);								// ミノを右に動かす
	int32 dy(int32 y);								// ミノを下に動かす
	int32 dxdydt(int32 x, int32 y, int32 theta);	// ミノを動かす 上下左右回転
	int32 lock();									// ミノを設置済みにする
	int32 goNext();									// 次のミノに乗り換える
	int32 putMino();								// ミノを表示する
	int32 holdChange();								// holdする
	int32 putHold(int32 x, int32 y);				// holdを表示する
	bool checkGameover();							// ゲームオーバー判定を行う
	bool checkOverlap(int32 T, int32 X, int32 Y, int32 D);	// 重なりと範囲外を確認
	int32 reset();
	int32 checkGhost();								// ghostの位置を確認する
	int32 putGhost();								// ghostを表示する
	int32 consCooltime();							// cooltimeを消費する
	/* getter */
	int32 getX();				// ミノのx座標
	int32 getY();				// ミノのy座標
	int32 getDire();			// ミノの方向
	int32 getLifespan();		// ミノの落下までの寿命
	int32 getType();			// ミノの種類
	int32 getCooltime(char c);	// 受付時間
	Grid<int32> getShape();		// ミノの形
	int32 getHold();			// hold
	bool getHoldable();			// holdable
	bool getGameover();			// gameover
	int32 getGhostX();			// ghostのx座標
	int32 getGhostY();			// ghostのy座標
	/* setter */
	int32 setX(int32 x);
	int32 setY(int32 y);
	int32 setDire(int32 d);
	int32 setLifespan(int32 t);				// ミノの落下までの寿命
	int32 setType(int32 t);					// ミノの種類
	int32 setCooltime(char c, int32 t);		// 受付時間
	int32 setHold(int32 t);					// hold
	int32 setHoldable(bool f);				// holdable
	int32 setGameover(bool f);				// gameover
	//int32 setShape(Grid<int32> G);		// ミノの形
	/* コンストラクタ */
	Mino();
};

int32 Mino::rRotate()
{
	int32 nextD = (getDire() - 1 + 4) % 4;
	if (!checkOverlap(getType(), getX(), getY(), nextD)) // 通常回転
	{
		setDire(nextD); // 回転する
		return 0;
	}
	if (getType() != 5) // Iミノ以外
	{
		// 回転後上向き(A) の場合
		if (nextD == 3)
		{
			if (!checkOverlap(getType(), getX() - 1, getY(), nextD)) // 1.左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-1);			// 左に軸をずらす(回転方向の逆)
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() + 1, nextD)) // 2.左下に軸をずらす
			{
				dxdydt(-1, 1, -1);	// 左下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX(), getY() - 2, nextD)) // 3.上上に軸をずらす
			{
				setDire(nextD); // 回転する
				dy(-2);			// 上上に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() - 2, nextD)) // 4.左上上に軸をずらす
			{
				dxdydt(-1, -2, -1);	// 左上上に軸をずらす
				return 0;
			}
		}
		// 左向き(D) の場合
		if (nextD == 0)
		{

			if (!checkOverlap(getType(), getX() + 1, getY(), nextD)) // 1. 右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(1);			// 右に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() - 1, nextD)) // 2.右上に軸をずらす
			{
				dxdydt(1, -1, -1); // 右上に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX(), getY() + 2, nextD)) // 3.下下に軸をずらす
			{
				setDire(nextD); // 回転する
				dy(2);			// 下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() + 2, nextD)) // 4.右下下に軸をずらす
			{
				dxdydt(1, 2, -1);	// 右下下に軸をずらす
				return 0;
			}
		}
		// 下向き(C) の場合
		if (nextD == 1)
		{
			if (!checkOverlap(getType(), getX() + 1, getY(), nextD)) // 1.右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(1);			// 右に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() + 1, nextD)) // 2.右下に軸をずらす
			{
				dxdydt(1, 1, -1);	// 右下に軸をずらす

				return 0;
			}
			if (!checkOverlap(getType(), getX(), getY() - 2, nextD)) // 3.上上に軸をずらす
			{
				setDire(nextD); // 回転する
				dy(-2);			// 下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() - 2, nextD)) // 4.右上上に軸をずらす
			{
				dxdydt(1, -2, -1);	// 右上上に軸をずらす
				return 0;
			}
		}
		// 右向き(B) の場合
		if (nextD == 2)
		{
			if (!checkOverlap(getType(), getX() - 1, getY(), nextD)) // 1.左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-1);			// 左に軸をずらす(回転方向の逆)
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() - 1, nextD)) // 2.左上に軸をずらす
			{
				dxdydt(-1, -1, -1);	// 左上に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX(), getY() + 2, nextD)) // 3.下下に軸をずらす
			{
				setDire(nextD); // 回転する
				dy(2);			// 下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() + 2, nextD)) // 4.左下下に軸をずらす
			{
				dxdydt(-1, 2, -1);	// 左下下に軸をずらす(回転方向の逆)
				return 0;
			}
		}
	}
	else // Iミノ
	{
		// 上向き(A) の場合
		if (nextD == 3)
		{
			if (!checkOverlap(getType(), getX() - 2, getY(), nextD)) // 1. 左左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-2);			// 左に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY(), nextD)) // 2.右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(1);			// 右に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() + 2, nextD)) // 3.右下下に軸をずらす
			{
				dxdydt(1, 2, -1);	// 右下下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 2, getY() - 1, nextD)) // 4. 左左上に軸をずらす
			{
				dxdydt(-1, -2, -1);	// 左左上に軸をずらす
				return 0;
			}
		}
		// 左向き(D) の場合
		if (nextD == 0)
		{
			if (!checkOverlap(getType(), getX() + 2, getY(), nextD)) // 1.右右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(2);			// 右右に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY(), nextD)) // 2.左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-1);			// 左に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 2, getY() - 1, nextD)) // 3.右右上に軸をずらす
			{
				dxdydt(2, -1, -1);	// 右右上に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() + 2, nextD)) // 4.左下下に軸をずらす
			{
				dxdydt(-1, 2, -1);	// 左下下に軸をずらす
				return 0;
			}
		}
		// 下向き(C) の場合
		if (nextD == 1)
		{
			if (!checkOverlap(getType(), getX() - 1, getY(), nextD)) // 1.左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-1);			// 左に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 2, getY(), nextD)) // 2.右右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(2);			// 右右に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() - 2, nextD)) // 3.左上上に軸をずらす
			{
				dxdydt(-1, -2, -1);	// 左上上に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 2, getY() + 1, nextD)) // 4.右右下に軸をずらす
			{
				dxdydt(2, 1, -1);	// 右右下に軸をずらす
				return 0;
			}
		}
		// 右向き(B) の場合
		if (nextD == 2)
		{
			if (!checkOverlap(getType(), getX() - 2, getY(), nextD)) // 1.左左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-2);			// 左左に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY(), nextD)) // 2.右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(1);			// 右に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 2, getY() + 1, nextD)) // 3.左左下に軸をずらす
			{
				dxdydt(-2, 1, -1);	// 左左下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() - 2, nextD)) // 4.右上上に軸をずらす
			{
				dxdydt(1, -2, -1);	// 右上上に軸をずらす
				return 0;
			}
		}
	}
	return 1;	// 回転できない

}
int32 Mino::lRotate()
{
	int32 nextD = (getDire() + 1 + 4) % 4;
	if (!checkOverlap(getType(), getX(), getY(), nextD)) // 通常回転
	{
		setDire(nextD); // 回転する
		return 0;
	}
	if (getType() != 5) // Iミノ以外
	{
		// 上向き(A) の場合
		if (nextD == 3)
		{
			if (!checkOverlap(getType(), getX() + 1, getY(), nextD)) // 1.右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(+1);			// 右に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() + 1, nextD)) // 2.右下に軸をずらす
			{
				dxdydt(1, 1, 1); // 右下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX(), getY() - 2, nextD)) // 3.上上に軸をずらす
			{
				setDire(nextD); // 回転する
				dy(-2);			// 上に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() - 2, nextD)) // 4.右上上に軸をずらす
			{
				dxdydt(1, -2, 1); // 右下下に軸をずらす
				return 0;
			}
		}
		// 左向き(D) の場合
		if (nextD == 0)
		{
			if (!checkOverlap(getType(), getX() + 1, getY(), nextD)) // 1.右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(1);			// 右に軸をずらす(回転方向の逆)
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() - 1, nextD)) // 2.右上に軸をずらす
			{
				dxdydt(1, -1, 1);	// 右上に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX(), getY() + 2, nextD)) // 3.下下に軸をずらす
			{
				setDire(nextD); // 回転する
				dy(2);			// 下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() + 2, nextD)) // 4.右下下に軸をずらす
			{
				dxdydt(1, 2, 1); // 右下下に軸をずらす
				return 0;
			}
		}
		// 下向き(C) の場合
		if (nextD == 1)
		{
			if (!checkOverlap(getType(), getX() - 1, getY(), nextD)) // 1.左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-1);			// 左に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() + 1, nextD)) // 2.左下に軸をずらす
			{
				dxdydt(-1, 1, 1); // 左下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX(), getY() - 2, nextD)) // 3.上上に軸をずらす
			{
				setDire(nextD); // 回転する
				dy(-2);			// 下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() + 2, nextD)) // 4.左上上に軸をずらす
			{
				dxdydt(-1, 2, 1);	// 右下に軸をずらす
				return 0;
			}
		}
		// 右向き(B) の場合
		if (nextD == 2)
		{
			if (!checkOverlap(getType(), getX() - 1, getY(), nextD)) // 1.左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-1);			// 左に軸をずらす(回転方向の逆)
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() - 1, nextD)) // 2.左上に軸をずらす
			{
				dxdydt(-1, -1, 1); // 左上に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX(), getY() + 2, nextD)) // 3.下下に軸をずらす
			{
				setDire(nextD); // 回転する
				dy(2);			// 下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() + 2, nextD)) // 4.左下下に軸をずらす
			{
				dxdydt(-1, 2, 1); // 左下下に軸をずらす
				return 0;
			}
		}
	}
	else // Iミノ
	{
		// 上向き(A) の場合
		if (nextD == 3)
		{
			if (!checkOverlap(getType(), getX() + 2, getY(), nextD)) // 1.右右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(2);			// 右右に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY(), nextD)) // 2.左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-1);			// 左に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 2, getY() - 1, nextD)) // 3.右右上に軸をずらす
			{
				dxdydt(2, -1, 1); // 右右上に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() + 2, nextD)) // 3.左下下に軸をずらす
			{
				dxdydt(-1, 2, 1); // 左下下に軸をずらす
				return 0;
			}
		}
		// 左向き(D) の場合
		if (nextD == 0)
		{
			if (!checkOverlap(getType(), getX() - 1, getY(), nextD)) // 1.左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-1);			// 左に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 2, getY(), nextD)) // 2.右右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(2);			// 右右に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 1, getY() - 21, nextD)) // 3.左上上に軸をずらす
			{
				dxdydt(-1, -2, 1); // 左上上に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 2, getY() + 1, nextD)) // 4.右右下に軸をずらす
			{
				dxdydt(2, 1, 1); // 右右下に軸をずらす
				return 0;
			}
		}
		// 下向き(C) の場合
		if (nextD == 1)
		{
			if (!checkOverlap(getType(), getX() + 1, getY(), nextD)) // 1.右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(1);			// 右に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 2, getY(), nextD)) // 2.左左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-2);			// 左左に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 2, getY() + 1, nextD)) // 3.左左下に軸をずらす
			{
				dxdydt(-2, 1, 1); // 左左下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() - 2, nextD)) // 3.右上上に軸をずらす
			{
				dxdydt(1, -2, 1); // 右上上に軸をずらす
				return 0;
			}
		}
		// 右向き(B) の場合
		if (nextD == 2)
		{
			if (!checkOverlap(getType(), getX() + 1, getY(), nextD)) // 1.右に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(1);			// 右に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 2, getY(), nextD)) // 2.左左に軸をずらす
			{
				setDire(nextD); // 回転する
				dx(-2);			// 左左に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() + 1, getY() + 21, nextD)) // 3.右下下に軸をずらす
			{
				dxdydt(1, 2, 1); // 右下下に軸をずらす
				return 0;
			}
			if (!checkOverlap(getType(), getX() - 2, getY() - 1, nextD)) // 4.左左上に軸をずらす
			{
				dxdydt(-2, -1, 1); // 左左上に軸をずらす
				return 0;
			}
		}
	}
	return 1;	// 回転できない
}
int32 Mino::fallMino()
{
	if (checkOverlap(getType(), getX(), getY() + 1, getDire())) // 落下後の判定をする
	{
		lock();
		return 1;
	}
	else {
		dy(1);		// 落下する
		return 0;
	}
}
int32 Mino::dx(int32 deltaX) // xだけずらす
{
	if (checkOverlap(getType(), getX() + deltaX, getY(), getDire())) // 移動後の判定をする
	{
		return 1;
	}
	else {
		setX(getX() + deltaX); // 移動する
		return 0;
	}
}
int32 Mino::dy(int32 deltaY) // yだけずらす
{
	if (checkOverlap(getType(), getX(), getY() + deltaY, getDire())) // 移動後の判定をする
	{
		return 1;
	}
	else {
		setY(getY() + deltaY); // 移動する
		return 0;
	}
}
int32 Mino::dxdydt(int32 deltaX, int32 deltaY, int32 deltaTheta) // ずらす
{
	if (checkOverlap(getType(), getX() + deltaX, getY() + deltaY, (getDire() + deltaTheta + 4) % 4)) // 移動後の判定をする
	{
		return 1;
	}
	else {
		setX(getX() + deltaX);	// 移動する
		setY(getY() + deltaY);	// 移動する
		setDire((getDire() + deltaTheta + 4) % 4); // 回転する
		return 0;
	}
}
int32 Mino::lock()
{
	Grid<int32> G = getShape();
	for (int32 i = 0; i < G.height(); i++)
	{
		for (int32 j = 0; j < G.width(); j++)
		{
			if (G[i][j] == 1)
				Blocks[getX() + i - 1][getY() + j - 1] = getType(); // 固定する
		}
	}
	goNext();
	return 0;
}
int32 Mino::goNext() // 次のミノへ行く
{
	type = NextQ.front();
	NextQ.pop_front();
	Pos = { 4,1 };
	setHoldable(true); // hold可能
	if (type >= 5)
	{
		Pos = { 4, 0 }; // I Oミノだけずれる
	}
	setDire(-1);
	return 0;
}
int32 Mino::putMino()
{
	Grid<int32> G = getShape();
	for (int32 i = 0; i < G.height(); i++)
	{
		for (int32 j = 0; j < G.width(); j++)
		{
			if (G[i][j] == 1)
				putBlock(getX() + i - 1, getY() + j - 1, blockColor[getType()]);
		}
	}
	return 0;
}
int32 Mino::holdChange() // holdする
{
	if (getHoldable())
	{
		if (getHold() == -1)
		{
			setHold(getType());
		}
		else {
			int32 tmp = getHold();
			setHold(getType());
			NextQ.push_front(tmp);
		}
		goNext();
		setHoldable(false);
		return 0;
	}
	return 1;
}
int32 Mino::putHold(int32 x, int32 y)
{
	Grid<int32> G = rotate(blockShape[getHold()], -1);
	for (int32 i = 0; i < G.height(); i++)
	{
		for (int32 j = 0; j < G.width(); j++)
		{
			if (G[i][j] == 1)
			{
				if (getHoldable())
					putBlock(x + i, y + j, blockColor[getHold()]);
				else
					putBlock(x + i, y + j, Palette::White);
			}
		}
	}
	return 0;
}
bool Mino::checkGameover()
{
	/* 出現時, ミノが重なってたらNG */
	/* 20より上に固定されるという条件は一旦置いておく */
	if (checkOverlap(getType(), getX(), getY(), getDire()))
		return true;
	return false;
}
bool Mino::checkOverlap(int32 T, int32 X, int32 Y, int32 D) // 種類 位置 向き で判定する
{
	Grid<int32> G = rotate(blockShape[T], D);
	for (int32 i = 0; i < G.height(); i++)
	{
		for (int32 j = 0; j < G.width(); j++)
		{
			if (G[i][j] == 1)
			{
				int32 x = X + i - 1;
				int32 y = Y + j - 1;
				if (x >= 0 && x < 10 && y >= 0 && y < 20)
				{
					if (Blocks[x][y] != -1) // 置く範囲で被ったら
					{
						return true;
					}
				}
				else {
					return true;
				}
			}
		}
	}
	return false;
}
int32 Mino::reset() // 初期化する
{
	score = 0;
	lines = 0;
	NextQ.clear();
	for (int32 i = 0; i < Blocks.height(); i++)
	{
		for (int32 j = 0; j < Blocks.width(); j++)
		{
			Blocks[i][j] = -1;
		}
	}
	return 0;
}
int32 Mino::checkGhost()
{
	for (int32 i = 20; i >= getY(); i--)
	{
		if (checkOverlap(getType(), getX(), i, getDire()))
		{
			ghostPos = { getX(), i - 1 };
		}
		else {

		}
	}
	return 0;
}
int32 Mino::putGhost()
{
	Grid<int32> G = getShape();
	for (int32 i = 0; i < G.height(); i++)
	{
		for (int32 j = 0; j < G.width(); j++)
		{
			if (G[i][j] == 1)
				putFlame(getGhostX() + i - 1, getGhostY() + j - 1, blockColor[getType()]);
		}
	}
	return 0;
}
int32 Mino::consCooltime()
{
	cooltimeA--;
	cooltimeD--;
	cooltimeQ--;
	cooltimeE--;
	return 0;
}
/* getter */
int32 Mino::getX()
{
	return Pos.x;
}
int32 Mino::getY()
{
	return Pos.y;
}
int32 Mino::getDire()
{
	return dire;
}
int32 Mino::getLifespan()
{
	return lifespan;
}
int32 Mino::getType()
{
	return type;
}
Grid<int32> Mino::getShape()
{
	Grid<int32> G = rotate(blockShape[getType()], dire);
	return G;
}
int32 Mino::getCooltime(char c)
{
	switch (c)
	{
	case 'A':
		return cooltimeA;
	case 'D':
		return cooltimeD;
	case 'Q':
		return cooltimeQ;
	case 'E':
		return cooltimeE;
	}
}
int32 Mino::getHold()
{
	return hold;
}
bool Mino::getHoldable()
{
	return holdable;
}
bool Mino::getGameover()
{
	return gameover;
}
int32 Mino::getGhostX()
{
	return ghostPos.x;
}
int32 Mino::getGhostY()
{
	return ghostPos.y;
}
/* setter */
int32 Mino::setX(int32 x)
{
	Pos.x = x;
	return 0;
}
int32 Mino::setY(int32 y)
{
	Pos.y = y;
	return 0;
}
int32 Mino::setDire(int32 d)
{
	dire = (d + 4) % 4;
	return 0;
}
int32 Mino::setLifespan(int32 t)
{
	lifespan = t;
	return 0;
}
int32 Mino::setType(int32 t)
{
	type = t;
	return 0;
}
int32 Mino::setCooltime(char c, int32 t)
{
	switch (c)
	{
	case 'A':
		cooltimeA = t;
		return 0;
	case 'D':
		cooltimeD = t;
		return 0;
	case 'Q':
		cooltimeQ = t;
		return 0;
	case 'E':
		cooltimeE = t;
		return 0;
	}

	return 1;
}
int32 Mino::setHold(int32 t)
{
	hold = t;
	return 0;
}
int32 Mino::setHoldable(bool f)
{
	holdable = f;
	return 0;
}
int32 Mino::setGameover(bool f)
{
	gameover = f;
	return 0;
}

//int32 Mino::setShape(Grid<int32> G)
//{
//	for (int32 i = 0; i < G.height(); i++)
//	{
//		for (int32 j = 0; j < G.width(); j++)
//		{
//			shape[i][j] = G[i][j];
//		}
//	}
//	return 0;
//}
/* コンストラクタ */
Mino::Mino() // ミノの種類 T を指定する
{
	Pos = { 0, 0 };
	dire = 0;
	type = 0;
	hold = -1;
	gameover = false;
	holdable = true;
	lifespan = defaltLifespan;
	ghostPos = { 0, 0 };
	cooltimeA = 0;
	cooltimeD = 0;
	cooltimeQ = 0;
	cooltimeE = 0;

	/* 次のミノを取り出す */
}

using App = SceneManager<String>; // シーンの型を決める

class Title : public App::Scene {
private:
	const Font font{ 120 };
public:
	Title(const InitData& init) : IScene{ init }
	{
		//Print << U"Title::Title()";
	}
	void update() override
	{
		font(U"tartris").drawAt(defaltX + defaltW + 60, defaltY * 2, Palette::Black);	// 文字を入れる

		if (SimpleGUI::Button(U"Game Start", Vec2{ 400 , 300 }) || KeyEnter.pressed() || KeySpace.pressed())
		{
			changeScene(U"Game");
		}
	}
	void draw() const override
	{
		Scene::SetBackground(ColorF{ 0.3, 0.4, 0.5 });

		//FontAsset(U"TitleFont")(U"My Game").drawAt(400, 100);

		//Circle{ Cursor::Pos(), 50 }.draw(Palette::Orange);
	}
};
class Game : public App::Scene
{
private:
	Mino tetriMino;
	Texture m_texture;
	const Font font{ 20 };
	bool pause;
public:
	// 音声ファイルの作成
	const Audio bgm{ U"../music/game_bgm.wav" , Loop::Yes };
	const Audio pui{ U"../music/pui.mp3" };
	const Audio don{ U"../music/don.mp3" };
	bool getPause()
	{
		return pause;
	}
	bool setPause(bool flag)
	{
		pause = flag;
		return 0;
	}

	Game(const InitData& init)
		: IScene{ init }
		, m_texture{ U"🐈"_emoji }
	{
		//Print << U"Game::Game()";
		// 背景の色を設定 | Set background color
		Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });

		//App manager;
		//manager.add<Title>(U"Title");


		tetriMino.reset();
		nextReplesh(); // QにNextを追加
		tetriMino.goNext();
		setPause(false); // pauseじゃない状態で始まる


		// 音声ファイルの再生
		bgm.play();
		// 音量の調整
		bgm.setVolume(bgmVolume);
		pui.setVolume(seVolume);
		don.setVolume(seVolume);
	}

	~Game()
	{
		//Print << U"Game::~Game()";
	}

	void update() override
	{
		if (!getPause()) // pause中じゃないなら
		{
			nextReplesh(); // QにNextを追加
			if (tetriMino.getLifespan() <= 0)	// 落下判定をする
			{
				if (tetriMino.fallMino() == 1); // ブロックが設置されたなら
				{
					//don.play();
				}
				tetriMino.setLifespan(defaltLifespan);
			}
			else {								// 時間を進める
				tetriMino.setLifespan(tetriMino.getLifespan() - 1);
			}
			if (tetriMino.checkGameover())
			{
				// デバッグのためにフラグを建てないでおく
				tetriMino.setGameover(true); // ゲームオーバーフラグを立てる
			}

			/* キー入力 */
			if (SimpleGUI::Button(U"Pause", Vec2{ 640, 300 }) || KeyP.pressed())
			{
				setPause(true);
			}
			if (SimpleGUI::Button(U"←", Vec2{ 580, 500 }) || KeyA.down() || 0.3s <= KeyA.pressedDuration() || KeyLeft.down())
			{
				if (tetriMino.getCooltime('A') <= 0)
				{
					tetriMino.dx(-1);
					tetriMino.setCooltime('A', defaltCooltime);
					if (tetriMino.getLifespan() < defaltLifespan / 2)
						tetriMino.setLifespan(defaltLifespan / 2);
				}
			}
			if (SimpleGUI::Button(U"→", Vec2{ 700, 500 }) || KeyD.down() || 0.3s <= KeyD.pressedDuration() || KeyRight.down())
			{
				if (tetriMino.getCooltime('D') <= 0)
				{
					tetriMino.dx(1);
					tetriMino.setCooltime('D', defaltCooltime);
					if (tetriMino.getLifespan() < defaltLifespan / 2)
						tetriMino.setLifespan(defaltLifespan / 2);
				}
			}
			if (SimpleGUI::Button(U"↓", Vec2{ 640, 500 }) || KeyS.pressed() || KeyDown.pressed())
			{
				tetriMino.setLifespan(tetriMino.getLifespan() - 19);
			}
			if (SimpleGUI::Button(U"↑", Vec2{ 640, 450 }) || KeyW.down() || KeyUp.down())
			{

				while (!tetriMino.fallMino())
				{
				}
				if (tetriMino.getLifespan() < defaltLifespan / 2)
					tetriMino.setLifespan(defaltLifespan / 2);
			}
			if (SimpleGUI::Button(U"↖", Vec2{ 580, 450 }) || KeyQ.down())
			{
				tetriMino.lRotate();
				if (tetriMino.getLifespan() < defaltLifespan / 2)
					tetriMino.setLifespan(defaltLifespan / 2);
			}
			if (SimpleGUI::Button(U"↗", Vec2{ 700, 450 }) || KeyE.down())
			{
				tetriMino.rRotate();
				if (tetriMino.getLifespan() < defaltLifespan / 2)
					tetriMino.setLifespan(defaltLifespan / 2);
			}
			if (SimpleGUI::Button(U"Hold", Vec2{ 640, 400 }) || KeySpace.down())
			{
				if (tetriMino.getHoldable()) {
					tetriMino.holdChange();
					if (tetriMino.getLifespan() < defaltLifespan / 2)
						tetriMino.setLifespan(defaltLifespan / 2);
				}
			}
			if (GravPoint >= GravNeedPoint)
			{
				if (SimpleGUI::Button(U"Gravity", Vec2{ defaltX - 130, 500 }) || KeyG.down()) {
					// ぐらびてぃ
					if (Grav() == 0)
					{
						findErase();
						GravPoint = 0; // ポイントを0に戻す
					}
				}
			}
			if (SimpleGUI::Button(U"Back to Title", Vec2{ 640 , 100 }))
			{
				changeScene(U"Title");
			}
			if (SimpleGUI::Button(U"Go Result", Vec2{ 640 , 200 }))
			{
				changeScene(U"Result");
			}

			tetriMino.checkGhost();		// ghostの更新
			tetriMino.consCooltime();	// クールタイム 1F 消化
		}
		/* ブロック配置位置の背景を描く */
		Rect{ defaltX, defaltY, 200, 400 }.draw(Palette::Black);
		for (int32 i = 0; i <= 10; i++) {
			for (int32 j = 0; j <= 20; j++) {
				Line{ defaltX, defaltY + defaltR * j, defaltX + defaltW, defaltY + defaltR * j }.draw(Palette::Gray);
				Line{ defaltX + defaltR * i, defaltY, defaltX + defaltR * i, defaltY + defaltH }.draw(Palette::Gray);
			}
		}
		/* 配置済みブロックを描画する */
		for (int32 i = 0; i < 10; i++) {
			for (int32 j = 0; j < 20; j++) {
				if (Blocks[i][j] != -1)
					putBlock(i, j, blockColor[Blocks[i][j]]);
			}
		}
		/* Ghostを描画する */
		tetriMino.putGhost();

		/* next を表示する */
		Rect{ defaltX + defaltW + 40, defaltY - 20, 120, 100 }.draw(Palette::Black);	// next置き場
		Rect{ defaltX + defaltW + 40, defaltY + 120, 120, 220 }.draw(Palette::Black);	// next置き場
		font(U"Next").drawAt(defaltX + defaltW + 60, defaltY - 40, Palette::Dimgray);	// 文字を入れる
		putNext(13, 0, NextQ[0]);	// 次
		putNext(13, 7, NextQ[1]);	// 次の次
		putNext(13, 12, NextQ[2]);	// 次の次の次

		/* 消した列数を表示する */
		font(lines, U" Lines").drawAt(defaltX - 60, defaltY - 40, Palette::Dimgray);	// 文字を入れる

		/* hold を表示する */
		Rect{ defaltX - 130, defaltY - 20, 120, 100 }.draw(Palette::Black);	// hold置き場
		if (tetriMino.getHold() != -1)
			tetriMino.putHold(-5, 0);	// hold

		/* 操作中のミノを描画する */
		tetriMino.putMino();

		/* ブロックの消去をする */
		if (findErase() != 0)
		{
			tetriMino.setLifespan(90);
			pui.play();
		}

		/* ゲームオーバー */
		if (tetriMino.getGameover())
		{
			font(U"Gameover").drawAt(defaltX + defaltW / 2, defaltY - 50, Palette::Dimgray);	// 文字を入れる
			tetriMino.setLifespan(defaltLifespan);
			changeScene(U"Result");
		}

		/* ポーズ画面を描画する */
		if (getPause())
		{
			Rect{ 200, 100, 400, 400 }.draw(Palette::Gray);
			if (SimpleGUI::Button(U"Continue", Vec2{ 350, 150 }, 100) || KeySpace.pressed())	// 続きから
			{
				setPause(false);
			}
			if (SimpleGUI::Button(U"Restart", Vec2{ 350, 200 }, 100) || KeyR.pressed())		// 最初から
			{
				// リスタート処理
				score = 0;
				lines = 0;
				tetriMino.setHold(-1);
				tetriMino.reset();
				nextReplesh();		// QにNextを追加
				tetriMino.goNext();
				setPause(false);	// pauseじゃない状態で始まる
			}
			if (SimpleGUI::Button(U"Back to Title", Vec2{ 300, 250 }, 200) || KeyB.pressed())	// タイトルに戻る
			{
				changeScene(U"Title");	// タイトルに戻る
			}
			/* 音量調整 */
			//SimpleGUI::VerticalSlider(U"BGM",  bgmVolume, 0.0, 1.0, Vec2{ 350, 300  }, 100);
			SimpleGUI::Slider(U"BGM", bgmVolume, 0.0, 1.0, Vec2{ 300, 300 }, 50, 150);
			SimpleGUI::Slider(U"SE", seVolume, 0.0, 1.0, Vec2{ 300, 350 }, 50, 150);
			bgm.setVolume(bgmVolume);
			pui.setVolume(seVolume);
			don.setVolume(seVolume);
		}

		// デバッグ表示
		//Print << tetriMino.getDire();
	}

	void draw() const override
	{
		Scene::SetBackground(ColorF(0.2, 0.8, 0.6));

		//m_texture.drawAt(Cursor::Pos());
	}


};
class Result : public App::Scene {
private:
	const Font font{ 20 };
	const Font bigFont{ 60 };
public:
	Result(const InitData& init) : IScene{ init }
	{
		//Print << U"Title::Title()";
	}
	void update() override
	{
		/* ゲームオーバー表示 */
		bigFont(U"Gameover").draw(Vec2{400, 100}, Palette::Black);	// 文字を入れる
		/* ブロック配置位置の背景を描く */
		Rect{ defaltX, defaltY, 200, 400 }.draw(Palette::Black);
		for (int32 i = 0; i <= 10; i++) {
			for (int32 j = 0; j <= 20; j++) {
				Line{ defaltX, defaltY + defaltR * j, defaltX + defaltW, defaltY + defaltR * j }.draw(Palette::Red);
				Line{ defaltX + defaltR * i, defaltY, defaltX + defaltR * i, defaltY + defaltH }.draw(Palette::Red);
			}
		}
		/* 配置済みブロックを描画する */
		for (int32 i = 0; i < 10; i++) {
			for (int32 j = 0; j < 20; j++) {
				if (Blocks[i][j] != -1)
					putBlock(i, j, blockColor[Blocks[i][j]]);
			}
		}/* 消した列数を表示する */
		font(lines, U" Lines").drawAt(defaltX - 60, defaltY - 40, Palette::Red);	// 文字を入れる

		/* ゲームに戻るボタン */
		if (SimpleGUI::Button(U"ReStart", Vec2{ 400 , 300 }) || KeySpace.pressed())
		{
			changeScene(U"Game");
		}
		/* share */
		if (SimpleGUI::Button(U"Share on X", Vec2{ 400 , 400 }) || KeyT.pressed())
		{
			Twitter::OpenTweetWindow(U"score:{} lines @tart0082 より たるたろテトリスを遊んだよ、みんなもプレイしてみよう！ #tartris https://tart082.github.io/games/unyo.html "_fmt(lines));
		}
		font(U"twitterにて感想意見改善点バグの報告などをお待ちしております。#tartris ").drawAt(Vec2{400, 550}, Palette::Black);	// 文字を入れる
	}
	void draw() const override
	{
		Scene::SetBackground(ColorF{ 0.3, 0.4, 0.5 });

		//FontAsset(U"TitleFont")(U"My Game").drawAt(400, 100);

		//Circle{ Cursor::Pos(), 50 }.draw(Palette::Orange);
	}
};

void Main()
{
	// タイトルを表示する
	Window::SetTitle(U"tartris");
	// 背景の色を設定 | Set background color
	Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });

	// 通常のフォントを作成 | Create a new font
	const Font font{ 20 };

	// 絵文字用フォントを作成 | Create a new emoji font
	const Font emojiFont{ 60, Typeface::ColorEmoji };

	// `font` が絵文字用フォントも使えるようにする | Set emojiFont as a fallback
	font.addFallback(emojiFont);

	// 画像ファイルからテクスチャを作成 | Create a texture from an image file
	const Texture texture{ U"example/windmill.png" };

	// 絵文字からテクスチャを作成 | Create a texture from an emoji
	const Texture emoji{ U"🐈"_emoji };

	// 絵文字を描画する座標 | Coordinates of the emoji
	Vec2 emojiPos{ 300, 150 };

	// テキストを画面にデバッグ出力 | Print a text
	//Print << U"Push [A] key";
	App manager;
	// タイトルシーン（名前は "Title"）を登録
	manager.add<Title>(U"Title");

	// ゲームシーン（名前は "Game"）を登録
	manager.add<Game>(U"Game");

	// リザルトシーン（名前は "Result"）を登録
	manager.add<Result>(U"Result");

	while (System::Update())
	{
		if (not manager.update())
		{
			break;
		}
	}

}

//
// - Debug ビルド: プログラムの最適化を減らす代わりに、エラーやクラッシュ時に詳細な情報を得られます。
//
// - Release ビルド: 最大限の最適化でビルドします。
//
// - [デバッグ] メニュー → [デバッグの開始] でプログラムを実行すると、[出力] ウィンドウに詳細なログが表示され、エラーの原因を探せます。
//
// - Visual Studio を更新した直後は、プログラムのリビルド（[ビルド]メニュー → [ソリューションのリビルド]）が必要です。
//
// チュートリアル
// https://siv3d.github.io/ja-jp/tutorial/tutorial/
//
// Tutorial
// https://siv3d.github.io/tutorial/tutorial/
//
// Siv3D コミュニティへの参加（Discord などで気軽に質問や交流, 最新情報の入手ができます）
// https://siv3d.github.io/ja-jp/community/community/
//
// Siv3D User Community
// https://siv3d.github.io/community/community/
//
// 新機能の提案やバグの報告 | Feedback
// https://siv3d.github.io/ja-jp/develop/report/
//
// Sponsoring Siv3D
// https://github.com/sponsors/Reputeless
//
