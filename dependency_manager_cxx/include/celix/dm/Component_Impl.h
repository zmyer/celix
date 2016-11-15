/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "celix/dm/Component.h"
#include "celix/dm/DependencyManager.h"
#include "celix/dm/ServiceDependency.h"

#include <memory>
#include <iostream>
#include <iomanip>

using namespace celix::dm;

template<class T>
Component<T>::Component(const bundle_context_pt context, std::string name) : BaseComponent(context, name) {}

template<class T>
Component<T>::~Component() {
    this->dependencies.clear();
}

template<class T>
template<class I>
Component<T>& Component<T>::addInterfaceWithName(const std::string serviceName, const std::string version, const Properties properties) {
    if (!serviceName.empty()) {
        //setup c properties
        properties_pt cProperties = properties_create();
        properties_set(cProperties, CELIX_FRAMEWORK_SERVICE_LANGUAGE, CELIX_FRAMEWORK_SERVICE_CXX_LANGUAGE);
        for (auto iter = properties.begin(); iter != properties.end(); iter++) {
            properties_set(cProperties, (char *) iter->first.c_str(), (char *) iter->second.c_str());
        }

        T* cmpPtr = &this->getInstance();
        I* intfPtr = static_cast<I*>(cmpPtr); //NOTE T should implement I

        const char *cVersion = version.empty() ? nullptr : version.c_str();
        component_addInterface(this->cComponent(), (char *) serviceName.c_str(), (char *) cVersion,
                               intfPtr, cProperties);
    } else {
        std::cerr << "Cannot add interface with a empty name\n";
    }

    return *this;
}

template<class T>
template<class I>
Component<T>& Component<T>::addInterface(const std::string version, const Properties properties) {
    //get name if not provided
    std::string serviceName = typeName<I>();
    if (serviceName.empty()) {
        std::cerr << "Cannot add interface, because type name could not be inferred. function: '"  << __PRETTY_FUNCTION__ << "'\n";
    }

    return this->addInterfaceWithName<I>(serviceName, version, properties);
};

template<class T>
Component<T>& Component<T>::addCInterface(const void* svc, const std::string serviceName, const std::string version, const Properties properties) {
    properties_pt cProperties = properties_create();
    properties_set(cProperties, CELIX_FRAMEWORK_SERVICE_LANGUAGE, CELIX_FRAMEWORK_SERVICE_C_LANGUAGE);
    for (auto iter = properties.begin(); iter != properties.end(); iter++) {
        properties_set(cProperties, (char*)iter->first.c_str(), (char*)iter->second.c_str());
    }

    const char *cVersion = version.empty() ? nullptr : version.c_str();
    component_addInterface(this->cComponent(), (char*)serviceName.c_str(), (char*)cVersion, svc, cProperties);

    return *this;
};

template<class T>
template<class I>
ServiceDependency<T,I>& Component<T>::createServiceDependency(const std::string name) {
#ifdef __EXCEPTIONS
    auto dep = std::shared_ptr<ServiceDependency<T,I>> {new ServiceDependency<T,I>()};
#else
    static ServiceDependency<T,I> invalidDep{std::string{}, false};
    auto dep = std::shared_ptr<ServiceDependency<T,I>> {new(std::nothrow) ServiceDependency<T,I>(name)};
    if (dep == nullptr) {
        return invalidDep;
    }
#endif
    this->dependencies.push_back(dep);
    component_addServiceDependency(cComponent(), dep->cServiceDependency());
    dep->setComponentInstance(&getInstance());
    return *dep;
}

template<class T>
template<class I>
Component<T>& Component<T>::remove(ServiceDependency<T,I>& dep) {
    component_removeServiceDependency(cComponent(), dep.cServiceDependency());
    this->dependencies.remove(dep);
    return *this;
}

template<class T>
template<typename I>
CServiceDependency<T,I>& Component<T>::createCServiceDependency(const std::string name) {
#ifdef __EXCEPTIONS
    auto dep = std::shared_ptr<CServiceDependency<T,I>> {new CServiceDependency<T,I>()};
#else
    static CServiceDependency<T,I> invalidDep{std::string{}, false};
    auto dep = std::shared_ptr<CServiceDependency<T,I>> {new(std::nothrow) CServiceDependency<T,I>(name)};
    if (dep == nullptr) {
        return invalidDep;
    }
#endif
    this->dependencies.push_back(dep);
    component_addServiceDependency(cComponent(), dep->cServiceDependency());
    dep->setComponentInstance(&getInstance());
    return *dep;
}

template<class T>
template<typename I>
Component<T>& Component<T>::remove(CServiceDependency<T,I>& dep) {
    component_removeServiceDependency(cComponent(), dep.cServiceDependency());
    this->dependencies.remove(dep);
    return *this;
}

template<class T>
Component<T>* Component<T>::create(bundle_context_pt context, std::string name) {
    std::string n = name.empty() ? typeName<T>() : name;
#ifdef __EXCEPTIONS
    Component<T>* cmp = new Component<T>{context, n};
#else
    static Component<T> invalid{nullptr, std::string{}};
    Component<T>* cmp = new(std::nothrow) Component<T>(context, n);
    if (cmp == nullptr) {
        cmp = &invalid;
    }
#endif
    return cmp;
}

template<class T>
bool Component<T>::isValid() const {
    return this->bundleContext() != nullptr;
}

template<class T>
T& Component<T>::getInstance() {
    if (this->refInstance.size() == 1) {
        return refInstance.front();
    } else {
        if (this->instance.get() == nullptr) {
#ifdef __EXCEPTIONS
            this->instance = std::shared_ptr<T> {new T()};
#else
            this->instance = std::shared_ptr<T> {new(std::nothrow) T()};

#endif
        }
        return *this->instance;
    }
}

template<class T>
Component<T>& Component<T>::setInstance(std::shared_ptr<T> inst) {
    this->instance = inst;
    return *this;
}

template<class T>
Component<T>& Component<T>::setInstance(T&& inst) {
    this->refInstance.clear();
    this->refInstance.push_back(std::move(inst));
    return *this;
}

template<class T>
Component<T>& Component<T>::setCallbacks(
            void (T::*init)(),
            void (T::*start)(),
            void (T::*stop)(),
            void (T::*deinit)() ) {

    this->initFp = init;
    this->startFp = start;
    this->stopFp = stop;
    this->deinitFp = deinit;

    int (*cInit)(void *) = [](void *handle) {
        Component<T>* cmp = (Component<T>*)(handle);
        T* inst = &cmp->getInstance();
        void (T::*fp)() = cmp->initFp;
        if (fp != nullptr) {
            (inst->*fp)();
        }
        return 0;
    };
    int (*cStart)(void *) = [](void *handle) {
        Component<T>* cmp = (Component<T>*)(handle);
        T* inst = &cmp->getInstance();
        void (T::*fp)() = cmp->startFp;
        if (fp != nullptr) {
            (inst->*fp)();
        }
        return 0;
    };
    int (*cStop)(void *) = [](void *handle) {
        Component<T>* cmp = (Component<T>*)(handle);
        T* inst = &cmp->getInstance();
        void (T::*fp)() = cmp->stopFp;
        if (fp != nullptr) {
            (inst->*fp)();
        }
        return 0;
    };
    int (*cDeinit)(void *) = [](void *handle) {
        Component<T>* cmp = (Component<T>*)(handle);
        T* inst = &cmp->getInstance();
        void (T::*fp)() = cmp->deinitFp;
        if (fp != nullptr) {
            (inst->*fp)();
        }
        return 0;
    };

    component_setCallbacks(this->cComponent(), cInit, cStart, cStop, cDeinit);

    return *this;
}
