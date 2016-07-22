//
// Created by koncord on 23.01.16.
//

#ifndef SOURCEPAWN_SCRIPTFUNCTION_HPP
#define SOURCEPAWN_SCRIPTFUNCTION_HPP

#include <boost/any.hpp>
#include <string>
#include <vector>
#if defined (ENABLE_LUA)
#include <terra/terra.h>
#endif

#if defined (ENABLE_PAWN)
#include <amx.h>
#endif

typedef unsigned long long(*ScriptFunc)();
#if defined (ENABLE_PAWN)
typedef std::string ScriptFuncPAWN;
#endif
#if defined (ENABLE_LUA)
typedef std::string ScriptFuncLua;
#endif

class ScriptFunction
{
protected:
    union
    {
        ScriptFunc fCpp;
#if defined (ENABLE_PAWN)
        struct
        {
            AMX *amx;
            ScriptFuncPAWN name;
        } fPawn;
#endif
#if defined (ENABLE_LUA)
        struct
        {
            lua_State *lua;
            ScriptFuncLua name;
        } fLua;
#endif
    };

protected:
    std::string def;
    char ret_type;
    int script_type;
    enum
    {
        SCRIPT_CPP,
        SCRIPT_PAWN,
        SCRIPT_LUA
    };

    ScriptFunction(ScriptFunc fCpp, char ret_type, const std::string &def);
#if defined (ENABLE_LUA)
    ScriptFunction(const ScriptFuncLua &fPawn, lua_State *lua, char ret_type, const std::string &def);
#endif
#if defined (ENABLE_PAWN)
    ScriptFunction(const ScriptFuncPAWN &fPawn, AMX *amx, char ret_type, const std::string &def);
#endif
    virtual ~ScriptFunction();

    boost::any Call(const std::vector<boost::any> &args);
};

#endif //SOURCEPAWN_SCRIPTFUNCTION_HPP