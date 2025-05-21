//
// Created by Yibuz Pokopodrozo on 2025-05-12.
//

#pragma once
class Component {
public:
    explicit Component(Component* parent_) :parent(parent_), isCreated(false) {}
    virtual ~Component() = default;
    virtual bool OnCreate() = 0;
    virtual void OnDestroy() = 0;
    virtual void Update(float deltaTime_) = 0;
    virtual void Render()const = 0;
protected:
    Component* parent;
    /// Just a flag to indicate if the component or actor that inherits this
    /// base class has called OnCreate (true)
    bool isCreated;
};