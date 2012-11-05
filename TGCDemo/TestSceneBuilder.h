#pragma once
#include "basescenebuilder.h"

class BaseGeometry;
class App;

class TestSceneBuilder :
	public BaseSceneBuilder
{
public:
	explicit TestSceneBuilder(MainApp*);


	~TestSceneBuilder(void);

	bool Init();
};

