#pragma once

#if !defined(DATADIR)
	#define DATADIR ""
#endif

// TODO(thismarvin): f64 GameGetFps(void);
// TODO(thismarvin): void GameRun(const GameConfig* config);

void GameInitialize(void);
void GameUpdate(void);
void GameDraw(void);
void GameRun(void);
