/*
 * Copyright (c) 2011-2012 Promit Roy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include "ng/json.h"
#include <vector>
#include <list>
#include <set>
#include <map>
#include <type_traits>
#include <iterator>
#include <string>


class JsonSerializer
{
private:

    //SFINAE garbage to detect whether a type has a Serialize member
    typedef char SerializeNotFound;
    struct SerializeFound { char x[2]; };
    struct SerializeFoundStatic { char x[3]; };

    template<typename T, void (T::*)(JsonSerializer&)>
    struct SerializeTester { };
    template<typename T, void(*)(JsonSerializer&)>
    struct SerializeTesterStatic { };
    template<typename T>
    static SerializeFound SerializeTest(SerializeTester<T, &T::Serialize>*);
    template<typename T>
    static SerializeFoundStatic SerializeTest(SerializeTesterStatic<T, &T::Serialize>*);
    template<typename T>
    static SerializeNotFound SerializeTest(...);

    template<typename T>
    struct HasSerialize
    {
        static const bool value = sizeof(SerializeTest<T>(0)) == sizeof(SerializeFound);
    };

    //Serialize using a free function defined for the type (default fallback)
    template<typename TValue>
    void SerializeImpl(TValue& value,
                       typename std::enable_if<!HasSerialize<TValue>::value >::type* dummy = 0)
    {
        //prototype for the serialize free function, so we will get a link error if it's missing
        //this way we don't need a header with all the serialize functions for misc types (eg math)
        void Serialize(TValue&, JsonSerializer&);

        Serialize(value, *this);
    }

    //Serialize using a member function Serialize(JsonSerializer&)
    template<typename TValue>
    void SerializeImpl(TValue& value, typename std::enable_if<HasSerialize<TValue>::value >::type* dummy = 0)
    {
        value.Serialize(*this);
    }

public:
    JsonSerializer(bool isWriter)
    : IsWriter(isWriter)
    { }

    template<typename TKey, typename TValue>
    void Serialize(TKey key, TValue& value, typename std::enable_if<std::is_class<TValue>::value >::type* dummy = 0)
    {
        JsonSerializer subVal(IsWriter);
        if(!IsWriter)
            subVal.JsonValue = JsonValue[key];

        subVal.SerializeImpl(value);

        if(IsWriter) {
            JsonValue[key] = subVal.JsonValue;
        }
    }

    //Serialize a string value
    template<typename TKey>
    void Serialize(TKey key, std::string& value)
    {
        if(IsWriter)
            Write(key, value);
        else
            Read(key, value);
    }

    //Serialize a non class type directly using JsonCpp
    template<typename TKey, typename TValue>
    void Serialize(TKey key, TValue& value, typename std::enable_if<std::is_fundamental<TValue>::value >::type* dummy = 0)
    {
        if(IsWriter)
            Write(key, value);
        else
            Read(key, value);
    }

    //Serialize an enum type to JsonCpp
    template<typename TKey, typename TEnum>
    void Serialize(TKey key, TEnum& value, typename std::enable_if<std::is_enum<TEnum>::value >::type* dummy = 0)
    {
        int ival = (int) value;
        if(IsWriter)
        {
            Write(key, ival);
        }
        else
        {
            Read(key, ival);
            value = (TEnum) ival;
        }
    }

    //Serialize only when writing (saving), useful for r-values
    template<typename TKey, typename TValue>
    void WriteOnly(TKey key, TValue value, typename std::enable_if<std::is_fundamental<TValue>::value >::type* dummy = 0)
    {
        if(IsWriter)
            Write(key, value);
    }

    //Serialize a series of items by start and end iterators
    template<typename TKey, typename TItor>
    void WriteOnly(TKey key, TItor first, TItor last)
    {
        if(!IsWriter)
            return;

        JsonSerializer subVal(IsWriter);
        int index = 0;
        for(TItor it = first; it != last; ++it)
        {
            typename std::iterator_traits<TItor>::value_type val = *it;
            subVal.Serialize(index, val);
            ++index;
        }
        JsonValue[key] = subVal.JsonValue;
    }

    template<typename TItor>
    void ReadOnly(TItor first, TItor last)
    {
        if (IsWriter)
            return;
        if (!JsonValue.isArray())
            return;

        int i = 0;
        for (TItor it = first; it != last; ++it)
        {
            typename std::iterator_traits<TItor>::value_type val;
            Serialize(i, val);
            *it = val;
        }
    }

    template<typename TKey, typename TValue>
    void ReadOnly(TKey key, TValue& value, typename std::enable_if<std::is_fundamental<TValue>::value >::type* dummy = 0)
    {
        if(!IsWriter)
            Read(key, value);
    }

    template<typename TValue>
    void ReadOnly(std::vector<TValue>& vec)
    {
        vec.clear();
        if(IsWriter)
            return;
        if(!JsonValue.isArray())
            return;

        vec.reserve(vec.size() + JsonValue.size());
        for(size_t i = 0; i < JsonValue.size(); ++i)
        {
            TValue val;
            Serialize(i, val);
            vec.push_back(val);
        }
    }

    template<typename TKey, typename TValue>
    void Serialize(TKey key, std::vector<TValue>& vec)
    {
        if(IsWriter)
        {
            WriteOnly(key, vec.begin(), vec.end());
        }
        else
        {
            JsonSerializer subVal(IsWriter);
            subVal.JsonValue = JsonValue[key];
            subVal.ReadOnly(vec);
        }
    }

    template<typename TValue>
    void ReadOnly(std::set<TValue>& values)
    {
        values.clear();
        if (IsWriter)
            return;
        if (!JsonValue.isArray())
            return;

        for (size_t i = 0; i < JsonValue.size(); ++i)
        {
            TValue val;
            Serialize(i, val);
            values.insert(val);
        }
    }

    template<typename TKey, typename TValue>
    void Serialize(TKey key, std::set<TValue>& values)
    {
        if (IsWriter)
        {
            WriteOnly(key, values.begin(), values.end());
        }
        else
        {
            JsonSerializer subVal(IsWriter);
            subVal.JsonValue = JsonValue[key];
            subVal.ReadOnly(values);
        }
    }

    template<typename TValue>
    void ReadOnly(std::list<TValue> &list)
    {
        if (IsWriter)
        {
            return;
        }

        if (!JsonValue.isArray())
        {
            return;
        }

        list.clear();

        for (size_t i = 0; i < JsonValue.size(); ++i)
        {
            TValue val;
            Serialize(i, val);
            list.push_back(val);
        }
    }

    template<typename TKey, typename TValue>
    void Serialize(TKey key, std::list<TValue> &vec)
    {
        if (IsWriter)
        {
            WriteOnly(key, vec.begin(), vec.end());
        }
        else
        {
            JsonSerializer subVal(IsWriter);
            subVal.JsonValue = JsonValue[key];
            subVal.ReadOnly(vec);
        }
    }

    template<typename TKey, typename TMapKey, typename TMapValue>
    void Serialize(TKey key, std::map<TMapKey, TMapValue>& map)
    {
        JsonSerializer subVal(IsWriter);

        if (IsWriter)
        {
            JsonSerializer pairVal(true);
            for (auto it = map.begin(); it != map.end(); it++)
            {
                pairVal.WriteOnly("key", it->first);
                pairVal.WriteOnly("value", it->second);

                subVal.JsonValue.push_back(pairVal.JsonValue);

                pairVal.JsonValue.clear();
            }

            JsonValue[key] = subVal.JsonValue;
        }
        else
        {
            subVal.JsonValue = JsonValue[key];

            for (size_t i = 0; i < JsonValue.size(); i++)
            {
                TMapKey mapKey;
                TMapValue mapValue;

                subVal.JsonValue = JsonValue[i];
                subVal.Serialize("key", mapKey);
                subVal.Serialize("value", mapValue);

                map[mapKey] = mapValue;
            }
        }
    }

    //Append a ng::JsonValue directly
    template<typename TKey>
    void WriteOnly(TKey key, const ng::JsonValue& value)
    {
        Write(key, value);
    }

    //Forward a pointer
    template<typename TKey, typename TValue>
    void Serialize(TKey key, TValue* value, typename std::enable_if<!std::is_fundamental<TValue>::value >::type* dummy = 0)
    {
        Serialize(key, *value);
    }

    template<typename TKey, typename TValue>
    void WriteOnly(TKey key, TValue* value, typename std::enable_if<!std::is_fundamental<TValue>::value >::type* dummy = 0)
    {
        Serialize(key, *value);
    }

    template<typename TKey, typename TValue>
    void ReadOnly(TKey key, TValue* value, typename std::enable_if<!std::is_fundamental<TValue>::value >::type* dummy = 0)
    {
        ReadOnly(key, *value);
    }

    //Shorthand operator to serialize
    template<typename TKey, typename TValue>
    void operator()(TKey key, TValue& value)
    {
        Serialize(key, value);
    }

    ng::JsonValue JsonValue;
    bool IsWriter;

private:
    template<typename TKey, typename TValue>
    void Write(TKey key, TValue value)
    {
        JsonValue[key] = value;
    }

    template<typename TKey, typename TValue>
    void Read(TKey key, TValue& value, typename std::enable_if<std::is_arithmetic<TValue>::value >::type* dummy = 0)
    {
        int ival = JsonValue[key].numberD(value);
        value = (TValue) ival;
    }

    template<typename TKey>
    void Read(TKey key, bool& value)
    {
        value = JsonValue[key].booleanD(value);
    }

    template<typename TKey>
    void Read(TKey key, int& value)
    {
        value = (int)JsonValue[key].numberD(value);
    }

    template<typename TKey>
    void Read(TKey key, unsigned int& value)
    {
        value = (unsigned int)JsonValue[key].numberD(value);
    }

    template<typename TKey>
    void Read(TKey key, float& value)
    {
        value = (float) JsonValue[key].numberD(value);
    }

    template<typename TKey>
    void Read(TKey key, double& value)
    {
        value = JsonValue[key].numberD(value);
    }

    template<typename TKey>
    void Read(TKey key, std::string& value)
    {
        value = JsonValue[key].stringD(value.c_str());
    }
};

//"name value pair", derived from boost::serialization terminology
#define NVP(name) #name, name
#define SerializeNVP(name) Serialize(NVP(name))

#endif
