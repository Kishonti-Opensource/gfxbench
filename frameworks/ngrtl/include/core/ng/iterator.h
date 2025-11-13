/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_iterator_1346160484
#define INCLUDE_GUARD_iterator_1346160484

#include "ng/scoped_ptr.h"
#include "ng/move.h"

#include <algorithm>

/* Polymorphic iterator helper classes

	You need polymorphic (virtual) iterators defined when your container interface class needs iterators.

	The basic concept is that you implement the an iterator interface.
	Since we use iterators as stack objects and can't handle virtual objects as stack object
	you need a interface holder object, as well. It's like a smart pointer, sole purpose is
	to forward usual iterator methods (++, !=) to the actual virtual iterator contained by it.

	1. This is your base virtual container interface. Inside the typedef defines the iterator:

	   class my_double_container_interface
	   {
	   public:
	       typedef const_forward_virtual_iterator_holder<double> const_iterator;
	       ....
		   virtual double& operator[](size_t idx) = 0;
		   ....
	   };

	2. This is an implementation of the base container interface:
	   
	   class my_double_container_implementation
	   : public my_double_container_interface
	   {
	   public:
		   virtual double& operator[](size_t idx) { ... }
	   };


	3. Implement the virtual iterator interface

	   class my_double_container_implementation
	   : public my_double_container_interface
	   {
	       class my_iterator_impl
		   : public my_iterator::interface_type
		   {
			....
		   };
	   public:
		   virtual double& operator[](size_t idx) { ... }
	   };

	4. return objects of type my_iterator_impl in begin() and end() methods

		my_iterator begin() { return my_iterator(new my_double_container_interface(...) ); }
		my_iterator end() { return my_iterator(new my_double_container_interface(...)); } //end iterator can be indicated by default-constructed iterator

	5. for loops can be optimized by storing the end value before the loop
		my_it itEnd = cont.end();
		
			for(my_it it = cont.begin(); it != itEnd; ++it) ...

		since virtual end() calls will not be optimized away.
	
*/


namespace ng
{

	/* ---- const_iterator_interface ----
	
	   Interface class for polymorphic iterators
	   Supports the nullptr object = end() conception (operator==)
	*/

	template<typename VALUE_TYPE>
	class const_forward_virtual_iterator_interface
	{
	public:
		typedef VALUE_TYPE value_type;
		typedef const_forward_virtual_iterator_interface<value_type> this_type;

		virtual ~const_forward_virtual_iterator_interface() {}

		this_type& operator++() //prefix form, postfix is not viable because of the lack of auto garb coll
		{
			increment();
			return *this;
		}

		virtual bool operator==(const this_type& y) const = 0;
		bool operator!=(const this_type& y) const { return !(*this == y); }
		const value_type* operator->() const { return &operator*(); }

		// ---- virtual interface ----
		virtual const value_type& operator*() const = 0;
		virtual void increment() = 0;
	};
	
	template<typename VALUE_TYPE>
	class const_forward_virtual_iterator_holder
	{
	public:
		typedef VALUE_TYPE value_type;
		typedef const_forward_virtual_iterator_interface<value_type> interface_type;
		typedef const_forward_virtual_iterator_holder<value_type> this_type;

		const_forward_virtual_iterator_holder() {}
		const_forward_virtual_iterator_holder(interface_type* p)
			: _iit(p)
		{}
		const_forward_virtual_iterator_holder(const this_type& y) //move semantics
			: _iit(y._iit.release())
		{}
	private:
		this_type& operator=(const this_type& y); //disabled
	public:
		bool operator!=(const this_type& y) const { return *_iit != *y._iit; }
		bool operator==(const this_type& y) const { return *_iit == *y._iit; }
		this_type& operator++() //prefix
		{
			++*_iit;
			return *this;
		}

		void increment()
		{ ++*_iit; }

		const value_type& operator*() const { return **_iit; }
		const value_type* operator->() const { return &**_iit; }
	private:
		scoped_ptr<interface_type> _iit;
	};

	template<typename VALUE_TYPE>
	class const_forward_virtual_ending_iterator_interface
	{
	public:
		typedef VALUE_TYPE value_type;
		typedef const_forward_virtual_ending_iterator_interface<value_type> this_type;

		virtual ~const_forward_virtual_ending_iterator_interface() {}

		this_type& operator++() //prefix form, postfix is not viable because of the lack of auto garb coll
		{
			increment();
			return *this;
		}

		const value_type* operator->() const { return &operator*(); }

		// ---- virtual interface ----
		virtual const value_type& operator*() const = 0;

		virtual bool end() const = 0;
		virtual void increment() = 0;
	};
	
	template<typename VALUE_TYPE>
	class const_forward_virtual_ending_iterator_holder
	{
	public:
		typedef VALUE_TYPE value_type;
		typedef const_forward_virtual_ending_iterator_interface<value_type> interface_type;
		typedef const_forward_virtual_ending_iterator_holder<value_type> this_type;
		struct end_type {};

		const_forward_virtual_ending_iterator_holder()
		: _iit(nullptr)
		{}

		explicit const_forward_virtual_ending_iterator_holder(interface_type* p) //takes ownership
			: _iit(p)
		{}

		~const_forward_virtual_ending_iterator_holder()
		{ delete _iit; }

		const_forward_virtual_ending_iterator_holder(NG_RV_REF(this_type) y)
			: _iit(y._iit)
		{ y._iit = 0; }

		this_type& operator=(NG_RV_REF(this_type) y)
		{ swap(y); return *this; }

		void swap(this_type& y)
		{
			std::swap(_iit, y._iit);
		}

		this_type& operator++() //prefix
		{
			++*_iit;
			return *this;
		}

		void increment()
		{
			++*_iit;
		}

		const value_type& operator*() const { return **_iit; }
		const value_type* operator->() const { return &**_iit; }

		bool end() const
		{ return _iit->end(); }

		bool operator==(const end_type&) const { return end(); }
		bool operator!=(const end_type&) const { return !end(); }
	private:
		NG_MOVABLE_BUT_NOT_COPYABLE(const_forward_virtual_ending_iterator_holder);

		interface_type* _iit; //owner
	};
}


#endif

