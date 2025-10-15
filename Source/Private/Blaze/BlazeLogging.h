/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "Logging/LogCategory.h"

// Log categories that are primarily used when debugging or understanding plugin interactions and
// are not expected to be used at runtime.
//
// Category usage:
//	Warning/Error - Unexpected state or problem occurred. Should be addressed.
//	Log - What occurred? Plugin users care about this.
//	Verbose - Why did it occur? Plugin developers care about this.
//  VeryVerbose - What didn't occur and why did it NOT occur? Plugin developers care about this.

#ifdef UE_BUILD_SHIPPING
DECLARE_LOG_CATEGORY_EXTERN(LogBlaze, Warning, Warning);
#else
DECLARE_LOG_CATEGORY_EXTERN(LogBlaze, Log, All);
#endif
