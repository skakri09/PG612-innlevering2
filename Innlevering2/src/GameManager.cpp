#include "GameManager.h"
#include "GameException.h"
#include "GLUtils/GLUtils.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

using std::cerr;
using std::endl;
using GLUtils::BO;
using GLUtils::Program;
using GLUtils::readFile;

const float GameManager::near_plane = 0.5f;
const float GameManager::far_plane = 50.0f;
const float GameManager::fovy = 45.0f;
const float GameManager::cube_scale = GameManager::far_plane*0.75f;


#pragma region cube_data
const float GameManager::cube_vertices_data[] = {
    -0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,

    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, 0.5f,
    0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,

    0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    -0.5f, 0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
	
    -0.5f, 0.5f, -0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
	
    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, -0.5f,
    0.5f, 0.5f, -0.5f,

    0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f,
    0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
};

const float GameManager::cube_normals_data[] = {
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,

    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,

    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,

    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
	
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,

    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
};

#pragma endregion


inline void checkSDLError(int line = -1) {
#ifndef NDEBUG
	const char *error = SDL_GetError();
	if (*error != '\0') {
		std::cout << "SDL Error";
		if (line != -1) {
			std::cout << ", line " << line;
		}
		std::cout << ": " << error << std::endl;
		SDL_ClearError();
	}
#endif
}

GLuint GameManager::gui_vbo = -1;
GLuint GameManager::gui_vao = -1;

GameManager::GameManager() {
	my_timer.restart();
	zoom = 1;
	render_gui_and_depth = true;
	rotate_light = true;
	current_environment = PLAIN_CUBE_ROOM;
}

GameManager::~GameManager() {
}

void GameManager::createOpenGLContext() {
	//Set OpenGL major an minor versions
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // Use double buffering
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16); // Use framebuffer with 16 bit depth buffer
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8); // Use framebuffer with 8 bit for red
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8); // Use framebuffer with 8 bit for green
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8); // Use framebuffer with 8 bit for blue
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // Use framebuffer with 8 bit for alpha
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// Initalize video
	main_window = SDL_CreateWindow("NITH - PG612 Assignment 2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!main_window) {
		THROW_EXCEPTION("SDL_CreateWindow failed");
	}
	checkSDLError(__LINE__);

	main_context = SDL_GL_CreateContext(main_window);
	SDL_GL_SetSwapInterval(1);
	checkSDLError(__LINE__);
	
	cam_trackball.setWindowSize(window_width, window_height);

	// Init glew
	// glewExperimental is required in openGL 3.3
	// to create forward compatible contexts 
	glewExperimental = GL_TRUE;
	GLenum glewErr = glewInit();
	if (glewErr != GLEW_OK) {
		std::stringstream err;
		err << "Error initializing GLEW: " << glewGetErrorString(glewErr);
		THROW_EXCEPTION(err.str());
	}

	// Unfortunately glewInit generates an OpenGL error, but does what it's
	// supposed to (setting function pointers for core functionality).
	// Lets do the ugly thing of swallowing the error....
	glGetError();


	glViewport(0, 0, window_width, window_height);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	CHECK_GL_ERRORS();
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GameManager::init() {
	//Create opengl context before we do anything OGL-stuff
	createOpenGLContext();
	
	//Initialize IL and ILU
	ilInit();
	iluInit();
	
	//Initialize the different stuff we need
	bunny.reset(new Model("models/bunny.obj", false));
	room.reset(new Model("models/room_hardbox.obj", false));

	cube_vertices.reset(new BO<GL_ARRAY_BUFFER>(cube_vertices_data, sizeof(cube_vertices_data)));
	cube_normals.reset(new BO<GL_ARRAY_BUFFER>(cube_normals_data, sizeof(cube_normals_data)));

	shadow_fbo.reset(new ShadowFBO(window_width, window_height));

	diffuse_cubemap.reset(new CubeMap("cubemaps/diffuse/", "jpg"));
	spacebox.reset(new CubeMap("cubemaps/skybox/", "jpg"));

	Init_SetMatrices();

	//Create the random transformations and colors for the bunnys
	srand(static_cast<int>(time(NULL)));
	for (int i=0; i<number_of_models; ++i) {
		float tx = rand() / (float) RAND_MAX - 0.5f;
		float ty = rand() / (float) RAND_MAX - 0.5f;
		float tz = rand() / (float) RAND_MAX - 0.5f;

		glm::mat4 transformation = bunny->getTransform();
		transformation = glm::translate(transformation, glm::vec3(tx, ty, tz));

		model_matrices.push_back(transformation);
		model_inverse_matrices.push_back(glm::inverse(transformation));
		model_colors.push_back(glm::vec3(tx+0.5, ty+0.5, tz+0.5));
	}

	Init_CreateShaderPrograms();
	Init_SetShaderUniforms();
	Init_set_vao_0_attribPtrs();
	Init_set_vao_1_attribPtrs();
	Init_set_vao_2_attribPtrs();
	Init_set_vao_3_attribPtrs(); 
	gui::GUITextureFactory::Inst()->Init(gui_program, gui_vao);
	current_program = phong_program;

	Init_CreateGUIObjects();
}

void GameManager::Init_SetMatrices(){
	//Set the matrices we will use
	camera.projection = glm::perspective(fovy/zoom,
		window_width / (float) window_height, near_plane, far_plane);
	camera.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));

	gui_camera.projection = glm::ortho(0.0f, (GLfloat)window_width, 0.0f, (GLfloat)window_height, -1.0f, 30.0f);
	gui_camera.view = glm::mat4(1.0);

	light.position = glm::vec3(0, 0, 8);
	light.projection = glm::perspective(90.0f, 1.0f, near_plane, far_plane);
	light.view = glm::lookAt(light.position, glm::vec3(0), glm::vec3(0.0, 1.0, 0.0));

	fbo_projectionMatrix = glm::mat4(1);
	fbo_viewMatrix = glm::mat4(1);
	fbo_modelMatrix = glm::mat4(1);
	fbo_modelMatrix = glm::translate(fbo_modelMatrix, glm::vec3(-0.69f, -0.69f, 0));
	fbo_modelMatrix = glm::scale(fbo_modelMatrix, glm::vec3(0.3));

	room_model_matrix = glm::scale(glm::mat4(1), glm::vec3(15));
	room_model_matrix_inverse = glm::inverse(room_model_matrix);

	cube_model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(cube_scale));
	cube_model_matrix_inverse = glm::inverse(cube_model_matrix);
}

void GameManager::Init_CreateShaderPrograms(){
	//Create the programs we will use
	phong_program.reset(new Program("shaders/phong.vert", "shaders/phong.geom", "shaders/phong.frag"));
	wireframe_program.reset(new Program("shaders/wireframe.vert", "shaders/wireframe.geom", "shaders/wireframe.frag"));
	hidden_line_program.reset(new Program("shaders/hidden_line.vert", "shaders/hidden_line.geom", "shaders/hidden_line.frag"));
	gui_program.reset(new Program("shaders/GUI.vert", "shaders/GUI.frag"));

	light_pov_program.reset(new Program("shaders/light_pov.vert", "shaders/light_pov.frag"));
	depth_dump_program.reset(new Program("shaders/depth_dump.vert", "shaders/depth_dump.frag"));
	CHECK_GL_ERRORS();
}

void GameManager::Init_SetShaderUniforms(){

	phong_program->use();
	glUniform1i(phong_program->getUniform("shadowmap_texture"), 0);
	glUniform1i(phong_program->getUniform("diffuse_map"), 1);
	phong_program->disuse();

	hidden_line_program->use();
	glUniform1i(hidden_line_program->getUniform("shadowmap_texture"), 0);
	glUniform1i(hidden_line_program->getUniform("diffuse_map"), 1);
	hidden_line_program->disuse();

	depth_dump_program->use();
	glUniformMatrix4fv(depth_dump_program->getUniform("modelviewprojection_matrix"), 1, 0, 
						glm::value_ptr(fbo_projectionMatrix*fbo_viewMatrix*fbo_modelMatrix));
	glUniform1i(depth_dump_program->getUniform("fbo_texture"), 0);
	depth_dump_program->disuse();

	gui_program->use();
	glUniformMatrix4fv(gui_program->getUniform("projection"), 1, 0, glm::value_ptr(gui_camera.projection));
	glUniformMatrix4fv(gui_program->getUniform("view"), 1, 0, glm::value_ptr(gui_camera.view));
	gui_program->disuse();

	CHECK_GL_ERRORS();
}

void GameManager::Init_set_vao_0_attribPtrs()
{
	glGenVertexArrays(4, &vao[0]);

	glBindVertexArray(vao[0]);
	bunny->getInterleavedVBO()->bind();
	bunny->getIndices()->bind();
	phong_program->setAttributePointer("position", 3, GL_FLOAT, GL_FALSE, bunny->getStride(), bunny->getVerticeOffset());
	phong_program->setAttributePointer("normal", 3, GL_FLOAT, GL_FALSE, bunny->getStride(), bunny->getNormalOffset());

	wireframe_program->setAttributePointer("position", 3, GL_FLOAT, GL_FALSE, bunny->getStride(), bunny->getVerticeOffset());
	wireframe_program->setAttributePointer("normal", 3, GL_FLOAT, GL_FALSE, bunny->getStride(), bunny->getNormalOffset());

	hidden_line_program->setAttributePointer("position", 3, GL_FLOAT, GL_FALSE, bunny->getStride(), bunny->getVerticeOffset());
	hidden_line_program->setAttributePointer("normal", 3, GL_FLOAT, GL_FALSE, bunny->getStride(), bunny->getNormalOffset());

	bunny->getInterleavedVBO()->unbind();
	glBindVertexArray(0);
}

void GameManager::Init_set_vao_1_attribPtrs()
{
	glBindVertexArray(vao[1]);

	cube_vertices->bind();
	phong_program->setAttributePointer("position", 3);
	wireframe_program->setAttributePointer("position", 3);
	hidden_line_program->setAttributePointer("position", 3);

	cube_normals->bind();
	phong_program->setAttributePointer("normal", 3);
	wireframe_program->setAttributePointer("normal", 3);
	hidden_line_program->setAttributePointer("normal", 3);

	glBindVertexArray(0);
}

void GameManager::Init_set_vao_2_attribPtrs()
{
	glBindVertexArray(vao[2]);
	room->getInterleavedVBO()->bind();
	room->getIndices()->bind();
	phong_program->setAttributePointer("position", 3, GL_FLOAT, GL_FALSE, room->getStride(), room->getVerticeOffset());
	phong_program->setAttributePointer("normal", 3, GL_FLOAT, GL_FALSE, room->getStride(), room->getNormalOffset());

	wireframe_program->setAttributePointer("position", 3, GL_FLOAT, GL_FALSE, room->getStride(), room->getVerticeOffset());
	wireframe_program->setAttributePointer("normal", 3, GL_FLOAT, GL_FALSE, room->getStride(), room->getNormalOffset());

	hidden_line_program->setAttributePointer("position", 3, GL_FLOAT, GL_FALSE, room->getStride(), room->getVerticeOffset());
	hidden_line_program->setAttributePointer("normal", 3, GL_FLOAT, GL_FALSE, room->getStride(), room->getNormalOffset());
	room->getInterleavedVBO()->unbind();
	glBindVertexArray(0);
}

void GameManager::Init_set_vao_3_attribPtrs()
{
	glBindVertexArray(vao[3]);
	static float positions[8] = {
		-1.0, 1.0,
		-1.0, -1.0,
		1.0, 1.0,
		1.0, -1.0,
	};

	glGenBuffers(1, &fbo_vertex_bo);
	glBindBuffer(GL_ARRAY_BUFFER, fbo_vertex_bo);
	glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(float), &positions[0], GL_STATIC_DRAW);

	depth_dump_program->setAttributePointer("in_Position", 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &gui_vao);
	glBindVertexArray(gui_vao);

	const float gui_positions[8] = {
		0.0, 1.0,
		0.0, 0.0,
		1.0, 1.0,
		1.0, 0.0
	};

	glGenBuffers(1, &gui_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, gui_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(float), &gui_positions[0], GL_STATIC_DRAW);
	gui_program->setAttributePointer("in_Position", 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GameManager::Init_CreateGUIObjects(){
	slider_line_threshold = std::make_shared<gui::SliderWithText>("GUI/hiddenline/line_threashold.png",glm::vec2(950.0f, 5.0f));
	slider_line_scale	  = std::make_shared<gui::SliderWithText>("GUI/hiddenline/amplify_scale.png",  glm::vec2(950.0f, 75.0f));
	slider_line_offset	  = std::make_shared<gui::SliderWithText>("GUI/hiddenline/amplify_offset.png", glm::vec2(950.0f, 145.0f));
	slider_diffuse_mix	  = std::make_shared<gui::SliderWithText>("GUI/diffuse_colormix_value.png", glm::vec2(950.0f, 650.0f));
	slider_gui_alpha	  = std::make_shared<gui::SliderWithText>("GUI/gui_alpha.png", glm::vec2(10.0f, 220.0f), glm::vec2(0.4f, 0.4f));
	slider_gui_alpha->SetClampRange(0.2f, 1.0f);
	gui_sliders.push_back(slider_line_threshold);
	gui_sliders.push_back(slider_line_scale);
	gui_sliders.push_back(slider_line_offset);
	gui_sliders.push_back(slider_diffuse_mix);
	gui_sliders.push_back(slider_gui_alpha);

	std::vector<gui::RadioButtonEntry> rendermode_entries;
	rendermode_entries.push_back(gui::RadioButtonEntry(std::bind(&GameManager::UsePhongProgram, this), true, "GUI/Rendermode/PhongWShadows.png"));
	rendermode_entries.push_back(gui::RadioButtonEntry(std::bind(&GameManager::UseWireframeProgram, this), false, "GUI/Rendermode/Wireframe.png"));
	rendermode_entries.push_back(gui::RadioButtonEntry(std::bind(&GameManager::UseHiddenLineProgram, this), false, "GUI/Rendermode/Hidden Line.png"));
	rendermode_radiobtn.reset(new gui::RadioButtonCollection(rendermode_entries, glm::vec2(0, window_height-40), glm::vec2(0.5, 0.5)));


	std::vector<gui::RadioButtonEntry> environment_entries;
	environment_entries.push_back(gui::RadioButtonEntry(std::bind(&GameManager::SetBackgroundToCube, this), true, "GUI/CubeBackground.png"));
	environment_entries.push_back(gui::RadioButtonEntry(std::bind(&GameManager::SetBackgroundToOpenRoom, this), false, "GUI/OpenBackground.png"));
	environment_radiobtn.reset(new gui::RadioButtonCollection(environment_entries, glm::vec2(250, window_height-40), glm::vec2(0.5, 0.5)));
}

void GameManager::renderColorPass() {
	glViewport(0, 0, window_width, window_height);
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Create the new view matrix that takes the trackball view into account
	cam_trackball_view_matrix = camera.view*cam_trackball.getTransform();

	glDepthMask(GL_FALSE);	
	spacebox->render(camera.projection, cam_trackball_view_matrix);
	glDepthMask(GL_TRUE);

	current_program->use();

	if(current_program == hidden_line_program)
	{
		glUniform1f(current_program->getUniform("line_threshold"), slider_line_threshold->get_slider_value()/10);
		glUniform1f(current_program->getUniform("line_scale"), slider_line_scale->get_slider_value()*100);
		glUniform1f(current_program->getUniform("line_offset"), (slider_line_offset->get_slider_value()-0.5f)*10);
		
	}
	if(current_program != wireframe_program)
		glUniform1f(current_program->getUniform("diffuse_mix_value"), slider_diffuse_mix->get_slider_value());

	//Bind shadow map and diffuse cube map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow_fbo->getTexture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	diffuse_cubemap->bind(GL_TEXTURE1);

	if(current_environment == PLAIN_CUBE_ROOM)
		RenderCubeColorpass();
	else if(current_environment == OPEN_HALFROOM)
		RenderRoomModelColorpass();

	RenderModelsColorpass();
}

void GameManager::renderShadowPass() {	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	light_pov_program->use();

	if(current_environment == PLAIN_CUBE_ROOM)
		RenderCubeShadowpass();
	else if(current_environment == OPEN_HALFROOM)
		RenderRooomModelShadowpass();

	RenderModelsShadowpass();

	light_pov_program->disuse();
	shadow_fbo->unbind();
}

void GameManager::renderDepthDump(){
	depth_dump_program->use();
	glUniform1f(depth_dump_program->getUniform("gui_alpha"), slider_gui_alpha->get_slider_value());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow_fbo->getTexture());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glBindVertexArray(vao[3]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CHECK_GL_ERRORS();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	depth_dump_program->disuse();
}

void GameManager::render() {
	if(rotate_light){
		glm::mat4 rotation = glm::rotate(delta_time*10.f, 0.0f, 1.0f, 0.0f);
		light.position = glm::mat3(rotation)*light.position;
		light.view = glm::lookAt(light.position,  glm::vec3(0), glm::vec3(0.0, 1.0, 0.0));
	}
	
	shadow_fbo->bind();
	glViewport(0, 0, window_width, window_height);
	renderShadowPass();
	shadow_fbo->unbind();
	renderColorPass();
	
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	//Clearing the depth buffer to always draw on top of the previously rendered stuff
	//before rendering GUI
	glClear(GL_DEPTH_BUFFER_BIT);
	if(render_gui_and_depth){
		renderDepthDump();

		glDisable(GL_CULL_FACE);
		RenderGUI();
		glEnable(GL_CULL_FACE);
	}
	CHECK_GL_ERRORS();
}

void GameManager::RenderGUI(){
	glBindVertexArray(gui_vao);
	gui_program->use();
	glUniform1f(gui_program->getUniform("gui_alpha"), slider_gui_alpha->get_slider_value());
	rendermode_radiobtn->Draw();
	environment_radiobtn->Draw();
	slider_gui_alpha->Draw();

	if(current_program == hidden_line_program)
	{
		slider_line_threshold->Draw();
		slider_line_scale->Draw();
		slider_line_offset->Draw();
	}
	if(current_program != wireframe_program)
		slider_diffuse_mix->Draw();

	gui_program->disuse();
	glBindVertexArray(0);
}

void GameManager::play() {
	bool doExit = false;
	float fps = 0.0f;
	float fpsTimer = 0.0f;
	//SDL main loop
	while (!doExit) {
		delta_time = static_cast<float>(my_timer.elapsedAndRestart());
		SDL_Event event;
		while (SDL_PollEvent(&event)) {// poll for pending events
			switch (event.type) {
			case SDL_MOUSEWHEEL:
				if (event.wheel.y > 0 )
					zoomIn();
				else if (event.wheel.y < 0 )
					zoomOut();
				break;
			case SDL_MOUSEBUTTONDOWN:
				{
					bool started_interaction = false;
					unsigned int slider_counter = 0;
					while(!started_interaction && (slider_counter  < gui_sliders.size()) )
					{
						started_interaction = gui_sliders.at(slider_counter)->BeginInteraction(glm::vec2(event.motion.x, event.motion.y));
						slider_counter ++;
					}
					if(!started_interaction)
						cam_trackball.rotateBegin(event.motion.x, event.motion.y);

					rendermode_radiobtn->OnClick(glm::vec2(event.motion.x, event.motion.y));
					environment_radiobtn->OnClick(glm::vec2(event.motion.x, event.motion.y));
				}
				break;
			case SDL_MOUSEBUTTONUP:
				for(unsigned int i = 0; i < gui_sliders.size(); i++)
					gui_sliders.at(i)->EndInteraction(glm::vec2(event.motion.x, event.motion.y));
				cam_trackball.rotateEnd(event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEMOTION:
				for(unsigned int i = 0; i < gui_sliders.size(); i++)
					gui_sliders.at(i)->Update(delta_time, glm::vec2(event.motion.x, event.motion.y));
				cam_trackball.rotate(event.motion.x, event.motion.y, zoom);
				break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
				case SDLK_t:
					render_gui_and_depth = !render_gui_and_depth;
					break;
				case SDLK_ESCAPE:
					doExit = true;
					break;
				case SDLK_q:
					if (event.key.keysym.mod & KMOD_CTRL) 
						doExit = true;
					break;
				case SDLK_PLUS:zoomIn();break;
				case SDLK_MINUS:zoomOut();break;
				case SDLK_1:UsePhongProgram();break;
				case SDLK_2:UseWireframeProgram();break;
				case SDLK_3:UseHiddenLineProgram();break;
				case SDLK_5:
					rotate_light = !rotate_light;
				}
				break;
			case SDL_QUIT: //e.g., user clicks the upper right x
				doExit = true;
				break;
			}
		}

		//Render, and swap front and back buffers
		render();
		SDL_GL_SwapWindow(main_window);

		fpsTimer += delta_time;
		if(fpsTimer >= 0.3f)//updating the fps counter once every .3sec
		{
			fps = 1/delta_time;
			std::ostringstream captionStream;		
			captionStream << "FPS: " << fps;
			SDL_SetWindowTitle(main_window, captionStream.str().c_str());
			fps = 0;
			fpsTimer = 0;
		}
	}
	quit();
}

void GameManager::zoomIn() {
	zoom *= 1.1f;
	camera.projection = glm::perspective(fovy/zoom,
			window_width / (float) window_height, near_plane, far_plane);
}

void GameManager::zoomOut() {
	zoom = std::max(zoom*0.9f, 0.5f);
	camera.projection = glm::perspective(fovy/zoom,
			window_width / (float) window_height, near_plane, far_plane);
}

void GameManager::quit() {
	std::cout << "Bye bye..." << std::endl;
}

void GameManager::UsePhongProgram(){
	if(current_program != phong_program){
		current_program = phong_program;
		rendermode_radiobtn->SetActive(0);
	}
}

void GameManager::UseWireframeProgram(){
	if(current_program != wireframe_program){
		current_program = wireframe_program;
		rendermode_radiobtn->SetActive(1);
	}
}

void GameManager::UseHiddenLineProgram(){
	if(current_program != hidden_line_program){
		current_program = hidden_line_program;
		rendermode_radiobtn->SetActive(2);
	}
}

void GameManager::RenderCubeColorpass(){
	glBindVertexArray(vao[1]);

	glm::mat4 modelview_matrix = cam_trackball_view_matrix*cube_model_matrix;
	glm::mat4 modelview_matrix_inverse = glm::inverse(modelview_matrix);
	glm::mat4 modelviewprojection_matrix = camera.projection*modelview_matrix;
	glm::vec3 light_pos = glm::mat3(cube_model_matrix_inverse)*light.position/cube_model_matrix_inverse[3].w;

	glm::mat4 light_modelview_matrix = light.view*cube_model_matrix;
	glm::mat4 shadowMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5-0.01f));
	shadowMatrix = glm::scale(shadowMatrix, glm::vec3(0.5f, 0.5f, 0.5f*1.01f)) * light.projection * light_modelview_matrix;

	glUniformMatrix4fv(current_program->getUniform("shadow_matrix"), 1, 0, glm::value_ptr(shadowMatrix));

	glUniform1i(current_program->getUniform("shadowmap_texture"), 0);
	glUniform3fv(current_program->getUniform("light_pos"), 1, glm::value_ptr(light_pos));
	glUniform3fv(current_program->getUniform("color"), 1, glm::value_ptr(glm::vec3(0.1f, 0.1f, 0.7f)));
	glUniformMatrix4fv(current_program->getUniform("modelviewprojection_matrix"), 1, 0, glm::value_ptr(modelviewprojection_matrix));
	glUniformMatrix4fv(current_program->getUniform("modelview_matrix_inverse"),	1, 0, glm::value_ptr(modelview_matrix_inverse));

	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void GameManager::RenderCubeShadowpass(){
	glBindVertexArray(vao[1]);

	glm::mat4 modelview_matrix = light.view*cube_model_matrix;
	glm::mat4 modelview_matrix_inverse = glm::inverse(modelview_matrix);
	glm::mat4 modelviewprojection_matrix = light.projection*modelview_matrix;
	glm::vec3 light_pos = glm::mat3(cube_model_matrix_inverse)*light.position/cube_model_matrix_inverse[3].w;

	glUniformMatrix4fv(light_pov_program->getUniform("modelviewprojection_matrix"), 1, 0, glm::value_ptr(modelviewprojection_matrix));
	CHECK_GL_ERRORS();
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void GameManager::RenderModelsColorpass(){
	glBindVertexArray(vao[0]);
	for (int i=0; i<number_of_models; ++i) {
		glm::mat4 model_matrix = model_matrices.at(i);
		glm::mat4 model_matrix_inverse = model_inverse_matrices.at(i);
		glm::mat4 modelview_matrix = cam_trackball_view_matrix*model_matrix;
		glm::mat4 modelview_matrix_inverse = glm::inverse(modelview_matrix);
		glm::mat4 modelviewprojection_matrix = camera.projection*modelview_matrix;
		glm::vec3 light_pos = glm::mat3(model_matrix_inverse)*light.position/model_matrix_inverse[3].w;

		glm::mat4 light_modelview_matrix = light.view*model_matrix;
		glm::mat4 shadowMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5-0.01f));
		shadowMatrix = glm::scale(shadowMatrix, glm::vec3(0.5f, 0.5f, 0.5f*1.01f)) * light.projection * light_modelview_matrix;

		glUniformMatrix4fv(current_program->getUniform("shadow_matrix"), 1, 0, glm::value_ptr(shadowMatrix));

		glUniform3fv(current_program->getUniform("light_pos"), 1, glm::value_ptr(light_pos));
		glUniform3fv(current_program->getUniform("color"), 1, glm::value_ptr(model_colors.at(i)));
		glUniformMatrix4fv(current_program->getUniform("modelviewprojection_matrix"), 1, 0, glm::value_ptr(modelviewprojection_matrix));
		glUniformMatrix4fv(current_program->getUniform("modelview_matrix_inverse"), 1, 0, glm::value_ptr(modelview_matrix_inverse));

		MeshPart& mesh = bunny->getMesh();
		glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh.first));
	}
}

void GameManager::RenderModelsShadowpass(){
	glBindVertexArray(vao[0]);
	for (int i=0; i<number_of_models; ++i) {
		glm::mat4 model_matrix = model_matrices.at(i);
		glm::mat4 modelview_matrix = light.view*model_matrix;
		glm::mat4 modelviewprojection_matrix = light.projection*modelview_matrix;

		glUniformMatrix4fv(light_pov_program->getUniform("modelviewprojection_matrix"), 1, 0, glm::value_ptr(modelviewprojection_matrix));

		MeshPart& mesh = bunny->getMesh();
		glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh.first));
	}

}

void GameManager::RenderRoomModelColorpass(){
	glBindVertexArray(vao[2]);

	glm::mat4 modelview_matrix = cam_trackball_view_matrix*room_model_matrix;
	glm::mat4 modelviewprojection_matrix = camera.projection*modelview_matrix;

	glUniform3fv(current_program->getUniform("color"), 1, glm::value_ptr(glm::vec3(0.1f, 0.5f, 0.7f)));
	glUniformMatrix4fv(current_program->getUniform("modelviewprojection_matrix"), 1, 0, glm::value_ptr(modelviewprojection_matrix));
	glUniformMatrix4fv(current_program->getUniform("modelview_matrix_inverse"), 1, 0, glm::value_ptr(room_model_matrix_inverse));

	MeshPart& mesh = room->getMesh();
	glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh.first));
	glBindVertexArray(0);
}

void GameManager::RenderRooomModelShadowpass(){
	glBindVertexArray(vao[2]);

	glm::mat4 modelview_matrix = light.view*room_model_matrix;
	glm::mat4 modelviewprojection_matrix = light.projection*modelview_matrix;

	glUniformMatrix4fv(light_pov_program->getUniform("modelviewprojection_matrix"), 1, 0, glm::value_ptr(modelviewprojection_matrix));

	CHECK_GL_ERRORS();
	MeshPart& mesh = room->getMesh();
	glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * mesh.first));

	CHECK_GL_ERRORS();
}

void GameManager::SetBackgroundToCube(){
	current_environment = PLAIN_CUBE_ROOM;
}

void GameManager::SetBackgroundToOpenRoom(){
	current_environment = OPEN_HALFROOM;
}


