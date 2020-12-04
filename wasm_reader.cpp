//
// Created by me on 27.11.20.
//

//#include <wasm3.h>
//#include <m3_env.h>
#include <wast-lexer.h>
#include <error.h>
#include <shared-validator.h>
#include <vector>
#include "wasm_reader.h"
#include "wasm_emitter.h"

#include "binary-writer.h"
#include "common.h"
#include "error-formatter.h"
#include "feature.h"
#include "filenames.h"
#include "ir.h"
#include "result.h"
#include "option-parser.h"
#include "resolve-names.h"
#include "stream.h"
#include "validator.h"
#include "wast-parser.h"

static wabt::Features wabt_features;
static bool validate_wasm = true;
static wabt::WriteBinaryOptions write_binary_options;
static bool dump_module= false;//debug


#define consume(len,bytes) if(!consume_x(code,&pos,len,bytes)){printf("\nNOT consuming %s:%d\n",__FILE__,__LINE__);exit(0);}

#define check(test) if(test){log("OK check passes: ");log(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

#define pointer std::unique_ptr

bool consume_x(byte *code, int *pos, int len, byte *bytes) {
//	if(bytes)
	int i = 0;
	while (i < len) {
		if (bytes and code[*pos] != bytes[i])
			return false;
		*pos = *pos + 1;
		i++;
	}
	return true;
}

int pos = 0;
byte *code;

byte typ(){
	return code[pos++];
}

int unsignedLEB128() {
	int n=0;
	do {
		byte b = code[pos++];
		n = n << 7;
		n = n ^ (b & 0x7f);
		if(b & 0x80 == 0)break;
	} while (n != 0);
	return n;
}
int unsignedLEB128(Code code) {
	int n=0;
	do {
		byte b = code[code.pos++];
		n = n << 7;
		n = n ^ (b & 0x7f);
		if((b & 0x80) == 0)break;
	} while (n != 0);
	return n;
}
int siz() {
	return unsignedLEB128();
}
Code vec(){
	int from=pos;
	int len=siz();
	consume(len, 0);
	return Code(code, from, pos);
}
Code consumeTypeSection(){
	int from=pos;
	byte type=typ();
	Code type_vector=vec();
	int typeCount = unsignedLEB128(type_vector);
	printf("typeCount %d\n", typeCount);
	Code type_data = type_vector.rest();
	return Code(code, from, pos);
//	return Code(type, encodeVector(Code(typeCount) + type_data));
}

Code read(byte *code0, int length) {
	pos = 0;
	int from=pos;
	code = code0;
	consume(4, reinterpret_cast<byte *>(magicModuleHeader));
	consume(4, reinterpret_cast<byte *>(moduleVersion));
	consumeTypeSection();
//	consumeImportSection();
	return Code(code, from, pos);
}

int fileSize(char const *file) {
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)error("File not found "s + file);
	fseek(ptr, 0L, SEEK_END);
	int sz = ftell(ptr);
	return sz;
}


Code mergeModules(Code api, Code code){

}

using namespace wabt;


static pointer<FileStream> s_log_stream;// = FileStream::CreateStdout();


static void DebugBuffer(const OutputBuffer& buffer) {
	pointer<FileStream> stream = FileStream::CreateStdout();
		if (!buffer.data.empty()) {
			stream->WriteMemoryDump(buffer.data.data(), buffer.data.size());
		}
}



static void ParseOptions(int argc, char** argv); // wasm-link.cc
int ProgramMain(const char* infile) {
	string_view s_infile = "t.wat";
	std::vector<uint8_t> file_data;
	Result result = ReadFile(s_infile, &file_data);
	pointer<WastLexer> lexer = WastLexer::CreateBufferLexer(s_infile, file_data.data(), file_data.size());
	if (Failed(result)) {
		WABT_FATAL("unable to read file: %s\n", s_infile);
	}

	Errors errors;
	pointer<Module> module;

	WastParseOptions parse_wast_options(wabt_features);
	result = ParseWatModule(lexer.get(), &module, &errors, &parse_wast_options);
	bool found=false;
	for(Func* f : module->funcs){
		if(f->name=="abc")found = true;
		if(f->name=="$abc")found = true;// todo rename ON CONSTRUCTION
	}
	check(found)
//	Func* abc=module->funcs.front();
//	check(abc->name=="abc");

	if (Succeeded(result) && validate_wasm) {
		ValidateOptions options(wabt_features);
		result = ValidateModule(module.get(), &errors, options);
	}

	if (Succeeded(result)) {
		MemoryStream stream;
		write_binary_options.features = wabt_features;
		result = WriteBinaryModule(&stream, module.get(), write_binary_options);
		if (Succeeded(result)) {
			OutputBuffer &buffer = stream.output_buffer();
			buffer.WriteToFile(s_infile.substr(0,s_infile.find(".wat")));
			if(dump_module)DebugBuffer(buffer);
		}
	}

	auto line_finder = lexer->MakeLineFinder();
	FormatErrorsToFile(errors, Location::Type::Text, line_finder.get());

	return result != Result::Ok;
}

Code readWasm(char const *file) {
	ProgramMain(file);
	printf("parsing: %s\n", file);
	int sz = fileSize(file);
	FILE *ptr;
	ptr = fopen(file, "rb");  // r for read, b for binary
	if (!ptr)error("File not found "s + file);
	unsigned char buffer[sz];
	fread(buffer, sizeof(buffer), sz, ptr);
	Code c(buffer, 0, sz);
	return c;
//	c.run();
//	c.debug();
}

#ifdef WASM3
Code readWasmW3(char const *file) {
	result = ParseWatModule(lexer.get(), &module, &errors, &parse_wast_options);

	if (Succeeded(result) && validate_wasm) {
		ValidateOptions options(wabt_features);
		result = ValidateModule(module.get(), &errors, options);
	}

	if (Succeeded(result)) {
		MemoryStream stream(s_log_stream.get());
		write_binary_options.features = wabt_features;
		result = WriteBinaryModule(&stream, module.get(), write_binary_options);

	IM3Environment environment=m3_NewEnvironment();
	IM3Module module;
	M3Result result = m3_ParseModule(environment, &module, buffer, sz);
	printf("parsed: %s\n", result);
	printf("Module: %s\n", module->name);
//	M3Result  Module_AddFunction  (IM3Module io_module, u32 i_typeIndex, IM3ImportInfo i_importInfo)


//	read(buffer, sz);
}
#endif