using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;
using XLua;

public class GameControler : SingletonObject<GameControler>
{
	// Use this for initialization
	void Start()
	{
		CreateInstance(this);
		DontDestroyOnLoad(this);
		H5Manager.Instance().GetGameSessionResetCallBack().Add(OnGameSessionReset);
		if (H5Manager.Instance().GetGameSession() != null)
		{
			OnGameSessionReset(H5Manager.Instance().GetGameSession());
		}
	}

	public void OnGameSessionReset(SessionObject obj)
	{
		m_pSession = obj;
		m_pSession.pro_cbOnConnect.Add(OnConnect);
		m_pSession.pro_cbfOnError.Add(OnError);
		m_pSession.pro_cbOnClose.Add(OnClose);

		m_pSession.RegistMessage("GameProto.PlayerRequestGameTest", OnPlayerRequestGameTest);
		m_pSession.RegistMessage("GameProto.GameAckPlayerEnter", OnGameAckPlayerEnter);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerPrepareTime", OnGameNotifyPlayerPrepareTime);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameReadyTime", OnGameNotifyPlayerGameReadyTime);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameConfig", OnGameNotifyPlayerGameConfig);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameRoleData", OnGameNotifyPlayerGameRoleData);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameSceneInfo", OnGameNotifyPlayerGameSceneInfo);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameState", OnGameNotifyPlayerGameState);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameInitTetris", OnGameNotifyPlayerGameInitTetris);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameTetrisData", OnGameNotifyPlayerGameTetrisData);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayeMove", OnGameNotifyPlayeMove);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayeRotation", OnGameNotifyPlayeRotation);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerNextTetris", OnGameNotifyPlayerNextTetris);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerDead", OnGameNotifyPlayerDead);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayeGameEnd", OnGameNotifyPlayeGameEnd);
    }

	// Update is called once per frame
	void Update()
	{

	}

	public void OnConnect()
	{
		SampleDebuger.Log("game connected");
		GameData.Instance().Reset();

		GameProto.PlayerRequestGameEnter oRequest = new GameProto.PlayerRequestGameEnter();

		oRequest.QwPlayerId = PlayerData.Instance().proPlayerId;

		SysUtil.SendMessage(m_pSession, oRequest, "GameProto.PlayerRequestGameEnter");
	}

	public void OnClose()
	{
		SampleDebuger.LogError("session close");
	}
	public void OnError(uint dwErrorNo)
	{
		SampleDebuger.LogError("session error " + dwErrorNo.ToString());
	}

	public void OnDestroy()
	{
		if (m_pSession != null)
		{
			m_pSession.pro_cbOnConnect.Remove(OnConnect);
			m_pSession.pro_cbfOnError.Remove(OnError);
			m_pSession.pro_cbOnClose.Remove(OnClose);

			m_pSession.UnRegistMessage("GameProto.PlayerRequestGameTest");
		}
		H5Manager.Instance().GetGameSessionResetCallBack().Remove(OnGameSessionReset);
	}

	int dw1 = 0;
	public void OnPlayerRequestGameTest(byte[] pBuf)
	{
		GameProto.PlayerRequestGameTest oTest = GameProto.PlayerRequestGameTest.Parser.ParseFrom(pBuf);
		if (oTest == null)
		{
			SampleDebuger.LogYellow("OnTest error parse");
			return;
		}

		SampleDebuger.Log(oTest.SzTest.ToString());

		oTest.SzTest = String.Format("{0}, {1}, {2}, {3}, {4}, {5}",
			"sessionobject.cs", 106, "SessionObject::OnRecv", dw1++,
			ToString(), DateTime.Now.ToLocalTime().ToString());

		SysUtil.SendMessage(m_pSession, oTest, "GameProto.PlayerRequestGameTest");
	}

	public void OnGameAckPlayerEnter(byte[] pBuf)
	{
		GameProto.GameAckPlayerEnter oRet = GameProto.GameAckPlayerEnter.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("OnGameAckPlayerEnter error parse");
			return;
		}
		SampleDebuger.Log("game enter : " + oRet.DwResult.ToString());

		TetrisDataManager.Instance().SetOwner(PlayerData.Instance().proPlayerId);
	}

	public void OnGameNotifyPlayerPrepareTime(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerPrepareTime oRet = GameProto.GameNotifyPlayerPrepareTime.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("OnGameNotifyPlayerPrepareTime error parse");
			return;
		}
		SampleDebuger.Log("game prepare time left : " + oRet.DwLeftTime.ToString());

		PrepareTime.SetPrepareTime(oRet.DwLeftTime);
	}

	public void OnGameNotifyPlayerGameReadyTime(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerGameReadyTime oRet = GameProto.GameNotifyPlayerGameReadyTime.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("GameNotifyPlayerGameReadyTime error parse");
			return;
		}
		SampleDebuger.Log("game ready time left : " + oRet.DwLeftTime.ToString());

		ReadyTime.SetReadyTime(oRet.DwLeftTime);
    }

	public void OnGameNotifyPlayerGameConfig(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerGameConfig oRet = GameProto.GameNotifyPlayerGameConfig.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("OnGameNotifyPlayerGameConfig error parse");
			return;
		}

		TetrisData.InitGameConfig(oRet);
	}

	public void OnGameNotifyPlayerGameRoleData(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerGameRoleData oRet = GameProto.GameNotifyPlayerGameRoleData.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnGameNotifyPlayerGameRoleData error parse");
			return;
		}
	}

	public void OnGameNotifyPlayerGameSceneInfo(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerGameSceneInfo oRet = GameProto.GameNotifyPlayerGameSceneInfo.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("OnGameNotifyPlayerGameSceneInfo error parse");
			return;
		}

		SampleDebuger.LogBlue("game scene state : " + oRet.State.ToString());

		for (int i = 0; i < oRet.Players.Count; i++)
		{
			if (oRet.Players[i].QwPlayerId != 0)
			{
				SampleDebuger.LogBlue(string.Format("game scene info : [{0}, {1}, {2}, {3}]",
					oRet.Players[i].QwPlayerId.ToString(), oRet.Players[i].SzNickName, oRet.Players[i].SzAvatar, oRet.Players[i].DwSex.ToString()));

				GameData.Instance().SetSlotId((uint)i, oRet.Players[i].QwPlayerId);
				GameData.Instance().SetRoleDate(oRet.Players[i].QwPlayerId, oRet.Players[i]);

				TetrisDataManager.Instance().SetPlayer(oRet.Players[i].QwPlayerId);
			}
		}

		if (UnityEngine.SceneManagement.SceneManager.GetActiveScene().name != SysUtil.GetScesneNameBySceneState(oRet.State))
		{
			AssetBundleLoader.Instance().LoadLevelAsset(SysUtil.GetScesneNameBySceneState(oRet.State), delegate ()
				{
					SampleDebuger.LogBlue("scene info load level : " + SysUtil.GetScesneNameBySceneState(oRet.State));
				}
			);
		}
	}

	public void OnGameNotifyPlayerGameState(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerGameState oRet = GameProto.GameNotifyPlayerGameState.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("OnGameNotifyPlayerGameState error parse");
			return;
		}
		SampleDebuger.LogBlue("game state : " + oRet.State.ToString());

		GameData.Instance().SetGameSceneState(oRet.State);

		if (UnityEngine.SceneManagement.SceneManager.GetActiveScene().name != SysUtil.GetScesneNameBySceneState(oRet.State))
		{
			AssetBundleLoader.Instance().LoadLevelAsset(SysUtil.GetScesneNameBySceneState(oRet.State), delegate ()
				{
					SampleDebuger.LogBlue("game state load level : " + SysUtil.GetScesneNameBySceneState(oRet.State));
				}
			);
		}

		switch (oRet.State)
		{
			case GameProto.EGameSceneState.EssNone:
				break;
			case GameProto.EGameSceneState.EssPrepare:
			case GameProto.EGameSceneState.EssGameReady:
				{
				}
				break;
			case GameProto.EGameSceneState.EssGaming:
				{
						ReadyTime.SetGameBegin();
				}
				break;
			case GameProto.EGameSceneState.EssTransact:
				{
				}
				break;
			default:
				{
					SampleDebuger.LogError("error game state : " + ((uint)(oRet.State)).ToString());
				}
				break;
		}

		TetrisData.SetGameSceneState(oRet.State);
	}

	public void OnGameNotifyPlayerGameInitTetris(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerGameInitTetris oRet = GameProto.GameNotifyPlayerGameInitTetris.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("GameNotifyPlayerGameInitTetris error parse");
			return;
		}
		TetrisData pTetrisData = TetrisDataManager.Instance().GetTetrisData(oRet.DwPlayerId);
		if (pTetrisData == null)
		{
			SampleDebuger.LogYellow("can't find tetris data player id : " + oRet.DwPlayerId.ToString());
			return;
		}
		pTetrisData.Sync(oRet);
	}

	public void OnGameNotifyPlayerGameTetrisData(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerGameTetrisData oRet = GameProto.GameNotifyPlayerGameTetrisData.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("GameNotifyPlayerGameInitTetris error parse");
			return;
		}
		TetrisData pTetrisData = TetrisDataManager.Instance().GetTetrisData(oRet.DwPlayerId);
		if (pTetrisData == null)
		{
			SampleDebuger.LogYellow("can't find tetris data player id : " + oRet.DwPlayerId.ToString());
			return;
		}
		pTetrisData.Sync(oRet);
	}

	public void OnGameNotifyPlayeMove(byte[] pBuf)
	{
		GameProto.GameNotifyPlayeMove oRet = GameProto.GameNotifyPlayeMove.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("GameNotifyPlayeMove error parse");
			return;
		}

		TetrisData pTetrisData = TetrisDataManager.Instance().GetTetrisData(oRet.DwPlayerId);
		if (pTetrisData == null)
		{
			SampleDebuger.LogYellow("can't find tetris data player id : " + oRet.DwPlayerId.ToString());
			return;
		}

		if (!pTetrisData.TetrisOperator(oRet.EDirection))
		{
			SampleDebuger.LogYellow("error rotation : " + oRet.EDirection.ToString());
		}

		SampleDebuger.LogBlue(string.Format("player : {0} move dir {1}", oRet.DwPlayerId.ToString(), oRet.EDirection.ToString()));
	}

	public void OnGameNotifyPlayeRotation(byte[] pBuf)
	{
		GameProto.GameNotifyPlayeRotation oRet = GameProto.GameNotifyPlayeRotation.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("GameNotifyPlayeRotation error parse");
			return;
		}

		TetrisData pTetrisData = TetrisDataManager.Instance().GetTetrisData(oRet.DwPlayerId);
		if (pTetrisData == null)
		{
			SampleDebuger.LogYellow("can't find tetris data player id : " + oRet.DwPlayerId.ToString());
			return;
		}

		if (!pTetrisData.TetrisOperator(oRet.EDirection))
		{
			SampleDebuger.LogYellow("error rotation : " + oRet.EDirection.ToString());
		}
		SampleDebuger.LogBlue(string.Format("player : {0} rot dir {1}", oRet.DwPlayerId.ToString(), oRet.EDirection.ToString()));
	}

	public void OnGameNotifyPlayerNextTetris(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerNextTetris oRet = GameProto.GameNotifyPlayerNextTetris.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("GameNotifyPlayerNextTetris error parse");
			return;
		}
		TetrisData pTetrisData = TetrisDataManager.Instance().GetTetrisData(oRet.DwPlayerId);
		if (pTetrisData == null)
		{
			SampleDebuger.LogYellow("can't find tetris data player id : " + oRet.DwPlayerId.ToString());
			return;
		}
		pTetrisData.Sync(oRet);
	}

	public void OnGameNotifyPlayerDead(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerDead oRet = GameProto.GameNotifyPlayerDead.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("GameNotifyPlayerDead error parse");
			return;
		}
		SampleDebuger.LogGreen("player : " + oRet.DwPlayerId.ToString() + " dead");
	}

	public void OnGameNotifyPlayeGameEnd(byte[] pBuf)
	{
		GameProto.GameNotifyPlayeGameEnd oRet = GameProto.GameNotifyPlayeGameEnd.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.LogYellow("OnGameNotifyPlayeGameEnd error parse");
			return;
		}
		SampleDebuger.LogGreen("game end!!!!!!!!!");
	}

	public SessionObject proSession { get{ return m_pSession; } }
	[SerializeField]
	SessionObject m_pSession = null;
	[SerializeField]
	GameData m_pGameData = GameData.Instance();
}
