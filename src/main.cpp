#include "ECS.h"
#include "MathUtils.h"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <iostream>

ECS ecs;

struct Position
{
    sf::Vector2<double> pos;
};

struct Velocity
{
    sf::Vector2<double> velocity;
};

struct Mass
{
    double mass{1.0f};
};

class PhysicsSystem : public System
{
public:
    static constexpr double GRAVITY = 100;

    void update(double dt)
    {
        for (const Entity &ent_0 : entities_)
        {
            const auto &pos_0 = ecs.getComponent<Position>(ent_0).pos;
            auto &speed_0 = ecs.getComponent<Velocity>(ent_0).velocity;
            const auto &mass_0 = ecs.getComponent<Mass>(ent_0).mass;

            for (const Entity &ent_1 : entities_)
            {
                if (&ent_0 == &ent_1)
                {
                    continue;
                }

                const auto &pos_1 = ecs.getComponent<Position>(ent_1).pos;
                auto &speed_1 = ecs.getComponent<Velocity>(ent_1).velocity;
                const auto &mass_1 = ecs.getComponent<Mass>(ent_1).mass;

                const sf::Vector2<double> dir_from_0_to_1 = pos_1 - pos_0;
                const double distance = Math::length(dir_from_0_to_1);
                const double force_module = GRAVITY * mass_0 * mass_1 / distance / distance;
                sf::Vector2<double> force = dir_from_0_to_1 * force_module;
                speed_0 += force * dt / mass_0;
            }
        }

        for (const Entity &ent : entities_)
        {
            auto &pos = ecs.getComponent<Position>(ent).pos;
            const auto &speed = ecs.getComponent<Velocity>(ent).velocity;
            pos += speed * dt;
        }
    }
};


class RenderSystem
    : public System
    , public sf::Drawable
{
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override
    {
        for (const Entity &entity : entities_)
        {
            sf::CircleShape shape{};
            const auto &pos = ecs.getComponent<Position>(entity);
            const auto &mass = ecs.getComponent<Mass>(entity);
            shape.setPosition((sf::Vector2f)pos.pos);
            const float radius = std::sqrt((float)mass.mass);
            shape.setRadius(radius);
            shape.setOrigin(Math::getBoundCenter(shape));
            target.draw(shape, states);
        }
    }
};

DECLARE_TYPE_INFO(Position);
DECLARE_TYPE_INFO(Mass);
DECLARE_TYPE_INFO(Velocity);

DECLARE_TYPE_INFO(PhysicsSystem);
DECLARE_TYPE_INFO(RenderSystem);

int main()
{
    sf::RenderWindow window(sf::VideoMode(1024, 768), "ecs");
    window.setVerticalSyncEnabled(true);

    ecs.registerComponents<Position, Mass, Velocity>();

    ecs.registerSystem<PhysicsSystem>();
    ecs.setSystemComponents<PhysicsSystem, Position, Mass, Velocity>();

    ecs.registerSystem<RenderSystem>();
    ecs.setSystemComponents<RenderSystem, Position, Mass>();

    auto *physic_sys = ecs.getSystem<PhysicsSystem>();
    auto *render_sys = ecs.getSystem<RenderSystem>();

    const auto create_ent = [](sf::Vector2<double> pos, sf::Vector2<double> vel, double mass) {
        const Entity ent = ecs.createEntity();
        ecs.addComponent<Position>(ent, Position{pos});
        ecs.addComponent<Velocity>(ent, Velocity{vel});
        ecs.addComponent<Mass>(ent, Mass{mass});
    };

    create_ent({}, {}, 170);
    create_ent({90, 0}, {0, -60}, 15);
    create_ent({-120, 0}, {0, 90}, 2);




    std::vector<sf::Vertex> axis;
    constexpr float AXIS_LENGTH = 10000.0f;
    axis.emplace_back(sf::Vector2f{-AXIS_LENGTH, 0.f}, sf::Color::Red);
    axis.emplace_back(sf::Vector2f{AXIS_LENGTH, 0.f}, sf::Color::Red);
    axis.emplace_back(sf::Vector2f{0.f, -AXIS_LENGTH}, sf::Color::Green);
    axis.emplace_back(sf::Vector2f{0.f, AXIS_LENGTH}, sf::Color::Green);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
            }
        }

        /////////////////////////////////////////////////////
        sf::View view = window.getView();

        sf::Vector2f camera_move_dir;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            camera_move_dir.y -= 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            camera_move_dir.y += 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            camera_move_dir.x -= 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            camera_move_dir.x += 1;
        constexpr float CAMERA_MOVE_SPEED = 4.0f;
        view.move(camera_move_dir * CAMERA_MOVE_SPEED);

        constexpr float ZOOM_FACTOR = 0.04;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
            view.zoom(1 + ZOOM_FACTOR);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
            view.zoom(1 / (1 + ZOOM_FACTOR));

        window.setView(view);

        /////////////////////////////////////////////////////

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
            constexpr int REPEATS = 10;
            constexpr double DELTA_TIME = 1.0 / 60.0 / (double)REPEATS;

            for (int i = 0; i < REPEATS; ++i)
            {
                physic_sys->update(DELTA_TIME);
            }
        }

        window.clear();
        window.draw(axis.data(), 4, sf::Lines);
        window.draw(*render_sys);
        window.display();
    }

    return 0;
}
