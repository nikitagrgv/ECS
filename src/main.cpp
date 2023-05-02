#include <iostream>

#include <array>
#include <cassert>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define DECLARE_COMPONENT(name) \
    static inline const char *getComponentName() { return #name; }

using Entity = int;

class IComponentArray
{
public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
};

template <class T>
class ComponentArray final : public IComponentArray
{
public:
    void addData(Entity entity, T data)
    {
        assert(entity_to_index_.find(entity) == entity_to_index_.end());
        const int index = component_arr_.size();
        component_arr_.push_back(data);
        entity_to_index_[entity] = index;
        index_to_entity_[index] = entity;
    }

    void removeData(Entity entity)
    {
        assert(entity_to_index_.find(entity) != entity_to_index_.end());
        const int cur_index = entity_to_index_[entity];
        const int last_index = component_arr_.size() - 1;

        component_arr_[cur_index] = component_arr_[last_index];
        component_arr_.pop_back();

        const Entity last_ent = index_to_entity_[last_index];
        index_to_entity_[cur_index] = last_ent;
        entity_to_index_[last_ent] = cur_index;

        index_to_entity_.erase(last_index);
        entity_to_index_.erase(entity);
    }

    T &getData(Entity entity)
    {
        assert(entity_to_index_.find(entity) != entity_to_index_.end());
        return component_arr_[entity_to_index_[entity]];
    }

    void entityDestroyed(Entity entity) override
    {
        if (entity_to_index_.find(entity) != entity_to_index_.end())
        {
            removeData(entity);
        }
    }

private:
    std::unordered_map<Entity, int> entity_to_index_;
    std::unordered_map<int, Entity> index_to_entity_;
    std::vector<T> component_arr_;
};


class ComponentManager
{
public:
    template<class T>
    void registerComponent()
    {
        const char *name = get_component_name<T>();
        assert(component_arrays_.find(name) == component_arrays_.end());
        component_arrays_.emplace(name, std::make_unique<ComponentArray<T>>());
    }

    


private:
    template<class T>
    static inline const char *get_component_name()
    {
        return T::getComponentName();
    }



private:
    using ComponentArrayPtr = std::unique_ptr<IComponentArray>;
    std::unordered_map<const char *, ComponentArrayPtr> component_arrays_;
    //    std::vector<ComponentArrayPtr> component_arrays;


};


class ECS
{
public:
    template<class T>
    T &getComponent(Entity entity)
    {
    }
};

struct Position
{
    DECLARE_COMPONENT(Position);

    float x{};
    float y{};
    float z{};
};


int main()
{
//    ECS ecs;
    ComponentManager component_manager;
    component_manager.registerComponent<Position>();


    return 0;
}
