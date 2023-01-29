#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Core::Log
{

	void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
}