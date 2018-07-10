using System;
using System.Collections;
using System.Collections.Generic;

[System.Serializable]
public class PlayerData : Singleton<PlayerData>
{
	public void SetPlatform(string szPlatform) { m_szPlatform = szPlatform; }
	public void SetName(string szName) { m_szName = szName; }
	public void SetHeadImage(string szHeadImage) { m_szHeadImage = szHeadImage; }
	public void SetHeadTex(UnityEngine.Texture2D tex) { m_texHeadImage = tex; }
	public void SetSex(uint dwSex) { m_dwSex = dwSex; }
	public void SetAccessToken(string szAccessToken) { m_szAccessToken = szAccessToken; }
	public void SetExpiresDate(uint dwExpiresDate) { m_dwExpiresDate = dwExpiresDate; }
	public void SetOpenId(string szOpenId) { m_szOpenId = szOpenId; }
	public void SetBalance(uint dwBalance) { m_dwBalance = dwBalance; }
	public void SetToken(string szToken) { m_szToken = szToken; }

	public void SetGameIp(string szGameIp) { m_szGameIp = szGameIp; }
	public void SetGamePort(ushort wGamePort) { m_wGamePort = wGamePort; }

	public void SetPlayerId(UInt64 qwUserId) { m_qwPlayerId = qwUserId; }

	public string proPlatform { get { return m_szPlatform; } }
	public string proName { get { return m_szName; } }
	public string proHeadImage { get { return m_szHeadImage; } }
	public UnityEngine.Texture2D proHeadTex { get { return m_texHeadImage; } }
	public uint proSex { get { return m_dwSex; } }
	public string proAccessToken { get { return m_szAccessToken; } }
	public uint proExpiresDate { get { return m_dwExpiresDate; } }
	public string proOpenId { get { return m_szOpenId; } }
	public uint proBalance { get { return m_dwBalance; } }
	public string proToken{ get { return m_szToken; } }

	public string proGameIp { get { return m_szGameIp; } }
	public ushort proGamePort { get { return m_wGamePort; } }

	public UInt64 proPlayerId { get { return m_qwPlayerId; } }

	[UnityEngine.SerializeField]
	string m_szPlatform = "";
	[UnityEngine.SerializeField]
	string m_szName = "";
	[UnityEngine.SerializeField]
	string m_szHeadImage = "";
	[UnityEngine.SerializeField]
	uint m_dwSex = 0;
	[UnityEngine.SerializeField]
	string m_szAccessToken = "";
	[UnityEngine.SerializeField]
	uint m_dwExpiresDate = 0;
	[UnityEngine.SerializeField]
	string m_szOpenId = "";
	[UnityEngine.SerializeField]
	uint m_dwBalance = 0;
	[UnityEngine.SerializeField]
	string m_szToken = "";

	[UnityEngine.SerializeField]
	string m_szGameIp = "";
	[UnityEngine.SerializeField]
	ushort m_wGamePort = 0;

	[UnityEngine.SerializeField]
	UInt64 m_qwPlayerId = 0;

	[UnityEngine.SerializeField]
	UnityEngine.Texture2D m_texHeadImage;
}
