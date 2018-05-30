using System;
using System.Collections;
using System.Collections.Generic;

[System.Serializable]
public class PlayerData : Singleton<PlayerData>
{
	public void SetPlatform(string szPlatform) { m_szPlatform = szPlatform; }
	public void SetName(string szName) { m_szName = szName; }
	public void SetHeadImage(string szHeadImage) { m_szHeadImage = szHeadImage; }
	public void SetSex(uint dwSex) { m_dwSex = dwSex; }
	public void SetAccessToken(string szAccessToken) { m_szAccessToken = szAccessToken; }
	public void SetExpiresDate(uint dwExpiresDate) { m_dwExpiresDate = dwExpiresDate; }
	public void SetOpenId(string szOpenId) { m_szOpenId = szOpenId; }

	public void SetGameIp(string szGameIp) { m_szGameIp = szGameIp; }
	public void SetGamePort(ushort wGamePort) { m_wGamePort = wGamePort; }

	public void SetPlayerId(UInt64 qwUserId) { m_qwPlayerId = qwUserId; }

	public string proPlatform { get { return m_szPlatform; } }
	public string proName { get { return m_szName; } }
	public string proHeadImage { get { return m_szHeadImage; } }
	public uint proSex { get { return m_dwSex; } }
	public string proAccessToken { get { return m_szAccessToken; } }
	public uint proExpiresDate { get { return m_dwExpiresDate; } }
	public string proOpenId { get { return m_szOpenId; } }

	public string proGameIp { get { return m_szGameIp; } }
	public ushort proGamePort { get { return m_wGamePort; } }

	public UInt64 proPlayerId { get { return m_qwPlayerId; } }

	public string m_szPlatform = "";
	public string m_szName = "";
	public string m_szHeadImage = "";
	public uint m_dwSex = 0;
	public string m_szAccessToken = "";
	public uint m_dwExpiresDate = 0;
	public string m_szOpenId = "";

	public string m_szGameIp = "";
	public ushort m_wGamePort = 0;

	public UInt64 m_qwPlayerId = 0;
}
