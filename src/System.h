#pragma once

#include "Common.h"
#include "ComponentRegistry.h"

#include <concepts>
#include <functional>
#include <memory>
#include <vector>

namespace bad {

// TODO (bgluzman): support first arg being EntityHandle
class System {
public:
  template <Component... Args>
  static System construct(std::invocable<Args...> auto&& system) {
    return System(std::forward<decltype(system)>(system), tag<Args...>{});
  }

  std::vector<EntityId> getQueryComponents(ComponentRegistry& components) {
    return system_->getQueryComponents(components);
  }
  void execute(ComponentRegistry& components, EntityId entityId) {
    system_->execute(components, entityId);
  }

private:
  // TODO (bgluzman): can this be done with a deduction template instead?
  template <Component... Args>
  struct tag {};
  template <Component... Args>
  explicit System(std::invocable<Args...> auto&& system, tag<Args...>)
      : system_(std::make_unique<SystemImpl<Args...>>(
            std::forward<decltype(system)>(system))) {}

  struct SystemInterface {
    virtual ~SystemInterface() noexcept = default;
    virtual std::vector<EntityId>
                 getQueryComponents(ComponentRegistry& components) = 0;
    virtual void execute(ComponentRegistry& components,
                         EntityId           entityId) const = 0;
  };

  template <Component... Args>
  struct SystemImpl : public SystemInterface {
  public:
    explicit SystemImpl(std::invocable<Args...> auto&& system)
        : system_(std::forward<decltype(system)>(system)) {}

    std::vector<EntityId>
    getQueryComponents(ComponentRegistry& components) override {
      return components.getQueryComponents<Args...>();
    }

    void execute(ComponentRegistry& components,
                 EntityId           entityId) const override {
      system_(components.getUnchecked<Args>(entityId)...);
    }

  private:
    std::function<void(Args...)> system_;
  };

  std::unique_ptr<SystemInterface> system_;
};

}  // namespace bad