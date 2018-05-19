#ifndef __Player_H__
#define __Player_H__


class CPlayerBase
{
public:
	CPlayerBase();
	virtual ~CPlayerBase();
};

class CCommonPlayer : public CPlayerBase
{
public:
	CCommonPlayer();

	~CCommonPlayer();

private:

};



#endif // __Player_H__
