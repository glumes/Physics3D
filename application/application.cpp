#include "application.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>

#include "gui/screen.h"

#include "../util/log.h"

#include "tickerThread.h"

#include "gui\loader.h"
#include "resourceManager.h"
#include "objectLibrary.h"
#include "../engine/geometry/shape.h"
#include "../engine/geometry/managedShape.h"
#include "../engine/part.h"
#include "../engine/world.h"
#include "worlds.h"

#include "debug.h"

#include "../engine/math/mathUtil.h"

#include "../engine/geometry/convexShapeBuilder.h"
#include "../engine/engineException.h"

#include "../engine/physicsProfiler.h"

#define _USE_MATH_DEFINES
#include "math.h"

#define TICKS_PER_SECOND 500.0

#define TICK_SKIP_TIME std::chrono::milliseconds(3000)

// #define PROFILING_MAIN

Screen screen;
GravityFloorWorld world(Vec3(0.0, -10.0, 0.0));

TickerThread physicsThread;

Part* player;
bool flying = true;


void init();
void setupPhysics();
Part createVisiblePart(NormalizedShape s, CFrame position, double density, double friction);
Part createVisiblePart(Shape s, CFrame position, double density, double friction);

/*void debugCallback(unsigned  source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* userParam) {
	if (type == GL_DEBUG_TYPE_ERROR) 
		Log::error("(type 0x%x) %s", type, message);
	else
		Log::warn("(type=0x%x) %s", type, message);
}*/

Vec3 dominoBuf[8];
Shape dominoShape = BoundingBox{-0.1, -0.7, -0.3, 0.1, 0.7, 0.3}.toShape(dominoBuf);
int dominoID;

void createDominoAt(Vec3 pos, Mat3 rotation) {
	Part* domino = new Part(dominoShape, CFrame(pos, rotation), 1, 0.1);
	domino->drawMeshId = dominoID;
	world.addObject(domino);
}

void makeDominoStrip(int dominoCount) {
	for(int i = 0; i < dominoCount; i++) {
		createDominoAt(Vec3(i*0.5, 0.7, 1.3), Mat3());
	}
}

void makeDominoTower(int floors, int circumference, Vec3 origin) {
	double radius = circumference / 4.4;
	Mat3 sideways = fromEulerAngles(M_PI / 2, 0.0, 0.0);
	for(int floor = 0; floor < floors; floor++) {
		for(int j = 0; j < circumference; j++) {
			double angle = (2 * M_PI * (j + (floor % 2) / 2.0)) / circumference;
			Vec3 pos = Vec3(std::cos(angle)*radius, floor * 0.7 + 0.30, std::sin(angle) * radius);
			createDominoAt(pos + origin, rotY(-angle) * sideways);
		}
	}
}

inline int furthestIndexInDirection(Vec3* vertices, int vertexCount, Vec3 direction) {
	double bestDot = vertices[0] * direction;
	int bestVertexIndex = 0;
	for(int i = 1; i < vertexCount; i++) {
		double newD = vertices[i] * direction;
		if(newD > bestDot) {
			bestDot = newD;
			bestVertexIndex = i;
		}
	}

	return bestVertexIndex;
}

void profiling() {
	std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::high_resolution_clock::now();
	for(int i = 0; i < 10000; i++) {
		world.tick(0.002);
	}
	std::chrono::time_point<std::chrono::steady_clock> endTime = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds delta = endTime - startTime;
	Log::debug("Finished profiling, 10000 laps, %.9f seconds", delta * 1.0E-9);

	std::cin.get();
}




int main(void) {
#ifndef PROFILING_MAIN
	init();
#endif
	// player = new Part(BoundingBox(0.2, 1.5, 0.2).toShape(new Vec3[8]), CFrame(Vec3(0.0, 1.0, 0.0)), 1.0, 0.000001);
	
	int* builderRemovalBuffer = new int[1000];
	EdgePiece* builderAddingBuffer = new EdgePiece[1000];

	Part boxPart = createVisiblePart(dominoShape, CFrame(Vec3(1.5, 0.7, 0.3), fromEulerAngles(0.0, 0.2, 0.0)), 2.0, 0.7);
	//world.addObject(boxPart);

	dominoID = boxPart.drawMeshId;

	Vec2 floorSize(40.0, 80.0);
	double wallHeight = 3.0;

	Part floorPart = createVisiblePart(BoundingBox(floorSize.x, 0.3, floorSize.y).toShape(new Vec3[8]), CFrame(Vec3(0.0, -0.15, 0.0)), 0.2, 1.0);
	world.addObject(floorPart, true);

	Part xWallTemplate = createVisiblePart(BoundingBox(0.2, wallHeight, floorSize.y).toShape(new Vec3[8]), CFrame(Vec3(floorSize.x/2, wallHeight/2, 0.0)), 0.2, 1.0);
	Part zWallTemplate = createVisiblePart(BoundingBox(floorSize.x, wallHeight, 0.2).toShape(new Vec3[8]), CFrame(Vec3(0.0, wallHeight / 2, floorSize.y/2)), 0.2, 1.0);
	world.addObject(xWallTemplate, true);
	world.addObject(zWallTemplate, true);
	xWallTemplate.cframe = CFrame(Vec3(-floorSize.x/2, wallHeight/2, 0.0));
	world.addObject(xWallTemplate, true);
	zWallTemplate.cframe = CFrame(Vec3(0.0, wallHeight/2, -floorSize.y/2));
	world.addObject(zWallTemplate, true);

	Part ramp = createVisiblePart(BoundingBox(10.0, 0.17, 3.0).toShape(new Vec3[8]), CFrame(Vec3(12.0, 1.5, 0.0), fromEulerAngles(M_PI / 2 * 0.2, M_PI/2, 0.0)), 0.2, 1.0);
	world.addObject(ramp, true);

	/*Shape plate = BoundingBox(1.0, 0.03, 1.0).toShape(new Vec3[8]);
	Part platePart = createVisiblePart(plate, CFrame(), 1.0, 0.3);

	Vec3 center(-10, 10.0, 10);
	long long seed = 1368734354L;

	for(int i = 0; i < 1000; i++) {
		seed = (seed * seed + 3638738438487L) * seed + 387464576768764353L + seed >> 9;
		double x = ((seed & 0xFFFF) * (1.0 / (2 << 16)) - 0.5) * 10;
		seed = (seed * seed + 3638738438487L) * seed + 387464576768764353L + seed >> 9;
		double y = ((seed & 0xFFFF) * (1.0 / (2 << 16)) - 0.5) * 10;
		seed = (seed * seed + 3638738438487L) * seed + 387464576768764353L + seed >> 9;
		double z = ((seed & 0xFFFF) * (1.0 / (2 << 16)) - 0.5) * 10;

		seed = (seed * seed + 3638738438487L) * seed + 387464576768764353L + seed >> 9;
		double rx = ((seed & 0xFFFF) * (1.0 / (2 << 16)) - 0.5) * 10;
		seed = (seed * seed + 3638738438487L) * seed + 387464576768764353L + seed >> 9;
		double ry = ((seed & 0xFFFF) * (1.0 / (2 << 16)) - 0.5) * 10;
		seed = (seed * seed + 3638738438487L) * seed + 387464576768764353L + seed >> 9;
		double rz = ((seed & 0xFFFF) * (1.0 / (2 << 16)) - 0.5) * 10;

		platePart.cframe = CFrame(center + Vec3(x, y, z), fromEulerAngles(rx, ry, rz));
		world.addObject(platePart, true);
	}*/


	//makeDominoStrip(20);
	//makeDominoTower(25, 20, Vec3(-4.0, 0.0, -4.0));

	// Shape tetrahedronShape = 

	Vec3 newIcosaVerts[30];
	Triangle newIcosaTriangles[40];
	TriangleNeighbors icosaNeighBuf[40];

	ConvexShapeBuilder icosaBuilder(icosahedron, newIcosaVerts, newIcosaTriangles, icosaNeighBuf, builderRemovalBuffer, builderAddingBuffer);

	if(!icosaBuilder.toIndexedShape().isValid()) {
		throw "BAD";
	}

	icosaBuilder.addPoint(Vec3(0, 1.1, 0));
	icosaBuilder.addPoint(Vec3(0, -1.1, 0));
	icosaBuilder.addPoint(Vec3(1.1, 0, 0));
	icosaBuilder.addPoint(Vec3(-1.1, 0, 0));
	icosaBuilder.addPoint(Vec3(0, 0, 1.1));
	icosaBuilder.addPoint(Vec3(0, 0, -1.1));

	if(!icosaBuilder.toIndexedShape().isValid()) {
		throw "BAD";
	}

	Shape newIcosa = icosaBuilder.toShape();
	Part constructedIcosa = createVisiblePart(newIcosa, CFrame(Vec3(10, 0, 0)), 2.0, 0.7);
	//world.addObject(constructedIcosa);

	Vec3 verts[10]{Vec3(0.0, 0.0, 0.0), Vec3(1.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0), Vec3(0.0, 1.0, 0.0)};
	Triangle triangles[20]{{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
	TriangleNeighbors neighBuf[20];

	ConvexShapeBuilder builder(verts, triangles, 4, 4, neighBuf, builderRemovalBuffer, builderAddingBuffer);

	builder.addPoint(Vec3(0.4, 0.4, 0.4), 3);

	builder.addPoint(Vec3(-0.4, 1, -0.4));

	builder.addPoint(Vec3(-0.8, 0.5, -0.8));

	builder.addPoint(Vec3(-0.9, 0.6, -0.9));

	Shape constructedShape = builder.toShape();

	Part constructedPart = createVisiblePart(constructedShape, CFrame(), 2.0, 0.7);
	//world.addObject(constructedPart);

	

	Part cube = createVisiblePart(BoundingBox{-0.49, -0.49, -0.49, 0.49, 0.49, 0.49}.toShape(new Vec3[8]), CFrame(), 1.0, 0.5);
	Shape sphereShape = loadMesh((std::istream&) std::istringstream(getResourceAsString(SPHERE_MODEL)));
	Part spherePart = createVisiblePart(sphereShape, CFrame(Vec3(0.0, 10.0, 0.0), fromEulerAngles(0.9, 0.1, 0.5)), 2, 0.7);
	Part trianglePart = createVisiblePart(triangleShape, CFrame(Vec3(-2.0, 1.0, -2.0)), 10.0, 0.7);
	for(int x = 0; x < 4; x++) {
		for(int y = 0; y < 4; y++) {
			for(int z = 0; z < 4; z++) {
				cube.cframe.position = Vec3(x + 5, y + 1, z);
				world.addObject(cube);
				spherePart.cframe.position = Vec3(x - 5, y + 1, z);
				world.addObject(spherePart);
				trianglePart.cframe.position = Vec3(x, y + 1, z);
				world.addObject(trianglePart);
			}
		}
	}
	
	/*Part icosaPart = createVisiblePart(icosahedron, CFrame(Vec3(0.0, 2.0, 3.0), fromEulerAngles(0.1, 0.1, 0.1)), 10, 0.7);
	world.addObject(icosaPart);

	Part housePart = createVisiblePart(house, CFrame(Vec3(-1.5, 1.0, 0.0), fromEulerAngles(0.7, 0.9, 0.7)), 1.0, 0.0);
	world.addObject(housePart);*/

	/*Shape sphereShape = loadMesh((std::istream&) std::istringstream(getResourceAsString(SPHERE_MODEL)));
	Part spherePart = createVisiblePart(sphereShape, CFrame(Vec3(0.0, 10.0, 0.0), fromEulerAngles(0.9, 0.1, 0.5)), 2, 0.7);
	for(int i = 0; i < 10; i++) {
		spherePart.cframe = CFrame(Vec3(0.0, 10.0 + i, 0.0));
		world.addObject(spherePart);
	}*/

	player = new Part(spherePart);
	player->properties.friction = 0;
	player->drawMeshId = -1;

	//Part trianglePart = createVisiblePart(triangleShape, CFrame(Vec3(-2.0, 1.0, -2.0)), 10.0, 0.7);
	//world.addObject(trianglePart);
	
#ifdef PROFILING_MAIN
	profiling();
	return 0;
#endif

	/* Loop until the user closes the window */
	Log::info("Started rendering");
	while (!screen.shouldClose()) {
		screen.graphicsMeasure.mark(GraphicsProcess::UPDATE);
		screen.update();
		screen.refresh();
		screen.graphicsMeasure.end();
		// test
		glfwSetWindowTitle(screen.getWindow(), std::to_string(physicsThread.getTPS()).c_str());
	}

	Log::info("Closing screen");
	screen.close();
	Log::info("Closed screen");

	stop(0);
}

void init() {
	AppDebug::setupDebugHooks();

	if (!initGLFW()) {
		Log::error("GLFW not initialised");
		std::cin.get();
		stop(-1);
	}

	screen = Screen(800, 640, &world);

	if (!initGLEW()) {
		Log::error("GLEW not initialised");
		std::cin.get();
		stop(-1);
	}

	Log::info("Initializing screen");
	screen.init();
	Log::info("Initialized screen");

	setupPhysics();
}

void stop(int returnCode) {
	physicsThread.stop();

	glfwTerminate();
	exit(returnCode);
}

bool paused = true;
void togglePause() {
	if (paused)
		unpause();
	else
		pause();
}

void pause() {
	physicsThread.stop();
	paused = true;
}

void unpause() {
	physicsThread.start();
	paused = false;
}

bool isPaused() {
	return paused;
}

void setSpeed(double newSpeed) {
	physicsThread.setSpeed(newSpeed);
}

double getSpeed() {
	return physicsThread.getSpeed();
}

void runTick() {
	physicsThread.runTick();
}

void setupPhysics() {
	physicsThread = TickerThread(TICKS_PER_SECOND, TICK_SKIP_TIME, [](){
		physicsMeasure.mark(PhysicsProcess::OTHER);
		AppDebug::logTickStart();
		//try {
			world.tick(1 / physicsThread.getTPS());
		/*} catch(EngineException& ex) {
			Log::error("EngineException: \n %s", ex.what());
			__debugbreak();
		} catch(std::exception& ex) {
			Log::error("std::exception occurred! \n %s", ex.what());
			__debugbreak();
		} catch(const char* ex) {
			Log::error("char* exception occurred! \n %s", ex);
			__debugbreak();
		}*/
		AppDebug::logTickEnd();
		physicsMeasure.end();
	});
}

Part createVisiblePart(NormalizedShape shape, CFrame position, double density, double friction) {
#ifndef PROFILING_MAIN
	int id = screen.addMeshShape(shape);
#else
	int id = 0;
#endif
	Part part(shape, position, density, friction);
	part.drawMeshId = id;
	return part;
}

Part createVisiblePart(Shape shape, CFrame position, double density, double friction) {
	Part part(shape, position, density, friction);
#ifndef PROFILING_MAIN
	int id = screen.addMeshShape(part.hitbox);
#else
	int id = 0;
#endif
	part.drawMeshId = id;
	return part;
}

void toggleFlying() {
	if(flying) {
		player->cframe = screen.camera.cframe;
		screen.camera.attachment = player;
		screen.world->addObject(player);
		flying = false;
	} else {
		screen.camera.attachment = nullptr;
		screen.world->removePart(player);
		flying = true;
	}
}

Camera& getCamera() {
	return screen.camera;
}
