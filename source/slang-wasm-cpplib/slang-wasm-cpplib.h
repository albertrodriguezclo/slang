#pragma once

#ifndef SLANG_WASM_CPPLIB_H
#define SLANG_WASM_CPPLIB_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle types - no C++ dependencies
typedef struct SlangGlobalSession SlangGlobalSession;
typedef struct SlangSession SlangSession;
typedef struct SlangModule SlangModule;
typedef struct SlangComponentType SlangComponentType;
typedef struct SlangEntryPoint SlangEntryPoint;
typedef struct SlangProgramLayout SlangProgramLayout;

// Simple result and enum types
typedef int32_t SlangResult;
typedef int32_t SlangStage;
typedef int32_t SlangCompileTarget;

// Result codes (subset of what you need)
#define SLANG_OK 0
#define SLANG_FAIL -1

// Compile targets (subset)
#define SLANG_TARGET_SPIRV 0
#define SLANG_TARGET_HLSL 1
#define SLANG_TARGET_GLSL 2

// Stages (subset)
#define SLANG_STAGE_VERTEX 0
#define SLANG_STAGE_FRAGMENT 1
#define SLANG_STAGE_COMPUTE 2

// Memory management
typedef struct {
    void* data;
    size_t size;
} SlangBlob;

// === Core API ===

// Global session management
SlangResult slang_wasm_createGlobalSession(
    int32_t apiVersion,
    SlangGlobalSession** outGlobalSession);

void slang_wasm_destroyGlobalSession(SlangGlobalSession* globalSession);

// Session management
SlangResult slang_wasm_createSession(
    SlangGlobalSession* globalSession,
    SlangCompileTarget compileTarget,
    SlangSession** outSession);

void slang_wasm_destroySession(SlangSession* session);

// Module loading
SlangResult slang_wasm_loadModuleFromSourceString(
    SlangSession* session,
    const char* moduleName,
    const char* path,
    const char* sourceString,
    SlangModule** outModule,
    char** outDiagnostics);

SlangResult slang_wasm_loadModule(
    SlangSession* session,
    const char* moduleName,
    SlangModule** outModule,
    char** outDiagnostics);

// Entry point discovery
SlangResult slang_wasm_findEntryPointByName(
    SlangModule* module,
    const char* name,
    SlangEntryPoint** outEntryPoint);

int32_t slang_wasm_getDefinedEntryPointCount(SlangModule* module);

SlangResult slang_wasm_getDefinedEntryPoint(
    SlangModule* module,
    int32_t index,
    SlangEntryPoint** outEntryPoint);

// Component type creation
SlangResult slang_wasm_createCompositeComponentType(
    SlangSession* session,
    SlangComponentType** componentTypes,
    int32_t componentTypeCount,
    SlangComponentType** outCompositeComponentType,
    char** outDiagnostics);

SlangResult slang_wasm_linkComponentType(
    SlangComponentType* componentType,
    SlangComponentType** outLinkedComponentType,
    char** outDiagnostics);

// Code generation
SlangResult slang_wasm_getEntryPointCode(
    SlangComponentType* componentType,
    int32_t entryPointIndex,
    int32_t targetIndex,
    SlangBlob* outCode,
    char** outDiagnostics);

SlangResult slang_wasm_getTargetCode(
    SlangComponentType* componentType,
    int32_t targetIndex,
    SlangBlob* outCode,
    char** outDiagnostics);

// Reflection
SlangResult slang_wasm_getLayout(
    SlangComponentType* componentType,
    int32_t targetIndex,
    SlangProgramLayout** outLayout);

// Utility functions
SlangResult slang_wasm_compileShaderFromString(
    SlangGlobalSession* globalSession,
    const char* shaderSource,
    const char* entryPointName,
    SlangStage stage,
    SlangCompileTarget target,
    SlangBlob* outCode,
    char** outDiagnostics);

// Memory management helpers
void slang_wasm_freeString(char* str);
void slang_wasm_freeBlob(SlangBlob* blob);

// Type conversion helpers (module -> component type, etc.)
SlangComponentType* slang_wasm_moduleAsComponentType(SlangModule* module);
SlangComponentType* slang_wasm_entryPointAsComponentType(SlangEntryPoint* entryPoint);

// Capability queries
SlangResult slang_wasm_checkCompileTargetSupport(
    SlangGlobalSession* globalSession,
    SlangCompileTarget target);

#ifdef __cplusplus
}
#endif

#endif // SLANG_WASM_CPPLIB_H