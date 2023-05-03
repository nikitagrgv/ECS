#include "ECS.h"

#include <iostream>

ECS ecs;

struct Position {
    DECLARE_TYPE_INFO(Position);

public:
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

struct Mass {
    DECLARE_TYPE_INFO(Mass);

public:
    float mass{1.0f};
};

class PhysicsSystem : public System
{
    DECLARE_TYPE_INFO(PhysicsSystem);

public:
    void update()
    {
        for (const Entity &entity: entities_)
        {
            auto &pos = ecs.getComponent<Position>(entity);
            auto &mass = ecs.getComponent<Mass>(entity);

            std::cout << "pos: " << pos.x << " " << pos.y << " " << pos.z << " | mass: " << mass.mass << std::endl;
            pos.x += 1;
            mass.mass -= 0.1;
        }
    }
};

int main()
{
    auto *system = ecs.registerSystem<PhysicsSystem>();

    ecs.registerComponents<Position, Mass>();
    ecs.setSystemComponents<PhysicsSystem, Position, Mass>();



    for (int i = 0; i < 5; ++i)
    {
        const Entity ent = ecs.createEntity();

        Position pos = {(float) i, (float) i * 10, (float) i * 100};
        ecs.addComponent<Position>(ent, pos);

        Mass mass = {(float) i};
        ecs.addComponent(ent, mass);
    }

    for (int i = 0; i < 5; ++i)
    {
        const Entity ent = ecs.createEntity();
        Position pos = {(float) i, (float) i * 10, (float) i * 100};
        ecs.addComponent<Position>(ent, pos);
    }


    for (int i = 0; i < 5; ++i)
    {
        std::cout << "iterate " << i << std::endl;
        system->update();
    }


    //    //    ECS ecs;
    //    EntityManager entity_manager;
    //    Entity e0 = entity_manager.createEntity();
    //    Entity e1 = entity_manager.createEntity();
    //
    //
    //    ComponentManager component_manager;
    //    component_manager.registerComponent<Position>();
    //    component_manager.addComponent(e0, Position{1, 2, 3});
    //    component_manager.addComponent(e1, Position{1, 2, 3});
    //
    //    entity_manager.removeEntity(e0);
    //    component_manager.entityDestroyed(e0);
    //
    //    auto &pos = component_manager.getComponent<Position>(1);


    return 0;
}
