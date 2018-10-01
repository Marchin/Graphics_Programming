#pragma once
#include "Headers\GameBase.h"
#include "Headers\Triangle.h"
#include "Headers\ColorSquare.h"
#include "Headers\Circle.h"

class Game: public GameBase {
public:
	Game();
	~Game();
	
private:
	bool OnStart() override;
	bool OnStop() override;
	bool OnUpdate() override;
	int m_counter;
	int m_sidesCounter;
	Triangle* m_pTriangle;
	ColorSquare* m_pColorSquare;
	Circle* m_pCircle;
};
