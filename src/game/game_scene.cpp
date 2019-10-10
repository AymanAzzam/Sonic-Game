#include <application.hpp>
#include "game_scene.hpp"

#include <mesh/mesh_utils.hpp>
#include <textures/texture_utils.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define startx 120 //the beginning point of obstacles
#define minimumspeedobstacle 1 //the beginning speed of obstacles (the easiest level)
#define maximumspeedobstacle 5//(the hardest level)

#define runnerspeed 10 //the speed that runner moves left and right
#define maximumz 10 //maximum right that runner can move
#define minimumz -10 //maximum left that runner can move
#define RunnerJump 9 //runner jump 9 in y axis
#define minimumjump 2 //runner can jump again before touch the ground with this distance
#define obstaclesdistance 30 //the distance between every two obstacles
#define disappearobstacle -30 //the distance when obstacle will disappear (become back of the camira)
/*
* The goal of this scene is just to showcase simple static model loading
* Also to show Anisotropic filtering
*
*/

void GameScene::Initialize() {
    shader = new Shader();
	shader->attach("assets/shaders/directional.vert", GL_VERTEX_SHADER);
	shader->attach("assets/shaders/directional.frag", GL_FRAGMENT_SHADER);
    shader->link();

	skyShader = new Shader();
	skyShader->attach("assets/shaders/sky.vert", GL_VERTEX_SHADER);
	skyShader->attach("assets/shaders/sky.frag", GL_FRAGMENT_SHADER);
	skyShader->link();

	
	//intialize the ground mesh
	PlaneMesh = MeshUtils::Plane({0,0}, {10,10});
	//intialize the sky mesh
	SkyMesh = MeshUtils::Box();

	//intialize the Runner mesh (Loading Runner model)
	RunnerMesh = MeshUtils::LoadObj("assets/models/sonic/sonic-the-hedgehog.obj");

	//intialize the Obstacle mesh (Loading Obstacle model)
	ObstacleMesh = MeshUtils::LoadObj("assets/models/obstacle/RoadBlockade_01.obj");


	//the line on the ground
	BoardTex[AMBIENT_OCCLUSION] = TextureUtils::Board({ 100 ,5 }, { 0,0,0,1 });
	BoardTex[ALBEDO] = TextureUtils::Board({ 100 ,5 }, { 0,0,0,1 });
	BoardTex[SPECULAR] = TextureUtils::Board({ 100 ,5 }, { 0,0,0,1 });
	BoardTex[ROUGHNESS] = TextureUtils::Board({ 100 ,5 }, { 0,0,0,1 });
	BoardTex[EMISSIVE] = TextureUtils::SingleColor({ 0,0,0,1 });
    // Fog effect
	texloc = glGetUniformLocation(shader->getID(),"trans");
	texlocsky = glGetUniformLocation(skyShader->getID(), "trans");
	scloc = glGetUniformLocation(shader->getID(), "skyColour");
	//intialize the trnasformation effect
	trans_effect = false;
	transformation_effect = { glm::vec4(1,0,0,0),glm::vec4(0,1,0,0), glm::vec4(0,0,1,0), glm::vec4(0,0,0,1) };
	fog_effect = false;
	shader->set("fog", fog_effect);
	
	/////////////////////////////////
	//Loading the ground texture
	FloorTex[AMBIENT_OCCLUSION] = TextureUtils::Load2DTextureFromFile("assets/textures/brick_ground.jpg");
	FloorTex[ALBEDO] = TextureUtils::Load2DTextureFromFile("assets/textures/brick_ground.jpg");
	FloorTex[SPECULAR] = TextureUtils::Load2DTextureFromFile("assets/textures/Asphalt_spc.jpg");
	FloorTex[ROUGHNESS] = TextureUtils::Load2DTextureFromFile("assets/textures/Asphalt_rgh.jpg");
	FloorTex[EMISSIVE] = TextureUtils::SingleColor({ 0,0,0,1 });
	FloorTex[tex]= TextureUtils::Load2DTextureFromFile("assets/textures/brick_ground.jpg");
    
	//Lading the Runner texture
	RunnerTex[AMBIENT_OCCLUSION] = TextureUtils::Load2DTextureFromFile("assets/models/sonic/bab97353.jpg");
	RunnerTex[ALBEDO] = TextureUtils::Load2DTextureFromFile("assets/models/sonic/body.jpg");
	RunnerTex[SPECULAR] = TextureUtils::Load2DTextureFromFile("assets/models/sonic/sonic_spc.jpg");
	RunnerTex[ROUGHNESS] = TextureUtils::Load2DTextureFromFile("assets/models/sonic/sonic_rgh.jpg");
	RunnerTex[EMISSIVE] = TextureUtils::SingleColor({ 0,0,0,1 });
	RunnerTex[tex] = TextureUtils::Load2DTextureFromFile("assets/models/sonic/bab97353.jpg");

	//Loading the Obstacle Textures
	ObstacleTex1[AMBIENT_OCCLUSION] = TextureUtils::Load2DTextureFromFile("assets/models/obstacle/white_obstacle.jpg");
	ObstacleTex1[ALBEDO] = TextureUtils::Load2DTextureFromFile("assets/models/obstacle/obstacle_col.jpg");
	ObstacleTex1[SPECULAR] = TextureUtils::Load2DTextureFromFile("assets/models/obstacle/obstacle_spc.jpg");
	ObstacleTex1[ROUGHNESS] = TextureUtils::Load2DTextureFromFile("assets/models/obstacle/obstacle_rgh.jpg");
	ObstacleTex1[EMISSIVE] = TextureUtils::SingleColor({ 0,0,0,1 });
	ObstacleTex1[tex] = TextureUtils::Load2DTextureFromFile("assets/models/obstacle/white_obstacle.jpg");


	
	BoardTex[ALBEDO] = TextureUtils::SingleColor({ 1,1,1,0 });
	BoardTex[SPECULAR] = TextureUtils::SingleColor({ 1,1,1,0 });
	BoardTex[ROUGHNESS] = TextureUtils::SingleColor({ 1,1,1,0 });
	BoardTex[EMISSIVE] = TextureUtils::SingleColor({ 0,0,0,1 });

    camera = new Camera();
    glm::ivec2 windowSize = getApplication()->getWindowSize();
    camera->setupPerspective(glm::pi<float>()/2, (float)windowSize.x / windowSize.y, 0.1f, 1000.0f);
    camera->setUp({0, 1,0});
	camera->setDirection({ 1,-0.5,0 });
	camera->setPosition({ -7, 12, 0 });

	controller = new FlyCameraController(this, camera);
	controller->setYaw(0);
	controller->setPitch(-0.35);
	controller->setPosition({ -7, 12, 0 });

	sunYaw = sunPitch = glm::quarter_pi<float>();

	InitializeData();

	//intialize the random size of obstacles 1,3
	//size3 = large_obstacle;
	//size1 = small_obstacle;

	//Intialize the Bullet
	InitializeBullet();
	//Create Rigid Bodies
	CreateRigidBodies();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//set the color of sky
	glClearColor(0.027f, 0.043f, 0.20f, 0.0f);
}
void GameScene::InitializeData()
{
	//intialize the position of Runner
	RunnerPosition = { 0, 0, 0 };
	jump_flag = 0;

	//intialize the position of obstacles
	x1 = startx;
	x2 = x1 - obstaclesdistance;
	x3 = x2 - obstaclesdistance;
	x4 = x3 - obstaclesdistance;
	x5 = x4 - obstaclesdistance;

	obstacle_speed = minimumspeedobstacle;

	//set the size of obstacles
	large_obstacle = 5;
	small_obstacle = 2;

	//set the center position of obstacles size
	right_large_obstacle = 4;
	left_large_obstacle = -4;
	right_small_obstacle = 8;
	left_small_obstacle = -8;

	//intialize the position 
	position5 = { x5,0,0 };
	position4 = { x4,0,left_small_obstacle };
	position3 = { x3,0,right_large_obstacle };
	position2 = { x2,0,right_small_obstacle };
	position1 = { x1,0,left_large_obstacle };
}

bool GameScene::Simulation(double delta_time, long long timestamp, bool & pause)
{
	Update(delta_time, timestamp, pause);

	if (pause)
		return false;

	if ((TimeStamp % 1000 == 0) && (obstacle_speed< maximumspeedobstacle))
		obstacle_speed++;

	//random function for obstacles position on z-axis 
	random = (11 * TimeStamp*TimeStamp + 13 * (long long)glfwGetTime()) % 3;

	//the limit moving of Runner
	if (RunnerPosition.z > maximumz)
		RunnerPosition.z = maximumz;
	else if (RunnerPosition.z < minimumz)
		RunnerPosition.z = minimumz;

	//to make the Runner jump continuously
	//jump>0 mean he goes up
	//jump<0 mean he goes down
	//jump = 0 mean (he is on the ground || he is on the maximum hight)
	if (jump_flag > 0 && TimeStamp % 2 == 0) //every 2 timestamp go up 1 step 
	{
		RunnerPosition.y += (obstacle_speed - ((float)obstacle_speed / 5));

		if (RunnerPosition.y >= RunnerJump) //now he is on the maximum height
		{
			RunnerPosition.y = RunnerJump;
			jump_flag = -1; //set the jump =-1 to go down
		}
	}
	else if (jump_flag < 0 && TimeStamp % 2 == 0) //every 2 timestamp go down 1 step
	{
		RunnerPosition.y -= (obstacle_speed - ((float)obstacle_speed / 5));
		
		if (RunnerPosition.y <= 0)
		{
			RunnerPosition.y = 0;
			jump_flag = 0; //set the jump =0 to can jump again 
		}
	}

	
	//Move obstacles to the new positions on x-axis , draw thim and check if collision is happened
	return MoveObstacles();
}

void GameScene::Update(double delta_time,long long timestamp,bool & pause) {
	//controller->update(delta_time);
	Keyboard* kb = getKeyboard();

	TimeStamp = timestamp;


	if (kb->justPressed(GLFW_KEY_P)) pause = !pause;

	if (!pause)
	{
		if (kb->isPressed(GLFW_KEY_RIGHT)) RunnerPosition.z += runnerspeed * (float)delta_time;
		if (kb->isPressed(GLFW_KEY_LEFT)) RunnerPosition.z -= runnerspeed * (float)delta_time;
		if (kb->justPressed(GLFW_KEY_SPACE))
		{
			if (jump_flag == 0) 
				jump_flag = 1;
		}
		if (kb->justPressed(GLFW_KEY_E))
		{
			if (trans_effect)
				transformation_effect = { glm::vec4(1,0,0,0),glm::vec4(0,1,0,0), glm::vec4(0,0,1,0), glm::vec4(0,0,0,1) };
			else
				transformation_effect = { glm::vec4(0.393,0.349,0.272,0),glm::vec4(0.769,0.686,0.534,0), glm::vec4(0.189,0.168,0.131,0), glm::vec4(0,0,0,1) };
			trans_effect = !trans_effect;
		}
		if (kb->justPressed(GLFW_KEY_F))
			fog_effect = !fog_effect;
	}
}
inline glm::vec3 getTimeOfDayMix(float sunPitch) {
	sunPitch /= glm::half_pi<float>();
	if (sunPitch > 0) {
		float noon = glm::smoothstep(0.0f, 0.5f, sunPitch);
		return { noon, 1.0f - noon, 0 };
	}
	else {
		float dusk = glm::smoothstep(0.0f, 0.25f, -sunPitch);
		return { 0, 1.0f - dusk, dusk };
	}
}

void GameScene::Draw() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear colors and depth
	
	glm::mat4 VP = camera->getVPMatrix();
	glm::vec3 cam_pos = camera->getPosition();
	glm::vec3 sun_direction = glm::vec3(-1, 0, -0.3);

	const glm::vec3 noonSkyColor = { 0.53f, 0.81f, 0.98f };
	const glm::vec3 sunsetSkyColor = { 0.99f, 0.37f, 0.33f };
	const glm::vec3 duskSkyColor = { 0.04f, 0.05f, 0.19f };

	const glm::vec3 noonSunColor = { 0.9f, 0.8f, 0.6f };
	const glm::vec3 sunsetSunColor = { 0.8f, 0.6f, 0.4f };
	const glm::vec3 duskSunColor = { 0.0f, 0.0f, 0.0f };
	
	
	glm::vec3 mix = getTimeOfDayMix(sunPitch);

	glm::vec3 skyColor = mix.x * noonSkyColor + mix.y * sunsetSkyColor + mix.z * duskSkyColor;
	glm::vec3 sunColor = mix.x * noonSunColor + mix.y * sunsetSunColor + mix.z * duskSunColor;
	
	shader->use();
	shader->set("VP", VP);
	shader->set("cam_pos", cam_pos);
	shader->set("light.color", sunColor);
	shader->set("light.direction", -sun_direction);
	shader->set("ambient", 0.5f*skyColor);

	shader->set("material.albedo", 0);
	shader->set("material.specular", 1);
	shader->set("material.roughness", 2);
	shader->set("material.ambient_occlusion", 3);
	shader->set("material.emissive", 4);
	shader->set("tex", 5);

	shader->set("material.albedo_tint", { 1,1,1 });
	shader->set("material.specular_tint", { 1,1,1 });
	shader->set("material.roughness_scale", 1.0f);
	shader->set("material.emissive_tint", { 1,1,1 });


	glUniform3f(scloc, 0.8, 0.8, 0.8);
	glUniformMatrix4fv(texloc, 1, GL_FALSE, glm::value_ptr(transformation_effect));
	shader->set("fog", fog_effect);

	//draw the Ground on the screen
	DrawGround();
	//Draw the Runner on the screen
	DrawRunner();

		//Draw obstacle 5 in random position 
		DrawObstacle5(random);
		//Draw obstacle 4 in random position
		DrawObstacle4(random);
		//Draw obstacle 3 in random position with random size 
		DrawObstacle3(random);
		//Draw obstacle 2 in random position
		DrawObstacle2(random);
		//Draw obstacle 1 in random position with random size
		DrawObstacle1(random);

		//When Obstacles become out of the view Draw New obstacles in the view
		NewObstacles();

		//Draw SkyBox
		skyShader->use();
		skyShader->set("VP", VP);
		skyShader->set("cam_pos", cam_pos);
		skyShader->set("M", glm::translate(glm::mat4(), cam_pos));
		skyShader->set("sun_direction", sun_direction);
		skyShader->set("sun_size", 0.000000000005f);
		skyShader->set("sun_halo_size", 0.04f);
		//skyShader->set("sun_halo_size", 0.02f);
		skyShader->set("sun_brightness", 1.0f);
		//skyShader->set("sun_brightness", 1.0f);
		//skyShader->set("sun_color", glm::vec3{ 0,0,0 });
		skyShader->set("sun_color", sunColor);
		skyShader->set("sky_top_color", skyColor);
		skyShader->set("sky_bottom_color", 1.0f-0.25f*(1.0f-skyColor));
		skyShader->set("sky_smoothness", 0.5f);

		//glUniform3f(scloc, 0.8, 0.8, 0.8);
		glUniformMatrix4fv(texlocsky, 1, GL_FALSE, glm::value_ptr(transformation_effect));


		glCullFace(GL_FRONT);
		SkyMesh->draw();
		glCullFace(GL_BACK);
}

void GameScene::DrawGround()
{
	//Draw the ground
	glm::mat4 ground_mat = glm::scale(glm::mat4(), glm::vec3(130, 1, 130));
	shader->set("M", ground_mat);
	shader->set("M_it", glm::transpose(glm::inverse(ground_mat)));
	for (int i = 0; i < 5; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		FloorTex[i]->bind();
	}
	PlaneMesh->draw();
	
	//Draw the right line on the ground 
	glm::mat4 board_mat = glm::translate(glm::mat4(), glm::vec3(0, 0.1, 11.5));
	board_mat = glm::scale(board_mat, glm::vec3(130, 1, 0.5));
	shader->set("M", board_mat);
	shader->set("M_it", glm::transpose(glm::inverse(board_mat)));
	for (int i = 0; i < 5; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		BoardTex[i]->bind();
	}
	PlaneMesh->draw();
	
	//Draw the left line on the ground
	glm::mat4 board_mat2 = glm::translate(glm::mat4(), glm::vec3(0, 0.1, -11.5));
	board_mat2 = glm::scale(board_mat2, glm::vec3(130, 1, 0.5));
	shader->set("M", board_mat2);
	shader->set("M_it", glm::transpose(glm::inverse(board_mat2)));
	PlaneMesh->draw();

}

void GameScene::DrawRunner()
{

	//Draw the Runner
	glm::mat4 runner_mat = glm::translate(glm::mat4(), RunnerPosition);
	runner_mat = glm::scale(runner_mat, { 0.15, 0.15, 0.15 });
	runner_mat = glm::rotate(runner_mat, glm::pi<float>() / 2, { 0.0,1.0,0.0 });
	shader->set("M", runner_mat);
	shader->set("M_it", glm::transpose(glm::inverse(runner_mat)));
	for (int i = 0; i < 5; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		RunnerTex[i]->bind();
	}
	RunnerMesh->draw();
}

void GameScene::DrawObstacle5(int random)
{
	//to handle special case (the first obstacle in the game don't draw it)
	if (TimeStamp > 50 ) 
	{
		position5.r = x5; //take the new x after moving

		if (x5 == disappearobstacle) //draw new obstacle with random position in z-axis when become out of the view
		{
			if (random == 1)
				position5.b = right_small_obstacle;
			else if (random == 2)
				position5.b = left_small_obstacle;
			else
				position5.b = 0;
		}

		//Draw the obstacle 5
		glm::mat4 obstacle_mat5 = glm::translate(glm::mat4(), position5);
		obstacle_mat5 = glm::scale(obstacle_mat5, { 5, 5, small_obstacle });
		obstacle_mat5 = glm::rotate(obstacle_mat5, glm::pi<float>() / 2, { 0.0,-1.0,0.0 });
		shader->set("M", obstacle_mat5);
		shader->set("M_it", glm::transpose(glm::inverse(obstacle_mat5)));
		for (int i = 0; i < 6; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			ObstacleTex1[i]->bind();
		}
		ObstacleMesh->draw();
	}
}

void GameScene::DrawObstacle4(int random)
{
	//to handle special case (the first obstacle in the game don't draw it)
	if (TimeStamp > 100) 
	{
		position4.r = x4; //take the new x after moving

		if (x4 == disappearobstacle) //draw new obstacle with random position in z-axis when become out of the view
		{
			if (random == 2)
				position4.b = 0;
			else if (random == 0)
				position4.b = left_small_obstacle;
			else
				position4.b = right_small_obstacle;
		}

		//Draw the obstacle 4
		glm::mat4 obstacle_mat4 = glm::translate(glm::mat4(), position4);
		obstacle_mat4 = glm::scale(obstacle_mat4, { 5, 5, small_obstacle });
		obstacle_mat4 = glm::rotate(obstacle_mat4, glm::pi<float>() / 2, { 0.0,-1.0,0.0 });
		shader->set("M", obstacle_mat4);
		shader->set("M_it", glm::transpose(glm::inverse(obstacle_mat4)));
		ObstacleMesh->draw();
	}
}

void GameScene::DrawObstacle3(int random)
{
	//to handle special case (the first obstacle in the game don't draw it)
	if (TimeStamp > 150) 
	{
		position3.r = x3; //take the new x after moving
		
		if (x3 == disappearobstacle) //draw new obstacle with random position in z-axis and when become out of the view
		{
			if (random == 2)
				position3.b = 0;
			else if (random == 0)
				position3.b = right_large_obstacle;
			else
				position3.b = left_large_obstacle;
		}
		
		//Draw the obstacle 3
		glm::mat4 obstacle_mat3 = glm::translate(glm::mat4(), position3);
		obstacle_mat3 = glm::scale(obstacle_mat3, { 5, 5, large_obstacle });
		obstacle_mat3 = glm::rotate(obstacle_mat3, glm::pi<float>() / 2, { 0.0,-1.0,0.0 });
		shader->set("M", obstacle_mat3);
		shader->set("M_it", glm::transpose(glm::inverse(obstacle_mat3)));
		ObstacleMesh->draw();
	}
}

void GameScene::DrawObstacle2(int random)
{
	//to handle special case (the first obstacle in the game don't draw it)
	if (TimeStamp > 200) 
	{
		position2.r = x2; //take the new x after moving

		if (x2 == disappearobstacle) //draw new obstacle with random position in z-axis and when become out of the view
		{
			if (random == 1)
				position2.b = 0;
			else if (random == 0)
				position2.b = right_small_obstacle;
			else
				position2.b = left_small_obstacle;
		}

		//Draw the Obstacle 2
		glm::mat4 obstacle_mat2 = glm::translate(glm::mat4(), position2);
		obstacle_mat2 = glm::scale(obstacle_mat2, { 5, 5, small_obstacle });
		obstacle_mat2 = glm::rotate(obstacle_mat2, glm::pi<float>() / 2, { 0.0,-1.0,0.0 });
		shader->set("M", obstacle_mat2);
		shader->set("M_it", glm::transpose(glm::inverse(obstacle_mat2)));
		ObstacleMesh->draw();
	}
}

void GameScene::DrawObstacle1(int random)
{
	//to handle special case (the first obstacle in the game don't draw it)
	if (TimeStamp > 250) 
	{
		position1.r = x1; //take the new x after moving


		if (x1 == disappearobstacle) //draw new obstacle with random position in z-axis and when become out of the view
		{
			if (random == 1)
				position1.b = 0;
			else if (random == 2)
				position1.b = right_large_obstacle;
			else
				position1.b = left_large_obstacle;
		}

		//Draw the obstacle 1
		glm::mat4 obstacle_mat1 = glm::translate(glm::mat4(), position1);
		obstacle_mat1 = glm::scale(obstacle_mat1, { 5, 5, large_obstacle });
		obstacle_mat1 = glm::rotate(obstacle_mat1, glm::pi<float>() / 2, { 0.0,-1.0,0.0 });
		shader->set("M", obstacle_mat1);
		shader->set("M_it", glm::transpose(glm::inverse(obstacle_mat1)));
		ObstacleMesh->draw();
	}
}

bool GameScene::MoveObstacles()
{
		for (int i = 0; i < obstacle_speed; i++)
		{
			x1 -= 0.5;
			x2 -= 0.5;
			x3 -= 0.5;
			x4 -= 0.5;
			x5 -= 0.5;
			Draw();
			if (TimeStamp > 250 && CollisionHappened()) //if collision happened
				return true;
		}
		return false;
}

void GameScene::NewObstacles()
{
	if (x1 <= disappearobstacle)
		x1 = startx;
	if (x2 <= disappearobstacle)
		x2 = startx;
	if (x3 <= disappearobstacle)
		x3 = startx;
	if (x4 <= disappearobstacle)
		x4 = startx;
	if (x5 <= disappearobstacle)
		x5 = startx;
}

void GameScene::DrawScore(int score)
{
	//glMatrixMode(GL_PROJECTION);
}

void GameScene::InitializeBullet()
{
	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	overlappingPairCache = new btDbvtBroadphase();

	collisionWorld = new btCollisionWorld(dispatcher, overlappingPairCache, collisionConfiguration);
}


void GameScene::CreateRigidBodies()
{
	CreateRunnerRigidBody();
	Obstacle1RigidBody();
	Obstacle2RigidBody();
	Obstacle3RigidBody();
	Obstacle4RigidBody();
	Obstacle5RigidBody();
}

void::GameScene::CreateRunnerRigidBody()
{
	btCollisionShape* RunnerShape = new btBoxShape(btVector3(btScalar(1), btScalar(2), btScalar(1)));

	collisionShapes.push_back(RunnerShape);

	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(btVector3(RunnerPosition.x, RunnerPosition.y, RunnerPosition.z));

	btScalar mass(1.);

	btVector3 localInertia(0, 0, 0);
	RunnerShape->calculateLocalInertia(mass, localInertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(Transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, RunnerShape, localInertia);
	RunnerBody = new btRigidBody(rbInfo);

	//add the body to the dynamics world
	collisionWorld->addCollisionObject(RunnerBody);
}

void GameScene::Obstacle1RigidBody()
{
	btCollisionShape* Obstacle1Shape = new btBoxShape(btVector3(btScalar(1), btScalar(3), btScalar(large_obstacle+3)));

	collisionShapes.push_back(Obstacle1Shape);

	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(btVector3(position1.x, position1.y, position1.z));

	btScalar mass(1.);

	btVector3 localInertia(0, 0, 0);
	Obstacle1Shape->calculateLocalInertia(mass, localInertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(Transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, Obstacle1Shape, localInertia);
	Obstacle1Body = new btRigidBody(rbInfo);

	//add the body to the dynamics world
	collisionWorld->addCollisionObject(Obstacle1Body);
}

void GameScene::Obstacle2RigidBody()
{
	btCollisionShape* Obstacle2Shape = new btBoxShape(btVector3(btScalar(1), btScalar(3), btScalar(small_obstacle+1)));

	collisionShapes.push_back(Obstacle2Shape);

	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(btVector3(position2.x, position2.y, position2.z));

	btScalar mass(1.);

	btVector3 localInertia(0, 0, 0);
	Obstacle2Shape->calculateLocalInertia(mass, localInertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(Transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, Obstacle2Shape, localInertia);
	Obstacle2Body = new btRigidBody(rbInfo);

	//add the body to the dynamics world
	collisionWorld->addCollisionObject(Obstacle2Body);
}

void GameScene::Obstacle3RigidBody()
{
	btCollisionShape* Obstacle3Shape = new btBoxShape(btVector3(btScalar(1), btScalar(3), btScalar(large_obstacle+3)));

	collisionShapes.push_back(Obstacle3Shape);

	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(btVector3(position3.x, position3.y, position3.z));

	btScalar mass(1.);

	btVector3 localInertia(0, 0, 0);
	Obstacle3Shape->calculateLocalInertia(mass, localInertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(Transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, Obstacle3Shape, localInertia);
	Obstacle3Body = new btRigidBody(rbInfo);

	//add the body to the dynamics world
	collisionWorld->addCollisionObject(Obstacle3Body);
}

void GameScene::Obstacle4RigidBody()
{
	btCollisionShape* Obstacle4Shape = new btBoxShape(btVector3(btScalar(1), btScalar(3), btScalar(small_obstacle+1)));

	collisionShapes.push_back(Obstacle4Shape);

	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(btVector3(position4.x, position4.y, position4.z));

	btScalar mass(1.);

	btVector3 localInertia(0, 0, 0);
	Obstacle4Shape->calculateLocalInertia(mass, localInertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(Transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, Obstacle4Shape, localInertia);
	Obstacle4Body = new btRigidBody(rbInfo);

	//add the body to the dynamics world
	collisionWorld->addCollisionObject(Obstacle4Body);
}

void GameScene::Obstacle5RigidBody()
{
	btCollisionShape* Obstacle5Shape = new btBoxShape(btVector3(btScalar(1), btScalar(3), btScalar(small_obstacle+1)));

	collisionShapes.push_back(Obstacle5Shape);

	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(btVector3(position5.x, position5.y, position5.z));

	btScalar mass(1.);

	btVector3 localInertia(0, 0, 0);
	Obstacle5Shape->calculateLocalInertia(mass, localInertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(Transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, Obstacle5Shape, localInertia);
	Obstacle5Body = new btRigidBody(rbInfo);
	
	//add the body to the dynamics world
	collisionWorld->addCollisionObject(Obstacle5Body);
}

bool GameScene::CollisionHappened()
{
	btTransform update;

	//update the Runner Postition
	update.setIdentity();
	update.setOrigin(btVector3(RunnerPosition.x,RunnerPosition.y,RunnerPosition.z));
	RunnerBody->setWorldTransform(update);

	//update obstacle 1 position
	update.setIdentity();
	update.setOrigin(btVector3(position1.x, position1.y, position1.z));
	Obstacle1Body->setWorldTransform(update);

	//update obstacle 2 position
	update.setIdentity();
	update.setOrigin(btVector3(position2.x, position2.y, position2.z));
	Obstacle2Body->setWorldTransform(update);

	//update obstacle 3 position
	update.setIdentity();
	update.setOrigin(btVector3(position3.x, position3.y, position3.z));
	Obstacle3Body->setWorldTransform(update);

	//update obstacle 4 position
	update.setIdentity();
	update.setOrigin(btVector3(position4.x, position4.y, position4.z));
	Obstacle4Body->setWorldTransform(update);

	//update obstacle 5 position
	update.setIdentity();
	update.setOrigin(btVector3(position5.x, position5.y, position5.z));
	Obstacle5Body->setWorldTransform(update);

	collisionWorld->performDiscreteCollisionDetection();
	int numManifolds = collisionWorld->getDispatcher()->getNumManifolds();
	//For each contact manifold
	for (int i = 0; i < numManifolds; i++) {
		btPersistentManifold* contactManifold = collisionWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* obA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
		const btCollisionObject* obB = static_cast<const btCollisionObject*>(contactManifold->getBody1());
		contactManifold->refreshContactPoints(obA->getWorldTransform(), obB->getWorldTransform());
		int numContacts = contactManifold->getNumContacts();
		if (numContacts)
			return true;
	}
	
	return false;
}

void GameScene::DestructBullet()
{
	//delete collision shapes
	for (int j = 0; j < collisionShapes.size(); j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	//delete dynamics world
	delete collisionWorld;

	//delete broadphase
	delete overlappingPairCache;

	//delete dispatcher
	delete dispatcher;

	delete collisionConfiguration;

	//next line is optional: it will be cleared by the destructor when the array goes out of scope
	collisionShapes.clear();
}

void GameScene::Finalize() {
	DestructBullet();
    delete camera;
	delete controller;
	delete ObstacleMesh;
	delete RunnerMesh;
    delete PlaneMesh;
	delete SkyMesh;
    delete shader,skyShader;
	for (int i = 0; i < 5; i++) {
		delete FloorTex[i];
		delete ObstacleTex1[i];
		delete RunnerTex[i];
		delete BoardTex[i];
	}
}