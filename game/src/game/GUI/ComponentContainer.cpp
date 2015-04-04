#include "game/GUI/ComponentContainer.hpp"
#include "game/GameStates/GameState.hpp"
#include "game/Core/GameEngine.hpp"
#include "game/GUI/GUIComponent.hpp"

ComponentContainer::ComponentContainer() :
    _componentList(),
    _componentListMutex(),
    _componentDestroyed(false)
{
	//ctor
}

void ComponentContainer::addComponent(std::shared_ptr<GUIComponent> component)
{
    _componentListMutex.lock();
    _componentList.push_back(component);
    _componentListMutex.unlock();
    component->_parent = this;
}

void ComponentContainer::removeComponent(std::shared_ptr<GUIComponent> component)
{
    _componentListMutex.lock();
    _componentList.erase(std::remove(_componentList.begin(), _componentList.end(), component), _componentList.end());
    _componentListMutex.unlock();
}

void ComponentContainer::clearComponents()
{
    _componentListMutex.lock();
    _componentList.clear();
    _componentListMutex.unlock();	
}

size_t ComponentContainer::getComponentCount() const
{
	return _componentList.size();
}

void ComponentContainer::drawAll()
{
    //Render the container itself
    drawContainer();

    //Draw reach GUI component
    _gameEngine->getUIManager()->beginRenderUI();
    _componentListMutex.lock();
    for(const std::shared_ptr<GUIComponent> component : _componentList)
    {
        if(!component->isVisible()) continue;  //Ignore hidden/destroyed components
        component->draw();
    }
    _componentListMutex.unlock();
    _gameEngine->getUIManager()->endRenderUI();
}

bool ComponentContainer::notifyMouseMoved(const int x, const int y)
{
    for(const std::shared_ptr<GUIComponent> &component : _componentList)
    {
        if(!component->isEnabled()) continue;
        if(component->notifyMouseMoved(x, y)) return true;
    }

    cleanDestroyedComponents();
    return false;
}

bool ComponentContainer::notifyKeyDown(const int keyCode)
{
    for(std::shared_ptr<GUIComponent> &component : _componentList)
    {
        if(!component->isEnabled()) continue;
        if(component->notifyKeyDown(keyCode)) return true;
    }
    cleanDestroyedComponents();
    return false;
}

bool ComponentContainer::notifyMouseClicked(const int button, const int x, const int y)
{
    for(const std::shared_ptr<GUIComponent> &component : _componentList)
    {
        if(!component->isEnabled()) continue;
        if(component->notifyMouseClicked(button, x, y)) return true;
    }
    cleanDestroyedComponents();
    return false;
}

bool ComponentContainer::notifyMouseScrolled(const int amount)
{
    for(const std::shared_ptr<GUIComponent> &component : _componentList)
    {
        if(!component->isEnabled()) continue;
        if(component->notifyMouseScrolled(amount)) return true;
    }
    cleanDestroyedComponents();
    return false;
}

void ComponentContainer::notifyDestruction()
{
    _componentDestroyed = true;
}

void ComponentContainer::cleanDestroyedComponents()
{
    if(!_componentDestroyed) return;

    _componentDestroyed = false;
    _componentListMutex.lock();
    _componentList.erase(std::remove_if(_componentList.begin(), _componentList.end(), 
        [](const std::shared_ptr<GUIComponent> &component) {return component->isDestroyed(); }), 
        _componentList.end());
    _componentListMutex.unlock();
}
