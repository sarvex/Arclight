/*
 *	 Copyright (c) 2021 - Arclight Team
 *
 *	 This file is part of Arclight. All rights reserved.
 *
 *	 physicsengine.hpp
 */

#pragma once

#include "dynamicsworld.hpp"
#include "time/profiler.hpp"
#include "time/timer.hpp"
#include "types.hpp"

#include <unordered_map>



//class RigidBody;
class ActorManager;

class PhysicsEngine {

public:

	explicit PhysicsEngine(ActorManager& actorManager) : actorManager(actorManager), tps(1) {}

	void init(u32 ticksPerSecond);
	void update();
	void destroy();

	bool createWorld(u32 worldID);
	void destroyWorld(u32 worldID);
	DynamicsWorld& getWorld(u32 worldID);
/*
	void onRigidBodyAdded(u32 worldID, RigidBody& body, ActorID actor);
	void onRigidBodyDeleted(u32 worldID, RigidBody& body);
*/
private:

	ActorManager& actorManager;
	std::unordered_map<u32, DynamicsWorld> dynamicWorlds;

	Profiler profiler;
	Timer simTimer;
	u32 tps;

};