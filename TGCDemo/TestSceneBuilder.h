#pragma once
#include "basescenebuilder.h"

class BaseGeometry;
class App;

class TestSceneBuilder :
	public BaseSceneBuilder
{
public:
	explicit TestSceneBuilder(App*);


	~TestSceneBuilder(void);

	bool Init();
};

