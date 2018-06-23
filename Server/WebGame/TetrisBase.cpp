#include "TetrisBase.h"
#include <string.h>
#include <stdlib.h>
#include <string>
#include "fxmeta.h"
#include "Player.h"
#include "GameScene.h"
#include "GameConfigBase.h"
#include "PlayerSession.h"

#define COLOR_NUM 8
static const unsigned int g_dwColors[COLUMN_NUM]
{
	0xFF000000,
	0x00FF0000,
	0x0000FF00,
	0xFFFF0000,
	0x00FFFF00,
	0xFF00FF00,
	0xF00F0000,
	0x00F0F000,
};

static const unsigned int g_dwTetrisTable[7][4][7][4]
{
	// I�� { 0x00F0, 0x2222, 0x00F0, 0x2222 },	�����д���������Ŀ��λ��
	{
		{
			{ 0x0,	0xFFFFFFFF, 0x0,	 0x0, },
			{ 0x0,	0xFFFFFFFF, 0x0,	 0x0, },
			{ 0x0,	0xFFFFFFFF, 0x0,	 0x0, },
			{ 0x0,	0xFFFFFFFF, 0x0,	 0x0, },
			{ 0x0,	0xFFFFFFF0, 0x0,	 0x0, },	//�±߽�
			{ 0xFFFFFFF1,  0xFFFFFFF1, 0xFFFFFFF1,     0xFFFFFFF1, },	//��߽�
			{ 0xFFFFFFF1,  0xFFFFFFF1,         0xFFFFFFF1,            0xFFFFFFF1, },	//�ұ߽�
		},
		{
			{ 0x0,			0x0,		0x0,			0x0, },
			{ 0xFFFFFFFF,	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,			0x0,		0x0,			0x0, },
			{ 0x0,			0x0,		0x0,			0x0, },
			{ 0xFFFFFFF2,	0xFFFFFFF2, 0xFFFFFFF2,	0xFFFFFFF2, },	//�±߽�
			{ 0x0,			0xFFFFFFF0,			0x0,	 0x0, },	//��߽�
			{ 0x0,			0xFFFFFFF3,			0x0,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0xFFFFFFFF, 0x0,	 0x0, },
			{ 0x0,	0xFFFFFFFF, 0x0,	 0x0, },
			{ 0x0,	0xFFFFFFFF, 0x0,	 0x0, },
			{ 0x0,	0xFFFFFFFF, 0x0,	 0x0, },
			{ 0x0,	0xFFFFFFF0, 0x0,	 0x0, },	//�±߽�
			{ 0xFFFFFFF1,  0xFFFFFFF1, 0xFFFFFFF1,     0xFFFFFFF1, },	//��߽�
			{ 0xFFFFFFF1,  0xFFFFFFF1,         0xFFFFFFF1,            0xFFFFFFF1, },	//�ұ߽�
		},
		{
			{ 0x0,			0x0,		0x0,			0x0, },
			{ 0xFFFFFFFF,	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,			0x0,		0x0,			0x0, },
			{ 0x0,			0x0,		0x0,			0x0, },
			{ 0xFFFFFFF2,	0xFFFFFFF2, 0xFFFFFFF2,	0xFFFFFFF2, },	//�±߽�
			{ 0x0,			0xFFFFFFF0,			0x0,	 0x0, },	//��߽�
			{ 0x0,			0xFFFFFFF3,			0x0,			0x0, },	//�ұ߽�
		},
	},
	// T�� { 0x0072, 0x0262, 0x0270, 0x0232 },	�����д���������Ŀ��λ��
	{
		{
			{ 0x0,	0x0,	 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,	 0x0,			0x0, },
			{ 0x0,	0x0,	 0xFFFFFFF1,	0xFFFFFFF2, },	//�±߽�
			{ 0xFFFFFFF2,	0xFFFFFFF2,		0xFFFFFFF2,	 0x0, },	//��߽�
			{ 0xFFFFFFF2,	0xFFFFFFF3,		0xFFFFFFF2,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,			0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF2, 0xFFFFFFF1, 0xFFFFFFF2, },	//�±߽�
			{ 0x0,	0xFFFFFFF1,			0xFFFFFFF2,	 0x0, },	//��߽�
			{ 0x0,	0xFFFFFFF3,		0xFFFFFFF2,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,			0xFFFFFFFF, 0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF2, 0xFFFFFFF1, 0x0, },	//�±߽�
			{ 0xFFFFFFF2,	0xFFFFFFF1,			0xFFFFFFF2,	 0x0, },	//��߽�
			{ 0xFFFFFFF2,	0xFFFFFFF2,		 0xFFFFFFF2,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,			0xFFFFFFFF, 0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF2, 0xFFFFFFF2, 0xFFFFFFF2, },	//�±߽�
			{ 0xFFFFFFF2,	0xFFFFFFF1,			0x0,	 0x0, },	//��߽�
			{ 0xFFFFFFF2,	0xFFFFFFF3,			0x0,			0x0, },	//�ұ߽�
		},
	},
	//L�� //{ 0x0223, 0x0074, 0x0622, 0x0170 },	�����д���������Ŀ��λ��
	{
		{
			{ 0x0,	0x0,			0x0,			0xFFFFFFFF, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF2, 0xFFFFFFF2, 0xFFFFFFF2, },	//�±߽�
			{ 0xFFFFFFF3,	0xFFFFFFF1,			0x0,	 0x0, },	//��߽�
			{ 0xFFFFFFF3,	0xFFFFFFF3,			0x0,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,	 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,	 0x0,			0x0, },
			{ 0x0,	0x0,	 0xFFFFFFF1, 0xFFFFFFF1, },	//�±߽�
			{ 0xFFFFFFF2,	0xFFFFFFF2,			0xFFFFFFF2,	 0x0, },	//��߽�
			{ 0xFFFFFFF2,	0xFFFFFFF2,		 0xFFFFFFF3,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0xFFFFFFFF, 0x0,			0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF1, 0xFFFFFFF2, 0xFFFFFFF2, },	//�±߽�
			{ 0x0,	0xFFFFFFF1,			0xFFFFFFF1,	 0x0, },	//��߽�
			{ 0xFFFFFFF3,	0xFFFFFFF1,			0x0,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF3, 0xFFFFFFF1, 0x0, },	//�±߽�
			{ 0x0,	0xFFFFFFF1,			0xFFFFFFF2,	 0x0, },	//��߽�
			{ 0xFFFFFFF2,	0xFFFFFFF2,		 0xFFFFFFF2,			0x0, },	//�ұ߽�
		},
	},
	//J�� { 0x0226, 0x0470, 0x0322, 0x0071 },	�����д���������Ŀ��λ��
	{
		{
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,			0x0,			0xFFFFFFFF, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF2, 0xFFFFFFF2, 0xFFFFFFF1, },	//�±߽�
			{ 0x0,	0xFFFFFFF1,			0xFFFFFFF2,	 0x0, },	//��߽�
			{ 0x0,	0xFFFFFFF3,		 0xFFFFFFF3,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,			0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0xFFFFFFFF, 0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF1, 0xFFFFFFF1, 0x0, },	//�±߽�
			{ 0xFFFFFFF2,	0xFFFFFFF2,			0xFFFFFFF1,	 0x0, },	//��߽�
			{ 0xFFFFFFF2,	0xFFFFFFF2,		 0xFFFFFFF2,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0xFFFFFFFF, 0x0,			0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF2, 0xFFFFFFF2, 0xFFFFFFF2, },	//�±߽�
			{ 0xFFFFFFF1,	0xFFFFFFF1,			0x0,	 0x0, },	//��߽�
			{ 0xFFFFFFF1,	0xFFFFFFF3,			0x0,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,	 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,	 0x0,			0x0, },
			{ 0x0,	0x0,	 0xFFFFFFF1, 0xFFFFFFF3, },	//�±߽�
			{ 0xFFFFFFF2,	0xFFFFFFF2,			0xFFFFFFF2,	 0x0, },	//��߽�
			{ 0xFFFFFFF3,	0xFFFFFFF2,		 0xFFFFFFF2,			0x0, },	//�ұ߽�
		},
	},
	//Z�� { 0x0063, 0x0264, 0x0063, 0x0264 },	�����д���������Ŀ��λ��
	{
		{
			{ 0x0,	0x0,	 0x0,			0xFFFFFFFF, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,	 0x0,			0x0, },
			{ 0x0,	0x0,	 0xFFFFFFF1, 0xFFFFFFF2, },	//�±߽�
			{ 0xFFFFFFF3,	0xFFFFFFF2,			0xFFFFFFF2,	 0x0, },	//��߽�
			{ 0xFFFFFFF3,	0xFFFFFFF3,		 0xFFFFFFF2,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF2, 0xFFFFFFF1, 0xFFFFFFF1, },	//�±߽�
			{ 0x0,	0xFFFFFFF1,			0xFFFFFFF2,	 0x0, },	//��߽�
			{ 0x0,	0xFFFFFFF2,		 0xFFFFFFF3,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,	 0x0,			0xFFFFFFFF, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,	 0x0,			0x0, },
			{ 0x0,	0x0,	 0xFFFFFFF1, 0xFFFFFFF2, },	//�±߽�
			{ 0xFFFFFFF3,	0xFFFFFFF2,			0xFFFFFFF2,	 0x0, },	//��߽�
			{ 0xFFFFFFF3,	0xFFFFFFF3,		 0xFFFFFFF2,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF2, 0xFFFFFFF1, 0xFFFFFFF1, },	//�±߽�
			{ 0x0,	0xFFFFFFF1,			0xFFFFFFF2,	 0x0, },	//��߽�
			{ 0xFFFFFFF2,	0xFFFFFFF3,			0x0,			0x0, },	//�ұ߽�
		},
	},
	//S�� { 0x006C, 0x0462, 0x006C, 0x0462 },	�����д���������Ŀ��λ��
	{
		{
			{ 0x0,	0x0,	 0x0,			0x0, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,	 0x0,			0xFFFFFFFF, },
			{ 0x0,	0x0,	 0xFFFFFFF1, 0xFFFFFFF0, },	//�±߽�
			{ 0x0,	0xFFFFFFF2,			0xFFFFFFF2,	 0xFFFFFFF3, },	//��߽�
			{ 0xFFFFFFFF,	0xFFFFFFF3,		 0xFFFFFFF3,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0x0,			0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF1, 0xFFFFFFF1, 0xFFFFFFF2, },	//�±߽�
			{ 0x0,	0xFFFFFFF2,			0xFFFFFFF1,	 0x0, },	//��߽�
			{ 0x0,	0xFFFFFFF3,			0xFFFFFFF2,			0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,	 0x0,			0x0, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,	 0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0x0,	 0x0,			0xFFFFFFFF, },
			{ 0x0,	0x0,	 0xFFFFFFF1, 0xFFFFFFF0, },	//�±߽�
			{ 0x0,	0xFFFFFFF2,			0xFFFFFFF2,	 0xFFFFFFF3, },	//��߽�
			{ 0x0,	0xFFFFFFF2,		 0xFFFFFFF3,		 0xFFFFFFF3, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0x0,			0xFFFFFFFF, 0xFFFFFFFF, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF1, 0xFFFFFFF1, 0xFFFFFFF2, },	//�±߽�
			{ 0x0,	0xFFFFFFF2,			0xFFFFFFF1,	 0x0, },	//��߽�
			{ 0x0,	0xFFFFFFF3,			0xFFFFFFF2,			0x0, },	//�ұ߽�
		},
	},
	//O�� { 0x0660, 0x0660, 0x0660, 0x0660 }	�����д���������Ŀ��λ��
	{
		{
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF1, 0xFFFFFFF1, 0x0, },	//�±߽�
			{ 0x0,	0xFFFFFFF1,			0xFFFFFFF1,	 0x0, },	//��߽�
			{ 0x0,	0xFFFFFFF2,		 0xFFFFFFF2,			0x0, },	//�ұ߽�
		},{
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF1, 0xFFFFFFF1, 0x0, },	//�±߽�
			{ 0x0,	0xFFFFFFF1,			0xFFFFFFF1,	 0x0, },	//��߽�
			{ 0x0,	0xFFFFFFF2,		 0xFFFFFFF2,		 0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF1, 0xFFFFFFF1, 0x0, },	//�±߽�
			{ 0x0,	0xFFFFFFF1,			0xFFFFFFF1,	 0x0, },	//��߽�
			{ 0x0,	0xFFFFFFF2,		 0xFFFFFFF2,		 0x0, },	//�ұ߽�
		},
		{
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0xFFFFFFFF, 0xFFFFFFFF, 0x0, },
			{ 0x0,	0x0,			0x0,			0x0, },
			{ 0x0,	0xFFFFFFF1, 0xFFFFFFF1, 0x0, },	//�±߽�
			{ 0x0,	0xFFFFFFF1,			0xFFFFFFF1,	 0x0, },	//��߽�
			{ 0x0,	0xFFFFFFF2,		 0xFFFFFFF2,		 0x0, },	//�ұ߽�
		},
	},
};

//////////////////////////////////////////////////////////////////////////
void Tetris::FillTetris(GameProto::Tetris& refInfo)
{
	refInfo.set_dw_tetris_shape(m_dwTetrisShape);
	refInfo.set_dw_tetris_direct(m_dwTetrisDirect);
	refInfo.set_dw_tetris_color(m_dwTetrisColor);
	refInfo.set_dw_pos_x(m_dwPosX);
	refInfo.set_dw_pos_y(m_dwPosY);
}

//////////////////////////////////////////////////////////////////////////
TetrisBase::TetrisBase(CPlayerBase& refPlayer)
	: m_refPlayer(refPlayer)
	, m_fTick(0.0f)
{
	memset(m_dwTetrisPools, 0, sizeof(m_dwTetrisPools));
}

TetrisBase::~TetrisBase()
{
}

void TetrisBase::Init()
{

}

void TetrisBase::DownTetris()
{
	//�����λ�ó�ʼ�����м�ķ���
	//�����Ƿ�������½�
	//�½���ʱ�� ��Ҫ�ҳ���ǰ����������ļ����������
	if (!CheckDownTetris())
	{
		//�����ƶ�һ��
		m_oCurrentTetris.m_dwPosY += 1;
	}
	else
	{
		//�̶�ס ��ô�Ͳ��������ƶ��� ����һ������
		for (int i = 0; i < TETRIS_UNIT; ++i)
		{
			for (int j = 0; j < TETRIS_UNIT; ++j)
			{
				unsigned int dwBlockInfo = g_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape][m_oCurrentTetris.m_dwTetrisDirect][i][j];
				if (dwBlockInfo == 0)
				{
					continue;
				}
				m_dwTetrisPools[m_oCurrentTetris.m_dwPosY - TETRIS_UNIT + i][m_oCurrentTetris.m_dwPosX + j] = m_oCurrentTetris.m_dwTetrisColor;
			}
		}

		if (m_oCurrentTetris.m_dwPosY <= TETRIS_UNIT)
		{
			OnEnd();
			GameProto::GameNotifyPlayerDead oDead;
			oDead.set_dw_player_id(m_refPlayer.GetPlayerId());
			CGameSceneBase::Instance()->NotifyPlayer(oDead);
			return;
		}

		m_oCurrentTetris = m_oNextTetris;
		m_oNextTetris.m_dwTetrisShape = rand() % SHAPE_COUNT;
		m_oNextTetris.m_dwTetrisDirect = rand() % 4;
		m_oNextTetris.m_dwPosX = (COLUMN_NUM - TETRIS_UNIT) / 2;
		m_oNextTetris.m_dwPosY = TETRIS_UNIT;
		m_oNextTetris.m_dwTetrisColor = g_dwColors[rand() % COLOR_NUM];

		//����Ϣ
		GameProto::GameNotifyPlayerNextTetris oNextTetris;
		oNextTetris.set_dw_player_id(m_refPlayer.GetPlayerId());
		oNextTetris.set_f_tick(m_fTick);
		m_oNextTetris.FillTetris(*(oNextTetris.mutable_next_tetris()));
		CGameSceneBase::Instance()->NotifyPlayer(oNextTetris);
	}

	PrintInfo();

	m_bNeedRefresh = true;
}

void TetrisBase::LeftTetris()
{
	if (!CheckLeftTetris())
	{
		m_oCurrentTetris.m_dwPosX -= 1;
		m_bNeedRefresh = true;
	}
	else
	{
		LogExe(LogLv_Critical, "player move error");
	}
}

void TetrisBase::RightTetris()
{
	if (!CheckRightTetris())
	{
		m_oCurrentTetris.m_dwPosX += 1;
		m_bNeedRefresh = true;
	}
	else
	{
		LogExe(LogLv_Critical, "player move error");
	}
}

void TetrisBase::LeftRotation()
{
	unsigned int dwTempDir = (m_oCurrentTetris.m_dwTetrisDirect + 1) % TETRIS_UNIT;
	for (int i = 0; i < TETRIS_UNIT; ++i)
	{
		for (int j = 0; j < TETRIS_UNIT; ++j)
		{
			unsigned int dwBlockInfo = g_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape][dwTempDir][i][j];
			if (dwBlockInfo == 0)
			{
				continue;
			}
			if (CheckTetris(m_oCurrentTetris.m_dwPosX + j, m_oCurrentTetris.m_dwPosY + i))
			{
				return;
			}
		}
	}
	m_oCurrentTetris.m_dwTetrisDirect = dwTempDir;
}

void TetrisBase::RightRotation()
{
	unsigned int dwTempDir = (m_oCurrentTetris.m_dwTetrisDirect - 1) % TETRIS_UNIT;
	for (int i = 0; i < TETRIS_UNIT; ++i)
	{
		for (int j = 0; j < TETRIS_UNIT; ++j)
		{
			unsigned int dwBlockInfo = g_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape][dwTempDir][i][j];
			if (dwBlockInfo == 0)
			{
				continue;
			}
			if (CheckTetris(m_oCurrentTetris.m_dwPosX + j, m_oCurrentTetris.m_dwPosY + i))
			{
				return;
			}
		}
	}
	m_oCurrentTetris.m_dwTetrisDirect = dwTempDir;
}

bool TetrisBase::CheckTetris(int dwCol, int dwRow)
{
	if (dwRow >= ROW_NUM)
	{
		return true;
	}
	if (dwCol >= COLUMN_NUM)
	{
		return true;
	}
	if (dwCol < 0)
	{
		return true;
	}
	unsigned int dwColor = m_dwTetrisPools[dwRow][dwCol] & 0xFFFFFF00;
	return dwColor != 0;
}

bool TetrisBase::CheckLeftTetris()
{
	//��鷽��������û��
	for (int i = 0; i < TETRIS_UNIT; i++)
	{
		unsigned int dwBlockInfo = g_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape][m_oCurrentTetris.m_dwTetrisDirect][5][i];
		if (dwBlockInfo == 0)
		{
			continue;
		}

		int dwCheckX = m_oCurrentTetris.m_dwPosX + ((int)dwBlockInfo & 0x0000000F) - 1;
		int dwCheckY = m_oCurrentTetris.m_dwPosY - (int)TETRIS_UNIT + i;
		if (CheckTetris(dwCheckX, dwCheckY))
		{
			return true;
		}
	}
	return false;
}

bool TetrisBase::CheckRightTetris()
{
	//��鷽��������û��
	for (int i = 0; i < TETRIS_UNIT; i++)
	{
		unsigned int dwBlockInfo = g_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape][m_oCurrentTetris.m_dwTetrisDirect][6][i];
		if (dwBlockInfo == 0)
		{
			continue;
		}

		int dwCheckX = m_oCurrentTetris.m_dwPosX + ((int)dwBlockInfo & 0x0000000F) + 1;
		int dwCheckY = m_oCurrentTetris.m_dwPosY - (int)TETRIS_UNIT + i;
		if (CheckTetris(dwCheckX, dwCheckY))
		{
			return true;
		}
	}
	return false;
}

bool TetrisBase::CheckDownTetris()
{
	//��鷽�黹�ܲ��ܼ����½�
	for (int i = 0; i < TETRIS_UNIT; i++)
	{
		unsigned int dwBlockInfo = g_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape][m_oCurrentTetris.m_dwTetrisDirect][4][i];
		if (dwBlockInfo == 0)
		{
			continue;
		}

		int dwCheckY = m_oCurrentTetris.m_dwPosY - ((int)dwBlockInfo & 0x0000000F);
		if (CheckTetris(m_oCurrentTetris.m_dwPosX + i, dwCheckY))
		{
			return true;
		}
	}
	return false;
}

void TetrisBase::PrintInfo()
{
#if 1
	char szBuffer[4096] = "";
	int nLenStr = 0;
	nLenStr += sprintf(szBuffer + nLenStr, "\ncurrent shape : %d, dir : %d, color : %x, pos x  : %d, y : %d\n",
		m_oCurrentTetris.m_dwTetrisShape, m_oCurrentTetris.m_dwTetrisDirect, m_oCurrentTetris.m_dwTetrisColor,
		m_oCurrentTetris.m_dwPosX, m_oCurrentTetris.m_dwPosY);
	nLenStr += sprintf(szBuffer + nLenStr, "next shape : %d, dir : %d, color : %x, pos x  : %d, y : %d\n",
		m_oNextTetris.m_dwTetrisShape, m_oNextTetris.m_dwTetrisDirect, m_oNextTetris.m_dwTetrisColor,
		m_oNextTetris.m_dwPosX, m_oNextTetris.m_dwPosY);

	unsigned int dwTetrisPools[ROW_NUM][COLUMN_NUM];
	memcpy(dwTetrisPools, m_dwTetrisPools, sizeof(dwTetrisPools));

	for (int i = 0; i < TETRIS_UNIT; ++i)
	{
		for (int j = 0; j < TETRIS_UNIT; ++j)
		{
			unsigned int dwBlockInfo = g_dwTetrisTable[m_oCurrentTetris.m_dwTetrisShape][m_oCurrentTetris.m_dwTetrisDirect][i][j];
			if (dwBlockInfo == 0)
			{
				continue;
			}
			dwTetrisPools[m_oCurrentTetris.m_dwPosY - TETRIS_UNIT + i][m_oCurrentTetris.m_dwPosX + j] = m_oCurrentTetris.m_dwTetrisColor;
		}
	}

	for (int i = 0; i < ROW_NUM ; ++i)
	{
		for (int j =  0; j < COLUMN_NUM; ++j)
		{
			nLenStr += sprintf(szBuffer + nLenStr, "%10x", dwTetrisPools[i][j]);
		}
		nLenStr += sprintf(szBuffer + nLenStr, "%s", "\n");
	}
	LogExe(LogLv_Debug3, "%s", szBuffer);
#endif
}

//////////////////////////////////////////////////////////////////////////
CommonTetris::CommonTetris(CPlayerBase& refPlayer)
	: TetrisBase(refPlayer)
	, m_fSuspendTime(0.0f)
{ }

CommonTetris::~CommonTetris() { }

void CommonTetris::Init()
{
	m_oCurrentTetris.m_dwTetrisShape = rand() % SHAPE_COUNT;
	m_oCurrentTetris.m_dwTetrisDirect = rand() % 4;
	m_oCurrentTetris.m_dwPosX = (COLUMN_NUM - TETRIS_UNIT) / 2;
	m_oCurrentTetris.m_dwPosY = TETRIS_UNIT;
	m_oCurrentTetris.m_dwTetrisColor = g_dwColors[rand() % COLOR_NUM];

	m_oNextTetris.m_dwTetrisShape = rand() % SHAPE_COUNT;
	m_oNextTetris.m_dwTetrisDirect = rand() % 4;
	m_oNextTetris.m_dwPosX = (COLUMN_NUM - TETRIS_UNIT) / 2;
	m_oNextTetris.m_dwPosY = TETRIS_UNIT;
	m_oNextTetris.m_dwTetrisColor = g_dwColors[rand() % COLOR_NUM];

	memset(m_dwTetrisPools, 0, sizeof(m_dwTetrisPools));

	m_fTick = 0.0f;

	GameProto::GameNotifyPlayerGameInitTetris oGameInitTetris;
	oGameInitTetris.set_dw_player_id(m_refPlayer.GetPlayerId());
	oGameInitTetris.set_f_tick(m_fTick);
	m_oCurrentTetris.FillTetris(*(oGameInitTetris.mutable_curr_tetris()));
	m_oNextTetris.FillTetris(*(oGameInitTetris.mutable_next_tetris()));

	CGameSceneBase::Instance()->NotifyPlayer(oGameInitTetris);
}

void CommonTetris::Update(float fDelta)
{
	m_fTick += fDelta;
	if (m_refPlayer.GetPlayerSession()->IsConnected())
	{
		return;
	}
	m_fSuspendTime += fDelta;
	if (m_fSuspendTime >= CGameConfigBase::Instance()->GetSuspendTime())
	{
		DownTetris();
		m_fSuspendTime = 0.0f;
	}
}

void CommonTetris::OnEnd()
{

}


