//
// Created by koncord on 23.01.16.
//

#include<iostream>
#include <stdexcept>
#include "ScriptFunction.hpp"

#if defined (ENABLE_LUA)
#include "LangLua/LangLua.hpp"
#endif
#if defined(ENABLE_MONO)
#include "LangMono/LangMono.hpp"
#endif

using namespace std;

ScriptFunction::ScriptFunction(ScriptFunc fCpp,char ret_type, const string &def) :
        fCpp(fCpp), ret_type(ret_type), def(def), script_type(SCRIPT_CPP)
{

}
#if defined (ENABLE_LUA)
ScriptFunction::ScriptFunction(const ScriptFuncLua &fLua, lua_State *lua, char ret_type, const std::string &def) :
        fLua({lua, fLua}), ret_type(ret_type), def(def), script_type(SCRIPT_LUA)
{

}
#endif
#if defined (ENABLE_MONO)
ScriptFunction::ScriptFunction(MonoObject *delegate, char ret_type, const std::string &def) :
        fMono({delegate}), ret_type(ret_type), def(def), script_type(SCRIPT_MONO)
{
}
#endif

ScriptFunction::~ScriptFunction()
{
#if defined (ENABLE_LUA)
    if (script_type == SCRIPT_LUA)
        fLua.name.~ScriptFuncLua();
#endif
}

boost::any ScriptFunction::Call(const vector<boost::any> &args)
{
    boost::any result = boost::any();

    if (def.length() != args.size())
        throw runtime_error("Script call: Number of arguments does not match definition");
#if defined (ENABLE_LUA)
    else if (script_type == SCRIPT_LUA)
    {
        LangLua langLua(fLua.lua);
        boost::any any = langLua.Call(fLua.name.c_str(), def.c_str(), args);

        switch (ret_type)
        {
            case 'i':
                result = boost::any_cast<luabridge::LuaRef>(any).cast<unsigned int>();
                break;
            case 'q':
                result = boost::any_cast<luabridge::LuaRef>(any).cast<signed int>();
                break;
            case 'f':
                result = boost::any_cast<luabridge::LuaRef>(any).cast<double>();
                break;
            case 's':
                result = boost::any_cast<luabridge::LuaRef>(any).cast<const char*>();
                break;
            case 'v':
                break;
            default:
                throw runtime_error("Lua call: Unknown return type" + ret_type);
        }
    }
#endif
#if defined (ENABLE_MONO)
    else if (script_type == SCRIPT_MONO)
    {
        std::vector<void *> argList(args.size());
        TMonoArgsStore argsStore(args.size());

        for (size_t index = 0; index < args.size(); index++)
        {
            switch (def[index])
            {
                case 'i':
                {
                    argList[index] = MonoStoreAndGetPtr<unsigned int>(argsStore, args, index);
                    break;
                }
                case 'q':
                {
                    argList[index] = MonoStoreAndGetPtr<signed int>(argsStore, args, index);
                    break;
                }
                case 'l':
                {
                    argList[index] = MonoStoreAndGetPtr<unsigned long long>(argsStore, args, index);
                    break;
                }
                case 'w':
                {
                    argList[index] = MonoStoreAndGetPtr<signed long long>(argsStore, args, index);
                    break;
                }
                case 'f':
                {
                    argList[index] = MonoStoreAndGetPtr<double>(argsStore, args, index);
                    break;
                }
                case 'p':
                {
                    argList[index] = MonoStoreAndGetPtr<void *>(argsStore, args, index);
                    break;
                }
                case 's':
                {
                    if (args.at(index).type() == typeid(std::string)) // mono to mono call
                        argList[index] = mono_string_new(mono_domain_get(),
                                                         boost::any_cast<std::string>(args.at(index)).c_str());
                    else // lua to mono
                        argList[index] = mono_string_new(mono_domain_get(),
                                                         boost::any_cast<const char *>(args.at(index)));
                    break;
                }
                case 'b':
                {
                    argList[index] = MonoStoreAndGetPtr<int>(argsStore, args, index);
                    break;
                }

                default:
                    throw std::runtime_error("Call: Unknown argument identifier " + def[index]);
            }
        }

        MonoObject *monoRet = mono_runtime_delegate_invoke(fMono.delegate, argList.data(), NULL);
        if (monoRet != nullptr)
            result = LangMono::ObjectToAny(monoRet);
    }
#endif

    return result;
}
