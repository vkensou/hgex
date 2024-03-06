#include "wasm_export.h"
#include <filesystem>
#include <fstream>
#include <tuple>
#include "hge.h"
#include "hgex_wrapper.h"

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

wasm_module_t main_module = NULL;
wasm_module_inst_t main_module_inst = NULL;
wasm_exec_env_t main_exec_env = NULL;
wasm_function_inst_t func_config = NULL;
wasm_function_inst_t func_init = NULL;
wasm_function_inst_t func_frame = NULL;
wasm_function_inst_t func_render = NULL;
wasm_function_inst_t func_exit = NULL;

bool FrameFunc()
{
	wasm_val_t results[1] = { {.kind = WASM_I32, .of = {.i32 = 0}} };

	if (wasm_runtime_call_wasm_a(main_exec_env, func_frame, 1, results, 0, nullptr))
	{
		int ret_val;
		ret_val = results[0].of.i32;
		return ret_val;
	}
	else
	{
		const char* exception;
		if ((exception = wasm_runtime_get_exception(main_module_inst)))
			std::printf("%s\n", exception);
		return true;
	}
}

bool RenderFunc()
{
	return false;
}

void exec_main_module()
{
	char error_buf[128];
	size_t stack_size = 8092, heap_size = 8092;
	static char global_heap_buf[512 * 1024];

	main_module_inst = wasm_runtime_instantiate(main_module, stack_size, heap_size, error_buf, sizeof(error_buf));
	if (!main_module_inst)
	{
		std::printf(error_buf);
	}

	if (main_module_inst)
	{
		main_exec_env = wasm_runtime_create_exec_env(main_module_inst, stack_size);
		if (!main_exec_env)
		{
			printf("Create wasm execution environment failed.\n");
		}
	}

	if (main_exec_env)
	{
		auto func_config_name = "_app_config";
		auto func_init_name = "_app_init";
		auto func_frame_name = "_app_frame";
		auto func_render_name = "_app_render";
		auto func_exit_name = "_app_exit";

		func_config = wasm_runtime_lookup_function(main_module_inst, func_config_name, NULL);
		func_init = wasm_runtime_lookup_function(main_module_inst, func_init_name, NULL);
		func_frame = wasm_runtime_lookup_function(main_module_inst, func_frame_name, NULL);
		func_render = wasm_runtime_lookup_function(main_module_inst, func_render_name, NULL);
		func_exit = wasm_runtime_lookup_function(main_module_inst, func_exit_name, NULL);

		if (!func_frame)
		{
			std::printf("The %s function is not found.\n", func_frame_name);
		}
	}

	if (func_frame)
	{
		hge->System_SetState(HGE_TITLE, "HGE Wasm App");
		hge->System_SetState(HGE_WINDOWED, true);

		if (func_config && !wasm_runtime_call_wasm_a(main_exec_env, func_config, 0, nullptr, 0, nullptr))
		{
			const char* exception;
			if ((exception = wasm_runtime_get_exception(main_module_inst)))
				std::printf("%s\n", exception);
		}

		hge->System_SetState(HGE_FRAMEFUNC, FrameFunc);
		if (func_render)
			hge->System_SetState(HGE_RENDERFUNC, RenderFunc);

		if (hge->System_Initiate())
		{
			if (func_init && !wasm_runtime_call_wasm_a(main_exec_env, func_init, 0, nullptr, 0, nullptr))
			{
				const char* exception;
				if ((exception = wasm_runtime_get_exception(main_module_inst)))
					std::printf("%s\n", exception);
			}

			hge->System_Start();
		}

		if (func_exit && !wasm_runtime_call_wasm_a(main_exec_env, func_exit, 0, nullptr, 0, nullptr))
		{
			const char* exception;
			if ((exception = wasm_runtime_get_exception(main_module_inst)))
				std::printf("%s\n", exception);
		}
	}

	if (main_exec_env)
		wasm_runtime_destroy_exec_env(main_exec_env);

	if (main_module_inst)
		wasm_runtime_deinstantiate(main_module_inst);

	main_module_inst = NULL;
	main_exec_env = NULL;
	func_init = NULL;
	func_frame = NULL;
	func_render = NULL;
	func_exit = NULL;
}

int main(int argc, char *argv[])
{
	std::string path;
	if (argc > 1)
		path = argv[1];

	hge = hgeCreate(HGE_VERSION);

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

	wasm_register_hge_apis();

	auto [main_module, main_module_data] = wasm_load_module_from_file(path);
	if (main_module)
	{
		::main_module = main_module;
		exec_main_module();

		wasm_runtime_unload(main_module);
		free(main_module_data);
		::main_module = NULL;
	}

	wasm_runtime_destroy();

	hge->System_Shutdown();
	hge->Release();
	hge = NULL;

	return 0;
}