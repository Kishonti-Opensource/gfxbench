/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_object_1377856261
#define INCLUDE_GUARD_object_1377856261

#include <string>

namespace ng {

/*
	The Object is really a 'PolymorphObject'. It's purpose is to provide a Java/Object like
	base class for all objects.
*/

class Object
{
public:
	Object() {}
	virtual std::string toString() const; //default impl: same as java.lang.Object.toString
	virtual ~Object() {}
protected:
	Object(const Object&) {}	
	Object& operator=(const Object&) { return *this; }
};

}


#endif

