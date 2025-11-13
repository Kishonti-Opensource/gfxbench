/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_smart_vector_1350909053
#define INCLUDE_GUARD_smart_vector_1350909053

namespace ng {

/** @brief vector containing not-copyable objects
	@detail Usage example:
	Let's create a file descriptor class. It can be something like this:
	
	class FileDesc
	{
	public:
		FileDesc(FILE* f);
		~FileDesc(); //calls fclose()
	private:
		FILE* _f; //embedded file
		
		FileDesc(const FileDesc&); //disabled
		void operator=(const FileDesc&); //disabled
	};
	
	You need to add a method and a typedef to this class
	to use it with smart_vector
	class FileDesc
	{
		[...]
		typedef FILE* FileDesc::raw_type;
		raw_type release()
		{
			FILE* f = _f;
			_f = 0;
			return f;
		}
		[...]
	};
	
	The smart_vector will store raw_type objects, returns T& elements and
	frees them in dtor:

		ng::smart_vector<FileDesc> fileDescList;

	Alternatively, the raw_type can be defined as a template par to smart_vector

		ng::smart_vector<FileDesc, FILE*> fileDescList;

	In that case you don't need to define raw_type in your class, only release.
*/

template<typename T, typename R = typename T::raw_type>
class smart_vector
{
public:
	typedef size_t size_type;
	typedef T value_type;
	typedef typename T::raw_type raw_type;
	
	//typedef smart_vector_iterator<T> iterator;
	//typedef smart_vector_const_iterator<T> const_iterator;
	~smart_vector()
	{
		deleteItems();
	}

	size_type size() const { return _v.size(); }
	bool empty() const { return _v.empty(); }

	T& operator[](size_type i) { return *(T*)&_v[i]; }
	const T& operator[](size_type i) const { return *(T*)&_v[i]; }

	T& back() { return *(T*)&_v.back(); }
	const T& back() const { return *(T*)&_v.back(); }

	void push_back(T& p) //takes ownership!!!
	{
		raw_type tmp = p.release();
		_v.push_back(tmp);
	}

	/*iterator begin() { return iterator(_v.begin()); }
	const_iterator begin() const { return const_iterator(_v.begin()); }
	iterator end() { return iterator(_v.end()); }
	const_iterator end() const { return const_iterator(_v.end()); }
	iterator erase (iterator position)
	{
		delete &*position;
		return iterator(_v.erase(position.base()));
	}
	*/
	void clear()
	{
		deleteItems();
		_v.clear();
	}
	/*
	//moves all elements from 'y' before 'before'
	void transfer(iterator before, ptr_vector<T>& y)
	{
		_v.insert(before.base(), y._v.begin(), y._v.end());
		y._v.erase(y._v.begin(), y._v.end());
	}
	*/
	void reserve(size_type s)
	{
		_v.reserve(s);
	}
private:
	typedef std::vector<raw_type> Cont;
	Cont _v;

	void deleteItems()
	{
		for(typename Cont::iterator it = _v.begin(); it != _v.end(); ++it)
		{
			T tmp(*it); //takes pointer and calls dtor
		}
	}
};

}


#endif

