#include "Renderer.h"
#include <algorithm>
//my code start
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//my code end

Camera* Renderer::m_camera = new Camera();

Lighting* Renderer::m_lightings = new Lighting();

nanogui::Screen* Renderer::m_nanogui_screen = nullptr;

bool Renderer::keys[1024];

//my code start
std::string objectname;
nanogui::Color object_color(0.416f, 0.353f, 0.806f, 1.0f);
nanogui::Color light_color(1.0f, 1.0f, 1.0f, 0.0f);
nanogui::Color direction_light_ambient_color(0.0f, 0.0f, 0.0f, 1.0f);
nanogui::Color direction_light_diffuse_color(0.0f, 0.0f, 0.0f, 1.0f);
nanogui::Color direction_light_specular_color(0.0f, 0.0f, 0.0f, 1.0f);
nanogui::Color point_light_ambient_color(0.0f, 0.0f, 0.0f, 1.0f);
nanogui::Color point_light_diffuse_color(0.0f, 0.0f, 0.0f, 1.0f);
nanogui::Color point_light_specular_color(0.0f, 0.0f, 0.0f, 1.0f);
int object_shininess = 32;
bool direction_light_status = false;
bool point_light_status = false;
bool point_light_rotate_X = false;
bool point_light_rotate_Y = false;
bool point_light_rotate_Z = false;
glm::vec3 direction_light_direction(0.0f, -1.0f, -1.0f);
bool texture_status = false;
bool normal_map_status = false;
std::string diffuse_image_path = "";
std::string normal_image_path = "";
GLuint diffuse_image_id;
GLuint normal_image_id;


enum object_select_range
{
	item_rock = 0,
	item_cube,
	item_cyborg,
	item_twocube
};
object_select_range r = item_rock;

enum object_render_type_range 
{
	item_points = 0,
	item_lines,
	item_triangles
};
object_render_type_range rt = item_lines;

enum culling_type_range 
{
	item_cw = 0,
	item_CCW
};
culling_type_range cr = item_cw;


enum shading_type_range 
{
	item_smooth = 0,
	item_flat
};
shading_type_range sr = item_smooth;
shading_type_range temp_sr = sr;

enum depth_type_range
{
	item_less = 0,
	item_always
};
depth_type_range dr = item_less;

float rotate_value = 2.0f;
glm::vec3 obj_position;
float z_near = 0.1;
float z_far = 100;
Object target("./objs/rock.obj");

//my code end

Renderer::Renderer()
{
}


Renderer::~Renderer()
{
}

void Renderer::nanogui_init(GLFWwindow* window)
{
	m_nanogui_screen = new nanogui::Screen();
	m_nanogui_screen->initialize(window, true);

	glViewport(0, 0, m_camera->width, m_camera->height);

	//glfwSwapInterval(0);
	//glfwSwapBuffers(window);

	//screen->setPosition(Eigen::Vector2i(-width/2 + 200, -height/2 + 300));

	//my code start
	// Create nanogui gui
	nanogui::FormHelper* gui_1 = new nanogui::FormHelper(m_nanogui_screen);
	nanogui::ref<nanogui::Window> nanoguiWindow_1 = gui_1->addWindow(Eigen::Vector2i(0, 0), "Nanogui control bar_1");
	
	gui_1->addGroup("Position");
	m_camera->ori_position = target.obj_center;
	m_camera->position[0] = target.obj_center[0];
	m_camera->position[1] = target.obj_center[1];
	m_camera->position[2] = 5.0f;
	static auto camera_x_widget = gui_1->addVariable("X", m_camera->position[0]);
	static auto camera_y_widget = gui_1->addVariable("Y", m_camera->position[1]);
	static auto camera_z_widget = gui_1->addVariable("Z", m_camera->position[2]);
	camera_x_widget->setSpinnable(true);
	camera_y_widget->setSpinnable(true);
	camera_z_widget->setSpinnable(true);

	gui_1->addGroup("Rotate");
	gui_1->addVariable("Rotate valus", rotate_value)->setSpinnable(true);
	gui_1->addButton("Rotate right+", []() {
		m_camera->rotate_x(rotate_value);
		});
	gui_1->addButton("Rotate right-", []() {
		m_camera->rotate_x((-1.0) * rotate_value);
		});
	gui_1->addButton("Rotate up+", []() {
		m_camera->rotate_y(rotate_value);
		});
	gui_1->addButton("Rotate up-", []() {
		m_camera->rotate_y((-1.0) * rotate_value);
		});
	gui_1->addButton("Rotate front+", []() {
		m_camera->rotate_z(rotate_value);
		});
	gui_1->addButton("Rotate front-", []() {
		m_camera->rotate_z((-1.0) * rotate_value);
		});

	gui_1->addButton("Reset Camera", []() {
		m_camera->reset();
		m_camera->ori_position = target.obj_center;
		m_camera->position[0] = target.obj_center[0];
		m_camera->position[1] = target.obj_center[1];
		m_camera->position[2] = 5.0f;
		camera_x_widget->setValue(m_camera->position[0]);
		camera_y_widget->setValue(m_camera->position[1]);
		camera_z_widget->setValue(m_camera->position[2]);
		});

	gui_1->addGroup("Configuration");
	gui_1->addVariable("z near", z_near)->setSpinnable(true);
	gui_1->addVariable("z far", z_far)->setSpinnable(true);
	gui_1->addVariable("Render type", rt, true)->setItems({ "Point", "Line", "Triangle" });
	gui_1->addVariable("Culling type", cr, true)->setItems({ "CW", "CCW" });
	gui_1->addVariable("Shading type", sr, true)->setItems({ "SMOOTH" , "FLAT" });
	gui_1->addVariable("Depth type", dr, true)->setItems({ "LESS", "ALWAYS" });
	gui_1->addVariable("Model", r, true)->setItems({ "rock", "cube", "cyborg" , "two cube" });

	gui_1->addButton("Reload model", [this]() {
		load_models();
		m_camera->reset();
		m_camera->ori_position = target.obj_center;
		m_camera->position[0] = target.obj_center[0];
		m_camera->position[1] = target.obj_center[1];
		m_camera->position[2] = 5.0f;
		camera_x_widget->setValue(m_camera->position[0]);
		camera_y_widget->setValue(m_camera->position[1]);
		camera_z_widget->setValue(m_camera->position[2]);
		m_lightings->point_light.position = m_camera->position;
		});
	gui_1->addButton("Reset", [this]() { scene_reset(); });


	nanogui::FormHelper* gui_2 = new nanogui::FormHelper(m_nanogui_screen);
	nanogui::ref<nanogui::Window> nanoguiWindow_2 = gui_2->addWindow(Eigen::Vector2i(200, 0), "Nanogui control bar_2");
	gui_2->addGroup("Lighting");
	gui_2->addVariable("Object color:", object_color);
	gui_2->addVariable("Object shininess", object_shininess);
	gui_2->addVariable("Direction light status: ", direction_light_status);
	static auto direction_light_direction_x_widget = gui_2->addVariable("Direction light direction x:", direction_light_direction[0]);
	static auto direction_light_direction_y_widget = gui_2->addVariable("Direction light direction y:", direction_light_direction[1]);
	static auto direction_light_direction_z_widget = gui_2->addVariable("Direction light direction z:", direction_light_direction[2]);
	direction_light_direction_x_widget->setSpinnable(true);
	direction_light_direction_y_widget->setSpinnable(true);
	direction_light_direction_z_widget->setSpinnable(true);
	gui_2->addVariable("Direction light ambient color: ", direction_light_ambient_color);
	gui_2->addVariable("Direction light diffuse color: ", direction_light_diffuse_color);
	gui_2->addVariable("Direction light specular color: ", direction_light_specular_color);
	gui_2->addVariable("Point light status:", point_light_status);
	gui_2->addVariable("Point light ambient color:", point_light_ambient_color);
	gui_2->addVariable("Point light diffuse color:", point_light_diffuse_color);
	gui_2->addVariable("Point light specular color:", point_light_specular_color);
	gui_2->addVariable("Point light rotate on X", point_light_rotate_X);
	gui_2->addVariable("Point light rotate on Y", point_light_rotate_Y);
	gui_2->addVariable("Point light rotate on Z", point_light_rotate_Z);
	gui_2->addVariable("Texture status:", texture_status);
	gui_2->addVariable("Normal map status:", normal_map_status);
	gui_2->addButton("Reset Point Light", [this]() {
		m_lightings->point_light.position = m_camera->position;
		});

	//my code end

	m_nanogui_screen->setVisible(true);
	m_nanogui_screen->performLayout();

	glfwSetCursorPosCallback(window,
		[](GLFWwindow* window, double x, double y) {
			m_nanogui_screen->cursorPosCallbackEvent(x, y);
		}
	);

	glfwSetMouseButtonCallback(window,
		[](GLFWwindow*, int button, int action, int modifiers) {
			m_nanogui_screen->mouseButtonCallbackEvent(button, action, modifiers);
		}
	);

	glfwSetKeyCallback(window,
		[](GLFWwindow* window, int key, int scancode, int action, int mods) {
			//screen->keyCallbackEvent(key, scancode, action, mods);

			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
				glfwSetWindowShouldClose(window, GL_TRUE);
			if (key >= 0 && key < 1024)
			{
				if (action == GLFW_PRESS)
					keys[key] = true;
				else if (action == GLFW_RELEASE)
					keys[key] = false;
			}
			camera_x_widget->setValue(m_camera->position[0]);
			camera_y_widget->setValue(m_camera->position[1]);
			camera_z_widget->setValue(m_camera->position[2]);
		}
	);

	glfwSetCharCallback(window,
		[](GLFWwindow*, unsigned int codepoint) {
			m_nanogui_screen->charCallbackEvent(codepoint);
		}
	);

	glfwSetDropCallback(window,
		[](GLFWwindow*, int count, const char** filenames) {
			m_nanogui_screen->dropCallbackEvent(count, filenames);
		}
	);

	glfwSetScrollCallback(window,
		[](GLFWwindow*, double x, double y) {
			m_nanogui_screen->scrollCallbackEvent(x, y);
			//m_camera->ProcessMouseScroll(y);
		}
	);

	glfwSetFramebufferSizeCallback(window,
		[](GLFWwindow*, int width, int height) {
			m_nanogui_screen->resizeCallbackEvent(width, height);
		}
	);

}

void Renderer::init()
{
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

#if defined(__APPLE__)
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	m_camera->init();
	m_lightings->init();

	// Create a GLFWwindow object that we can use for GLFW's functions
	this->m_window = glfwCreateWindow(m_camera->width, m_camera->height, "Assignment 0", nullptr, nullptr);
	glfwMakeContextCurrent(this->m_window);

	glewExperimental = GL_TRUE;
	glewInit();

	nanogui_init(this->m_window);
}

void Renderer::display(GLFWwindow* window)
{
	Shader m_shader = Shader("./shader/basic.vert", "./shader/basic.frag");

	// Main frame while loop
	while (!glfwWindowShouldClose(window))
	{		
		glfwPollEvents();

		if (is_scene_reset) {
			scene_reset();
			is_scene_reset = false;
		}

		camera_move();

		m_shader.use();

		setup_uniform_values(m_shader);

		draw_scene(m_shader);

		m_nanogui_screen->drawWidgets();

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return;
}

void Renderer::run()
{
	init();
	display(this->m_window);
}

void Renderer::load_models()
{
	obj_list.clear();
	//my code start
	if (r == 0)
	{
		Object rock("./objs/rock.obj");
		target = rock;
		target.obj_name = "rock";
	}

	if (r == 1)
	{
		Object cube("./objs/cube.obj");
		target = cube;
		target.obj_name = "cube";
	}

	if (r == 2)
	{
		Object cyborg("./objs/cyborg.obj");
		target = cyborg;
		target.obj_name = "cyborg";
	}

	if (r == 3)
	{
		Object two_cubes("./objs/two_cubes.obj");
		target = two_cubes;
		target.obj_name = "two_cubes";
	}

	if (rt == 0)
	{
		target.m_render_type = RENDER_POINTS;
	}
	else if (rt == 1)
	{
		target.m_render_type = RENDER_LINES;
	}
	else if (rt == 2)
	{
		target.m_render_type = RENDER_TRIANGLES;
	}
	else
	{
		std::cout << "error in render type selection." << rt << std::endl;
	}
	bind_vaovbo(target);
	obj_list.push_back(target);

	//target.obj_center = glm::vec3(0.0f, 0.0f, 0.0f);
	m_camera->ori_position = target.obj_center;
	m_camera->position[0] = target.obj_center[0];
	m_camera->position[1] = target.obj_center[1];
	m_camera->position[2] = 5.0f;
	
	//m_lightings->point_light.position = m_camera->position;
	//my code end
}

void Renderer::texture_init()
{
	glGenTextures(1, &diffuse_image_id);
	glGenTextures(1, &normal_image_id);

	int image_width;
	int image_height;
	int chanel;
	unsigned char* diffuse_image_texture = stbi_load(diffuse_image_path.c_str(), &image_width, &image_height, &chanel, STBI_rgb);
	unsigned char* normal_image_texture = stbi_load(normal_image_path.c_str(), &image_width, &image_height, &chanel, STBI_rgb);

	if (diffuse_image_texture)
	{
		//std::cout << diffuse_image_path.c_str() << std::endl;
		glBindTexture(GL_TEXTURE_2D, diffuse_image_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, diffuse_image_texture);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(diffuse_image_texture);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glActiveTexture(GL_TEXTURE0);
	}
	else if (diffuse_image_path != "")
	{
		std::cout << "failed to load diffuse image" << std::endl;
	}

	if (normal_image_texture)
	{
		//std::cout << normal_image_path.c_str() << std::endl;
		glBindTexture(GL_TEXTURE_2D, normal_image_id);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, normal_image_texture);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(normal_image_texture);
		glBindTexture(GL_TEXTURE_2D, 0);
		//glActiveTexture(GL_TEXTURE1);
	}
	else if (normal_image_path != "")
	{
		std::cout << "failed to load normal image" << std::endl;
	}
	//glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::draw_texture()
{

}

void Renderer::draw_scene(Shader& shader)
{
	m_camera->near = z_near;
	m_camera->far = z_far;

	// Set up some basic parameters
	glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	if (dr == 0)
	{
		glDepthFunc(GL_LESS);
	}
	else if (dr == 1)
	{
		glDepthFunc(GL_ALWAYS);
	}
	else
	{
		std::cout << "error in depth type selection." << dr << std::endl;
	}

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	if (cr == 0)
	{
		glFrontFace(GL_CW);
	}
	if (cr == 1)
	{
		glFrontFace(GL_CCW);
	}
	if (rt == 0)
	{
		target.m_render_type = RENDER_POINTS;
	}
	else if (rt == 1)
	{
		target.m_render_type = RENDER_LINES;
	} 
	else if (rt == 2)
	{
		target.m_render_type = RENDER_TRIANGLES;
	}
	else
	{
		std::cout << "error in render type selection." << rt << std::endl;
	}

	m_lightings->direction_light.status = direction_light_status;
	m_lightings->direction_light.direction = direction_light_direction;
	m_lightings->direction_light.ambient = glm::vec4(direction_light_ambient_color[0], direction_light_ambient_color[1], direction_light_ambient_color[2], direction_light_ambient_color[3]);
	m_lightings->direction_light.diffuse = glm::vec4(direction_light_diffuse_color[0], direction_light_diffuse_color[1], direction_light_diffuse_color[2], direction_light_ambient_color[3]);
	m_lightings->direction_light.specular = glm::vec4(direction_light_specular_color[0], direction_light_specular_color[1], direction_light_specular_color[2], direction_light_specular_color[3]);

	m_lightings->point_light.status = point_light_status;
	m_lightings->point_light.ambient = glm::vec4(point_light_ambient_color[0], point_light_ambient_color[1], point_light_ambient_color[2], point_light_ambient_color[3]);
	m_lightings->point_light.diffuse = glm::vec4(point_light_diffuse_color[0], point_light_diffuse_color[1], point_light_diffuse_color[2], point_light_ambient_color[3]);
	m_lightings->point_light.specular = glm::vec4(point_light_specular_color[0], point_light_specular_color[1], point_light_specular_color[2], point_light_specular_color[3]);


	for (size_t i = 0; i < obj_list.size(); i++)
	{
		if (obj_list[i].obj_name == "rock" ||
			obj_list[i].obj_name == "cube" ||
			obj_list[i].obj_name == "cyborg" ||
			obj_list[i].obj_name == "two_cubes")
		{
			// Draw rock / cube / cyborg / two_cubes
			obj_list[i].obj_color = glm::vec4(object_color[0], object_color[1], object_color[2], object_color[3]);
			draw_object(shader, obj_list[i]);
		}
		else 
		{
			std::cout << "The model is not exist." << std::endl;
		}
		glm::mat4 target_model_mat = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(shader.program, "model"), 1, GL_FALSE, glm::value_ptr(target_model_mat));

	}

	//light rotate
	//std::cout << point_light_rotate_X << std::endl;
	if (point_light_rotate_X == true && point_light_rotate_Y == false && point_light_rotate_Z == false)
	{
		float ty = m_lightings->point_light.position[1];
		float tz = m_lightings->point_light.position[2];
		ty = (m_lightings->point_light.position[1] - target.obj_center[1]) * cos(glm::radians(m_lightings->point_light.angle_delta)) 
			- (m_lightings->point_light.position[2] - target.obj_center[2]) * sin(glm::radians(m_lightings->point_light.angle_delta)) 
			+ target.obj_center[1];
		tz = (m_lightings->point_light.position[2] - target.obj_center[2]) * cos(glm::radians(m_lightings->point_light.angle_delta)) 
			+ (m_lightings->point_light.position[1] - target.obj_center[1]) * sin(glm::radians(m_lightings->point_light.angle_delta))
			+ target.obj_center[2];
		m_lightings->point_light.position[1] = ty;
		m_lightings->point_light.position[2] = tz;
		
		return;
	}
	if (point_light_rotate_X == false && point_light_rotate_Y == true && point_light_rotate_Z == false)
	{
		float ty = m_lightings->point_light.position[2];
		float tz = m_lightings->point_light.position[0];
		ty = (m_lightings->point_light.position[2] - target.obj_center[2]) * cos(glm::radians(m_lightings->point_light.angle_delta))
			- (m_lightings->point_light.position[0] - target.obj_center[0]) * sin(glm::radians(m_lightings->point_light.angle_delta))
			+ target.obj_center[2];
		tz = (m_lightings->point_light.position[0] - target.obj_center[0]) * cos(glm::radians(m_lightings->point_light.angle_delta))
			+ (m_lightings->point_light.position[2] - target.obj_center[2]) * sin(glm::radians(m_lightings->point_light.angle_delta))
			+ target.obj_center[0];
		m_lightings->point_light.position[2] = ty;
		m_lightings->point_light.position[0] = tz;
		return;
	}
	if (point_light_rotate_X == false && point_light_rotate_Y == false && point_light_rotate_Z == true)
	{
		float ty = m_lightings->point_light.position[0];
		float tz = m_lightings->point_light.position[1];
		ty = (m_lightings->point_light.position[0] - target.obj_center[0]) * cos(glm::radians(m_lightings->point_light.angle_delta))
			- (m_lightings->point_light.position[1] - target.obj_center[1]) * sin(glm::radians(m_lightings->point_light.angle_delta))
			+ target.obj_center[0];
		tz = (m_lightings->point_light.position[1] - target.obj_center[1]) * cos(glm::radians(m_lightings->point_light.angle_delta))
			+ (m_lightings->point_light.position[0] - target.obj_center[0]) * sin(glm::radians(m_lightings->point_light.angle_delta))
			+ target.obj_center[1];
		m_lightings->point_light.position[0] = ty;
		m_lightings->point_light.position[1] = tz;
		return;
	}
	if (point_light_rotate_X == false && point_light_rotate_Y == false && point_light_rotate_Z == false)
	{
		//do nothing
		return;
	}
	else
	{
		//std::cout << "Wrong rotate selection." << std::endl;
		//do nothing
		return;
	}
}


void Renderer::camera_move()
{
	GLfloat current_frame = glfwGetTime();
	delta_time = current_frame - last_frame;
	last_frame = current_frame;
	// Camera controls
	if (keys[GLFW_KEY_W])
		m_camera->process_keyboard(FORWARD, delta_time);
	if (keys[GLFW_KEY_S])
		m_camera->process_keyboard(BACKWARD, delta_time);
	if (keys[GLFW_KEY_A])
		m_camera->process_keyboard(LEFT, delta_time);
	if (keys[GLFW_KEY_D])
		m_camera->process_keyboard(RIGHT, delta_time);
	if (keys[GLFW_KEY_Q])
		m_camera->process_keyboard(UP, delta_time);
	if (keys[GLFW_KEY_E])
		m_camera->process_keyboard(DOWN, delta_time);
	if (keys[GLFW_KEY_I])
		m_camera->process_keyboard(ROTATE_X_UP, delta_time);
	if (keys[GLFW_KEY_K])
		m_camera->process_keyboard(ROTATE_X_DOWN, delta_time);
	if (keys[GLFW_KEY_J])
		m_camera->process_keyboard(ROTATE_Y_UP, delta_time);
	if (keys[GLFW_KEY_L])
		m_camera->process_keyboard(ROTATE_Y_DOWN, delta_time);
	if (keys[GLFW_KEY_U])
		m_camera->process_keyboard(ROTATE_Z_UP, delta_time);
	if (keys[GLFW_KEY_O])
		m_camera->process_keyboard(ROTATE_Z_DOWN, delta_time);
	
}

void Renderer::draw_object(Shader& shader, Object& object)
{
	glBindVertexArray(object.vao);

	glUniform3f(glGetUniformLocation(shader.program, "m_object.object_color"), object.obj_color[0], object.obj_color[1], object.obj_color[2]);
	if (point_light_status == false && direction_light_status == false)
	{
		glUniform1f(glGetUniformLocation(shader.program, "m_object.shininess"), 0);

	}
	else
	{
		glUniform1f(glGetUniformLocation(shader.program, "m_object.shininess"), object.shininess);

	}

	glLineWidth(1.0);

	//if (object.m_render_type == RENDER_TRIANGLES)
	if (rt == 2)
	{
		if (object.m_obj_type == OBJ_POINTS)
		{
			std::cout << "Error: Cannot render triangles if input obj type is point\n";
			return;
		}
		if (object.m_obj_type == OBJ_TRIANGLES)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_TRIANGLES, 0, object.vao_vertices.size());
		}
	}

	//if (object.m_render_type == RENDER_LINES)
	if (rt == 1)
	{
		if (object.m_obj_type == OBJ_POINTS)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawArrays(GL_LINE_LOOP, 0, object.vao_vertices.size());
		}
		if (object.m_obj_type == OBJ_TRIANGLES)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawArrays(GL_TRIANGLES, 0, object.vao_vertices.size());
		}
	}

	//if (object.m_render_type == RENDER_POINTS)
	if (rt == 0)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);
		glDrawArrays(GL_POINTS, 0, object.vao_vertices.size());
	}
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);



	//texuture selection
	//std::cout << texture_status << "    " << normal_map_status << std::endl;

	if (texture_status == 1)
	{
		if (object.obj_name == "cube")
		{
			diffuse_image_path = "./textures/textures/cube_diffuse.png";
			//std::cout << "diffuse cube." << std::endl;
		}
		else if (object.obj_name == "cyborg")
		{
			diffuse_image_path = "./textures/textures/cyborg_diffuse.png";
		}
	}
	if (normal_map_status == 1)
	{
		if (object.obj_name == "cube")
		{
			normal_image_path = "./textures/textures/cube_normal.png";
		}
		else if (object.obj_name == "cyborg")
		{
			normal_image_path = "./textures/textures/cyborg_normal.png";
		}
	}
	
}

void Renderer::bind_vaovbo(Object& cur_obj)
{
	glGenVertexArrays(1, &cur_obj.vao);
	glGenBuffers(1, &cur_obj.vbo);

	glBindVertexArray(cur_obj.vao);

	glBindBuffer(GL_ARRAY_BUFFER, cur_obj.vbo);
	glBufferData(GL_ARRAY_BUFFER, cur_obj.vao_vertices.size() * sizeof(Object::Vertex), &cur_obj.vao_vertices[0], GL_STATIC_DRAW);

	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Object::Vertex), (GLvoid*)0);
	// Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Object::Vertex), (GLvoid*)offsetof(Object::Vertex, Normal));
	// Vertex Texture Coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Object::Vertex), (GLvoid*)offsetof(Object::Vertex, TexCoords));

	//my code start 
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Object::Vertex), (GLvoid*)offsetof(Object::Vertex, Tangent));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Object::Vertex), (GLvoid*)offsetof(Object::Vertex, BiTangent));
	//my code end

	glBindVertexArray(0);
}

void Renderer::setup_uniform_values(Shader& shader)
{
	// Camera uniform values
	GLenum err;
	err = glGetError();
	if (err != GL_NO_ERROR)
	{
		std::cout << "hey" << err << std::endl;
	}
	
	glUniform3f(glGetUniformLocation(shader.program, "camera_pos"), m_camera->position.x, m_camera->position.y, m_camera->position.z);
	
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "projection"), 1, GL_FALSE, glm::value_ptr(m_camera->get_projection_mat()));
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "view"), 1, GL_FALSE, glm::value_ptr(m_camera->get_view_mat()));

	// Light uniform values
	glUniform1i(glGetUniformLocation(shader.program, "dir_light.status"), m_lightings->direction_light.status);
	glUniform3f(glGetUniformLocation(shader.program, "dir_light.direction"), m_lightings->direction_light.direction[0], m_lightings->direction_light.direction[1], m_lightings->direction_light.direction[2]);
	glUniform3f(glGetUniformLocation(shader.program, "dir_light.ambient"), m_lightings->direction_light.ambient[0], m_lightings->direction_light.ambient[1], m_lightings->direction_light.ambient[2]);
	glUniform3f(glGetUniformLocation(shader.program, "dir_light.diffuse"), m_lightings->direction_light.diffuse[0], m_lightings->direction_light.diffuse[1], m_lightings->direction_light.diffuse[2]);
	glUniform3f(glGetUniformLocation(shader.program, "dir_light.specular"), m_lightings->direction_light.specular[0], m_lightings->direction_light.specular[1], m_lightings->direction_light.specular[2]);

	// Set current point light as camera's position
	//m_lightings->point_light.position = m_camera->position;
	glUniform1i(glGetUniformLocation(shader.program, "point_light.status"), m_lightings->point_light.status);
	glUniform3f(glGetUniformLocation(shader.program, "point_light.position"), m_lightings->point_light.position[0], m_lightings->point_light.position[1], m_lightings->point_light.position[2]);
	glUniform3f(glGetUniformLocation(shader.program, "point_light.ambient"), m_lightings->point_light.ambient[0], m_lightings->point_light.ambient[1], m_lightings->point_light.ambient[2]);
	glUniform3f(glGetUniformLocation(shader.program, "point_light.diffuse"), m_lightings->point_light.diffuse[0], m_lightings->point_light.diffuse[1], m_lightings->point_light.diffuse[2]);
	glUniform3f(glGetUniformLocation(shader.program, "point_light.specular"), m_lightings->point_light.specular[0], m_lightings->point_light.specular[1], m_lightings->point_light.specular[2]);
	glUniform1f(glGetUniformLocation(shader.program, "point_light.constant"), m_lightings->point_light.constant);
	glUniform1f(glGetUniformLocation(shader.program, "point_light.linear"), m_lightings->point_light.linear);
	glUniform1f(glGetUniformLocation(shader.program, "point_light.quadratic"), m_lightings->point_light.quadratic);

	glUniform1i(glGetUniformLocation(shader.program, "SmoothFlat"), (int)sr);
	//std::cout << sr << std::endl;

	texture_init();
	

	//if (texture_status == 1)
	//{
	glUniform1i(glGetUniformLocation(shader.program, "diffuse_switch"), (int)texture_status);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuse_image_id);
	glUniform1i(glGetUniformLocation(shader.program, "TextureDiffuse"), 0);
	//std::cout << "diffuse in" << std::endl;
//}
//if (normal_map_status == 1)
//{
	glUniform1i(glGetUniformLocation(shader.program, "normal_switch"), (int)normal_map_status);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_image_id);
	glUniform1i(glGetUniformLocation(shader.program, "TextureNormal"), 1);
	//std::cout << "normal in" << std::endl;
//}
	
}

void Renderer::scene_reset()
{
	load_models();
	m_camera->reset();
}