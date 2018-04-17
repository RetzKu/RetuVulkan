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

	RetuEngine::Engine m_engine;
};

