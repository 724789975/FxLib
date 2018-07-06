using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TeamPlayer : MonoBehaviour {

	// Use this for initialization
	void Start ()
	{
	}
	
	// Update is called once per frame
	void Update ()
	{
	}

	public void SetPlayerId(ulong qwPlayerId)
	{
		m_qwPlayerId = qwPlayerId;

		m_pButton.onClick.AddListener(delegate ()
		{
			InvitePlayer();
		});

		m_txtPlayerId.text = "invite : " + m_qwPlayerId.ToString();
	}

	public void InvitePlayer()
	{
		GameProto.PlayerRequestLoginInviteTeam oTeam = new GameProto.PlayerRequestLoginInviteTeam();
		oTeam.QwPlayerId = m_qwPlayerId;
		byte[] pData = new byte[1024];
		FxNet.NetStream pStream = new FxNet.NetStream(FxNet.NetStream.ENetStreamType.ENetStreamType_Write, pData, 1024);
		pStream.WriteString("GameProto.PlayerRequestLoginInviteTeam");
		byte[] pProto = new byte[oTeam.CalculateSize()];
		Google.Protobuf.CodedOutputStream oStream = new Google.Protobuf.CodedOutputStream(pProto);
		oTeam.WriteTo(oStream);
		pStream.WriteData(pProto, (uint)pProto.Length);

		H5Manager.Instance().GetLoginSession().Send(pData, 1024 - pStream.GetLeftLen());

		//m_pSession.Send(pData, 1024 - pStream.GetLeftLen());
	}

	[SerializeField]
	ulong m_qwPlayerId;
	[SerializeField]
	UnityEngine.UI.Text m_txtPlayerId = null;
	[SerializeField]
	UnityEngine.UI.Button m_pButton = null;
}
