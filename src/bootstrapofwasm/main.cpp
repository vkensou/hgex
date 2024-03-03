#include "wasm_export.h"
#include <filesystem>
#include <fstream>
#include <tuple>

std::tuple<wasm_module_t, uint8_t*> wasm_load_module_from_file(std::filesystem::path path)
{
	if (!std::filesystem::exists(path))
		return std::make_tuple(nullptr, nullptr);

	std::ifstream wasmFile(path, std::ios::binary);
	wasmFile.seekg(0, std::ios::end);
	auto size = wasmFile.tellg();
	wasmFile.seekg(0, std::ios::beg);

	uint8_t* data = (uint8_t*)malloc(size);
	wasmFile.read((char*)data, size);
	wasmFile.close();

	char error_buf[128];
	auto module = wasm_runtime_load(data, size, error_buf, sizeof(error_buf));
	if (!module)
	{
		std::printf(error_buf);
		return std::make_tuple(nullptr, nullptr);
	}

	return std::make_tuple(module, data);
}

void exec_main_module(wasm_module_t main_module)
{
	char error_buf[128];
	size_t stack_size = 8092, heap_size = 8092;
	static char global_heap_buf[512 * 1024];

	auto main_module_inst = wasm_runtime_instantiate(main_module, stack_size, heap_size, error_buf, sizeof(error_buf));
	if (!main_module_inst)
	{
		std::printf(error_buf);
	}

	wasm_exec_env_t exec_env = NULL;
	if (main_module_inst)
	{
		exec_env = wasm_runtime_create_exec_env(main_module_inst, stack_size);
		if (!exec_env)
		{
			printf("Create wasm execution environment failed.\n");
		}
	}

	wasm_function_inst_t func = NULL;
	if (exec_env)
	{
		auto func_name = "add";
		func = wasm_runtime_lookup_function(main_module_inst, func_name, NULL);
		if (!func)
		{
			std::printf("The %s wasm function is not found.\n", func_name);
		}
	}

	if (func)
	{
		wasm_val_t results[1] = { {.kind = WASM_I32, .of = {.i32 = 0}} };
		wasm_val_t arguments[2] =
		{
			{.kind = WASM_I32, .of = {.i32 = 10} },
			{.kind = WASM_I32, .of = {.i32 = 114}},
		};

		if (wasm_runtime_call_wasm_a(exec_env, func, 1, results, 2, arguments))
		{
			int ret_val;
			ret_val = results[0].of.i32;
			printf("Native finished calling wasm function add(), returned a "
				"float value: %d\n",
				ret_val);
		}
		else
		{
			const char* exception;
			if ((exception = wasm_runtime_get_exception(main_module_inst)))
				std::printf("%s\n", exception);
		}
	}

	if (exec_env)
		wasm_runtime_destroy_exec_env(exec_env);

	if (main_module_inst)
		wasm_runtime_deinstantiate(main_module_inst);
}

int main()
{
	std::string path = "wasm_tutorial01.wasm";

	RuntimeInitArgs init_args;
	memset(&init_args, 0, sizeof(RuntimeInitArgs));
	init_args.running_mode = Mode_Interp;
	init_args.mem_alloc_type = Alloc_With_Allocator;
	init_args.mem_alloc_option.allocator.malloc_func = malloc;
	init_args.mem_alloc_option.allocator.realloc_func = realloc;
	init_args.mem_alloc_option.allocator.free_func = free;

	if (!wasm_runtime_full_init(&init_args))
	{
		return 1;
	}

	auto [main_module, main_module_data] = wasm_load_module_from_file(path);
	if (main_module)
	{
		exec_main_module(main_module);

		wasm_runtime_unload(main_module);
		free(main_module_data);
	}

	wasm_runtime_destroy();

	return 0;
}