#ifndef CONTAINERS_CORE_CONTAINERMANAGER_H
#define CONTAINERS_CORE_CONTAINERMANAGER_H

#include <map>
#include <memory>
#include <string>
#include "IContainer.h"

namespace ContainerSystem::Core {

class ContainerManager {
public:
    static ContainerManager& instance() {
        static ContainerManager instance;
        return instance;
    }

    template<typename T>
    void registerContainer(const std::string& name, std::shared_ptr<IContainer<T>> container) {
        containers[name] = std::static_pointer_cast<void>(container);
    }

    template<typename T>
    std::shared_ptr<IContainer<T>> getContainer(const std::string& name) {
        auto it = containers.find(name);
        if (it != containers.end()) {
            return std::static_pointer_cast<IContainer<T>>(it->second);
        }
        return nullptr;
    }

private:
    ContainerManager() = default;
    // map name to container pointers, type erased
    std::map<std::string, std::shared_ptr<void>> containers;
};

} // namespace ContainerSystem::Core

#endif // CONTAINERS_CORE_CONTAINERMANAGER_H