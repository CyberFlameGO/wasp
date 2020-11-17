//https://blog.sentry.io/2020/08/11/the-pain-of-debugging-webassembly
//https://github.com/mdn/webassembly-examples/tree/master/understanding-text-format
// BASED ON https://github.com/ColinEberhardt/chasm/blob/master/src/emitter.ts
// https://github.com/ColinEberhardt/chasm/blob/master/src/encoding.ts
// https://pengowray.github.io/wasm-ops/
#include "Wasp.h"
#include "String.h"
#include "Map.h"
#include "wasm-emitter.h"
#include "WasmHelpers.h"
#include "wasm_runner.h"

#define check(test) if(test){log("OK check passes: ");log(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

#ifdef WASM
#import  "WasmHelpers.cpp"
#endif
//#include "string.h" // memcpy

//typedef char byter[];

enum nameSubSectionTypes {
	module_name = 0,
	function_names = 1,
	local_names = 2,
};

Code Call(char *symbol);//Node* args
Code &unsignedLEB128(long n);

Code &signedLEB128(long value);

//Code& unsignedLEB128(int);
//Code& flatten(byter);
//Code& flatten (Code& data);
void todo(char *message = "") {
	printf("TODO %s\n", message);
}

//typedef int number;
//typedef char byte;
//typedef char* Code;
//typedef char id;




void memcpy(bytes c, bytes a, int i);

bytes concat(bytes a, bytes b, int len_a, int len_b) {
	bytes c = new char[len_a + len_b + 4];
	memcpy(c, a, len_a);
	memcpy(c + len_a, b, len_b);
	c[len_a + len_b + 1] = 0;
	return c;
}

void memcpy(bytes dest, bytes source, int i) {
	while (i--)dest[i] = source[i];
}

bytes concat(bytes a, char b, int len) {
	bytes c = new char[len + 1];
	memcpy(c, a, len);
	c[len] = b;
	return c;
}

bytes concat(char a, bytes b, int len) {
	bytes c = new char[len + 1];
	c[0] = a;
	memcpy(c + 1, b, len);
	return c;
}
//
//class Nod{
//public:
//	Nod(Block &alternate, ExpressionNod &args, Block &consequent, ExpressionNod &expression,
//	    ExpressionNod &initializer, String &name, Block &nodes, String &typeName, String &value)
//			: alternate(alternate), args(args), consequent(consequent), expression(expression),
//			  initializer(initializer), name(name), nodes(nodes), type_name(typeName), value(value) {
//
//	}
//
//	Nod(Block *alternate, ExpressionNod *args, Block *consequent, ExpressionNod *expression,
//	    ExpressionNod *initializer, String *name, Block *nodes, String *typeName, String *value)
//	: alternate(*alternate), args(*args), consequent(*consequent), expression(*expression),
//	initializer(*initializer), name(*name), nodes(*nodes), type_name(*typeName), value(*value) {
//
//	}
//	Nod():Nod(0,0,0,0,0,0,0,0,0){}
//
//	String type_name;
//	int type;
//
//	String& value;// variable name or operator "+" …
//	ExpressionNod& expression;
//	ExpressionNod& initializer;
//	String& name;
//	ExpressionNod& args;
//	Block& nodes;
//	Block& consequent;
//	Block& alternate;
//	int index;// for args
//	Block *begin() const{
//		todo();
//		return &nodes;
//	}
//	Nod *end() const{
//		todo();
//		return 0;
//	}
//
//	Nod& operator[](int i){
//		todo();
//		return *this;
//	}
//};
//class ExpressionNod : public Nod{
//public:
//	ExpressionNod() {}
//
//	ExpressionNod& operator[](int i){
//		todo();
//		return *this;
//	}
//};
//
//class Block : public Nod{
//public:
//	Block() {}
//	Block& operator[](int i){
//		todo();
//		return *this;
//	}
//};
//
//class ProcStatementNod : public Nod{
//public:
//	ProcStatementNod() {}
//	ProcStatementNod& operator[](int i){
//		todo();
//		return *this;
//	}
//};

//#include <iostream>

class Bytes {
public:
	int length;
//	int length(){
//		return 1;
//	}
};

// nonsense, we already have bytes!?
//uint8_t uint8Array( std::initializer_list<Code> list ){
//	int total = 0;
//	for(Code more:list){total+=more.length;}
//	uint8_t all[total];
//int current = 0;
//for(Code more:list){
//	for (int i=0;i<more.length;i++){
//all[current++]=more.data[i];
//	}
//	}
//}



//bytes
Code signedLEB128(int i);

Code encodeString(char *String);

//typedef Code any;
//typedef Bytes any;

// class Code;

// Code flatten (any arr[]) {
// [].concat.apply([], arr);


bool eq(const char *op, const char *string);

// https://pengowray.github.io/wasm-ops/
byte opcodes(const char *s, byte kind = 0) {
//	if(eq(s,"$1="))return set_local;
//	if (eq(s, "=$1"))return get_local;
//	if (eq(s, "=$1"))return tee_local;

	if (kind == 0) { // INT32
		if (eq(s, "+"))return i32_add;
		if (eq(s, "-"))return i32_sub;
		if (eq(s, "*"))return i32_mul;
		if (eq(s, "/"))return i32_div;
		if (eq(s, "%"))return i32_rem;
		if (eq(s, "=="))return i32_eq;
		if (eq(s, "!="))return i32_ne;
		if (eq(s, ">"))return i32_gt;
		if (eq(s, "<"))return i32_lt;
		if (eq(s, ">="))return i32_ge;
		if (eq(s, "<="))return i32_le;
		if (eq(s, "≥"))return i32_ge;
		if (eq(s, "≤"))return i32_le;

		if (eq(s, "&"))return i32_and;
		if (eq(s, "&&"))return i32_and;
		if (eq(s, "and"))return i32_and;
		if (eq(s, "or"))return i32_or;
		if (eq(s, "xor"))return i32_xor;
		if (eq(s, "not"))return i32_eqz;
		if (eq(s, "||"))return i32_or;
		if (eq(s, "|"))return i32_or;
	} else {

		if (eq(s, "not"))return f32_eqz; // HACK: no such thing!
		if (eq(s, "+"))return f32_add;
		if (eq(s, "-"))return f32_sub;
		if (eq(s, "*"))return f32_mul;
		if (eq(s, "/"))return f32_div;
		if (eq(s, "=="))return f32_eq;
		if (eq(s, ">"))return f32_gt;
		if (eq(s, "<"))return f32_lt;
	}
	printf("unknown operator %s", s);
//		error("invalid operator");
	return 0;
}

// http://webassembly.github.io/spec/core/binary/modules.html#export-section
enum ExportType {
	func_export = (char) 0x00,
	table_export = 0x01,
	mem_export = 0x02,
	global_export = 0x03
};

// http://webassembly.github.io/spec/core/binary/types.html#function-types
char functionType = 0x60;

char emptyArray = 0x0;

// https://webassembly.github.io/spec/core/binary/modules.html#binary-module
bytes magicModuleHeader = new char[]{0x00, 0x61, 0x73, 0x6d};
bytes moduleVersion = new char[]{0x01, 0x00, 0x00, 0x00};


bytes flatten(bytes data) {
	todo();
	return data;
}

Code &flatten(Code &data) {
	todo();
	return data;
}

#ifdef WASM
typedef char uint8_t;
#endif

typedef uint8_t byt;

//https://en.wikipedia.org/wiki/LEB128
// little endian 257 = 0x81 (001) + 0x02 (256)
Code &unsignedLEB128(long n) {
	Code buffer;
//	Code* buffer=new Code(); // IF RETURNING Code&
	do {
		byt byte = n & 0x7f;
		n = n >> 7;
		if (n != 0) {
			byte |= 0x80;// continuation bit
		}
		buffer.add(byte);
	} while (n != 0);
	return buffer;
}

Code &signedLEB128(long value) {
	Code buffer;
	int more = 1;
	bool negative = (value < 0);
	long val = value;
/* the size in bits of the variable value, e.g., 64 if value's type is int64_t */
//	size = no. of bits in signed integer;
//	int size = 64;
	while (more) {
		byt byte = val & 0x7f;
		val >>= 7;
		/* the following is only necessary if the implementation of >>= uses a
		   logical shift rather than an arithmetic shift for a signed left operand */
//		if (negative)
//			val |= (~0 << (size - 7)); /* sign extend */

		/* sign bit of byte is second high order bit (0x40) */
		bool clear = (byte & 0x40) == 0;  /*sign bit of byte is clear*/
		bool set = byte & 0x40; /*sign bit of byte is set*/
		if ((val == 0 && clear) || (val == -1 && set))
			more = 0;
		else {
			byte |= 0x80;// continuation bit:  set high order bit of byte;
		}
		buffer.add(byte); //		emit byte;
	}
	return buffer;
}
// https://webassembly.github.io/spec/core/binary/conventions.html#binary-vec
// Vectors are encoded with their length followed by their element sequence
//Code encodeVector (char data[]) {
//	return Code(unsignedLEB128(sizeof(data)), flatten(data));
//}

//Code& encodeVector (Code& data) {
//	if(data.encoded)return data;
////	return Code(unsignedLEB128(data.length), flat(data),data.length);
//	Code code = unsignedLEB128(data.length) + flatten(data);
//	code.encoded = true;
//	return code;
//}
//Code& encodeVector (Code& data) {
Code encodeVector(Code data) {
//	return data.vector();
	if (data.encoded)return data;
//	Code code = unsignedLEB128(data.length) + flatten(data);
	Code code = Code((byte) data.length) + flatten(data);
	code.encoded = true;
	return code;
}

// https://webassembly.github.io/spec/core/binary/modules.html#code-section
Code encodeLocal(long count, Valtype type) {
	return unsignedLEB128(count).addByte(type);
}

// https://webassembly.github.io/spec/core/binary/modules.html#sections
// sections are encoded by their type followed by their vector contents
Code createSection(Section sectionType, Code data) {
	return Code(sectionType, encodeVector(data));
}

//std::map<String ,int> symbols;
Map<String, int> functionIndices;


enum NodTypes {
	numberLiteral,
	identifier,
	binaryExpression,
	printStatement,
	variableDeclaration,
	variableAssignment,
	whileStatement,
	ifStatement,
	callStatement,
	internalError,
};


bytes ieee754(float num) {
	char data[4];
	float *hack = ((float *) data);
	*hack = num;
	char *flip = static_cast<char *>(malloc(5));
	short i = 4;
//	while (i--)flip[3 - i] = data[i];
	while (i--)flip[i] = data[i];// don't flip, just copy to malloc
	return flip;
}
//Code emitExpression (Node* nodes);

Code emitBlock(Node node, Valtype returns);

//Code emitExpression(Node *node)__attribute__((warn_unused_result));
//Code emitExpression(Node *node)__attribute__((error_unused_result));

Code emitValue(Node value);

Map<int, String> collect_locals(Node node);

Valtype last_type = voids;// autocast if not int
Map<String, Valtype> return_types;
Map<int, Map<int, String>> locals;

Code emitValue(Node node) {
	Code code;
	switch (node.kind) {
		case nils:// also 0, false
//			code.opcode((byte)i64_auto);// nil is pointer
			code.addByte((byte) i32_auto);// nil is pointer
			code.push((long) 0);
			break;
		case bools:
//		case ints:
			code.addByte((byte) i32_auto);
			code.push(node.value.longy);
			last_type = i32;
//				code.opcode(ieee754(node.value.longy),4);
			break;
		case longs:
			// todo: ints!!!
			last_type = i32;
//			if(call_extern)
			code.addByte((byte) i32_const);
//			code.opcode((byte)i64_auto);
			code.push(node.value.longy);
//				code.opcode(ieee754(node.value.longy),4);
			break;
		case reals:
			last_type = f32;// auto cast return!
			code.addByte((byte) f32_auto);
			code.push(ieee754(node.value.real), 4);
			break;
//			case identifier:
//				code.opcode(get_local);
//				code.opcode(unsignedLEB128(localIndexForSymbol(node.value)), 8);
//				break;
//			case binaryExpression:
//				code.opcode(binaryOpcode[node.value]);
//				break;
		default:
			error("emitValue unknown type: "s + typeName(node.kind));
	}
	return code;
}
Code emitIf(Node &node);
Code emitExpression(Node *nodes);

Code emitExpression(Node &node) { // expression, node or BODY (list)
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	NodTypes type = internalError;// todo: type/kind
	Code code;
	int context = 0;// changes in declarations (also with lambdas as functions)
	functionIndices.setDefault(-1);
	return_types.setDefault(voids);
	int index = functionIndices[node.name];
	if (index >= 0 and not locals.has(index))
		locals.insert_or_assign(index, Map<int, String>());


	if (node.name == "if")
		return emitIf(node);


	switch (node.kind) {
		case function:
			if (index < 0) {
				breakpoint_helper
				error("MISSING WASM import/declaration for function %s\n"s % node.name);
			}

			for (Node arg : node) {
				code.push(emitExpression(arg));
			};
			code.addByte(call);
			code.addByte(index);// ok till index>127, then use unsignedLEB128
			last_type = return_types[node.name];// voids;// todo lookup return type
			break;
		case operators: {
//			if (node.length == 2) {// binary operators, and others
			Node &lhs = node["lhs"];
			Node &rhs = node["rhs"];
			if(lhs==node or rhs==node)
				error("op rhs lhs BUG");
			const Code &lhs_code = emitExpression(lhs);
			const Code &rhs_code = emitExpression(rhs);
			code.push(lhs_code);// might be empty ok
			code.push(rhs_code);// might be empty ok
//			}
			if (index >= 0) {// FUNCTION CALL
				log("OPERATOR FUNCTION CALL: %s\n"s % node.name);
//				for (Node arg : node) {
//					emitExpression(arg);
//				};
				code.addByte(call);
				code.addByte((index));// ok till index>127?
				break;
			}
			byte opcode = opcodes(node.name, last_type == f32);
			if (opcode == f32_eqz) { // hack for missing f32_eqz
//				0.0 + code.addByte(f32_eq);
				code.addByte(
						i32_reinterpret_f32);// f32->i32  i32_trunc_f32_s would also work, but reinterpret is cheaper
				code.addByte(i32_eqz);
				last_type = i32;// bool'ish
				break;
			} else if (opcode > 0)
				code.addByte(opcode);
			else {
				breakpoint_helper
				error("unknown opcode / call / symbol: "s + node.name);
			}
			if (opcode == i32_add or opcode == i32_modulo or opcode == i32_sub or opcode == i32_div or
			    opcode == i32_mul)
				last_type = i32;
			if (opcode == f32_eq or opcode == f32_gt or opcode == f32_lt)
				last_type = i32;// bool'ish
		}
			break;
		case nils:
		case longs:
		case reals:
		case bools:
		case strings:
			if (not node.isSetter() || node.value.longy == 0) // todo 0
				return emitValue(node);
//			else FALLTHROUGH!
		case reference:
			if (node.isSetter()) { //SET
				Map<int, String> current_local_names = locals[context];
				int local_index = current_local_names.position(node.name);// defined in block header
				if (local_index < 0)err("UNKNOWN local symbol "s + node.name);
				code = code + emitValue(node); // done above!
				code.addByte(set_local);
				code.addByte(local_index);
			} else {// GET
				Map<int, String> current_local_names = locals[context];
				int local_index = current_local_names.position(node.name);
				if (local_index < 0)err("UNKNOWN local symbol "s + node.name);
				code.addByte(get_local);
				code.addByte(local_index);
			}
			break;
		case expressions:
		case groups:
		case objects:
			for (Node child : node) {
				code.push(emitExpression(child));
			};
			break;
		case printStatement:
			emitExpression(node.children);// only int so far lol
			code.push(Call("print"));
			break;

		default:
			breakpoint_helper
			error("unhandled node type: "s + typeName(node.kind));
	}
	switch (type) {
		case whileStatement:
			// outer block
			code.addByte(block);
			code.addByte(void_block);
			// inner loop
			code.addByte(loop);
			code.addByte(void_block);
			// compute the while expression
			emitExpression(node[0]);// node.value.node or
			code.addByte(i32_eqz);
			// br_if $label0
			code.addByte(br_if);
			code.addByte(1);
//			code.push(signedLEB128(1));
			// the nested logic
			emitExpression(node[1]);// BODY
			// br $label1
			code.addByte(br);
			code.addByte(0);

//				code.push(signedLEB128(0));
			// end loop
			code.addByte(end_block);
			// end block
			code.addByte(end_block);
			break;
	};
	if (node.kind == reference and !node.isEmpty()) { //SET local

	}
	return code;
}


Code emitExpression(Node *nodes) {
	if (!nodes)return Code();
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	return emitExpression(*nodes);
}


Code emitIf(Node &node) {
	Code code;
//	Node *condition = node[0].value.node;
	Node &condition = node["condition"];
	code=code+emitExpression(condition);
	if(last_type!=int32){
		err("todo");
	}
	code.addByte(if_i);
	code.addByte(int32);
//	Node *then = node[1].value.node;
	Node& then = node["then"];
	code=code+emitExpression(then);// BODY
	if(node.length==3){
		code.addByte(elsa);
		Node *otherwise = node[2].value.node;//->clone();
		code=code+emitExpression(otherwise);
	}
	code.addByte(end_block);
	return code;
}

Code emitIf_OLD(Node &node) {
	Code code;
//	case ifStatement:
	// if block
	code.addByte(block);
	code.addByte(void_block);
	// compute the if expression
	Node *condition = node[0].value.node;
//	Node *condition = node["condition"];//.value.node->clone();
	emitExpression(condition);
	code.addByte(i32_eqz);
	// br_if $label0
	code.addByte(br_if);
	code.addByte(0);
//			code.opcode(signedLEB128(0));
	// the nested logic
	Node *then = node[1].value.node;
//	Node *then = node["then"].value.node->clone();
	emitExpression(then);// BODY
	// end block
	code.addByte(end_block);

	if(node.length==3){

	// else block
//			or if OR-IF YAY semantically beautiful if false {} or if 2>1 {}
//			// compute the if expression (elsif) elif orif
			code.addByte(block);
			code.addByte(void_block);
//
//			emitExpression(node.param);
			code.addByte(i32_auto);
////				code.opcode(signedLEB128(1));
			code.addByte(1);
			code.addByte(i32_eq);
//			// br_if $label0
			code.addByte(br_if);
			code.addByte(0);
////				code.opcode(signedLEB128(0));
//			// the nested logic
//		Node *otherwise = node["else"].value.node->clone();
		Node *otherwise = node[2].value.node;//->clone();
		emitExpression(otherwise);

//			// end block
			code.addByte(end_block);
	}
	return code;
}



Code Call(char *symbol) {//},Node* args=0) {
	Code code;
	code.addByte(call);
	int i = functionIndices[symbol];
//	code.opcode(unsignedLEB128(i),8);
	code.addByte(i);
	return Code();
}

class Program {
public:
	Node &ast;
	Node main;

	Program() : ast(NIL) {
	}

	Program(Node node) : ast(node) {
//		collectFunctions();
//		collectLambdas();// closures all over
//		collectImports();
//		collectExports();
//		collectGlobals();
//		collectStrings();
		collectMain();
	}

	void collectMain() {
//		if(!functions["main"])
		main = ast;// root nodes form 'main'
	}


//	char *mapp(char* (lambda)(String, int)) {
//		for(Nod n:*this){
//			lambda(n.value, n.index);
//		}
//	}
//	int findIndex(bool (lambda)(Nod)) {
//		int i=0;
//		for(Nod n:*this){
//			if(lambda(n)){
//				return i;
//			}
//			i++;
//		}
//		return -1;
//	}
	Node functions;
};


//Code codeFromProc (ProcStatementNod node, Program program_node) {
//	for(Nod arg:node.args){
//		symbols.insert_or_assign(arg.value, arg.index);
//	}
//	Code code;// not global ;)
//	emitStatements(node.nodes);
//
//	auto localCount = symbols.size();
//	bytes locals = localCount > 0 ? encodeLocal(localCount, f32).data : new char[]{};
//	todo();// check if ok: localCount == size of locals ???
////	return encodeVector([...encodeVector(locals), ...code, Opcodes.end]);
//	return encodeVector(Code(locals,localCount).push(code).push(end_block));
//};


Code encodeString(char *str) {
	size_t len = strlen0(str);
	Code code = Code(len, str, len);
	return code;//.push(0);
};

//bytes
Code signedLEB128(int n) {
	Code result;
	while (true) {
		const char byt = n & 0x7f;
		n >>= 7;
		if (
				(n == 0 && (byt & 0x40) == 0) ||
				(n == -1 && (byt & 0x40) != 0)
				) {
			result.addByte(byt);
			return result;
		}
		result.addByte(byt | 0x80);
	}
//	return result.data;
}


Code &emit(Program ast) {
	functionIndices.clear();
	return_types.setDefault(voids);
	locals.insert_or_assign(0, Map<int, String>());
	// Function types are vectors of parameters and return types. Currently
	// WebAssembly only supports single return values
//  bytes printFunctionType = new char[]{functionType,encodeVector(f32), emptyArray}
	Code printFunctionType = Code(functionType).push(encodeVector(Code(f32))).push(emptyArray);


	// optimise TODO - some of the procs might have the same type signature
	Code funcTypes;
	for (Node &proc : ast.functions) {
		Code args;
		for (Node arg: proc[0]) { args.addByte(f32); }
		Code functionTypes = Code(functionType).push(encodeVector(args)).push(emptyArray);
	}
//  ast.map(proc {[functionType, encodeVector(proc.args.map(_ -> f32)), emptyArray]);



	// the type section is a vector of function types
//	auto typeSection = createSection(type, encodeVector(printFunctionType).push(funcTypes));
//	char type0[]={0x01,0x60/*const type form*/,0x02/*param count*/,0x7F,0x7F,0x01/*return count*/,0x7F};
	char vi[] = {0x60/*const type form*/, 0x00/*param count*/, 0x01/*return count*/,
	             i32};// our main function! todo : be flexible!
	char iv[] = {0x60/*const type form*/, 0x01/*param count*/, i32, 0x00/*return count*/};
	char vv[] = {0x60/*const type form*/, 0x00/*param count*/, 0x00/*return count*/};
	char ii[] = {0x60/*const type form*/, 0x01/*param count*/, i32, 0x01/*return count*/, i32};
	char iii[] = {0x60/*const type form*/, 0x02/*param count*/, i32, i32, 0x01/*return count*/, i32};
	char fv[] = {0x60/*const type form*/, 0x01/*param count*/, f32, 0x00/*return count*/};

	int typeCount = 5;
	const Code &type_data = Code(vi, sizeof(vi)) + Code(iv, sizeof(iv)) + Code(vv, sizeof(vv)) + Code(ii, sizeof(ii)) +
	                        Code(fv, sizeof(fv)); //			+ Code(iii, sizeof(iii));
	auto typeSection = Code(type, encodeVector(Code(typeCount) + type_data));

	auto lambdo = [](String val, int index) { return index + 1; /* type index */};
//	char func_types[]={0x01,0x00};
//			encodeVector(ast.mapp(lambdo)) TODO

	/* limits https://webassembly.github.io/spec/core/binary/types.html#limits - indicates a min memory size of one page */
	auto memorySection = createSection(memory_section, encodeVector(Code(2)));
	auto memoryImport =
			encodeString("env") + encodeString("memory") + (byte) mem_export/*type*/+ (byte) 0x00 + (byte) 0x01;

	// the import section is a vector of imported functions
	Code logi_iv = encodeString("env") + encodeString("logi").addByte(func_export).addType(1);
	Code logf_fv = encodeString("env") + encodeString("logf").addByte(func_export).addType(4);
	Code square_ii = encodeString("env") + encodeString("square").addByte(func_export).addType(3);
	Code sqrt_ii = encodeString("env") + encodeString("√").addByte(func_export).addType(3);

	int import_count = 4;
	auto importSection = createSection(import, Code(import_count) + logi_iv + logf_fv + square_ii + sqrt_ii);
//
	functionIndices.insert_or_assign("logi", functionIndices.size());// import!
	functionIndices.insert_or_assign("logf", functionIndices.size());// import!
	functionIndices.insert_or_assign("square", functionIndices.size());
	functionIndices.insert_or_assign("√", functionIndices.size());
	return_types.insert_or_assign("square", int32);
	return_types.insert_or_assign("√", int32);
	return_types.insert_or_assign("id", int32);
//	return_types.insert_or_assign("logi", voids);

//	auto importSection = createSection(import, encodeVector(printFunctionImport));//+memoryImport

	// the export section is a vector of exported functions
	int main_offset = import_count;// first func after import functions (which have an index too!)
	const Code &exportsData = encodeVector(
			Code(0x01) + encodeString("main") + (byte) func_export + Code(main_offset)/*.push(0).push(0)*/ );
	auto exportSection = createSection(exports, exportsData);
//	Code(ast.findIndex([](Nod a) { return a.name == "main"; }) + 1)

	// the code section contains vectors of functions

	auto lambde = [](String val, int index) { return index + 1; /* type index */};
//	a -> codeFromProc(a, ast)
// ast.mapp(lambde(ast)); TODO
// https://pengowray.github.io/wasm-ops/
//	char code_data[] = createSection() 0x01,0x08,0x00,0x01,0x3f,0x0F,0x0B};// ok 0x01==nop
//  Code code_data=encodeVectors(encodeVector(1/*function_index/))
//	char code_data[] = {0x01,0x05,0x00,0x41,0x2A,0x0F,0x0B};// 0x41==i32_auto  0x2A==42 0x0F==return 0x0B=='end (function block)' opcode @+39
//function body count


	short function_offset = import_count;
// index needs to be known before emitting code, so call $i works
	functionIndices.insert_or_assign("main", function_offset++);
	functionIndices.insert_or_assign("nop", function_offset++);
	functionIndices.insert_or_assign("id", function_offset++);

	return_types["id"] = i32;
//	char code_data[] = {0x00, 0x41, 0x2A, 0x0F, 0x0B,0x01, 0x05, 0x00, 0x41, 0x2A, 0x0F, 0x0B};
//	char code_data[] = {0x00,0x41,0x2A,0x0F,0x0B};// 0x00 == unreachable as block header !?
//	0a 0e 02 09 00  41 2a 10 00 41 15 0f 0b 02 00
//	0a 0a 02 05 00              41 15 0f 0b 02 00
// fun #c #f        const 42 call oo

//	char code_data[] = {0/*locals_count*/,i32_const,48,call,0 /*logi*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
	char code_data[] = {0/*locals_count*/, i32_auto, 42, return_block,
	                    end_block};// 0x00 == unreachable as block header !?
	char code_data_nop[] = {0/*locals_count*/, end_block};// NOP
//	char code_data_id[] = {0/*locals_count*/, end_block};// NOP
	char code_data_id[] = {1/*locals_count*/, 1/*???*/, i32, get_local, 0, return_block, end_block};// NOP
//	char code_data[] = {0x00,0x0b,0x02,0x00,0x0b};// empty type:1 len:2


//	Code function1 = codeBlock(code_data);
//	Code da_code2=Code(code_data,sizeof(code_data));
	Code main_block = emitBlock(ast.main, Valtype::i32);
//	check(da_code2 == da_code);
	Code nop_block = Code(code_data_nop, sizeof(code_data_nop));
	Code id_block = Code(code_data_id, sizeof(code_data_id));

	char function_count = 3;// offset by imports!?
	auto codeSection = createSection(code_section,
	                                 Code(function_count) + encodeVector(main_block) + encodeVector(nop_block) +
	                                 encodeVector(id_block));


	// the function section is a vector of type indices that indicate the type of each function in the code section
//	char func_types[]={0x01,0x00};
	char types_of_functions[] = {function_count, 0x00, 0x01, 0x03};// mapping/connecting function index to type index
	// @ WASM : WHY DIDN'T YOU JUST ADD THIS AS A FIELD IN THE FUNC STRUCT???
	Code funcSection = createSection(func, Code(types_of_functions, sizeof(types_of_functions)));


	auto nameSubSectionModuleName = Code(module_name) + encodeVector(Code("wasp_module"));

	Code nameMap;
//	check(symbols["logi"]==0);
//	check(symbols["√"]==3);// "\e2\88\9a"
//	nameMap =  Code((byte) 0) + Code("logi");
	for (int index = 0; index < functionIndices.size(); index++) {
		// danger: utf names are NOT translated to wat env.√=√ =>  (import "env" "\e2\88\9a" (func $___ (type 3)))
		String &name = functionIndices.keys[index];
		nameMap = nameMap + Code(index) + Code(name);
	}


	auto nameSubSectionFunctionNames = Code(function_names) + encodeVector(Code(functionIndices.size()) + nameMap);
//	auto nameSubSectionFunctionNames = Code(function_names) + encodeVector(Code(1) + Code((byte) 0) + Code("logi"));

//	functions without parameters need  entry ( 00 01 00 00 )
//  functions 5 with local 'hello' :  05 01 00 05 68 65 6c 6c 6f
//  functions 5 with local 'hello' :  05 02 00 05 68 65 6c 6c 6f 01 00 AND unnamed (local i32)
// localMapEntry = (index nrLocals 00? string )

	Code localNameMap;
//	Map< localNames
	for (String key : functionIndices) {
		if (key != "main")continue;
		int function_index = functionIndices[key];
		Map<int, String> &currentLocalNames = locals[function_index - main_offset];
		int local_count = currentLocalNames.size();
		localNameMap = localNameMap + Code(function_index) + Code(local_count); /*???*/
		for (int i = 0; i < currentLocalNames.size(); ++i) {
//		String local_name = "test_"s+key+"#"+local_count;
			String local_name = currentLocalNames[i];
			localNameMap = localNameMap + Code(i) + Code(local_name);
		}


//		error: expected local name count (1) <= local count (0) FOR FUNCTION ...
	}
//	localNameMap = localNameMap + Code((byte) 0) + Code((byte) 1) + Code((byte) 0) + Code((byte) 0);// 1 unnamed local
//	localNameMap = localNameMap + Code((byte) 4) + Code((byte) 0);// no locals for function4
	localNameMap =
			localNameMap + Code((byte) 5) /*function index*/ + Code((byte) 1) /*count*/ + Code((byte) 0) /*l.nr*/ +
			Code("var1");// function 5 with one local : var1
	auto nameSubSectionLocalNames = Code(local_names) + encodeVector(Code((byte) 1) + localNameMap);


//	auto nameSubSectionFuncNames = Code(module_name) + encodeVector(Code("wasp_module"));
//	The name section is a custom section whose name string is itself ‘𝚗𝚊𝚖𝚎’. The name section should appear only once in a module, and only after the data section.
	const Code &nameSectionData = encodeVector(
			Code("name") + nameSubSectionModuleName + nameSubSectionFunctionNames + nameSubSectionLocalNames);
	auto nameSection = createSection(custom, nameSectionData);


//	+

	auto customSection = createSection(custom, encodeVector(Code("custom123") + Code("random custom section data")));

	Code code = Code(magicModuleHeader, 4) + Code(moduleVersion, 4) + typeSection + importSection + funcSection +
	            exportSection + codeSection + nameSection;// + customSection;
//	Code code = Code(magicModuleHeader, 4) + Code(moduleVersion, 4) + typeSection + funcSection + exportSection + codeSection;// + memorySection + ;
	code.debug();
	return code.clone();
}

Code emitBlock(Node node, Valtype returns) {
//	char code_data[] = {0/*locals_count*/,i32_const,42,call,0 /*logi*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	char code_data[] = {0/*locals_count*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	Code(code_data,sizeof(code_data)); // todo : memcopy, else stack value is LOST
	Code block;
	int context = 0;
	Map<int, String> current_local_names = collect_locals(node);
	int locals_count = current_local_names.size();
	locals.insert_or_assign(context, current_local_names);
	block.addByte(locals_count);
	for (int i = 0; i < locals_count; ++i) {
		block.addByte(i + 1);// index
		block.addByte(i32);// type
	}

	Code inner_code_data = emitExpression(node);
	Valtype x = last_type;
	block.push(inner_code_data);
	if (returns != last_type) {
		if (returns == Valtype::voids and last_type != Valtype::voids)
			block.addByte(drop);
		if (returns == Valtype::i32 and last_type == Valtype::f32)
			block.addByte(i32_trunc_f32_s);
		if (returns == Valtype::i32 and last_type == Valtype::voids)
			block.addByte(i32_const).addByte(0);// hack? return 0/false by default. ok? see python!
//		if(returns==Valtype::f32)…
	}

//if not return_block
	block.addByte(return_block);
	block.addByte(end_block);
	return block;
}

Map<int, String> collect_locals(Node node) {
	Map<int, String> current_locals;
	for (Node n : node) {
		if (n.kind == reference and not functionIndices.has(n.name))
			current_locals.insert_or_assign(current_locals.size(), n.name);
		else if (n.kind == longs and not n.name.empty() and not atoi0(n.name))
			current_locals.insert_or_assign(current_locals.size(), n.name);
	}
	return current_locals;
}


Code &emit(Node code) {
	return emit(Program(code));
}