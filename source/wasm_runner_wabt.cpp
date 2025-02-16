#include "String.h"
#include "src/binary-reader.h"
#include <wast-lexer.h>
#include <error.h>
#include "stream.h" // module->data
#include <shared-validator.h>
#include <binary-writer.h>
#include "src/interp/binary-reader-interp.h"
#include "src/interp/interp.h"

typedef unsigned char *bytes;
#define pointer std::unique_ptr

using namespace wabt::interp;

//using RefVec = std::vector<Ref>;

double root(double n) {
	double lo = 0, hi = n, mid;
	for (int i = 0; i < 100; i++) {
		mid = (lo + hi) / 2;
		if (mid * mid == n) return mid;
		if (mid * mid > n) hi = mid;
		else lo = mid;
	}
	return mid;
}


wabt::Result do_square(Thread &thread, const Values &params, Values &results, Trap::Ptr *trap) {
	int x = params.front().Get<int>();
	results.front() = wabt::interp::Value::Make(x * x);
	return wabt::Result::Ok;
};

wabt::Result do_puti(Thread &thread, const Values &params, Values &results, Trap::Ptr *trap) {
	printf("%d\n", params.front().Get<int>());
	return wabt::Result::Ok;
};

wabt::Result do_putf(Thread &thread, const Values &params, Values &results, Trap::Ptr *trap) {
	printf("%f\n", params.front().Get<float>());
	return wabt::Result::Ok;
};

wabt::Result do_exit(Thread &thread, const Values &params, Values &results, Trap::Ptr *trap) {
	exit(0);
}


wabt::Result do_puts(Thread &thread, const Values &params, Values &results, Trap::Ptr *trap) {
	Ref ref = params.front().Get<Ref>();
	char *x = (char *) (void *) params.data();
	printf("%s\n", x);
	return wabt::Result::Ok;
};


wabt::Result do_raise(Thread &thread, const Values &params, Values &results, Trap::Ptr *trap) {
	do_puts(thread, params, results, trap);
	exit(0);
}

wabt::Result do_sqrt(Thread &thread, const Values &params, Values &results, Trap::Ptr *trap) {
	int x = params.front().Get<int>();
	results.front() = wabt::interp::Value::Make((int) root(x));
	return wabt::Result::Ok;
};


void BindImports(Module *module, std::vector<Ref> &imports, Store &store) {
//	auto* stream = s_stdout_stream.get();
	bool hostPrint = true;//false;
	// convoluted shit, I don't like it
	for (auto &&import : module->desc().imports) {
		auto func_type = *wabt::cast<FuncType>(import.type.type.get());
		if (import.type.name == "square")imports.push_back(HostFunc::New(store, func_type, do_square).ref());
		else if (import.type.name == "puti")imports.push_back(HostFunc::New(store, func_type, do_puti).ref());
		else if (import.type.name == "putf")imports.push_back(HostFunc::New(store, func_type, do_putf).ref());
		else if (import.type.name == "puts")imports.push_back(HostFunc::New(store, func_type, do_puts).ref());
		else if (import.type.name == "print")imports.push_back(HostFunc::New(store, func_type, do_puts).ref());
		else if (import.type.name == "proc_exit")imports.push_back(HostFunc::New(store, func_type, do_exit).ref());
		else if (import.type.name == "panic")imports.push_back(HostFunc::New(store, func_type, do_exit).ref());
		else if (import.type.name == "raise")imports.push_back(HostFunc::New(store, func_type, do_raise).ref());
		else if (import.type.name == "√")imports.push_back(HostFunc::New(store, func_type, do_sqrt).ref());
		else imports.push_back(Ref::Null);
		// By default, just push an null reference. This won't resolve, and instantiation will fail.
	}
}

// wabt has HORRIBLE api, but ok
int run_wasm(bytes buffer, int buf_size) {
	Store store;
	ModuleDesc module_desc;
	bool kReadDebugNames = true;
	bool kStopOnFirstError = true;
	bool kFailOnCustomSectionError = true;
	bool validate_wasm = true;
	wabt::Features wabt_features;
	wabt::ReadBinaryOptions options(wabt_features, 0, kReadDebugNames, kStopOnFirstError, kFailOnCustomSectionError);
	wabt::Errors errors;
	const wabt::Result &result1 = ReadBinaryInterp(buffer, buf_size, options, &errors, &module_desc);
	if (Failed(result1)) {
		printf("FAILED ReadBinaryInterp\n");
		for (auto e : errors)
			printf("%s", e.message.data());
//		printf("This HANGS the IDE. Todo: why?\n");
		return -1;
	}
	auto module = wabt::interp::Module::New(store, module_desc);
	RefVec imports;
#if WASI
	uvwasi_t uvwasi;
	std::vector<const char*> argv; // ...
#endif
	BindImports(module.get(), imports, store);

	RefPtr<Trap> trap;
	Instance::Ptr instance = Instance::Instantiate(store, module.ref(), imports, &trap);
	if (trap) {
		printf("\nERROR in module\n");
		printf("%s\n\n", trap.get()->message().data());
		return -1;
	}

	for (wabt::interp::ExportDesc export_ : module_desc.exports) {
		if (export_.type.type->kind != wabt::ExternalKind::Func) continue;
		if (export_.type.name != "main" and export_.type.name != "maine") continue;
		auto *func_type = wabt::cast<wabt::interp::FuncType>(export_.type.type.get());
		if (func_type->params.empty()) {
			RefVec funcs = instance->funcs();
			auto func = store.UnsafeGet<wabt::interp::Func>(funcs[export_.index]);
			Values params;
			Values results;
			Trap::Ptr trap;
			wabt::Result ok = func->Call(store, params, results, &trap, 0);
			if (results.size() == 0)return 0;// no result
			int result0 = results.front().Get<int>();
			return result0;
		}
	}
	return -1;
}

int run_wasm(wabt::Module *module) {
	// really such bahuuba necessary?
	wabt::MemoryStream stream;
	wabt::WriteBinaryOptions write_binary_options;
//	write_binary_options.features = wabt_features;
	WriteBinaryModule(&stream, module, write_binary_options);
	wabt::OutputBuffer &outputBuffer = stream.output_buffer();
	bytes data = outputBuffer.data.data();
	return run_wasm(data, outputBuffer.size());
}


int fileSize1(char const *file) {
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)error("File not found: "s + file);
	fseek(ptr, 0L, SEEK_END);
	int sz = ftell(ptr);
	return sz;
}


int run_wasm(char *file) {
	int size = fileSize1(file);
	if (size <= 0)error("File not found: "s + file);
	unsigned char buffer[size];
	fread(buffer, sizeof(buffer), size, fopen(file, "rb"));
	return run_wasm(buffer, size);
}

