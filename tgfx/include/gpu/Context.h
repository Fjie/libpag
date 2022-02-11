/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making libpag available.
//
//  Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "core/Color4f.h"
#include "core/utils/BytesKey.h"
#include "gpu/Device.h"
#include "pag/gpu.h"

namespace pag {
class ProgramCache;

class GradientCache;

class ResourceCache;

class Caps;

class Context {
 public:
  virtual ~Context();

  /**
   * Returns the associated device.
   */
  Device* device() const {
    return _device;
  }

  /**
   * Returns the associated cache that manages the lifetime of all gradients.
   */
  GradientCache* gradientCache() const {
    return _gradientCache;
  }

  /**
   * Returns the associated cache that manages the lifetime of all Program instances.
   */
  ProgramCache* programCache() const {
    return _programCache;
  }

  /**
   * Returns the associated cache that manages the lifetime of all Resource instances.
   */
  ResourceCache* resourceCache() const {
    return _resourceCache;
  }

  /**
   * Purges GPU resources that haven't been used in the past 'usNotUsed' microseconds.
   */
  void purgeResourcesNotUsedIn(int64_t usNotUsed);

  /**
   * Returns the GPU backend of this context.
   */
  virtual Backend backend() const = 0;

  /**
   * Returns the capabilities info of this context.
   */
  virtual const Caps* caps() const = 0;

 protected:
  explicit Context(Device* device);

 private:
  Device* _device = nullptr;
  GradientCache* _gradientCache = nullptr;
  ProgramCache* _programCache = nullptr;
  ResourceCache* _resourceCache = nullptr;

  void releaseAll(bool releaseGPU);
  void onLocked();
  void onUnlocked();

  friend class Device;

  friend class Resource;
};

}  // namespace pag
