#include "wasm_export.h"
#include <filesystem>
#include <fstream>
#include <tuple>
#include "hge.h"
#include "hgex_wrapper.h"

#include <chrono>

std::vector<wasm_exec_env_t> thread_exec_envs;

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

	wasm_runtime_set_wasi_args_ex(module, NULL, 0, NULL, 0, NULL, 0, NULL, 0, -1, -1, -1);

	return std::make_tuple(module, data);
}

struct HgeWasmFuncEnv
{
	wasm_module_inst_t module_inst = NULL;
	wasm_exec_env_t exec_env = NULL;
	wasm_function_inst_t func = NULL;
};

bool exec_func_return_bool(wasm_module_inst_t module_inst, wasm_exec_env_t exec_env, wasm_function_inst_t func)
{
	wasm_val_t results[1] = { {.kind = WASM_I32, .of = {.i32 = 0}} };

	if (wasm_runtime_call_wasm_a(exec_env, func, 1, results, 0, nullptr))
	{
		int ret_val;
		ret_val = results[0].of.i32;
		return ret_val;
	}
	else
	{
		const char* exception;
		if ((exception = wasm_runtime_get_exception(module_inst)))
			std::printf("%s\n", exception);
		return true;
	}
}

void exec_func(wasm_module_inst_t module_inst, wasm_exec_env_t exec_env, wasm_function_inst_t func)
{
	if (!wasm_runtime_call_wasm_a(exec_env, func, 0, nullptr, 0, nullptr))
	{
		const char* exception;
		if ((exception = wasm_runtime_get_exception(module_inst)))
			std::printf("%s\n", exception);
	}
}

bool try_exec_func_return_bool(wasm_module_inst_t module_inst, wasm_exec_env_t exec_env, wasm_function_inst_t func)
{
	if (func)
		return exec_func_return_bool(module_inst, exec_env, func);
	else
		return false;
}

void try_exec_func(wasm_module_inst_t module_inst, wasm_exec_env_t exec_env, wasm_function_inst_t func)
{
	if (func)
		return exec_func(module_inst, exec_env, func);
}

bool FrameFunc(HGE* hge, void* userdata)
{
	auto env = (HgeWasmFuncEnv*)userdata;
	return exec_func_return_bool(env->module_inst, env->exec_env, env->func);
}

bool RenderFunc(HGE* hge, void* userdata)
{
	auto env = (HgeWasmFuncEnv*)userdata;
	return exec_func_return_bool(env->module_inst, env->exec_env, env->func);
}

void Thread_Start(uint32_t id)
{
	printf("thread %d start\n", id);
	if (!wasm_runtime_init_thread_env()) {
		printf("failed to initialize thread environment: %d\n", id);
	}
}

void Thread_End(uint32_t id)
{
	wasm_runtime_destroy_thread_env();
	printf("thread %d end\n", id);
}

class Timer {
public:
	Timer() : start_(std::chrono::high_resolution_clock::now()) {}

	void reset() {
		start_ = std::chrono::high_resolution_clock::now();
	}

	double elapsed() const {
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = now - start_;
		return diff.count();
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

void exec_main_module(wasm_module_t main_module, HGE* hge)
{
	hge->JS_Start(Thread_Start, Thread_End);

	char error_buf[128];
	size_t stack_size = 16 * 1024, heap_size = 1024 * 1024;

	auto main_module_inst = wasm_runtime_instantiate(main_module, stack_size, heap_size, error_buf, sizeof(error_buf));
	if (!main_module_inst)
	{
		std::printf(error_buf);
	}

	wasm_exec_env_t main_exec_env = NULL;
	if (main_module_inst)
	{
		main_exec_env = wasm_runtime_create_exec_env(main_module_inst, stack_size);
		if (!main_exec_env)
		{
			printf("Create wasm execution environment failed.\n");
		}
	}

	wasm_function_inst_t func_config = NULL;
	wasm_function_inst_t func_init = NULL;
	wasm_function_inst_t func_frame = NULL;
	wasm_function_inst_t func_render = NULL;
	wasm_function_inst_t func_exit = NULL;

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

		for (size_t i = 0; i < hge->JS_GetWorkerThreadCount(); ++i)
		{
			auto newenv = wasm_runtime_spawn_exec_env(main_exec_env);
			thread_exec_envs.push_back(newenv);
		}
		//最后一个是主线程
		thread_exec_envs.push_back(main_exec_env);

		Sleep(100);

		{
			Timer timer;
			try_exec_func(main_module_inst, main_exec_env, func_config);
			double elapsedTime = timer.elapsed();
			printf("config 函数耗时：%f\n", elapsedTime);
		}

		{
			Timer timer;

			uint64_t sum = 0;
			for (size_t i = 0; i < 10000000; ++i)
			{
				sum += i;
			}

			double elapsedTime = timer.elapsed();
			printf("native 普通循环耗时：%f  %lld\n", elapsedTime, sum);
		}


		HgeWasmFuncEnv frameFuncEnv = { .module_inst = main_module_inst, .exec_env = main_exec_env, .func = func_frame };
		HgeWasmFuncEnv renderFuncEnv = { .module_inst = main_module_inst, .exec_env = main_exec_env, .func = func_render };

		hge->System_SetState(HGE_FRAMEFUNC, hgeCallback(FrameFunc, &frameFuncEnv));
		if (func_render)
			hge->System_SetState(HGE_RENDERFUNC, hgeCallback(RenderFunc, &renderFuncEnv));

		if (hge->System_Initiate())
		{
			if (!try_exec_func_return_bool(main_module_inst, main_exec_env, func_init))
				hge->System_Start();
		}

		try_exec_func(main_module_inst, main_exec_env, func_exit);
	}

	hge->JS_Shutdown();

	thread_exec_envs.pop_back();
	for (size_t i = 0; i < thread_exec_envs.size(); ++i)
	{
		wasm_runtime_destroy_spawned_exec_env(thread_exec_envs[i]);
	}
	thread_exec_envs.clear();

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

	auto hge = hgeCreate(HGE_VERSION);

	RuntimeInitArgs init_args;
	memset(&init_args, 0, sizeof(RuntimeInitArgs));
	init_args.running_mode = Mode_Interp;
	init_args.mem_alloc_type = Alloc_With_Allocator;
	init_args.mem_alloc_option.allocator.malloc_func = malloc;
	init_args.mem_alloc_option.allocator.realloc_func = realloc;
	init_args.mem_alloc_option.allocator.free_func = free;
	init_args.max_thread_num = 12;

	if (!wasm_runtime_full_init(&init_args))
	{
		return 1;
	}

	wasm_register_hge_apis(hge, &thread_exec_envs);

	auto [main_module, main_module_data] = wasm_load_module_from_file(path);
	if (main_module)
	{
		exec_main_module(main_module, hge);

		wasm_runtime_unload(main_module);
		free(main_module_data);
	}

	wasm_runtime_destroy();

	hge->System_Shutdown();
	hge->Release();
	hge = NULL;

	return 0;
}