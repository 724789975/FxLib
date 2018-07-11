using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameLogic : SingletonObject<GameLogic>
{
	void Awake()
	{
		CreateInstance(this);
		for (int i = 0; i < TetrisData.s_dwColumn * TetrisData.s_dwRow; ++i)
		{
			m_arrBlockInfos[i / TetrisData.s_dwColumn, i % TetrisData.s_dwColumn] = m_arrBlocks[i];
		}
	}

	// Use this for initialization
	void Start ()
	{
		m_oData = TetrisDataManager.Instance().proUserTetrisData;
	}

	public void DownTetris()
	{
		if (TetrisData.proState != GameProto.EGameSceneState.EssGaming)
		{
			return;
		}
		TetrisDataManager.Instance().proUserTetrisData.TetrisOperator(GameProto.EMoveDirection.EmdDown);
	}

	public void LeftTetris()
	{
		if (TetrisData.proState != GameProto.EGameSceneState.EssGaming)
		{
			return;
		}
		TetrisDataManager.Instance().proUserTetrisData.TetrisOperator(GameProto.EMoveDirection.EmdLeft);
	}

	public void RightTetris()
	{
		if (TetrisData.proState != GameProto.EGameSceneState.EssGaming)
		{
			return;
		}
		TetrisDataManager.Instance().proUserTetrisData.TetrisOperator(GameProto.EMoveDirection.EmdRight);
	}

	public void LeftRotation()
	{
		if (TetrisData.proState != GameProto.EGameSceneState.EssGaming)
		{
			return;
		}
		TetrisDataManager.Instance().proUserTetrisData.TetrisOperator(GameProto.ERotationDirection.ErdLeft);
	}

	public void RightRotation()
	{
		if (TetrisData.proState != GameProto.EGameSceneState.EssGaming)
		{
			return;
		}
		TetrisDataManager.Instance().proUserTetrisData.TetrisOperator(GameProto.ERotationDirection.ErdRight);
	}

	// Update is called once per frame
	void Update ()
	{
		if (TetrisData.proState != GameProto.EGameSceneState.EssGaming)
		{
			return;
		}
		if (m_oData == null)
		{
			return;
		}

#if UNITY_STANDALONE_WIN || UNITY_EDITOR
		if (Input.GetKey(KeyCode.S))
		{
			SampleDebuger.Log("您按下了S键");
			DownTetris();
        }

		if (Input.GetKeyDown(KeyCode.A))
		{
			SampleDebuger.Log("您按下了A键");
			LeftTetris();
		}

		if (Input.GetKeyDown(KeyCode.D))
		{
			SampleDebuger.Log("您按下了D键");
			RightTetris();
		}

		if (Input.GetKeyDown(KeyCode.H))
		{
			SampleDebuger.Log("您按下了H键");
			LeftRotation();
		}

		if (Input.GetKeyDown(KeyCode.J))
		{
			SampleDebuger.Log("您按下了J键");
			RightRotation();
		}
#endif

		TetrisDataManager.Instance().proUserTetrisData.Update(Time.deltaTime);

		uint[,] dwBlockInfos = m_oData.GetTetrisInfo();
		if (dwBlockInfos == null)
		{
			return;
		}

		//刷新显示的方块
		Color32 c = new Color32(255, 255, 255, 35);
		for (int i = 0; i < TetrisData.s_dwRow; i++)
		{
			for (int j = 0; j < TetrisData.s_dwColumn; j++)
			{
				uint dwBlock = dwBlockInfos[i, j];
				if (dwBlock != 0)
				{
					c.r = (byte)((dwBlock & 0xFF000000) >> 24);
					c.g = (byte)((dwBlock & 0x00FF0000) >> 16);
					c.b = (byte)((dwBlock & 0x0000FF00) >> 8);
					c.a = 100;
				}
				else
				{
					c.r = 255;
					c.g = 255;
					c.b = 255;
					c.a = 35;
				}
				m_arrBlockInfos[i, j].color = c;
			}
		}

		if (m_oData.m_oNextTetris != null)
		{
			for (int i = 0; i < TetrisData.s_dwUnit; ++i)
			{
				for (int j = 0; j < TetrisData.s_dwUnit; ++j)
				{
					uint dwBlockInfo = TetrisData.s_dwTetrisTable[m_oData.m_oNextTetris.m_dwTetrisShape, m_oData.m_oNextTetris.m_dwTetrisDirect, i, j];
					if (dwBlockInfo != 0)
					{
						c.r = (byte)((m_oData.m_oNextTetris.m_dwTetrisColor & 0xFF000000) >> 24);
						c.g = (byte)((m_oData.m_oNextTetris.m_dwTetrisColor & 0x00FF0000) >> 16);
						c.b = (byte)((m_oData.m_oNextTetris.m_dwTetrisColor & 0x0000FF00) >> 8);
						c.a = 100;
					}
					else
					{
						c.r = 255;
						c.g = 255;
						c.b = 255;
						c.a = 35;
					}
					m_arrNextBlocks[i * TetrisData.s_dwUnit + j].color = c;
				}
			}
		}
	}

	public void SetTetrisData(TetrisData oData, System.UInt64 qwPlayerId)
	{
		m_oData = oData;
		if (qwPlayerId == PlayerData.Instance().proPlayerId)
		{
			m_goOperator.SetActive(true);
		}
		else
		{
			m_goOperator.SetActive(false);
		}
	}

	[SerializeField]
	UnityEngine.UI.Image[] m_arrBlocks = new UnityEngine.UI.Image[TetrisData.s_dwRow * TetrisData.s_dwColumn];
	[SerializeField]
	UnityEngine.UI.Image[] m_arrNextBlocks = new UnityEngine.UI.Image[TetrisData.s_dwUnit * TetrisData.s_dwUnit];
	[SerializeField]
	UnityEngine.UI.Image[] m_arrCurrBlocks = new UnityEngine.UI.Image[TetrisData.s_dwUnit * TetrisData.s_dwUnit];

	UnityEngine.UI.Image[,] m_arrBlockInfos = new UnityEngine.UI.Image[TetrisData.s_dwRow, TetrisData.s_dwColumn];

	[SerializeField]
	TetrisData m_oData = null;

	[SerializeField]
	GameObject m_goOperator = null;
}

