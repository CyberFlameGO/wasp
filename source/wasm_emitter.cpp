//https://blog.sentry.io/2020/08/11/the-pain-of-debugging-webassembly
//https://developer.mozilla.org/en-US/docs/WebAssembly/Understanding_the_text_format
//https://github.com/mdn/webassembly-examples/tree/master/understanding-text-format
// https://github.com/ColinEberhardt/chasm/blob/master/src/emitter.ts
// https://github.com/ColinEberhardt/chasm/blob/master/src/encoding.ts
// https://pengowray.github.io/wasm-ops/
#include <cmath> // pow
#include "Wasp.h"
#include "String.h"
#include "Map.h"
#include "Code.h"
#include "wasm_emitter.h"
#include "wasm_helpers.h"
//#include "wasm_runner.h"
#include "wasm_reader.h"

#define check(test) if(test){log("OK check passes: ");log(#test);}else{printf("\nNOT PASSING %s\n%s:%d\n",#test,__FILE__,__LINE__);exit(0);}

int runtime_offset = 0; // imports + funcs
int import_count = 0;
short builtin_count = 0;// function_offset - import_count - runtime_offset;

//bytes data;// any data to be stored to wasm: values of variables, strings, nodes etc
char *data;// any data to be stored to wasm: values of variables, strings, nodes etc
int data_index_end;// position to write more data = end + length of data section
int data_index_start = 0;
int last_data = 0;// last pointer outside stack
Map<String *, long> stringIndices; // wasm pointers to strings within wasm data WITHOUT runtime offset!
Map<String, long> referenceIndices;
//Map<long,int> dataIndices; // wasm pointers to strings etc (key: hash!)  within wasm data

Module runtime;
String start = "main";

Valtype rhs_type = voids;// autocast if not int
Valtype last_type = voids;// autocast if not int

enum MemoryHandling {
	import_memory,
	export_memory,
	internal_memory,
	no_memory,
};
MemoryHandling memoryHandling = export_memory;// import_memory;

//Map<String, Valtype> return_types;
//Map<int, List<String>> locals;
//Map<int, Map<int, String>> locals;
//List<String> declaredFunctions; only new functions that will get a Code block, no runtime/imports
Map<String, int> functionIndices;
Map<String, Code> functionCodes;
Map<String, int> typeMap;


Code Call(char *symbol);//Node* args

//Code& unsignedLEB128(int);
//Code& flatten(byter);
//Code& flatten (Code& data);
//void todo1(char *message = "") {
//	printf("TODO %s\n", message);
//}

//typedef int number;
//typedef char byte;
//typedef char* Code;
//typedef char id;




//class Bytes {
//public:
//	int length;
//};

//bytes
Code signedLEB128(int i);

Code encodeString(char *String);

long emitData(Node node, String context);
//typedef Code any;
//typedef Bytes any;

// class Code;

// Code flatten (any arr[]) {
// [].concat.apply([], arr);



// https://pengowray.github.io/wasm-ops/
byte opcodes(chars s, byte kind = 0) {
//	if(eq(s,"$1="))return set_local;
//	if (eq(s, "=$1"))return get_local;
//	if (eq(s, "=$1"))return tee_local;

	if (kind == 0 or kind == i32t) { // INT32
		if (eq(s, "+"))return i32_add;
		if (eq(s, "-"))return i32_sub;
		if (eq(s, "*"))return i32_mul;
		if (eq(s, "/"))return i32_div;
		if (eq(s, "%"))return i32_rem;
		if (eq(s, "=="))return i32_eq;
		if (eq(s, "eq"))return i32_eq;
		if (eq(s, "equals"))return i32_eq;
		if (eq(s, "is"))return i32_eq;// careful could be declaration := !
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
		if (eq(s, ">="))return f32_ge;
		if (eq(s, "<"))return f32_lt;
		if (eq(s, "<="))return f32_le;
	}
	if (eq(s, "√"))return f32_sqrt; // forces i32->f32

	// todo : peek 65536 as float directly via opcode
	if (eq(s, "peek"))return i64_load;  // memory.peek memory.get memory.read
	if (eq(s, "poke"))return i64_store; // memory.poke memory.set memory.write

	// todo : set_local,  global_get ...
	if (eq(s, "$"))return get_local; // $0 $1 ...

	printf("unknown or non-primitive operator %s\n", s);// can still be matched as function etc, e.g. 'a'+'b' is 'ab'
	breakpoint_helper
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
typedef unsigned char byte;
// https://webassembly.github.io/spec/core/binary/modules.html#binary-module



// https://webassembly.github.io/spec/core/binary/modules.html#code-section
Code encodeLocal(long count, Valtype type) {
	return unsignedLEB128(count).addByte(type);
}

// https://webassembly.github.io/spec/core/binary/modules.html#sections
// sections are encoded by their type followed by their vector contents


enum NodTypes {
	numberLiteral,
	identifier,
	binaryExpression,
	printStatement,
	variableDeclaration,
	variableAssignment,
	functionDeclaration,
	whileStatement,
	ifStatement,
	callStatement,
	internalError,
};


bytes ieee754(float num) {
	char data[4];
	float *hack = ((float *) data);
	*hack = num;
	byte *flip = static_cast<byte *>(alloc(1, 5));
	short i = 4;
//	while (i--)flip[3 - i] = data[i];
	while (i--)flip[i] = data[i];// don't flip, just copy to malloc
	return flip;
}
//Code emitExpression (Node* nodes);

Code emitBlock(Node node, String functionContext);

//Code emitExpression(Node *node)__attribute__((warn_unused_result));
//Code emitExpression(Node *node)__attribute__((error_unused_result));


//Map<int, String>
List<String> collect_locals(Node node, String context);


Code cast(Valtype from, Valtype to);

Code emitStringOp(Node op, String context);

Code emitValue(Node node, String context);

Valtype fixValtype(Valtype &valtype);

Code emitArray(Node &node, String context) {
	let code = Code();
	int pointer = data_index_end;
	for (Node &child:node) {
		// todo: smart pointers?
		emitData(child, context);// pointers in flat i32/i64 format!
	}
	String ref = node.name;
	if (node.name.empty() and node.parent) {
		ref = node.parent->name;
	}
	referenceIndices.insert_or_assign(ref, pointer);
	emitData(Node(0), context);// terminate list with 0.
	// todo: emit length header! 100% neccessary for [2 1 0 1 2] and index bound checks
	last_data = pointer;
	return code.addConst(pointer);// once written to data section, we also want to use it immediately
}


bool isAssignable(Node node, Node *type = 0) {
	//todo
	//	if(node.metas().has("constant") or node.metas().has("immutable")) // …
	//		return false;
	//	if(type and type!=node.type)
	//		return false;
	return true;
}

Code emitIndexWrite(Node op, String context) {
	int base = 1024;
	int size = 4;
	int offset = op["offset"].value.longy * size;
	int value = op["value"].value.longy;
	Code store;
	store.addConst(base + offset);
	store.addConst(value);
	store.add(i32_store);
	store.add(0x02);// alignment (?)
	store.add(0x00);// ?
	return store;
/*  000101: 41 94 08                   | i32.const 1044
	000104: 41 06                      | i32.const 6
    000106: 36 02 00                   | i32.store 2 0 */
}


// assumes value is on top of stack
Code emitIndexPattern(Node pattern, String context) {
	if (pattern.kind != patterns and pattern.kind != longs)error("pattern expected in emitIndexPattern");
	if (pattern.length != 1 and pattern.kind != longs)error("exactly one pattern expected in emitIndexPattern");
	int base = runtime.data_segments.length;// uh, todo?
	int size = 1;// todo char, codepoint, int , pointer …
	if (last_type == string)size = 1;// chars for now vs codepoint!
	if (last_type == int32)size = 4;
	if (last_type == int64)size = 8;
	if (last_type == float32)size = 4;
	if (last_type == float64)size = 8;
	int offset = (long) pattern.first().value.longy;
	if (offset < 1)error("operator # starts from 1, use [] for zero-indexing");
//	if (offset > array_length)error("operator # out of bounds %d>%d"s % offset % array_length);

	Code load;
	load.addConst(base + offset * size);
	if (size == 1)load.add(i8_load);
	if (size == 2)load.add(i16_load);
	if (size == 4)load.add(i32_load);
	if (size == 8)load.add(i64_load);
	load.add(0x02);// alignment (?)
	load.add(0x00);// ?
	return load;
}

// todo: merge emitIndexPattern with :
Code emitIndexRead(Node op, String context) {
	int base = runtime.data_segments.length;// uh, todo?
	int size = 4;
	size = 1;
//	if(op[0].kind==strings) todo?
	last_type = rhs_type;
	if (last_type == charp)size = 1;// chars for now vs codepoint!
	if (last_type == string)size = 1;// chars for now vs codepoint!
	if (last_type == int32)size = 4;
	if (last_type == int64)size = 8;
	if (last_type == float32)size = 4;
	if (last_type == float64)size = 8;
	if (op.length < 2)
		error("operator # needs two arguments: node/array/reference and position");
	Node &array = op[0];// also String: byte array or codepoint array todo
	if (array.kind == reference or array.kind == keyNode) {
		String ref = array.name;
//		last_type=array.data_kind;
		if (not referenceIndices.has(ref))
			error("reference not declared as array type: "s + ref);
		base += referenceIndices[ref];
	} else if (array.kind == strings) {
		base += stringIndices[array.value.string];
	} else
		base += last_data;// todo: pray!
	int offset = (long) op[1];
	if (offset < 1)error("operator # starts from 1, use [] for zero-indexing");
	if (op.name == "#") offset--;
//	if(offset>op[0].length)error("index out of bounds! %d > %d in %s (%s)"s % offset % ); // todo: get string size, array length etc
	Code load;
	load.addConst(base + offset * size);
	if (size == 1)load.add(i8_load);
	if (size == 2)load.add(i16_load);
	if (size == 4)load.add(i32_load);
	if (size == 8)load.add(i64_load);
	load.add(0x02);// alignment (?)
	load.add(0x00);// ?
	return load;
	//	i32.const 1028
	//	i32.const 3
	//	i32.load
}

// write data to data segment (vs emitValue on stack)
// returns pointer
long emitData(Node node, String context) {
	String &name = node.name;
	int last_pointer = data_index_end;
	switch (node.kind) {
		case nils:// also 0, false
		case bools:
//			error("reuse constants for nil/true/false");
		case longs:
			// todo: add header?
			// todo: wasteful? but compare to boxed values NOT wasteful!
			// todo: add smart-pointer header?
//			if(leb)
//			Code &leb128 = signedLEB128(node.value.longy);
//			data_index_end+=leb128.length
			if (node.value.longy > 0xF0000000) {
				error("true long big ints currently not supported");
				*(long *) (data + data_index_end) = node.value.longy;
				data_index_end += 8;
				last_type = i64t;
			}
			*(int *) (data + data_index_end) = node.value.longy;
			data_index_end += 4;
			last_type = int32;
			break;
		case reals:
//			bytes varInt = ieee754(node.value.real);
			*(double *) (data + data_index_end) = node.value.real;
			data_index_end += 8;
			last_type = float64;
			break;
		case reference:
			if (referenceIndices.has(node.name)) {
				error("can't save unknown reference pointer "s + name);
			} else
				todo("emitData reference");
			break;
		case strings: {
			int stringIndex = data_index_end + runtime.data_segments.length;// uh, todo?
			String *pString = node.value.string;
			if (stringIndices.has(
					pString)) // todo: reuse same strings even if different pointer, aor make same pointer before
				stringIndex = stringIndices[pString];
			else {
				stringIndices.insert_or_assign(pString, data_index_end);
				//				Code lens(pString->length);// we follow the standard wasm abi to encode pString as LEB-lenght + data:
				//				strcpy2(data + data_index_end, (char*)lens.data, lens.length);
				//				data_index_end += lens.length;// unsignedLEB128 encoded length of pString
				strcpy2(data + data_index_end, pString->data, pString->length);
				data[data_index_end + pString->length] = 0;
				// we add an extra 0, unlike normal wasm abi, because we have space in data section
				data_index_end += pString->length + 1;
			}
			last_type = string;
			break;
		}
		case objects:
		case groups:
			// keep last_type from last child, later if mixed types, last_type=ref or smarty
			emitArray(node, context);
			break;
		case keyNode:
		case patterns:
		default:
			error("emitData unknown type: "s + typeName(node.kind));
	}
	// todo: ambiguity emit("1;'a'") => result is pointer to [1,'a'] or 'a' ? should be 'a', but why?
	last_data = last_pointer;
	return last_pointer;
}

// put value on stack (vs emitData)
// todo: last_type not enough if operator left≠right, e.g. ['a']#1  2.3 == 4 ?
Code emitValue(Node node, String context) {
	Code code;
	String &name = node.name;
	switch (node.kind) {
		case nils:// also 0, false
			code.addByte((byte) i32_auto);// nil is pointer
			code.push((long) 0);
			break;
		case bools:
//		case ints:
			code.addByte((byte) i32_auto);
			code.push(node.value.longy);// LEB encoded!
			last_type = i32t;
			break;
		case longs:
			// todo: ints vs longs!!!
			last_type = i32t;
			code.addByte((byte) i32_const);
			code.push(node.value.longy);
			break;
		case reals:
			last_type = f32t;// auto cast return!
			code.addByte((byte) f32_auto);
			code.push(ieee754(node.value.real), 4);
			break;
//		case identifier:
		case reference: {
			int local_index = locals[context].position(name);
			if (local_index < 0)error("UNKNOWN symbol "s + name + " in context " + context);
			if (node.value.node) {
				// todo HOLUP! x:41 is a reference? then *node.value.node makes no sense!!!
				code.add(emitSetter(node, *node.value.node, context));
			} else {
				code.addByte(get_local);// todo skip repeats
				code.addByte(local_index);
			}
		}
			break;
//			case binaryExpression:
//				code.opcode(binaryOpcode[node.value]);
//				break;
		case operators:
			warn("operators should never be emitted as values");
			return emitExpression(node, context);
		case strings: {
			// append pString (as char*) to data section and access via stringIndex
			int stringIndex = data_index_end + runtime.data_segments.length;// uh, todo?
			String *pString = node.value.string;
			if (stringIndices.has(
					pString)) // todo: reuse same strings even if different pointer, aor make same pointer before
				stringIndex = stringIndices[pString];
			else {
				stringIndices.insert_or_assign(pString, data_index_end);
//				Code lens(pString->length);// we follow the standard wasm abi to encode pString as LEB-lenght + data:
//				strcpy2(data + data_index_end, (char*)lens.data, lens.length);
//				data_index_end += lens.length;// unsignedLEB128 encoded length of pString
				strcpy2(data + data_index_end, pString->data, pString->length);
				data[data_index_end + pString->length] = 0;
				if (referenceIndices.has(name)) {
					if (not isAssignable(node))
						error("can't reassign reference "s + name);
					else
						trace("reassigning reference "s + name + " to " + node.value.string);
				}
				if (node.parent and (node.parent->kind == reference or
				                     node.parent->kind == keyNode))// todo move up! todo keyNode bad criterion!!
					referenceIndices.insert_or_assign(node.parent->name, data_index_end);// safe ref to string
				// we add an extra 0, unlike normal wasm abi, because we have space in data section
				data_index_end += pString->length + 1;
			}
			last_type = charp;//
			code = Code(i32_const) + Code(stringIndex);// just a pointer
			if (node.length > 0) {
				if (node.length > 1)error("only 1 pattern allowed");
				Node &pattern = node.first();
				if (pattern.kind != patterns and pattern.kind != longs)
					error("only patterns allowed on string");
				code.add(emitIndexPattern(pattern, context));
			}
			return code;
//			return Code(stringIndex).addInt(pString->length);// pointer + length
		}
		case keyNode:
			return emitValue(*node.value.node,
			                 context);// todo: make sure it is called from right context (after isSetter …)
		case patterns:
			return emitIndexPattern(node, context);// todo: make sure to have something indexable on stack!
		case expressions:
//			error("expressions should not be put on stack (yet) (maybe serialize later)")
		default:
			error("emitValue unknown type: "s + typeName(node.kind));
	}
	return code;
}


Code emitOperator(Node node, String context) {
	Code code;
	String &name = node.name;
	int index = functionIndices.position(name);
	if (name == "then")return emitIf(*node.parent, context);// pure if handled before
	if (name == ":=") // todo or ... !
		return emitDeclaration(node, node.first());
	if (name == "=") // todo or ... !
		return emitSetter(node, node.first(), context);
	if (node.length < 1 and not node.value.node and not node.next) {
		node.log();
		error("missing args for operator "s + name);
	} else if (node.length == 1) {
//				if(name.in(function_list)) SHOULDN'T HAPPEN!
//				error("unexpected unary operator: "s + name);
		Node arg = node.children[0];
		const Code &arg_code = emitExpression(arg, context);// should ALWAYS just be value, right?
		code.push(arg_code);// might be empty ok
	} else if (node.length == 2) {
		Node lhs = node.children[0];//["lhs"];
		Node rhs = node.children[1];//["rhs"];
		const Code &lhs_code = emitExpression(lhs, context);
		rhs_type = last_type;
		const Code &rhs_code = emitExpression(rhs, context);
		code.push(lhs_code);// might be empty ok
		code.push(rhs_code);// might be empty ok
	} else if (node.length > 2) {// todo: n-ary? ∑? is just a function!
		error("Too many args for operator "s + name);
	} else if (node.next) { // todo really? handle ungrouped HERE? just hiding bugs?
		const Code &arg_code = emitExpression(*node.next, context);
		code.push(arg_code);// might be empty ok
	} else if (node.value.node) {
		const Code &arg_code = emitExpression(*node.value.node, context);
		code.push(arg_code);
	}
	if (index >= 0) {// FUNCTION CALL
		log("OPERATOR / FUNCTION CALL: %s\n"s % name);
//				for (Node arg : node) {
//					emitExpression(arg,context);
//				};
		code.addByte(function);
		code.add(index);
		return code;
	}
	byte opcode = opcodes(name, last_type);
	if (last_type == string)
		code.add(emitStringOp(node, String()));
	else if (opcode == f32_sqrt and last_type == i32t) {
		code.addByte(f32_convert_i32_s);// i32->f32
		code.addByte(f32_sqrt);
		code.addByte(i32_trunc_f32_s);// f32->i32  i32_trunc_f32_s would also work, but reinterpret is cheaper
//				last_type = f32t; todo: try upgrade type 2 + √2 -> float
	} else if (opcode == f32_eqz) { // hack for missing f32_eqz
//				0.0 + code.addByte(f32_eq);
		code.addByte(i32_reinterpret_f32);// f32->i32  i32_trunc_f32_s would also work, but reinterpret is cheaper
		code.addByte(i32_eqz);
		last_type = i32t;// bool'ish
	} else if (name == "++") {
		Node increased = Node("+").setType(operators);
		increased.add(node.first());
		increased.add(new Node(1));
		//		emitExpression(increased,context);
		code.add(emitSetter(node.first(), increased, context));
	} else if (name == "#") {// index operator
		if (node.parent and node.parent->name == "=")// setter!
			return emitIndexWrite(node[0], context);// todo
		else
			return emitIndexRead(node, context);
	} else if (opcode > 0xC0) {
		error("internal opcode not handled"s + opcode);
	} else if (opcode > 0) {
		code.addByte(opcode);
		if (last_type == 0)
			last_type = i32t;
	} else {
		error("unknown opcode / call / symbol: "s + name + " : " + index);
	}
	if (opcode == i32_add or opcode == i32_modulo or opcode == i32_sub or opcode == i32_div or opcode == i32_mul)
		last_type = i32t;
	if (opcode == f32_eq or opcode == f32_gt or opcode == f32_lt)
		last_type = i32t;// bool'ish
	return code;
}

Code emitStringOp(Node op, String context) {
//	Code stringOp;
	if (op == "+") {
		op = Node("concat");//demangled on readWasm, but careful, other signatures might overwrite desired one
		last_type = string;
		functionSignatures["concat"].returns(charp);// hack
//		op = Node("_Z6concatPKcS0_");//concat c++ mangled export:
//		op = Node("concat_char_const*__char_const*_");// wat name if not stripped in lib release build
		return emitCall(op, context);
//		stringOp.addByte();
	} else if (op == "=" or op == "==" or op == "is" or op == "equals") {
		op = Node("eq");//  careful : various signatures
		last_type = string;
		return Code(i32_const) + Code(-1) + emitCall(op, context);// third param required!
	} else if (op == "#") {// todo: all different index / pattern matches
		op = Node("getChar");//  careful : various signatures
		return emitCall(op, context);
	} else if (op == "logs" or op == "puts" or op == "print") {// should be handled before, but if not print anyways
		op = Node("logs");
		return emitCall(op, context);
	} else
		todo("string op not implemented: "s + op.name);
	return Code();
}

// starting with 0!
//inline haha you can't inline wasm
char getChar(chars string, int nr) {
	int len = strlen0(string);
	if (nr < 1)error("#index starts with 1, use [] if you want 0 indexing");
	if (nr > len)error("index out of bounds %i>%i "s % nr % len);
	return string[nr - 1 % len];
}

//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN BLOCK (index 0 after args, used for 'it'  after while(){} etc) !!!
int last_local = 0;


bool isVariableName(String name) {
	return name[0] >= 'a';// todo
}

Code emitExpression(Node &node, String context/*="main"*/) { // expression, node or BODY (list)
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	Code code;
	String &name = node.name;
//	int index = functionIndices.position(name);
//	if (index >= 0 and not locals[context].has(name))
//		error("locals should be analyzed in parser");
//		locals[name] = List<String>();
//	locals[index]= Map<int, String>();

	if (name == "if")
		return emitIf(node, context);
	if (name == "while")
		return emitWhile(node, context);
	if (name == "it") {
		code.addByte(get_local);
		code.addByte(last_local);
		return code;
	}
	//	or node.kind == groups ??? NO!
	if ((node.kind == call or node.kind == reference or node.kind == operators) and functionIndices.has(name))
		return emitCall(node, context);

	Node &first = node.first();
	switch (node.kind) {
		case objects:
		case groups:
			// todo: all cases of true list vs list of expressions
			if (node.length > 0 and (first.kind == longs or first.kind == strings)) {
				return Code().addConst(emitData(node, context));// pointer in const format!
//				return emitArray(node, context);
			}
		case expressions:
			for (Node child : node) {
				const Code &expression = emitExpression(child, context);
				code.push(expression);
			};
			break;
		case call:
			return emitCall(node, context);
			break;
		case operators:
			return emitOperator(node, context);
		case declaration:
			return emitDeclaration(node, first);
		case assignment:
			return emitSetter(node, first, context);
		case nils:
		case longs:
		case reals:
		case bools:
		case strings:
			if (not node.isSetter() || node.value.longy == 0) // todo 0  x="x" '123'="123" redundancy bites us here
				return emitValue(node, context);
//			else FALLTHROUGH to set x="123"!
		case keyNode: // todo i=ø
			if (not isVariableName(name))
				todo("proper keyNode emission");
			// else:
		case reference: {
//			Map<int, String>
			List<String> &current_local_names = locals[context];
			int local_index = current_local_names.position(name);// defined in block header
			if (name.startsWith("$")) {// wasm style $0 = first arg
				local_index = atoi0(name.substring(1));
			}
			if (local_index < 0) { // collected before, so can't be setter here
				if (functionCodes.has(name) or functionSignatures.has(name))
					return emitCall(node, context);
				else if (!node.isSetter())
					error("UNKNOWN local symbol "s + name + " in context " + context);
				else {
					error("local symbol "s + name.trim() + " in " + context + " should be registered in analyze()!");
					current_local_names.add(name);// ad hoc x=42
					local_index = current_local_names.size() - 1;
				}
			}
			if (node.isSetter()) { //SET
				code = code + emitValue(node, context); // done above!
				code.addByte(set_local);
				code.addByte(local_index);
				code.addByte(get_local);// make value available // todo: skip repeated get's / only when needed
				code.addByte(local_index);
			} else {// GET
				code.addByte(get_local);// todo: skip repeats
				code.addByte(local_index);
			}
		}
			break;
		case patterns: // x=[];x[1]=2;x[1]==>2
		{
			if (not node.parent)// todo: when is pattern not an operator? wrong: or node.parent->kind == groups)
				return emitArray(node, context);
			else if (node.parent->kind == declaration)
				return emitIndexWrite(*node.parent, context);
			else
				return emitIndexPattern(node, context);// make sure array is on stack!
		}
//		case groups: todo: true list vs list of expressions
//		case objects:
		case arrays:
		case buffers:
//for (Node child : node) {
//	const Code &expression = emitExpression(child, context);
//	code.push(expression);
//};
			return emitArray(node, context);
			break;
		default:
			error("unhandled node type: "s + typeName(node.kind));
	}
	return code;
}


Code emitWhile(Node &node, String context) {
	Code code;
	Node condition = node[0].values();
	Node then = node[1].values();
	code.addByte(loop);
	code.addByte(none);// type:void_block
//	code.addByte(int32);// type OR typeidx!?
	code = code + emitExpression(condition, context);// condition
	code.addByte(if_i);
	code.addByte(none);// type:void_block
//	code.addByte(int32);
	code = code + emitExpression(then, context);// BODY
	code.addByte(br);
	code.addByte(1);
	code.addByte(end_block);// end if condition then action
	code.addByte(end_block);// end while loop
	// type should fall through
//	int block_value= 0;// todo : ALWAYS MAKE RESULT VARIABLE FIRST IN FUNCTION!!!
//	code.addByte(block_value);
	return code;
}

// wasm loop…br_if is like do{}while(condition), so we have to rework while(condition){}
Code emitWhile2(Node &node, String context) {
	Code code;
	// outer block
//	code.addByte(block);
//	code.addByte(void_block);
	// inner loop
	code.addByte(loop);
	code.addByte(none);// void_block
	// compute the while expression
	code.add(emitExpression(node[0], context));// node.value.node or
//	code.add(emitExpression(node["condition"], context));// node.value.node or
	code.addByte(i32_eqz);
	// br_if $label0
	code.addByte(br_if);
	code.addByte(1);
//			code.push(signedLEB128(1));
	// the nested logic
	code.add(emitExpression(node[1], context));// BODY
//	code.add(emitExpression(node["then"], context));// BODY
	// br $label1
//	code.addByte(br);
//	code.addByte(0);
//				code.push(signedLEB128(0));
	code.addByte(end_block); // end loop
	code.addByte(get_local);
	int block_value = 0;// todo : ALWAYS MAKE RESULT VARIABLE FIRST IN FUNCTION!!!
	code.addByte(block_value);// todo: skip if last value is result
//	code.addByte(end_block); // end block
	return code;
}


Code emitExpression(Node *nodes, String context) {
	if (!nodes)return Code();
//	if(nodes==NIL)return Code();// emit nothing unless NIL is explicit! todo
	return emitExpression(*nodes, context);
}

Code emitCall(Node &fun, String context) {
	Code code;
	if (not functionSignatures.has(fun.name) or not functionIndices.has(fun.name))
		error("unknown function "s + fun.name);// checked before, remove

	Signature &signature = functionSignatures[fun.name];
	int index = functionIndices[fun.name];
	if (index < 0)
		error("MISSING import/declaration for function %s\n"s % fun.name);
	int i = 0;
	// args may have already been emitted, e.g. "A"+"B" concat
	for (Node arg : fun) {
		code.push(emitExpression(arg, context));
		Valtype argType = mapTypeToWasm(arg);
		Valtype &sigType = signature.types[i];
		if (sigType != argType)
			code.push(cast(argType, sigType));
	};
	code.addByte(function);
	code.addInt(index);// as LEB!
	code.addByte(nop);// padding for potential relocation
	signature.is_used = true;
	signature.emit = true;
	last_type = signature.return_type;
	return code;
}

Code cast(Valtype from, Valtype to) {
	Code casted;
	if (from == i32t and to == charp)return casted;// assume i32 is a pointer here. todo?
	if (from == i32t and to == float32) casted.addByte(i32_cast_to_f32_s);
	else if (from == float32 and to == i32t) casted.addByte(f32_cast_to_i32_s);
	else
		error("missing cast map "s + from + " -> " + to);
	return casted;
}

Code emitDeclaration(Node fun, Node &body) {
	// todo: x := 7 vs x := y*y
	//
	if (not functionIndices.has(fun.name)) {
		error("Declarations need to be registered before in the parser so they can be called from main code!");
//		functionIndices[fun.name] = functionIndices.size();
	}
//	else {
//		error("redeclaration of symbol: "s + fun.name);
//	}
	Signature &signature = functionSignatures[fun.name];
	signature.emit = true;// all are 'export' for now. also set in analyze!
	functionCodes[fun.name] = emitBlock(body, fun.name);
//	last_type = voids;// todo reference to new symbol x = (y:=z)
	return Code();// empty
}


Code emitSetter(Node node, Node &value, String context) {
	List<String> &current = locals[context];
	String &variable = node.name;
	if (!current.has(variable)) {
		current.add(variable);
		error("variable missed by parser! "_s + variable);
	}
	int local_index = current.position(variable);
	last_type = mapTypeToWasm(value);
	localTypes[context][local_index] = last_type;

	Code setter;
	Code value1 = emitValue(value, context);
//	variableTypes
	setter.add(value1);
	setter.add(set_local);
	setter.add(local_index);
	setter.add(get_local);// make value available
	setter.add(local_index);// todo skip repeats
	return setter;
}


Code emitIf(Node &node, String context) {
	Code code;
	Node condition = node[0].values();
//	Node &condition = node["condition"];
	code = code + emitExpression(condition, context);

	code.addByte(if_i);
	code.addByte(int32);
	Node then = node[1].values();
//	Node &then = node["then"];
	code = code + emitExpression(then, context);// BODY
	if (node.length == 3) {
		code.addByte(elsa);
//		Node otherwise = node["else"];//->clone();
		Node otherwise = node[2].values();
		code = code + emitExpression(otherwise, context);
	}
	code.addByte(end_block);
	if (last_type != int32) {
		todo("cast to int32 / smarty?");
		last_type = int32;
	}
	return code;
}

/*
Code emitIf_OLD(Node &node) {
	Code code;
//	case ifStatement:
	// if block
	code.addByte(block);
	code.addByte(void_block);
	// compute the if expression
	Node *condition = node[0].value.node;
//	Node *condition = node["condition"];//.value.node->clone();
	emitExpression(condition,context);
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

	if (node.length == 3) {

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
*/

Code Call(char *symbol) {//},Node* args=0) {
	Code code;
	code.addByte(function);
	int i = functionIndices.position(symbol);
	if (i < 0)error("UNKNOWN symbol "s + symbol);
//	code.opcode(unsignedLEB128(i),8);
	code.addByte(i);
	return Code();
}

Code encodeString(char *str) {
	size_t len = strlen0(str);
	Code code = Code(len, (bytes) str, len);
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


Code emitBlock(Node node, String context) {
//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN FUNCTION!!!
//	char code_data[] = {0/*locals_count*/,i32_const,42,call,0 /*logi*/,i32_auto,21,return_block,end_block};
// 0x00 == unreachable as block header !?
//	char code_data[] = {0/*locals_count*/,i32_auto,21,return_block,end_block};// 0x00 == unreachable as block header !?
//	Code(code_data,sizeof(code_data)); // todo : memcopy, else stack value is LOST
	Code block;
//	Map<int, String>
//	collect_locals(node, context);// DONE IN analyze
//	int locals_count = current_local_names.size();
//	locals[context] = current_local_names;

//	todo : ALWAYS MAKE RESULT VARIABLE FIRST IN BLOCK (index 0, used after while(){} etc) !!!
// todo: locals must ALWAYS be collected in analyze step, emitExpression is too late!
	last_local = 0;
	int locals_count = locals[context].size();
	trace("found %d locals for %s"s % locals_count % context);
	log(locals[context]);
	int argument_count = functionSignatures[context].size();
	if (locals_count >= argument_count)
		locals_count = locals_count - argument_count;
	else
		warn("locals consumed by arguments"); // ok in  double := it * 2; => double(it){it*2}
	block.addByte(locals_count);
	for (int i = 0; i < locals_count; ++i) {
		Valtype valtype = localTypes[context][i];
		block.addByte(i + 1);// index
		if (valtype == none or valtype == voids or valtype == charp or valtype == array)
			valtype = int32;
		block.addByte(valtype);
	}
	// todo block.addByte(i+1) // index seems to be wrong: i==NUMBER of locals of type xyz ??
//	013b74: 01 7f                      | local[0] type=i32
//	013b76: 02 7f                      | local[1..2] type=i32
//	013b78: 03 7f                      | local[3..5] type=i32

	Code inner_code_data = emitExpression(node, context);
	Valtype x = last_type;
	block.push(inner_code_data);
	Valtype returns = functionSignatures[context].return_type;// switch back to return_types[context] for block?
	if (returns != last_type) {
		if (returns == Valtype::voids and last_type != Valtype::voids)
			block.addByte(drop);
		if (returns == Valtype::i32t and last_type == Valtype::f32t)
			block.addByte(i32_trunc_f32_s);
		if (returns == Valtype::i32t and last_type == Valtype::voids)
			block.addByte(i32_const).addInt(0);//-999);// hack? return 0/false by default. ok? see python!
//		if(returns==Valtype::f32)…
	}

//if not return_block
	block.addByte(return_block);
	block.addByte(end_block);
	return block;
}

/*
//Map<int, String>
// todo: why can't they be all found in parser? 2021/9/4 : they can ;)
List<String> collect_locals(Node node, String context) {
	List<String> &current_locals = locals[context]; // no, because there might be gaps(unnamed locals!)
	for (Node n : node) {
		Valtype type = mapTypeToWasm(n);
		if (type != none and not current_locals.has(n.name)) {
			current_locals.add(n.name);
			localTypes[context].add(type);
		}
	}
	return current_locals;
}
*/


int last_index = -1;

Code typeSection() {
	// Function types are vectors of parameters and return types. Currently
	// TODO optimise - some of the procs might have the same type signature
	// the type section is a vector of function types
	int typeCount = 0;
	Code type_data;
//	log(functionIndices);
	for (String fun : functionSignatures) {
		if (!fun) {
//			log(functionIndices);
//			log(functionSignatures);
			breakpoint_helper
			warn("empty functionSignatures[ø] because context=start=''");
//			error("empty function creep functionSignatures[ø]");
			continue;
		}
		Signature &signature = functionSignatures[fun];
		if (not signature.emit /*export/declarations*/ and not signature.is_used /*imports*/) {
			trace("not signature.emit => skipping unused type for "s + fun);
			continue;
		}
		if (signature.is_runtime)
			continue;
		if (signature.is_handled)
			continue;
//		if(signature.is_import) // types in import section!
//			continue;
		if (not functionIndices.has(fun))
			functionIndices[fun] = ++last_index;
//			error("function %s should be registered in functionIndices by now"s % fun);

		typeMap[fun] = runtime.type_count /* lib offset */ + typeCount++;
		signature.is_handled = true;
		int param_count = signature.size();
//		Code td = {0x60 /*const type form*/, param_count};
		Code td = Code(0x60) + Code(param_count);

		for (int i = 0; i < param_count; ++i) {
			td = td + Code(fixValtype(signature.types[i]));
		}
		Valtype &ret = functionSignatures[fun].return_type;
		if (ret == voids) {
			td.addByte(0);
		} else {
			td.addByte(1/*return count*/).addByte(fixValtype(ret));
		}
		type_data = type_data + td;
	}
	return Code((char) type_section, encodeVector(Code(typeCount) + type_data)).clone();
}

Valtype fixValtype(Valtype &valtype) {
	if (valtype == charp) return int32;
	if (valtype > 0xC0)error("exposed internal Valtype");
	return valtype;
}

Code importSection() {
	if (runtime_offset) {
//		breakpoint_helper
//		printf("imports currently not supported\n");
		import_count = 0;
		return Code();
	}
	// the import section is a vector of imported functions
	Code imports;
	import_count = 0;
	for (String fun : functionSignatures) {
		Signature &signature = functionSignatures[fun];
		if (signature.is_used and not signature.is_builtin) {
			++import_count;
			imports = imports + encodeString("env") + encodeString(fun).addByte(func_export).addType(typeMap[fun]);
		}
	}

//	if (functionSignatures["logi"].is_used and ++import_count)
//		imports = imports + encodeString("env") + encodeString("logi").addByte(func_export).addType(typeMap["logi"]);
//	if (functionSignatures["logs"].is_used and ++import_count)
//		imports = imports + encodeString("env") + encodeString("logs").addByte(func_export).addType(typeMap["logs"]);
//	if (functionSignatures["logf"].is_used and ++import_count)
//		imports = imports + encodeString("env") + encodeString("logf").addByte(func_export).addType(typeMap["logf"]);
//	if (functionSignatures["square"].is_used and ++import_count)
//		imports =
//				imports + encodeString("env") + encodeString("square").addByte(func_export).addType(typeMap["square"]);
	if (memoryHandling == import_memory) {
		imports = imports + encodeString("env") + encodeString("memory") + (byte) mem_export/*type*/+ (byte) 0x00 +
		          (byte) 0x01;;
	}
	if (imports.length == 0)return Code();
	auto importSection = createSection(import_section, Code(import_count) + imports);// + sqrt_ii
	return importSection.clone();
}

//int function_count;// misleading name: declared functions minus import_count  (together : functionIndices.size() )
//int new_count;
int function_block_count;

//int builtins_used=0;
Code codeSection(Node root) {
	// the code section contains vectors of functions
	// index needs to be known before emitting code, so call $i works
	int new_count;

	new_count = declaredFunctions.size();
	for (auto declared : declaredFunctions) {
		print("declared function: "s + declared);
		if (!declared)error("empty function name (how?)");
		if (not functionIndices.has(declared))// used or not!
			functionIndices[declared] = ++last_index;// functionIndices.size();
	}
//	int index_size = functionIndices.size();
//	bool has_main = start and (declaredFunctions.has(start) or functionIndices.has(start));
//	if (import_count + builtin_count + has_main + new_count + runtime_offset != index_size) {
//		log(functionIndices);
//		String message = "inconsistent function_count %d import + %d builtin + %d new + %d runtime + %d main != %d"s;
// 		error(message % import_count % builtin_count % new_count % runtime_offset % has_main % index_size);
//	}
// https://pengowray.github.io/wasm-ops/
//	char code_data[] = {0x01,0x05,0x00,0x41,0x2A,0x0F,0x0B};// 0x41==i32_auto  0x2A==42 0x0F==return 0x0B=='end (function block)' opcode @+39
	byte code_data_fourty2[] = {0/*locals_count*/, i32_auto, 42, return_block, end_block};
	byte code_data_nop[] = {0/*locals_count*/, end_block};// NOP
	byte code_data_id[] = {1/*locals_count*/, 1/*WTF? first local has type: */, i32t, get_local, 0, return_block,
	                       end_block}; // NOP
//	byte code_data_logi_21[] = {0/*locals_count*/,i32_const,48,function,0 /*logi*/,i32_auto,21,return_block,end_block};
//	byte code_data[] = {0x00, 0x41, 0x2A, 0x0F, 0x0B,0x01, 0x05, 0x00, 0x41, 0x2A, 0x0F, 0x0B};
	Code code_blocks;
	if (runtime.code_count == 0) {
		// order matters, in functionType section!
		if (functionSignatures["nop"].is_used)
			code_blocks = code_blocks + encodeVector(Code(code_data_nop, sizeof(code_data_nop)));
		if (functionSignatures["id"].is_used)
			code_blocks = code_blocks + encodeVector(Code(code_data_id, sizeof(code_data_id)));
	}

	Code main_block = emitBlock(root, start);// after imports and builtins

	if (start) {
		code_blocks = code_blocks + encodeVector(main_block);
	} else {
		if (main_block.length > 5)
			error("no start function name given. null instead of 'main', can't assign block");
		else warn("no start block (ok)");
	}
	for (String fun : functionCodes) {// MAIN block extra ^^^
		Code &func = functionCodes[fun];
		code_blocks = code_blocks + encodeVector(func);
	}
	builtin_count = 0;
	if (functionSignatures["nop"].is_used) builtin_count++;// used
	if (functionSignatures["id"].is_used) builtin_count++;// used

	bool has_main = start and functionIndices.has(start);
	int function_codes = functionCodes.size();
	function_block_count = has_main /*main*/ + builtin_count + function_codes;
	auto codeSection = createSection(code_section, Code(function_block_count) + code_blocks);
	return codeSection.clone();
}


Code exportSection() {
	short exports_count = 1;// main
// the export section is a vector of exported functions etc
	if (!start)// todo : allow arbirtrary exports, or export all
		return createSection(export_section, Code(0));
	int main_offset = 0;
	if (functionIndices.has(start))
		main_offset = functionIndices[start];
	Code memoryExport;// empty by default
	if (memoryHandling == export_memory) {
		exports_count++;
		memoryExport = encodeString("memory") + (byte) mem_export + Code(0);
//code = code + createSection(export_section, encodeVector(Code(exports_count) + memoryExport));
	}
	Code exportsData = encodeVector(
			Code(exports_count) + encodeString(start) + (byte) func_export + Code(main_offset) + memoryExport);

	auto exportSection = createSection(export_section, exportsData);
	return exportSection;
}

Code dataSection() { // needs memory section too!
//	Contents of section Data:
//	0000042: 0100 4100 0b02 4869                      ..A...Hi
//https://webassembly.github.io/spec/core/syntax/modules.html#syntax-datamode
	Code datas;
	if (data_index_end == 0)return datas;//empty

	//Contents of section Data:
//0013b83: 0005 00 41 8108 0b fd1d 63 6f72 7275 7074  ...A.....corrupt
//	datas.addByte(00); WHY does module have extra 0x00 and 5 data sections???
//	datas.addByte(00); // it seems heading 0's get jumped over until #segments >0 :
	datas.addByte(01);// one memory initialization / data segment
	datas.addByte(00);// memory id always 0

	datas.addByte(0x41);// opcode for i32.const offset: followed by unsignedLEB128 value:
	datas.addInt(0x0 +
	             runtime.data_segments.length);// actual offset in memory todo: WHY cant it start at 0? wx  todo: module offset + module data length
	datas.addByte(0x0b);// mode: active?
	datas.addByte(data_index_end); // size of data
	const Code &actual_data = Code((bytes) data, data_index_end);
	datas.add(actual_data);// now comes the actual data  encodeVector()? nah manual here!
	return createSection(data_section, encodeVector(datas));// size added via actual_data
}

// Signatures
Code funcTypeSection() {// depends on codeSection, but must appear earlier in wasm
	// funcType_count = function_count EXCLUDING imports, they encode their type inline!
	// the function section is a vector of type indices that indicate the type of each function in the code section

	Code types_of_functions = Code(function_block_count);//  = Code(types_data, sizeof(types_data));
//	order matters in functionType section! must be same as in functionIndices
	for (int i = 0; i < function_block_count; ++i) {
		//	import section types separate WTF wasm
		int index = i + import_count + runtime_offset;
		String *fun = functionIndices.lookup(index);
		if (!fun) {
			log(functionIndices);
			error("missing typeMap for index "s + index);
		} else {
			int typeIndex = typeMap[*fun];
			if (typeIndex < 0) {
				if (runtime_offset == 0) // todo else ASSUME all handled correctly before
					error("missing typeMap for function %s index %d "s % fun % i);
			} else
				types_of_functions.push((byte) typeIndex);
		}
	}
	// @ WASM : WHY DIDN'T YOU JUST ADD THIS AS A FIELD IN THE FUNC STRUCT???
	Code funcSection = createSection(functypes_section, types_of_functions);
	return funcSection.clone();
}

Code functionSection() {
	return funcTypeSection();// (misnomer) vs codeSection() !
}

// todo : convert library referenceIndices to named imports!
Code nameSection() {
	Code nameMap;

	int total_func_count = last_index + 1;// functionIndices.size();// imports + function_count, all receive names
	int usedNames = 0;
	for (int index = runtime_offset; index < total_func_count; index++) {
		// danger: utf names are NOT translated to wat env.√=√ =>  (import "env" "\e2\88\9a" (func $___ (type 3)))
		String *name = functionIndices.lookup(index);
		if (functionSignatures[*name].is_import)continue;
		nameMap = nameMap + Code(index) + Code(*name);
		usedNames += 1;
	}

//	auto functionNames = Code(function_names) + encodeVector(Code(1) + Code((byte) 0) + Code("logi"));
//	functions without parameters need  entry ( 00 01 00 00 )
//  functions 5 with local 'hello' :  05 01 00 05 68 65 6c 6c 6f
//  functions 5 with local 'hello' :  05 02 00 05 68 65 6c 6c 6f 01 00 AND unnamed (local i32t)
// localMapEntry = (index nrLocals 00? string )

	Code localNameMap;
	int usedLocals = 0;
	for (int index = runtime_offset; index <= last_index; index++) {
		String *key = functionIndices.lookup(index);
		if (!key or key->empty())continue;
		List<String> localNames = locals[*key];// including arguments
		int local_count = localNames.size();
		if (local_count == 0)continue;
		usedLocals++;
		localNameMap = localNameMap + Code(index) + Code(local_count); /*???*/
		for (int i = 0; i < localNames.size(); ++i) {
			String local_name = localNames[i];
			localNameMap = localNameMap + Code(i) + Code(local_name);
		}
//		error: expected local name count (1) <= local count (0) FOR FUNCTION ...
	}

//	localNameMap = localNameMap + Code((byte) 0) + Code((byte) 1) + Code((byte) 0) + Code((byte) 0);// 1 unnamed local
//	localNameMap = localNameMap + Code((byte) 4) + Code((byte) 0);// no locals for function4
//	Code exampleNames= Code((byte) 5) /*function index*/ + Code((byte) 1) /*count*/ + Code((byte) 0) /*l.nr*/ + Code("var1");
	// function 5 with one local : var1

//	localNameMap = localNameMap + exampleNames;

	auto moduleName = Code(module_name) + encodeVector(Code("wasp_module"));
	auto functionNames = Code(function_names) + encodeVector(Code(usedNames) + nameMap);
	auto localNames = Code(local_names) + encodeVector(Code(usedLocals) + localNameMap);

//	The name section is a custom section whose name string is itself ‘𝚗𝚊𝚖𝚎’.
//	The name section should appear only once in a module, and only after the data section.
	const Code &nameSectionData = encodeVector(Code("name") + moduleName + functionNames + localNames);
	auto nameSection = createSection(custom_section, nameSectionData); // auto encodeVector AGAIN!
	nameSection.debug();
	return nameSection.clone();
}

//Code dataSection() {
//	return Code();
//}

Code eventSection() {
	return Code();
}

/*
 *  There are currently two ways in which function indices are stored in the code section:
    Immediate argument of the call instruction (calling a function)
    Immediate argument of the i32.const instruction (taking the address of a function).
    The immediate argument of all such instructions are stored as padded LEB128 such that they can be rewritten
    without altering the size of the code section. !
    For each such instruction a R_WASM_FUNCTION_INDEX_LEB or R_WASM_TABLE_INDEX_SLEB reloc entry is generated
    pointing to the offset of the immediate within the code section.

    R_WASM_FUNCTION_INDEX_LEB relocations may fail to be processed, in which case linking fails.
    This occurs if there is a weakly-undefined function symbol, in which case there is no legal value that can be
    written as the target of any call instruction. The frontend must generate calls to undefined weak symbols
    via a call_indirect instruction.
*/
Code linkingSection() {
//	https://github.com/WebAssembly/tool-conventions/blob/master/Linking.md#linking-metadata-section
	short version = 2;
	Code subsection;
	short type = 5;// SEGMENT alignment & flags
	short payload_len = 0;
	Code payload_data;
	subsection.add(type).add(payload_len).push(payload_data);
	Code subsections;
/*
	5 / WASM_SEGMENT_INFO - Extra metadata about the data segments.
	6 / WASM_INIT_FUNCS - Specifies a list of constructor functions to be called at startup. Called after memory has been initialized.
	7 / WASM_COMDAT_INFO - Specifies the COMDAT groups of associated linking objects, which are linked only once and all together.
	8 / WASM_SYMBOL_TABLE - Specifies extra information about the symbols present in the module.
// https://en.wikipedia.org/wiki/Relocatable_Object_Module_Format
// http://wiki.dwarfstd.org/index.php?title=COMDAT_Type_Sections
	COMDAT - (C2h/C3h) Initialized common data
	COMDEF - (B0h) Uninitialized common data
 Thread-agnostic objects can be safely linked with objects that do or do not use atomics, although not both at the same time.
*/
	return createSection(custom_section, encodeVector(Code("linking") + Code(version) + subsections));
}

Code dwarfSection() {
	return createSection(custom_section, encodeVector(Code("external_debug_info") + Code("main.dwarf")));
}

/*
 * The generally accepted features are:
    atomics
    bulk-memory
    exception-handling
    multivalue
    mutable-globals
    nontrapping-fptoint
    sign-ext
    simd128
    tail-call
 */

// see preRegisterSignatures
void add_builtins() {
	import_count = 0;
	builtin_count = 0;
	for (auto sig : functionSignatures) {// imports first
		Signature &signature = functionSignatures[sig];
		if (signature.is_import and signature.is_used) {
			functionIndices[sig] = ++last_index;// functionIndices.size();
			import_count++;
		}
	}
	for (auto sig : functionSignatures) {// now builtins
		if (functionSignatures[sig].is_builtin and functionSignatures[sig].is_used) {
			functionIndices[sig] = ++last_index;// functionIndices.size();
			builtin_count++;
		}
	}
}

Code memorySection() {
	if (memoryHandling == import_memory or memoryHandling == no_memory) return Code();// handled elsewhere

	/* limits https://webassembly.github.io/spec/core/binary/types.html#limits - indicates a min memory size of one page */
	int pages = 1;//54kb each
	auto code = createSection(memory_section, encodeVector(Code(1) + Code(0x00) + Code(pages)));
	return code;
}


Code &emit(Node root_ast, Module *runtime0, String _start) {
	if (root_ast.kind == objects)root_ast.kind = expressions;
	start = _start;
	typeMap.setDefault(-1);
	typeMap.clear();
	data = (char *) malloc(MAX_DATA_LENGTH);
	data_index_end = 0;
	functionIndices.setDefault(-1);
	functionCodes.setDefault(Code());
	if (runtime0) {
		memoryHandling = no_memory;// done by runtime?
		runtime = *runtime0;// else filled with 0's
		runtime_offset = runtime.import_count + runtime.code_count;//  functionIndices.size();
		import_count = 0;
		builtin_count = 0;
//		data_index_end = runtime.data_segments.length;// insert after module data!
		// todo: either write data_index_end DIRECTLY after module data and increase count of module data,
		// or increase memory offset for second data section! (AND increase index nontheless?)
		int newly_pre_registered = 0;//declaredFunctions.size();
		last_index = runtime_offset - 1;
	} else {
		last_index = -1;
		runtime = *new Module();// all zero
		runtime_offset = 0;
		typeMap.clear();
		functionIndices.clear();// ok preregistered functions are in functionSignatures
		add_builtins();
	}
	if (start) {// now AFTER imports and builtins
//		printf("start: %s\n", start.data);
		functionSignatures[start] = Signature().returns(i32t);
		functionSignatures[start].emit = true;
		if (!functionIndices.has(start))
			functionIndices[start] = ++last_index;
//			functionIndices[start] =runtime_offset ? runtime_offset + declaredFunctions.size() :  ++last_index;// functionIndices.size();  // AFTER collecting imports!!
		else
			error("start already declared: "s + start + " with index " + functionIndices[start]);
	} else {
//		functionSignatures["_default_context_"] = Signature();
//		start = "_default_context_";//_default_context_
//		start = "";
	}

	functionCodes.clear();
	locals.setDefault(List<String>());

	const Code &customSectionvector = encodeVector(Code("custom123") + Code("random custom section data"));
	auto customSection = createSection(custom_section, customSectionvector);
	Code typeSection1 = typeSection();// types can be defined in analyze(), not in code declaration
	Code importSection1 = importSection();// needs type indices
	Code codeSection1 = codeSection(root_ast);
	Code funcTypeSection1 = funcTypeSection();// signatures depends on codeSection, but must come before it in wasm
	Code memorySection1 = memorySection();
	Code exportSection1 = exportSection();// depends on codeSection, but must come before it!!
	Code code = Code(magicModuleHeader, 4)
	            + Code(moduleVersion, 4)
	            + typeSection1
	            + importSection1
	            + funcTypeSection1 // signatures
	            + memorySection1 // Wasm MVP can only define one memory per module WHERE?
	            + exportSection1
	            + codeSection1 // depends on importSection, yields data for funcTypeSection!
	            + dataSection()
	            //			+ linkingSection()
	            + nameSection()
//	 + dwarfSection() // https://yurydelendik.github.io/webassembly-dwarf/
//	 + customSection
	;
	code.debug();
	free(data);// written to wasm code ok
	if (runtime0)functionSignatures.clear(); // cleanup after NAJA
	return code.clone();
}