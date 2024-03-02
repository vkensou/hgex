#include "wasm_c_api.h"
#include "wasm_export.h"
#include <filesystem>
#include <fstream>

uint8_t* read_wasm_binary_to_buffer(std::filesystem::path path, size_t& size)
{
    if (!std::filesystem::exists(path))
        return nullptr;

    std::ifstream wasmFile(path, std::ios::binary);
    wasmFile.seekg(0, std::ios::end);
    size = wasmFile.tellg();
    wasmFile.seekg(0, std::ios::beg);

    uint8_t* data = (uint8_t*)malloc(size);
    wasmFile.read((char*)data, size);
    wasmFile.close();

    return data;
}

int main()
{
    std::string path = "wasm_tutorial01.wasm";

    uint8_t* buffer;
    char error_buf[128];
    wasm_module_t module;
    wasm_module_inst_t module_inst;
    wasm_function_inst_t func;
    wasm_exec_env_t exec_env;
    size_t size, stack_size = 8092, heap_size = 8092;

    /* initialize the wasm runtime by default configurations */
    wasm_runtime_init();

    /* read WASM file into a memory buffer */
    buffer = read_wasm_binary_to_buffer(path, size);
    if (!buffer)
    {
        std::printf("wasm file load failed: %s\n", path.c_str());
        wasm_runtime_destroy();
        return 1;
    }

    /* add line below if we want to export native functions to WASM app */
    //wasm_runtime_register_natives(...);

    /* parse the WASM file from buffer and create a WASM module */
    module = wasm_runtime_load(buffer, size, error_buf, sizeof(error_buf));
    free(buffer);
    if (!module)
    {
        std::printf("wasm module load failed: %s, error message: %s\n", path.c_str(), error_buf);
        wasm_runtime_destroy();
        return 2;
    }

    /* create an instance of the WASM module (WASM linear memory is ready) */
    module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
        error_buf, sizeof(error_buf));
    if (!module_inst)
    {
        std::printf("wasm module instantiate failed: %s, error message: %s\n", path.c_str(), error_buf);
    }

    if (module_inst)
        wasm_runtime_deinstantiate(module_inst);
    wasm_runtime_unload(module);
    wasm_runtime_destroy();

    return 0;
}