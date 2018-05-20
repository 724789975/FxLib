#ifndef __Player_H__
#define __Player_H__

class CPlayerSession;
class CPlayerBase
{
public:
	CPlayerBase();
	virtual ~CPlayerBase();

	void SetPlayerSession(CPlayerSession* pPlayerSession) { m_pPlayerSession = pPlayerSession; }
	CPlayerSession* GetPlayerSession() { return m_pPlayerSession; }
	
protected:
	CPlayerSession* m_pPlayerSession;
};

class CCommonPlayer : public CPlayerBase
{
public:
	CCommonPlayer();

	~CCommonPlayer();

private:

};



#endif // __Player_H__
