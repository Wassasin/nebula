#include "glfwcontext.hpp"

#include <cmath>
#include <iostream>

#include "gl.hpp"

#include "glm_include.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "../util/scope_guard.hpp"

glfwcontext::glfwcontext()
: rendercontext()
{}

std::map<GLFWwindow*, bool> focus_map;

void focus_callback(GLFWwindow* window, int status)
{
	focus_map[window] = (status == GL_TRUE);
}

void glfwcontext::run(int, char**)
{
	if(!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");

	scope_guard glfw_guard([]() { glfwTerminate(); });

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	m_size.first = mode->width;
	m_size.second = mode->height;

	GLFWwindow* window = glfwCreateWindow(size().first, size().second, "Nebula", NULL, NULL);
	if(window == NULL)
		throw std::runtime_error("Failed to open GLFW window.");

	glfwMakeContextCurrent(window);
	glfwSetWindowFocusCallback(window, &focus_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorPos(window, m_size.first/2, m_size.second/2);
	glfwSwapInterval(1);

	glewExperimental = true; // Needed for core profile
	gl::init();

	if(glGetError() == GL_INVALID_ENUM)
		std::cerr << "Swallowing false GL_INVALID_ENUM" << std::endl;

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	for(const callback_t& f : m_cbs[rcphase::init])
		f(*this);

	double last_time = glfwGetTime();
	do
	{
		double current_time = glfwGetTime();
		float delta_time = current_time - last_time;
		float fps = -1.0;
		if(delta_time > (1.0f / 62.0f))
			fps = 1.0f / delta_time;

		process_input(delta_time, window);

		double input_time = glfwGetTime();

		for(const callback_t& f : m_cbs[rcphase::update])
			f(*this);

		double update_time = glfwGetTime();

		for(const callback_t& f : m_cbs[rcphase::draw])
			f(*this);

		double draw_time = glfwGetTime();

		std::cout << fps << " FPS\t | "
				  << (input_time - current_time) << " input\t | "
				  << (update_time - input_time) << " update\t | "
				  << (draw_time - input_time) << " draw" << std::endl;

		glfwSwapBuffers(window);
		glfwPollEvents();

		last_time = current_time;
	}
	while(
		glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0
	);

	for(const callback_t& f : m_cbs[rcphase::cleanup])
		f(*this);
}

void glfwcontext::process_input(float delta, GLFWwindow* window)
{
	constexpr float speed = 0.5f;
	constexpr float mouseSpeed = 0.0005f;

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	glfwSetCursorPos(window, m_size.first/2, m_size.second/2);

	{
		auto focus_it = focus_map.find(window);
		if(focus_it == focus_map.end() || !focus_it->second)
			return;
	}

	camera.rotation.x += mouseSpeed * float(m_size.first/2 - xpos);
	camera.rotation.y = glm::clamp(camera.rotation.y + mouseSpeed * float(m_size.second/2 - ypos), -0.499f * (GLfloat)M_PI, 0.499f * (GLfloat)M_PI);

	glm::vec3 direction(
		glm::cos(camera.rotation.y) * glm::sin(camera.rotation.x),
		glm::sin(camera.rotation.y),
		glm::cos(camera.rotation.y) * glm::cos(camera.rotation.x)
	);

	glm::vec3 right(
		glm::sin(camera.rotation.x - M_PI/2.0f),
		0.0f,
		glm::cos(camera.rotation.x - M_PI/2.0f)
	);

	if(glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS)
		camera.position += direction * delta * speed;

	if(glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS)
		camera.position -= direction * delta * speed;

	if(glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS)
		camera.position += right * delta * speed;

	if(glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS)
		camera.position -= right * delta * speed;
}
