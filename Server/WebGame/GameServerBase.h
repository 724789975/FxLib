#ifndef __GameServerBase_H__
#define __GameServerBase_H__

class CGameServerBase
{
public:
	CGameServerBase();
	virtual ~CGameServerBase();

	void GetLoadPressure(unsigned short& wCPU, unsigned short& wMEMP, unsigned short& wMEM);
};

#endif	//!__GameServerBase_H__
