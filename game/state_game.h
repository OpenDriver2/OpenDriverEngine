#pragma once
#include "sys/sys_state.h"

class CState_Game : public CAppStateBase
{
public:
	int			GetType() const { return APP_STATE_MAIN_GAMELOOP; }

	// when changed to this state
	// @from - used to transfer data
	void		OnEnter(CAppStateBase* from) override;

	// when the state changes to something
	// @to - used to transfer data
	void		OnLeave(CAppStateBase* to) override;

	// when 'false' returned the next state goes on
	bool		Update(float fDt) override;

	void		HandleKeyPress(int key, bool down) override;
	void		HandleMouseClick(int x, int y, int buttons, bool down) override;
	void		HandleMouseMove(int x, int y, float deltaX, float deltaY) override;
	void		HandleMouseWheel(int x, int y, int scroll) override;
	void		HandleJoyAxis(short axis, short value) override;
protected:

};

extern CStaticAutoPtr<CState_Game> g_State_Game;