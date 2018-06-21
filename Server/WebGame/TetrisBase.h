#ifndef __TetrisBase_H__
#define  __TetrisBase_H__

#include "msg_proto/web_data.pb.h"

#define COLUMN_NUM 12
#define ROW_NUM 22
#define TETRIS_UNIT 4

#define SHAPE_COUNT 7
#define DIRECTCOUNT 4

struct Tetris
{
public:
	Tetris() : m_dwTetrisShape(0), m_dwTetrisDirect(0), m_dwTetrisColor(0), m_dwPosX(0), m_dwPosY(0) {}
	unsigned int  m_dwTetrisShape;
	unsigned int  m_dwTetrisDirect;
	unsigned int  m_dwTetrisColor;

	//坐标位置为左下角
	//列坐标
	int m_dwPosX;
	//行坐标
	int m_dwPosY;

	void FillTetris(GameProto::Tetris& refInfo);
};

class CPlayerBase;
class TetrisBase
{
public:
	TetrisBase(CPlayerBase& refPlayer);
	virtual ~TetrisBase();

public:
	virtual void Init();
	virtual void Update(float fDelta) = 0;

	void DownTetris();
	void LeftTetris();
	void RightTetris();
	void LeftRotation();
	void RightRotation();

	float GetTick() { return m_fTick; }

protected:
	virtual bool CheckTetris(int dwRow, int dwCol);
	virtual bool CheckLeftTetris();
	virtual bool CheckRightTetris();
	virtual bool CheckDownTetris();

	virtual void OnEnd() = 0;

	virtual void PrintInfo();

protected:
	unsigned int m_dwTetrisPools[ROW_NUM][COLUMN_NUM];

	Tetris m_oCurrentTetris;
	Tetris m_oNextTetris;

	bool m_bNeedRefresh;

	CPlayerBase& m_refPlayer;

	float m_fTick;
};

class CommonTetris : public TetrisBase
{
public:
	CommonTetris(CPlayerBase& refPlayer);
	virtual ~CommonTetris();

	virtual void Init();
	virtual void Update(float fDelta);

	virtual void OnEnd();

private:
	float m_fSuspendTime;
};


#endif	//!__TetrisBase_H__
