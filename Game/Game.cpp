#include "Game.h"

Game::Game()
{
}

Game::~Game()
{
}

void Game::Run()
{
	InitSystems();
}

void Game::InitSystems()
{
	m_engine.InitVulkan();
}

