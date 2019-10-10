#ifndef GAME_SCENE_HPP
#define GAME_SCENE_HPP

#define GLM_FORCE_CXX11
#include <glm/glm.hpp>

#include <btBulletDynamicsCommon.h>

#include <scene.hpp>
#include <shader.hpp>
#include <mesh/mesh.hpp>
#include <textures/texture2d.hpp>
#include <camera/camera.hpp>
#include <camera/controllers/fly_camera_controller.hpp>


enum TextureType {
	ALBEDO = 0,
	SPECULAR = 1,
	ROUGHNESS = 2,
	AMBIENT_OCCLUSION = 3,
	EMISSIVE = 4,
	tex=5
};

class GameScene : public Scene {
private:
	Shader *shader, *skyShader;
	Mesh *PlaneMesh, *RunnerMesh, *ObstacleMesh,*SkyMesh;
	Camera* camera;
	FlyCameraController* controller;
	glm::mat4 transformation_effect;
	bool trans_effect, fog_effect;
	int jump_flag;
	int obstacle_speed;

	GLuint texloc,texlocsky, scloc;
	float sunYaw, sunPitch;

	glm::vec3 position5, position4, position3, position2, position1;
	float x1, x2, x3, x4, x5;
	//int size3, size1;
	float large_obstacle, small_obstacle, right_large_obstacle, left_large_obstacle;
	int right_small_obstacle, left_small_obstacle;
	int random;
	long long TimeStamp;

	glm::vec3 RunnerPosition ;
	glm::vec3 cam_pos;

	Texture2D *FloorTex[5], *RunnerTex[6], *ObstacleTex1[6],*BoardTex[6];

    GLuint mvpLoc, texLoc;

	btCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btCollisionWorld* collisionWorld;
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	btRigidBody* RunnerBody;
	btRigidBody* Obstacle1Body;
	btRigidBody* Obstacle2Body;
	btRigidBody* Obstacle3Body;
	btRigidBody* Obstacle4Body;
	btRigidBody* Obstacle5Body;

	void InitializeBullet();
	void CreateRunnerRigidBody();
	void CreateRigidBodies();
	void Obstacle1RigidBody();
	void Obstacle2RigidBody();
	void Obstacle3RigidBody();
	void Obstacle4RigidBody();
	void Obstacle5RigidBody();
	void DestructBullet();
	bool CollisionHappened();

	void DrawGround();
	void DrawRunner();
	void DrawObstacle5(int);
	void DrawObstacle4(int);
	void DrawObstacle3(int);
	void DrawObstacle2(int);
	void DrawObstacle1(int);
	void DrawScore(int);

	bool MoveObstacles();
	void NewObstacles();

public:
	GameScene(Application* app): Scene(app) {}

	bool Simulation(double delta_time,long long timestamp,bool & pause);
    void Initialize() override;
    void Update(double delta_time,long long timestamp,bool & pause) override;
    void Draw() override;
    void Finalize() override;
	void InitializeData();

};

#endif