#pragma once
#ifndef __GLUTILS_PRECOMPILED_H__
#define __GLUTILS_PRECOMPILED_H__


#include "GLUtilsDefines.h"
#include <exception>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <memory>
#include "windows.h"

#ifdef USETHREADING
#include <mutex>
#include <thread>
#endif

#define GLM_PRECISION_HIGHP_DOUBLE
#include <glm\glm.hpp>
using namespace glm;

#include <GL/glew.h>


#endif