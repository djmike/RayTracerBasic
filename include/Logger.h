#pragma once

#include <iostream>
#include "cinder/app/App.h"

class Logger
{
public:
	static std::ostream& log()
	{
		return ci::app::console();
	}
};