#pragma once

#include <Window.h>
#include <Engine.h>

class Game
{
public:
	Game();
	~Game();

	void Run();

private:
	void InitSystems();

	RetuEngine::Motor m_engine;
};

