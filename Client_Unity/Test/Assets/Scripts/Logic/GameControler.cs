using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using UnityEngine;

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
		m_pSession.m_pfOnConnect.Add(OnConnect);
		m_pSession.m_pfOnError.Add(OnError);
		m_pSession.m_pfOnClose.Add(OnClose);

		m_pSession.RegistMessage("GameProto.PlayerRequestGameTest", OnPlayerRequestGameTest);
		m_pSession.RegistMessage("GameProto.GameAckPlayerEnter", OnGameAckPlayerEnter);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerPrepareTime", OnGameNotifyPlayerPrepareTime);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameReadyTime", OnGameNotifyPlayerGameReadyTime);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameConfig", OnGameNotifyPlayerGameConfig);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameRoleData", OnGameNotifyPlayerGameRoleData);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameSceneInfo", OnGameNotifyPlayerGameSceneInfo);
		m_pSession.RegistMessage("GameProto.GameNotifyPlayerGameState", OnGameNotifyPlayerGameState);
    }

	// Update is called once per frame
	void Update()
	{

	}

	public void OnConnect()
	{
		SampleDebuger.Log("game connected");
		GameProto.PlayerRequestGameEnter oRequest = new GameProto.PlayerRequestGameEnter();

		oRequest.QwPlayerId = PlayerData.Instance().proPlayerId;

		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString("GameProto.PlayerRequestGameEnter");
		byte[] pProto = new byte[oRequest.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oRequest.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
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
			m_pSession.m_pfOnConnect.Remove(OnConnect);
			m_pSession.m_pfOnError.Remove(OnError);
			m_pSession.m_pfOnClose.Remove(OnClose);

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
			SampleDebuger.Log("OnTest error parse");
			return;
		}

		SampleDebuger.Log(oTest.SzTest.ToString());

		oTest.SzTest = String.Format("{0}, {1}, {2}, {3}, {4}, {5}",
			"sessionobject.cs", 106, "SessionObject::OnRecv", dw1++,
			ToString(), DateTime.Now.ToLocalTime().ToString());

		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString("GameProto.PlayerRequestGameTest");
		byte[] pProto = new byte[oTest.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oTest.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
	}

	public void OnGameAckPlayerEnter(byte[] pBuf)
	{
		GameProto.GameAckPlayerEnter oRet = GameProto.GameAckPlayerEnter.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnGameAckPlayerEnter error parse");
			return;
		}
		SampleDebuger.Log("game enter : " + oRet.DwResult.ToString());
	}

	public void OnGameNotifyPlayerPrepareTime(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerPrepareTime oRet = GameProto.GameNotifyPlayerPrepareTime.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnGameNotifyPlayerPrepareTime error parse");
			return;
		}
		SampleDebuger.Log("game prepare time left : " + oRet.DwLeftTime.ToString());
	}

	public void OnGameNotifyPlayerGameReadyTime(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerGameReadyTime oRet = GameProto.GameNotifyPlayerGameReadyTime.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("GameNotifyPlayerGameReadyTime error parse");
			return;
		}
		SampleDebuger.Log("game ready time left : " + oRet.DwLeftTime.ToString());
	}

	public void OnGameNotifyPlayerGameConfig(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerGameConfig oRet = GameProto.GameNotifyPlayerGameConfig.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnGameNotifyPlayerGameConfig error parse");
			return;
		}
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
			SampleDebuger.Log("OnGameNotifyPlayerGameSceneInfo error parse");
			return;
		}

		switch (oRet.CommonSceneInfo.SceneInfo.State)
		{
			case GameProto.EGameSceneState.EssNone:
			case GameProto.EGameSceneState.EssPrepare:
				{
					if (UnityEngine.SceneManagement.SceneManager.GetActiveScene().name == GameConstant.g_szGamePrepareScene)
					{
						break;
					}
					AssetBundleLoader.Instance().LoadLevelAsset(GameConstant.g_szGamePrepareScene, delegate ()
						{
						}
					);
				}
				break;
			case GameProto.EGameSceneState.EssGameReady:
			case GameProto.EGameSceneState.EssGaming:
			case GameProto.EGameSceneState.EssTransact:
				{
					if (UnityEngine.SceneManagement.SceneManager.GetActiveScene().name == GameConstant.g_szGameScene)
					{
						break;
					}
					AssetBundleLoader.Instance().LoadLevelAsset(GameConstant.g_szGameScene, delegate ()
						{
						}
					);
				}
				break;
			default:
				{
					SampleDebuger.LogError("error game state : " + ((uint)(oRet.CommonSceneInfo.SceneInfo.State)).ToString());
				}
				break;
		}
	}

	public void OnGameNotifyPlayerGameState(byte[] pBuf)
	{
		GameProto.GameNotifyPlayerGameState oRet = GameProto.GameNotifyPlayerGameState.Parser.ParseFrom(pBuf);
		if (oRet == null)
		{
			SampleDebuger.Log("OnGameNotifyPlayerGameState error parse");
			return;
		}
		SampleDebuger.LogBlue("game state : " + oRet.State.ToString());

		switch (oRet.State)
		{
			case GameProto.EGameSceneState.EssNone:
			case GameProto.EGameSceneState.EssPrepare:
				{
					if (UnityEngine.SceneManagement.SceneManager.GetActiveScene().name == GameConstant.g_szGamePrepareScene)
					{
						break;
					}
					AssetBundleLoader.Instance().LoadLevelAsset(GameConstant.g_szGamePrepareScene, delegate ()
						{
						}
					);
				}
				break;
			case GameProto.EGameSceneState.EssGameReady:
			case GameProto.EGameSceneState.EssGaming:
			case GameProto.EGameSceneState.EssTransact:
				{
					if (UnityEngine.SceneManagement.SceneManager.GetActiveScene().name == GameConstant.g_szGameScene)
					{
						break;
					}
					AssetBundleLoader.Instance().LoadLevelAsset(GameConstant.g_szGameScene, delegate ()
						{
						}
					);
				}
				break;
			default:
				{
					SampleDebuger.LogError("error game state : " + ((uint)(oRet.State)).ToString());
				}
				break;
		}
	}

	public SessionObject m_pSession;
}
