/****************************************************************************
 Copyright (c) 2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#pragma once

#include "../config.h"
#include "base/Log.h"
#include <map>
#include <string>

//#define RECORD_JSB_INVOKING

#if SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8

    #if defined(CC_DEBUG) & defined(RECORD_JSB_INVOKING)
extern unsigned int __jsbInvocationCount;
extern std::map<std::string, unsigned int> __jsbFunctionInvokedRecords;
    #endif

template <typename T, typename STATE>
constexpr inline T *SE_THIS_OBJECT(STATE &s) {
    return (T *)s.nativeThisObject();
}

template <typename T>
constexpr const typename std::enable_if<std::is_enum<T>::value, char *>::type SE_UNDERLYING_TYPE_NAME() {
    return typeid(std::underlying_type_t<T>).name();
}

template <typename T>
constexpr const typename std::enable_if<!std::is_enum<T>::value, char *>::type SE_UNDERLYING_TYPE_NAME() {
    return typeid(T).name();
}

namespace {
bool cmp(std::pair<std::string, int> &a, std::pair<std::string, int> &b) {
    return a.second < b.second;
}

std::vector<std::pair<std::string, int>> pairs;

void recordJSBInvoke(const std::string &funcName) {
    #if defined(CC_DEBUG) & defined(RECORD_JSB_INVOKING)
    ++__jsbInvocationCount;
    ++__jsbFunctionInvokedRecords[funcName];
    #endif
}
void clearRecordJSBInvoke() {
    #if defined(CC_DEBUG) & defined(RECORD_JSB_INVOKING)
    __jsbInvocationCount = 0;
    __jsbFunctionInvokedRecords.clear();
    #endif
}

void printJSBInvoke() {
    #if defined(CC_DEBUG) & defined(RECORD_JSB_INVOKING)
    for (const auto &it : __jsbFunctionInvokedRecords)
        pairs.push_back(it);

    std::sort(pairs.begin(), pairs.end(), cmp);
    CC_LOG_DEBUG("Start print JSB function record info....... %d times", __jsbInvocationCount);
    for (const auto &pair : pairs) {
        CC_LOG_DEBUG("%s is invoked %u times.", pair.first.c_str(), pair.second);
    }
    pairs.clear();
    CC_LOG_DEBUG("End print JSB function record info.......\n");
    #endif
}
} // namespace

    #ifdef __GNUC__
        #define SE_UNUSED __attribute__((unused))
    #else
        #define SE_UNUSED
    #endif

    #define SAFE_INC_REF(obj) \
        if (obj != nullptr) obj->incRef()
    #define SAFE_DEC_REF(obj)   \
        if ((obj) != nullptr) { \
            (obj)->decRef();    \
            (obj) = nullptr;    \
        }

    #define _SE(name) name##Registry

    #define SE_DECLARE_FUNC(funcName) \
        void funcName##Registry(const v8::FunctionCallbackInfo<v8::Value> &v8args)

    #define SE_BIND_FUNC(funcName)                                                                        \
        void funcName##Registry(const v8::FunctionCallbackInfo<v8::Value> &_v8args) {                     \
            recordJSBInvoke(#funcName);                                                                   \
            bool ret = false;                                                                             \
            v8::Isolate *_isolate = _v8args.GetIsolate();                                                 \
            v8::HandleScope _hs(_isolate);                                                                \
            SE_UNUSED unsigned argc = (unsigned)_v8args.Length();                                         \
            se::ValueArray args;                                                                          \
            args.reserve(10);                                                                             \
            se::internal::jsToSeArgs(_v8args, &args);                                                     \
            void *nativeThisObject = se::internal::getPrivate(_isolate, _v8args.This());                  \
            se::State state(nativeThisObject, args);                                                      \
            ret = funcName(state);                                                                        \
            if (!ret) {                                                                                   \
                SE_LOGE("[ERROR] Failed to invoke %s, location: %s:%d\n", #funcName, __FILE__, __LINE__); \
            }                                                                                             \
            se::internal::setReturnValue(state.rval(), _v8args);                                          \
        }

    #define SE_BIND_FINALIZE_FUNC(funcName)                                                               \
        void funcName##Registry(void *nativeThisObject) {                                                 \
            recordJSBInvoke(#funcName);                                                                   \
            if (nativeThisObject == nullptr)                                                              \
                return;                                                                                   \
            auto se = se::ScriptEngine::getInstance();                                                    \
            se->_setGarbageCollecting(true);                                                              \
            se::State state(nativeThisObject);                                                            \
            bool ret = funcName(state);                                                                   \
            if (!ret) {                                                                                   \
                SE_LOGE("[ERROR] Failed to invoke %s, location: %s:%d\n", #funcName, __FILE__, __LINE__); \
            }                                                                                             \
            se->_setGarbageCollecting(false);                                                             \
        }

    #define SE_DECLARE_FINALIZE_FUNC(funcName) \
        void funcName##Registry(void *nativeThisObject);

    // v8 doesn't need to create a new JSObject in SE_BIND_CTOR while SpiderMonkey needs.
    #define SE_BIND_CTOR(funcName, cls, finalizeCb)                                                       \
        void funcName##Registry(const v8::FunctionCallbackInfo<v8::Value> &_v8args) {                     \
            recordJSBInvoke(#funcName);                                                                   \
            v8::Isolate *_isolate = _v8args.GetIsolate();                                                 \
            v8::HandleScope _hs(_isolate);                                                                \
            bool ret = true;                                                                              \
            se::ValueArray args;                                                                          \
            args.reserve(10);                                                                             \
            se::internal::jsToSeArgs(_v8args, &args);                                                     \
            se::Object *thisObject = se::Object::_createJSObject(cls, _v8args.This());                    \
            thisObject->_setFinalizeCallback(_SE(finalizeCb));                                            \
            se::State state(thisObject, args);                                                            \
            ret = funcName(state);                                                                        \
            if (!ret) {                                                                                   \
                SE_LOGE("[ERROR] Failed to invoke %s, location: %s:%d\n", #funcName, __FILE__, __LINE__); \
            }                                                                                             \
            se::Value _property;                                                                          \
            bool _found = false;                                                                          \
            _found = thisObject->getProperty("_ctor", &_property);                                        \
            if (_found) _property.toObject()->call(args, thisObject);                                     \
        }

    #define SE_BIND_SUB_CLS_CTOR SE_BIND_CTOR

    #define SE_BIND_PROP_GET(funcName)                                                                               \
        void funcName##Registry(v8::Local<v8::Name> _property, const v8::PropertyCallbackInfo<v8::Value> &_v8args) { \
            recordJSBInvoke(#funcName);                                                                              \
            v8::Isolate *_isolate = _v8args.GetIsolate();                                                            \
            v8::HandleScope _hs(_isolate);                                                                           \
            bool ret = true;                                                                                         \
            void *nativeThisObject = se::internal::getPrivate(_isolate, _v8args.This());                             \
            se::State state(nativeThisObject);                                                                       \
            ret = funcName(state);                                                                                   \
            if (!ret) {                                                                                              \
                SE_LOGE("[ERROR] Failed to invoke %s, location: %s:%d\n", #funcName, __FILE__, __LINE__);            \
            }                                                                                                        \
            se::internal::setReturnValue(state.rval(), _v8args);                                                     \
        }

    #define SE_BIND_PROP_SET(funcName)                                                                                                       \
        void funcName##Registry(v8::Local<v8::Name> _property, v8::Local<v8::Value> _value, const v8::PropertyCallbackInfo<void> &_v8args) { \
            recordJSBInvoke(#funcName);                                                                                                      \
            v8::Isolate *_isolate = _v8args.GetIsolate();                                                                                    \
            v8::HandleScope _hs(_isolate);                                                                                                   \
            bool ret = true;                                                                                                                 \
            void *nativeThisObject = se::internal::getPrivate(_isolate, _v8args.This());                                                     \
            se::Value data;                                                                                                                  \
            se::internal::jsToSeValue(_isolate, _value, &data);                                                                              \
            se::ValueArray args;                                                                                                             \
            args.reserve(10);                                                                                                                \
            args.push_back(std::move(data));                                                                                                 \
            se::State state(nativeThisObject, args);                                                                                         \
            ret = funcName(state);                                                                                                           \
            if (!ret) {                                                                                                                      \
                SE_LOGE("[ERROR] Failed to invoke %s, location: %s:%d\n", #funcName, __FILE__, __LINE__);                                    \
            }                                                                                                                                \
        }

    #define SE_TYPE_NAME(t) typeid(t).name()

    #define SE_QUOTEME_(x) #x
    #define SE_QUOTEME(x)  SE_QUOTEME_(x)

    //IDEA: implement this macro
    #define SE_REPORT_ERROR(fmt, ...) SE_LOGE("[ERROR] (" __FILE__ ", " SE_QUOTEME(__LINE__) "): " fmt "\n", ##__VA_ARGS__)

    #if CC_DEBUG > 0

        #define SE_ASSERT(cond, fmt, ...)                                                                 \
            do {                                                                                          \
                if (!(cond)) {                                                                            \
                    SE_LOGE("ASSERT (" __FILE__ ", " SE_QUOTEME(__LINE__) "): " fmt "\n", ##__VA_ARGS__); \
                    assert(false);                                                                        \
                }                                                                                         \
            } while (false)

    #else

        #define SE_ASSERT(cond, fmt, ...)

    #endif // #if CC_DEBUG > 0

#endif // #if SCRIPT_ENGINE_TYPE == SCRIPT_ENGINE_V8