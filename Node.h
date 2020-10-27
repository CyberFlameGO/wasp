#pragma once
//
// Created by pannous on 30.09.19.
//

#include "String.h"
//#import  "String.h" // FFS
#include <stdarg.h> // va_list OK IN WASM???
typedef char const * chars;
typedef unsigned char byte;//!

#define min(a,b) (a < b ? a : b)

extern bool debug;
extern bool throwing;
extern bool polish_notation;

extern unsigned int *memory;

#define let auto
#define var auto
typedef void *any;
typedef unsigned char byte;
typedef const char *chars;

//extern "C" int isalnum(int _c);

#ifdef WASM
#warning COMPILING TO WASM
#else
#warning COMPILING TO APPLE
#endif

typedef unsigned long size_t;


class Node;

extern Node True;
extern Node False;
extern Node NIL;
extern Node ERROR;
extern Node Infinity;// = Node("Infinity");
extern Node NaN;// = Node("NaN");

void log(Node &);
void log(Node*);
void printf(Node&);

//class String;
union Value {
//	Node node;//  incomplete type
	Node *node = 0;// todo DANGER, can be lost :( !!
//	Node **children = 0;
	String string;
	void *data;
	long number;
//	float floaty;
	double floaty;

	Value() {}// = default;
	Value(int i) {
		number = i;
	}
	Value(bool b) {
		number = 1;
	}

//	~Value() = default;
};


class Node {
public:
	String name = empty_name;// nil_name;
	Value value;
	Type kind = unknown;
	int length = 0;// children
//	int count = 0;// use param.length for arguments / param

	// TODO REFERENCES can never be changed. which is exactly what we want, so use these AT CONSTRUCTION:
//	Node &parent=NIL;
//	Node &param=NIL;
	Node *parent = nullptr;

	// a{b}(c)[d] == a{body=b}->c->d // param deep chain, attention in algorithms
	Node *param = nullptr;// LINK, not list. attributes meta modifiers decorators annotations
	Node *children = nullptr;// LIST, not link. block body content

	/* don't mix children with param, see for(i in xs) vs for(i of xs) hasOwnProperty, getOwnPropertyNames
	 * conceptual cleanup needed... => DONE?
	 * children["_body_"] => children / $html.body
	 * children["_attrib_"] => params
	 * children["_head_"] => params / params.head / $html.head
	 * children["_meta_"] => params.meta
	 * children["_name"] == name
	 *
	 * */
//	Node *next = nullptr;// NO, WE NEED TRIPLES cause objects can occur in many lists + danger: don't use for any ref/var


	//	Node(const char*);
//	Node(va_list args) {
//	}
	void* operator new(unsigned long size){
		return static_cast<Node *>(calloc(sizeof(Node),size));// WOW THAT WORKS!!!
	}
	void operator delete (void* a){
		printf("DELETING");
	}
//	~Node()= default; // destructor
//	virtual ~Node() = default;


	Node() {
		kind = objects;
//		if(debug)name = "[]";
	}

	Node& first(){
		if (length>0)return children[0];
		error("No such element");
		return ERROR;
	}

//	if you do not declare a copy constructor, the compiler gives you one implicitly.
//	Node( Node& other ){// copy constructor!!


	Node *clone() {// const cast?
		if (this == &NIL)return this;
		if (this == &True)return this;
		if (this == &False)return this;
		// todo ...
		Node *copy = new Node();
		*copy = *this;// copy value ok
		return copy;
	}


	explicit Node(int buffer[]) {
		value.data = buffer;
		kind = buffers;
//		todo ("type of array");
//		if (debug)name = "int[]";
//			buffer.encoding = "a85";
	}

	explicit Node(char c) {
		name = String(c);
		value.string = String(c);
		kind = strings;
	}

	explicit Node(double nr) {
		value.floaty = nr;
		kind = floats;
		if (debug)name = String(itoa0(nr,10)); // messes with setField contraction
	}


	explicit Node(float nr) {
		value.floaty = nr;
		kind = floats;
		if (debug)name = String((long) nr) + String(".…");//#+"#"; // messes with setField contraction
	}

// how to find how many no. of arguments actually passed to the function? YOU CAN'T! So …
// Pass the number of arguments as the first variable
// Require the last variable argument to be null, zero or whatever
	explicit Node(int a, int b, ...) {
		kind = objects;// groups list
		add(Node(a).clone());
#ifndef WASM
		va_list args;
		va_start(args, b);
		int i = b;
		while (i){
			add(Node(i).clone());
			i = (int) va_arg(args, int);
		}
		va_end(args);
#endif
	}

	// why not auto null terminated on mac?
	// vargs needs to be 0 terminated, otherwise pray!
	explicit Node(char *a, char *b, ...) {
		kind = objects;// groups list
		add(Node(a).clone(),false);
#ifndef WASM
		va_list args;
		va_start(args, b);
		char *i = b;
		while (i){
			Node *node = Node(i).clone();
			add(node);
			i = (char *) va_arg(args, char*);
		}
		va_end(args);
#endif
	}


//	explicit
	Node(long nr) {
		value.number = nr;
		kind = longs;
		if (debug)name = String(itoa(nr)); // messes with setField contraction
	}
	explicit Node(int nr) {
		value.number = nr;
		kind = longs;
		if (debug)name = String(itoa(nr)); // messes with setField contraction
	}

	explicit Node(const char *name) {
		this->name = String(name);
//		type = strings NAH;// unless otherwise specified!
	}
//
//	explicit Node(bool truth) {
//		error("DONT USE CONSTRUCTION, USE ok?True:False");
//		if (this == &NIL)
//			name = "HOW";
//		value.number = truth;
//		type = numbers;
//		if (debug)name = truth ? "✔️" : "✖️";
////		if (debug)name = nr ? "true" : "false";
////		this=True; todo
//	}


	explicit Node(String s, bool identifier = false) {
//		identifier = identifier || !s.contains(" "); BULLSHIT 'hi' is strings!!
		if (identifier) {
//			if(check_reference and not symbol)...
			name = s;
			kind = reference;
		}
//		else if (atoi(s) and s == itoa0(atoi(s))) {
//			value.number = atoi(s);
//			type = numbers;
//			}
//		else if (atof(s)) { value.floaty = atoi(s); }
		else {
			kind = strings;
			value.string = s;
			if (name == empty_name)name = s;
		}
	}

	explicit Node(Node **pNode) {
		children = pNode[0];
		kind = arrays;
		value.data = pNode[0];
	}

	explicit
	operator bool() {
		return value.number or length > 1 or (length == 1 and this != children and (bool) (children[0]));
	}

	bool operator==(int other);

	bool operator==(long other);

	bool operator==(float other);

	bool operator==(double other);

	bool operator==(String other);

//	bool operator==(Node other);
	bool operator==(Node &other);// equals

	bool operator!=(Node other);

	Node operator+(Node other);

//	bool operator!=(Node &other);// why not auto

//	 +=, -=, *=, /=, %=, <<=, >>=, &=, |=, ^=
//	bool operator<<=(Node &other);// EVIL MAGIC ;)
//	bool operator~();// MUST BE UNITARY:(


	String string() const {
		if (kind == strings)
			return value.string;
		return name;
		breakpoint_helper
				err(String("WRONG TYPE ") + typeName(kind));
	}

	// moved outside because circular dependency
	Node &operator[](int i);
	Node &operator[](int i) const;

	Node &operator[](char c);

	Node &operator[](String s);
	Node &operator[](String s) const;

	Node &operator=(int i);

	Node &operator=(chars s);

	Node &set(String string, Node *node);

	Node evaluate(bool expectOperator=false);

	Node insert(Node &node, int at = -1);// non-modifying

	void add(Node node);// modifying

//	void add(Node &node);

	void add(Node *node, bool flatten=true);

	void remove(Node *node); // directly from children
	void remove(Node &node); // via compare children

	[[nodiscard]] Node *begin() const;

	[[nodiscard]] Node *end() const;

	Node merge(Node &other);// non-modifying

	void log() {
		printf("Node ");
		if (this == &NIL || kind == nils) {
			printf("NIL\n");
			return;
		}
//		if || name==nil_name …
#ifndef WASM
#ifndef WASI
		if (name.data < (char *) 0xffff) {
			printf("BUG");
			return;
		}
		if (name and name.data and name.data > (char *) 0xffff and kind != objects)
#endif
#endif
		printf("name:"_s + name);
		printf(" length:"_s + itoa(length));
		printf(" type:"_s + typeName(kind));
		printf(" value:"_s + serializeValue());
//			printf("name:%s ", name.data);
//		printf("length:%i ", length);
//		printf("type:%s ", typeName(type).data);
//		printf("value:%s ", serializeValue());
//		if (this == &True)
//			printf("TRUE");
//		if (this == &False)
//			printf("FALSE");
//		if (type == objects and value.data)
//			printf(" value.name %s", value.string.data);// ???
//		if (type == bools)
//			printf(" value %s", value.number ? "TRUE" : "FALSE");
//		if (type == strings)
//			printf(" value %s", value.string.data);
//		if (type == numbers)
//			printf(" value %li", value.number);
//		if (type == floats)
//			printf(" value %f", value.floaty);
		printf(" [");
		for (int i = 0; i < min(length,10); i++) {
			Node &node = children[i];
//			if(check(node))
			printf(node.name);
			printf(" ");
		}
		printf("]");
		printf("\n");
	}

	static float precedence(Node &operater);

	static Node apply(Node left, Node op0, Node right);

	Node &setType(Type type);

	long numbere() {
		return kind == longs or kind == bools ? value.number : value.floaty;// danger
	}

	float floate() {
		return kind == longs ? value.number : value.floaty;// danger
	}

	Node *has(String s, bool searchParams = true) const;

// type conversions
	explicit operator bool() const { return value.number; }

	explicit operator int() const { return value.number; }

	explicit operator long() const { return value.number; }

	explicit operator float() const { return value.floaty; }

	explicit operator String() const { return value.string; }

	explicit operator char *() const { return value.string.data; }// or name()
	Node &last();

	bool empty();// same:
	bool isEmpty();

	bool isNil();

	const char *toString();
	const char *toString() const;

	const char * serialize() const;

	const char *serializeValue() const;

	void print();

	Node setValue(Value v);
};
typedef const Node Nodec;
