
#include "slang-wasm-cpplib.h"
#include <slang.h>
#include <slang-com-helper.h>
#include <slang-com-ptr.h>
#include <string>
#include <cstring>
#include <cstdlib>

using namespace Slang;

namespace {
    // Internal conversion functions for opaque handles
    SlangGlobalSession* toHandle(slang::IGlobalSession* session) {
        return reinterpret_cast<SlangGlobalSession*>(session);
    }

    slang::IGlobalSession* fromHandle(SlangGlobalSession* handle) {
        return reinterpret_cast<slang::IGlobalSession*>(handle);
    }

    SlangSession* toHandle(slang::ISession* session) {
        return reinterpret_cast<SlangSession*>(session);
    }

    slang::ISession* fromHandle(SlangSession* handle) {
        return reinterpret_cast<slang::ISession*>(handle);
    }

    SlangModule* toHandle(slang::IModule* module) {
        return reinterpret_cast<SlangModule*>(module);
    }

    slang::IModule* fromHandle(SlangModule* handle) {
        return reinterpret_cast<slang::IModule*>(handle);
    }

    SlangComponentType* toHandle(slang::IComponentType* componentType) {
        return reinterpret_cast<SlangComponentType*>(componentType);
    }

    slang::IComponentType* fromHandle(SlangComponentType* handle) {
        return reinterpret_cast<slang::IComponentType*>(handle);
    }

    SlangEntryPoint* toHandle(slang::IEntryPoint* entryPoint) {
        return reinterpret_cast<SlangEntryPoint*>(entryPoint);
    }

    slang::IEntryPoint* fromHandle(SlangEntryPoint* handle) {
        return reinterpret_cast<slang::IEntryPoint*>(handle);
    }

    SlangProgramLayout* toHandle(slang::ProgramLayout* layout) {
        return reinterpret_cast<SlangProgramLayout*>(layout);
    }

    slang::ProgramLayout* fromHandle(SlangProgramLayout* handle) {
        return reinterpret_cast<slang::ProgramLayout*>(handle);
    }

    // Helper to copy strings for output
    char* copyString(const char* src) {
        if (!src) return nullptr;
        size_t len = strlen(src) + 1;
        char* result = static_cast<char*>(malloc(len));
        if (result) {
            memcpy(result, src, len);
        }
        return result;
    }

    // Helper to extract diagnostics blob as string
    char* extractDiagnostics(slang::IBlob* diagnosticsBlob) {
        if (!diagnosticsBlob) return nullptr;

        const char* text = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
        size_t size = diagnosticsBlob->getBufferSize();

        // Ensure null termination
        char* result = static_cast<char*>(malloc(size + 1));
        if (result) {
            memcpy(result, text, size);
            result[size] = '\0';
        }
        return result;
    }

    // Helper to copy blob data
    void copyBlob(slang::IBlob* sourceBlob, SlangBlob* destBlob) {
        if (!sourceBlob || !destBlob) return;

        size_t size = sourceBlob->getBufferSize();
        void* data = malloc(size);
        if (data) {
            memcpy(data, sourceBlob->getBufferPointer(), size);
            destBlob->data = data;
            destBlob->size = size;
        } else {
            destBlob->data = nullptr;
            destBlob->size = 0;
        }
    }
}

// === Global Session Management ===

SlangResult slang_wasm_createGlobalSession(
    int32_t apiVersion,
    SlangGlobalSession** outGlobalSession)
{
    if (!outGlobalSession) return SLANG_FAIL;

    slang::IGlobalSession* session = nullptr;
    SlangResult result = slang_createGlobalSession(apiVersion, &session);

    if (SLANG_SUCCEEDED(result)) {
        *outGlobalSession = toHandle(session);
    } else {
        *outGlobalSession = nullptr;
    }

    return result;
}

void slang_wasm_destroyGlobalSession(SlangGlobalSession* globalSession) {
    if (globalSession) {
        fromHandle(globalSession)->release();
    }
}

// === Session Management ===

SlangResult slang_wasm_createSession(
    SlangGlobalSession* globalSession,
    SlangCompileTarget compileTarget,
    SlangSession** outSession)
{
    if (!globalSession || !outSession) return SLANG_FAIL;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;

    slang::TargetDesc targetDesc = {};
    targetDesc.format = static_cast<SlangCompileTarget>(compileTarget);
    sessionDesc.targets = &targetDesc;

    slang::ISession* session = nullptr;
    SlangResult result = fromHandle(globalSession)->createSession(sessionDesc, &session);

    if (SLANG_SUCCEEDED(result)) {
        *outSession = toHandle(session);
    } else {
        *outSession = nullptr;
    }

    return result;
}

void slang_wasm_destroySession(SlangSession* session) {
    if (session) {
        fromHandle(session)->release();
    }
}

// === Module Loading ===

SlangResult slang_wasm_loadModuleFromSourceString(
    SlangSession* session,
    const char* moduleName,
    const char* path,
    const char* sourceString,
    SlangModule** outModule,
    char** outDiagnostics)
{
    if (!session || !moduleName || !sourceString || !outModule) return SLANG_FAIL;

    ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = fromHandle(session)->loadModuleFromSourceString(
        moduleName, path ? path : "", sourceString, diagnosticsBlob.writeRef());

    if (module) {
        *outModule = toHandle(module);
        if (outDiagnostics) {
            *outDiagnostics = extractDiagnostics(diagnosticsBlob);
        }
        return SLANG_OK;
    } else {
        *outModule = nullptr;
        if (outDiagnostics) {
            *outDiagnostics = extractDiagnostics(diagnosticsBlob);
        }
        return SLANG_FAIL;
    }
}

SlangResult slang_wasm_loadModule(
    SlangSession* session,
    const char* moduleName,
    SlangModule** outModule,
    char** outDiagnostics)
{
    if (!session || !moduleName || !outModule) return SLANG_FAIL;

    ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = fromHandle(session)->loadModule(moduleName, diagnosticsBlob.writeRef());

    if (module) {
        *outModule = toHandle(module);
        if (outDiagnostics) {
            *outDiagnostics = extractDiagnostics(diagnosticsBlob);
        }
        return SLANG_OK;
    } else {
        *outModule = nullptr;
        if (outDiagnostics) {
            *outDiagnostics = extractDiagnostics(diagnosticsBlob);
        }
        return SLANG_FAIL;
    }
}

// === Entry Point Discovery ===

SlangResult slang_wasm_findEntryPointByName(
    SlangModule* module,
    const char* name,
    SlangEntryPoint** outEntryPoint)
{
    if (!module || !name || !outEntryPoint) return SLANG_FAIL;

    SlangInt entryPointCount = fromHandle(module)->getDefinedEntryPointCount();
    for (SlangInt i = 0; i < entryPointCount; i++) {
        slang::IEntryPoint* entryPoint = fromHandle(module)->getDefinedEntryPoint(i);
        if (entryPoint) {
            const char* entryPointName = entryPoint->getFunctionReflection()->getName();
            if (entryPointName && strcmp(entryPointName, name) == 0) {
                *outEntryPoint = toHandle(entryPoint);
                return SLANG_OK;
            }
        }
    }

    *outEntryPoint = nullptr;
    return SLANG_FAIL;
}

int32_t slang_wasm_getDefinedEntryPointCount(SlangModule* module) {
    if (!module) return 0;
    return static_cast<int32_t>(fromHandle(module)->getDefinedEntryPointCount());
}

SlangResult slang_wasm_getDefinedEntryPoint(
    SlangModule* module,
    int32_t index,
    SlangEntryPoint** outEntryPoint)
{
    if (!module || !outEntryPoint) return SLANG_FAIL;

    slang::IEntryPoint* entryPoint = fromHandle(module)->getDefinedEntryPoint(index);
    if (entryPoint) {
        *outEntryPoint = toHandle(entryPoint);
        return SLANG_OK;
    } else {
        *outEntryPoint = nullptr;
        return SLANG_FAIL;
    }
}

// === Component Type Creation ===

SlangResult slang_wasm_createCompositeComponentType(
    SlangSession* session,
    SlangComponentType** componentTypes,
    int32_t componentTypeCount,
    SlangComponentType** outCompositeComponentType,
    char** outDiagnostics)
{
    if (!session || !componentTypes || componentTypeCount <= 0 || !outCompositeComponentType) {
        return SLANG_FAIL;
    }

    // Convert handles to interfaces
    slang::IComponentType** interfaces = static_cast<slang::IComponentType**>(
        alloca(componentTypeCount * sizeof(slang::IComponentType*)));

    for (int32_t i = 0; i < componentTypeCount; i++) {
        interfaces[i] = fromHandle(componentTypes[i]);
    }

    ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IComponentType* composite = nullptr;

    SlangResult result = fromHandle(session)->createCompositeComponentType(
        interfaces, componentTypeCount, &composite, diagnosticsBlob.writeRef());

    if (SLANG_SUCCEEDED(result) && composite) {
        *outCompositeComponentType = toHandle(composite);
    } else {
        *outCompositeComponentType = nullptr;
    }

    if (outDiagnostics) {
        *outDiagnostics = extractDiagnostics(diagnosticsBlob);
    }

    return result;
}

SlangResult slang_wasm_linkComponentType(
    SlangComponentType* componentType,
    SlangComponentType** outLinkedComponentType,
    char** outDiagnostics)
{
    if (!componentType || !outLinkedComponentType) return SLANG_FAIL;

    ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IComponentType* linked = nullptr;

    SlangResult result = fromHandle(componentType)->link(&linked, diagnosticsBlob.writeRef());

    if (SLANG_SUCCEEDED(result) && linked) {
        *outLinkedComponentType = toHandle(linked);
    } else {
        *outLinkedComponentType = nullptr;
    }

    if (outDiagnostics) {
        *outDiagnostics = extractDiagnostics(diagnosticsBlob);
    }

    return result;
}

// === Code Generation ===

SlangResult slang_wasm_getEntryPointCode(
    SlangComponentType* componentType,
    int32_t entryPointIndex,
    int32_t targetIndex,
    SlangBlob* outCode,
    char** outDiagnostics)
{
    if (!componentType || !outCode) return SLANG_FAIL;

    ComPtr<slang::IBlob> codeBlob;
    ComPtr<slang::IBlob> diagnosticsBlob;

    SlangResult result = fromHandle(componentType)->getEntryPointCode(
        entryPointIndex, targetIndex, codeBlob.writeRef(), diagnosticsBlob.writeRef());

    if (SLANG_SUCCEEDED(result) && codeBlob) {
        copyBlob(codeBlob, outCode);
    } else {
        outCode->data = nullptr;
        outCode->size = 0;
    }

    if (outDiagnostics) {
        *outDiagnostics = extractDiagnostics(diagnosticsBlob);
    }

    return result;
}

SlangResult slang_wasm_getTargetCode(
    SlangComponentType* componentType,
    int32_t targetIndex,
    SlangBlob* outCode,
    char** outDiagnostics)
{
    if (!componentType || !outCode) return SLANG_FAIL;

    ComPtr<slang::IBlob> codeBlob;
    ComPtr<slang::IBlob> diagnosticsBlob;

    SlangResult result = fromHandle(componentType)->getTargetCode(
        targetIndex, codeBlob.writeRef(), diagnosticsBlob.writeRef());

    if (SLANG_SUCCEEDED(result) && codeBlob) {
        copyBlob(codeBlob, outCode);
    } else {
        outCode->data = nullptr;
        outCode->size = 0;
    }

    if (outDiagnostics) {
        *outDiagnostics = extractDiagnostics(diagnosticsBlob);
    }

    return result;
}

// === Reflection ===

SlangResult slang_wasm_getLayout(
    SlangComponentType* componentType,
    int32_t targetIndex,
    SlangProgramLayout** outLayout)
{
    if (!componentType || !outLayout) return SLANG_FAIL;

    slang::ProgramLayout* layout = fromHandle(componentType)->getLayout(targetIndex);
    if (layout) {
        *outLayout = toHandle(layout);
        return SLANG_OK;
    } else {
        *outLayout = nullptr;
        return SLANG_FAIL;
    }
}

// === Utility Functions ===

SlangResult slang_wasm_compileShaderFromString(
    SlangGlobalSession* globalSession,
    const char* shaderSource,
    const char* entryPointName,
    SlangStage stage,
    SlangCompileTarget target,
    SlangBlob* outCode,
    char** outDiagnostics)
{
    if (!globalSession || !shaderSource || !entryPointName || !outCode) {
        return SLANG_FAIL;
    }

    // Create session
    SlangSession* session = nullptr;
    SlangResult result = slang_wasm_createSession(globalSession, target, &session);
    if (SLANG_FAILED(result)) {
        return result;
    }

    // Load module
    SlangModule* module = nullptr;
    char* moduleLoadDiagnostics = nullptr;
    result = slang_wasm_loadModuleFromSourceString(
        session, "shader", "", shaderSource, &module, &moduleLoadDiagnostics);

    if (SLANG_FAILED(result)) {
        if (outDiagnostics) {
            *outDiagnostics = moduleLoadDiagnostics;
        } else {
            slang_wasm_freeString(moduleLoadDiagnostics);
        }
        slang_wasm_destroySession(session);
        return result;
    }

    slang_wasm_freeString(moduleLoadDiagnostics);

    // Find entry point
    SlangEntryPoint* entryPoint = nullptr;
    result = slang_wasm_findEntryPointByName(module, entryPointName, &entryPoint);
    if (SLANG_FAILED(result)) {
        if (outDiagnostics) {
            *outDiagnostics = copyString("Entry point not found");
        }
        slang_wasm_destroySession(session);
        return result;
    }

    // Create composite component type
    SlangComponentType* moduleComponent = slang_wasm_moduleAsComponentType(module);
    SlangComponentType* entryPointComponent = slang_wasm_entryPointAsComponentType(entryPoint);
    SlangComponentType* components[] = { moduleComponent, entryPointComponent };

    SlangComponentType* composite = nullptr;
    char* composeDiagnostics = nullptr;
    result = slang_wasm_createCompositeComponentType(
        session, components, 2, &composite, &composeDiagnostics);

    if (SLANG_FAILED(result)) {
        if (outDiagnostics) {
            *outDiagnostics = composeDiagnostics;
        } else {
            slang_wasm_freeString(composeDiagnostics);
        }
        slang_wasm_destroySession(session);
        return result;
    }

    slang_wasm_freeString(composeDiagnostics);

    // Link
    SlangComponentType* linked = nullptr;
    char* linkDiagnostics = nullptr;
    result = slang_wasm_linkComponentType(composite, &linked, &linkDiagnostics);

    if (SLANG_FAILED(result)) {
        if (outDiagnostics) {
            *outDiagnostics = linkDiagnostics;
        } else {
            slang_wasm_freeString(linkDiagnostics);
        }
        slang_wasm_destroySession(session);
        return result;
    }

    slang_wasm_freeString(linkDiagnostics);

    // Get code
    char* codeDiagnostics = nullptr;
    result = slang_wasm_getEntryPointCode(linked, 0, 0, outCode, &codeDiagnostics);

    if (outDiagnostics) {
        *outDiagnostics = codeDiagnostics;
    } else {
        slang_wasm_freeString(codeDiagnostics);
    }

    slang_wasm_destroySession(session);
    return result;
}

// === Memory Management ===

void slang_wasm_freeString(char* str) {
    if (str) {
        free(str);
    }
}

void slang_wasm_freeBlob(SlangBlob* blob) {
    if (blob && blob->data) {
        free(blob->data);
        blob->data = nullptr;
        blob->size = 0;
    }
}

// === Type Conversion Helpers ===

SlangComponentType* slang_wasm_moduleAsComponentType(SlangModule* module) {
    if (!module) return nullptr;
    // Modules are also component types in Slang
    return reinterpret_cast<SlangComponentType*>(module);
}

SlangComponentType* slang_wasm_entryPointAsComponentType(SlangEntryPoint* entryPoint) {
    if (!entryPoint) return nullptr;
    // Entry points are also component types in Slang
    return reinterpret_cast<SlangComponentType*>(entryPoint);
}

// === Capability Queries ===

SlangResult slang_wasm_checkCompileTargetSupport(
    SlangGlobalSession* globalSession,
    SlangCompileTarget target)
{
    if (!globalSession) return SLANG_FAIL;

    return fromHandle(globalSession)->checkCompileTargetSupport(
        static_cast<SlangCompileTarget>(target));
}