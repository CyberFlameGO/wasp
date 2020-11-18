//
// Created by pannous on 18.05.20.
//
#define _main_

#include "Wasp.h"
#include "Angle.h"
#import "WasmHelpers.h" // IMPORT so that they don't get mangled!
#include "Node.h"
#include "wasm-emitter.h"
#include "math.h" // sqrt

bool recursive = true;// whats that?



String functor_list[] = {"if", "while", 0};// MUST END WITH 0, else BUG

// functions group externally square 1 + 2 == square(1 + 2) VS √4+5=√(4)+5
String function_list[] = {"square", "log", "puts", "print", "printf", "println", "logi", "logf", "log_f32", "logi64",
                          "logx", "logc", "id", 0};// MUST END WITH 0, else BUG
String control_flows[] = {"if", "while", "unless", "until", "as soon as", 0};

int main4(int argp, char **argv) {
#ifdef register_global_signal_exception_handler
	register_global_signal_exception_handler();
#endif
	try {
		error("HhhU");
//		testCurrent();
		return 42;
	} catch (chars err) {
		printf("\nERROR\n");
		printf("%s", err);
	} catch (String err) {
		printf("\nERROR\n");
		printf("%s", err.data);
	} catch (SyntaxError *err) {
		printf("\nERROR\n");
		printf("%s", err->data);
	}
	return -1;
}

int start() { // for wasm-ld
	main4(0, 0);
}

int _start() { // for wasm-ld
	main4(0, 0);
}


Node &Node::setType(Type type) {
	if (value.data and (type == groups or type == objects))
		return *this;
	if (kind == nils and not value.data)
		return *this;
	this->kind = type;
//	if(name.empty() and debug){
//		if(type==objects)name = object_name;
//		if(type==groups)name = groups_name;
//		if(type==patterns)name = patterns_name;
//	}
	return *this;
}

Node constants(Node n) {
	if (eq(n.name, "not"))return True;// not () == True; hack for missing param todo: careful!
	if (eq(n.name, "one"))return Node(1);
	if (eq(n.name, "two"))return Node(2);
	if (eq(n.name, "three"))return Node(3);
	return n;
}

Node eval(Node n) {
	return n.evaluate();
}

Node If(Node condition, Node then) {
	Node ok0 = condition.evaluate();
	bool ok = (bool) ok0;
	if (condition.name == "0")ok = false;// hack
	if (ok)return then.evaluate();
	else return False;
}

Node If(Node n) {
	if (n.length == 0)return If(n, n.values());
	breakpoint_helper
	if (n.length == 0 and !n.value.data)
		error("no if condition given");
	if (n.length == 1 and !n.value.data)
		error("no if block given");
	Node &condition = n.children[0];
	Node then = n[1];
	if (n.has("then")) {
		condition = n.to("then");
		then = n.from("then");
	}

	if (condition.value.data and !condition.next)
		then = condition.values();
	if (condition.next and condition.next->name == "else")
		then = condition.values();

	// todo: UNMESS how?
	if (n.has(":") /*before else: */) {
		condition = n.to(":");
		if (condition.has("else"))
			condition = condition.to("else");// shouldn't happen?
		then = n.from(":");
	} else if (condition.has(":")) {// as child
		then = condition.from(":");
		condition = condition.evaluate();
	}
	if (then.has("then"))
		then = n.from("then");
	if (then.has("else"))
		then = then.to("else");
//	if(condition.name=="condition")
//		condition = condition.values();

	Node condit = condition.evaluate();
	bool condition_fulfilled = (bool) condit;
	if (condition.kind == reals or condition.kind == longs)
		condition_fulfilled = condition.name.empty() and condition.value.data or condition.name != "0";
	else if (condition.value.data and condition.kind == objects) // or ...
		error("If statements need a space after colon");
	if (condition_fulfilled) {
		if (then.name == "then") {
			if (then.value.data or then.children) // then={} as arg
				return eval(then.values());
			return eval(n[2]);
		}
		return eval(then);
	} else {
		if (n.has("else"))
			return eval(n.from("else"));
		if (n.length == 3 and not n.has(":"))
			return eval(n[2]);// else
		else
			return False;
	}
}

bool isFunction(String op) {
	return op.in(function_list);
}


Node Node::evaluate(bool expectOperator /* = true*/) {
	if (length == 0)return constants(*this);
	if (length == 1) {
		if (kind == operators or isFunction(name))
			return apply_op(NIL, *this, *children);
		else return constants(children[0]);
	}

	if (length > 1)
		if (kind == operators or precedence(*this))
			return apply_op(NIL, *this, this->clone()->setType(objects).setName(empty_name));

	if (length == 2 and children[1].kind == expressions) {
		length = 1;
		return this->merge(children[1]).evaluate();// stop hacking
	}
//	if (type != expression and type != keyNode)
//		return *this;
	float max = 0; // do{
	float min = 999999;
	Node right;
	Node left;
	Node unknown_symbols;
	for (Node &node : *this) {// foreach
		float p = precedence(node);
		if (p > max) max = p;
		if (p < min and p != 0) min = p;
		if (p == 0 and node.kind == reference)unknown_symbols.add(node);
	}
	if (max == 0) {
		if (!name.empty() or length > 1) {
			breakpoint_helper // ok need not always have known operators
			info(String("No operator in : ") + serialize());
			if (unknown_symbols > (long) 0 and expectOperator)
				error("unknown symbol "s + unknown_symbols.serialize());
		}
		for (int i = 0; i < length; ++i) {
			Node child = children[i];
			Node evaled = child.evaluate();
			children[i] = evaled;
		}
		return *this;
	}
	Node *op = 0;
	Node *inner = 0;// NIL;
	Node *prev = 0;
	for (Node &n : *this) {
		float p = precedence(n);
		if (p == min and not inner) inner = &n;
		else if (not inner) prev = &n;
		if (p == max and not op) {
			op = &n;
		} else if (op)
			right.addRaw(n);
		else
			left.addRaw(n);
	}
//	const Node &arg = inner.next ? *inner.next : NIL;
//	replace ... apply_op(prev, inner, arg);

	if (op->children and right.empty()) {
		right = op->children[0];
	}// DONT work around bugs like this!!
//		remove(&op);// fucks up pointers?
	if (recursive and op) {
		right = right.flat();
		Node left1 = left.flat();
		Node result = apply_op(left1, *op, right);// <<<<<<<<<
		if (isFunction(op->name) and not left.empty()) {
			result = left.addRaw(result).evaluate();
		}
		return result;
	}
//	};// while (max > 0);
	return *this;
}

bool interpret = true;

Node eval(String code) {
	if (interpret)
		return parse(code).evaluate();
	else
		return emit(analyze(parse(code))).run();// int -> Node todo: int* -> Node*
}

Node groupIf(Node n);


// if a then b else c == a and b or c
// (a op c) => op(a c)
// further right means higher prescedence/binding, gets grouped first
// todo "=" ":" handled differently?
String operator_list[] = {":=", "else", "then", "be", "is", "equal", "equals", "==", "!=", "≠", "xor", "or", "||", "|", "&&", "&", "and",
                          "not", "<=", ">=", "≥", "≤", "<", ">", "less", "bigger", "⁰", "¹", "²", "³", "⁴", "+", "-",
                          "*", "×", "⋅", "⋆", "/", "÷", "^", "√", "++", "--", "∈", "∉", "⊂", "⊃", "in", "of",
                          "from",}; // "while" ...

Node groupOperators(Node &expression) {
	if (expression.name == "if")return groupIf(expression);
//	if(expression.kind==function)return expression;// already grouped
	if (expression.length == 0)return expression;
	if (expression.kind == longs)return expression;
	if (expression.length == 1)
		if (expression.kind != function)
			return groupOperators(expression.children[0]); // Nothing to be grouped
	expression.log();
	Node lhs;
	for (Node &op : expression) {
		int isFunc = op.name.in(function_list);
		int isControl = op.name.in(control_flows);
		if ((isControl or isFunc) and op.length == 0) { // todo: op.length>0 means already has body?
			if (isFunc) op.kind = function;
			if (!op.children) {
				Node *n = &op;
				if (n->next and n->next->kind == groups)
					op.add(n->next); //f(x,y)+1
				else
					while (n = n->next) // f x+1
						op.addRaw(n);
			}
			Node &flat = op.flat();
			Node *right = groupOperators(flat).clone();// applied on children
			if (lhs.empty())return *right;
			lhs.add(right);
			return groupOperators(lhs);
		} else lhs.add(op);

	}
	for (String operator_name : operator_list) {
		for (Node &op : expression) {
			if (op.name == operator_name) {
				Node lhs = expression.to(op);
				Node rhs = expression.from(op);
				op["lhs"] = groupOperators(lhs);
				op["rhs"] = groupOperators(rhs);
				if (expression.kind == function) {// f 3*3 => f(*(3 3))
					expression.children = op.clone();
					expression.length = 1;
					return expression;
				}
				// should NOT MODIFY original AST, because iterate by value, right?
				return op;// (a op c) => op(a c)
			}
		}
	}
	return expression;// no op
}


Node groupIf(Node n) {
	breakpoint_helper
	if (n.length == 0 and !n.value.data)
		error("no if condition given");
	if (n.length == 1 and !n.value.data)
		error("no if block given");
	Node &condition = n.children[0];
	Node then;
	if (n.length > 0)then = n[1];
	if (n.length == 0) then = n.values();
	if (n.has("then")) {
		condition = n.to("then");
		then = n.from("then");
	}

	if (condition.value.data and !condition.next)
		then = condition.values();
	if (condition.next and condition.next->name == "else")
		then = condition.values();

	// todo: UNMESS how?
	if (n.has(":") /*before else: */) {
		condition = n.to(":");
		if (condition.has("else"))
			condition = condition.to("else");// shouldn't happen?
		then = n.from(":");
	} else if (condition.has(":")) {// as child
		then = condition.from(":");
		condition = condition.evaluate();
	}
	Node otherwise;
	if (n.has("else"))
		otherwise = n["else"];
	if (then.has("then"))
		then = n.from("then");
	if (then.has("else")) {
		otherwise = then.from("else");
		then = then.to("else");
	}
	if (n.length == 3 and otherwise.empty())
		otherwise = n[3];
//	if(condition.name=="condition")
//		condition = condition.values();


	Node ef = Node("if");
	ef.kind = expressions;
	ef["condition"] = groupOperators(condition);
	ef["then"] = groupOperators(then);
	ef["else"] = groupOperators(otherwise);
	Node &node = ef["condition"];// debug
	return *ef.clone();

	Node condit = condition.evaluate();
	bool condition_fulfilled = (bool) condit;
	if (condition.kind == reals or condition.kind == longs)
		condition_fulfilled = condition.name.empty() and condition.value.data or condition.name != "0";
	else if (condition.value.data and condition.kind == objects) // or ...
		error("If statements need a space after colon");
	if (condition_fulfilled) {
		if (then.name == "then") {
			if (then.value.data or then.children) // then={} as arg
				return eval(then.values());
			return eval(n[2]);
		}
		return eval(then);
	} else {
		if (n.has("else"))
			return eval(n.from("else"));
		if (n.length == 3 and not n.has(":"))
			return eval(n[2]);// else
		else
			return False;
	}
}

Node do_call(Node left, Node op0, Node right) {
	String op = op0.name;
	if (op == "id")return right;// identity
	if (op == "square")return square(right.numbere());
	if (op == "√")return sqrtl(right.numbere());

	breakpoint_helper
	error("Unregistered function "s + op);
}

Node matchPattern(Node object, Node pattern0) {
//	[1 2 3]#1 == 1 == [1 2 3][0]
	Node pattern = pattern0.evaluate(); // [1 2 3][3-2]==2
	if (pattern.kind == longs)return object[(int) pattern.numbere()];
	if (pattern.kind == strings)return object[pattern.value.string];
// todo proper matches, references...
	return object[pattern.name];
}

/*
0x2218	8728	RING OPERATOR	∘
 */
Node Node::apply_op(Node left, Node op0, Node right) {
//	printf("apply_op\n");
//	left.log();
//	op0.log();
//	right.log();
//	if(right.length==0 and op0.param){
//		warn("using param for args");
//		right = *op0.param;
//	}
	String &op = op0.name;
	if (!isFunction(op)) // 1 + square 2  => "1+" kept dangling
		left = left.evaluate(false);
	bool lazy = (op == "or") and (bool) left;
	lazy = lazy || (op == "and") and not(bool) left;
	lazy = lazy || (op == "#");// length and index
	lazy = lazy || (op == "if");
//	lazy = lazy || arg#n is block

	if (!lazy)
		right = right.evaluate(false);

	if (op.in(function_list))
		return do_call(left, op0, right);

	if (op0.kind == patterns)
		return matchPattern(left, op0);

	if (op == ".") {
		return matchPattern(left, right);
	}

	if (op == "of" or op == "in") {
		return matchPattern(right, left);
	}

	if (op == "#") {
		if (left.length == 0) // length operator #{a b c} == 3
			return Node(right.length);// or right["size"] or right["count"]  or right["length"]
		else {  // index operator [a b c]#2 == b
			long index = right.value.longy;
			if (index <= 0)
				error("index<=0 ! Angle index operator # starts from 1. So [a b c]#2 == b. Use [] operator for zero based indexing");
			if (index > left.length)error("Index out of range: %d > %d !"s % index % left.length);
			return left.children[index - 1];
		}
	}


	if (op == "not" or op == "¬" or op == "!") {
		// todo: what if left is present?
		Node x = right.evaluate();
		return x.empty() ? True : False;
	}

	if (op == "√") { // why String( on mac?
		if (right.kind == reals)
			left.addSmart(Node(sqrt(right.value.real)));
		if (right.kind == longs)
			left.addSmart(Node(sqrt(right.value.longy)).setType(reals));
		return left.evaluate();
	}

//	if(!is_KNOWN_operator(op0))return call(left, op0, right);

	if (op == "|") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == longs and right.kind == longs) return Node((long) (left.value.longy | right.value.longy));
		// pipe todo
	}

	if (op == "&") {// todo
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == bools or right.kind == bools)
			return left.value.data and right.value.data ? True : False;
		return Node(left.value.longy & right.value.longy);
	}

	if (op == "xor" or op == "^|") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == bools or right.kind == bools) {
			return left.value.longy ^ right.value.longy ? True : False;
		}
		return Node(left.value.longy ^ right.value.longy);
	}

	if (op == "and" or op == "&&") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == bools or right.kind == bools) return left.value.data and right.value.data ? True : False;
		if (left.value.longy)return right;
		return Node(left.value.longy and right.value.longy);// todo just False?
	}
/*
 `or`/`else` ARE NOT IDENTICAL:
`if 1 then 0 else 2 == 0`
`1 and 0 or 2 == 2`  !!!
*/
	if (op == "or" or op == "||" or op == "&") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (!left.empty() and left != NIL and left != False)return left;
		return right;
	}

	if (op == "==" or op == "equals") {
		return left == right ? True : False;
	}

	if (op == "!=" or op == "^=" or op == "≠" or op == "is not") {
		return left != right ? True : False;
	}
	if (op == "<" or op == "less" or op == "lt") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() < right.string());
		if (left.kind == reals and right.kind == reals) return Node(left.value.real < right.value.real);
		if (left.kind == reals and right.kind == longs) return Node(left.value.real < right.value.longy);
		if (left.kind == longs and right.kind == reals) return Node(left.value.longy < right.value.real);
		if (left.kind == longs and right.kind == longs) return Node(left.value.longy < right.value.longy);
	}

	if (op == "<=" or op == "le" or op == "≤") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() <= right.string());
		if (left.kind == reals and right.kind == reals) return Node(left.value.real <= right.value.real);
		if (left.kind == reals and right.kind == longs) return Node(left.value.real <= right.value.longy);
		if (left.kind == longs and right.kind == reals) return Node(left.value.longy <= right.value.real);
		if (left.kind == longs and right.kind == longs) return Node(left.value.longy <= right.value.longy);
	}

	if (op == ">=" or op == "ge" or op == "≥") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() >= right.string());
		if (left.kind == reals and right.kind == reals) return Node(left.value.real >= right.value.real);
		if (left.kind == reals and right.kind == longs) return Node(left.value.real >= right.value.longy);
		if (left.kind == longs and right.kind == reals) return Node(left.value.longy >= right.value.real);
		if (left.kind == longs and right.kind == longs) return Node(left.value.longy >= right.value.longy);
	}

	if (op == ">" or op == "gt") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() > right.string());
		if (left.kind == reals and right.kind == reals) return Node(left.value.real > right.value.real);
		if (left.kind == reals and right.kind == longs) return Node(left.value.real > right.value.longy);
		if (left.kind == longs and right.kind == reals) return Node(left.value.longy > right.value.real);
		if (left.kind == longs and right.kind == longs) return Node(left.value.longy > right.value.longy);
	}

	if (op == "+" or op == "add" or op == "plus") {
		if (left.kind == strings or right.kind == strings) return Node(left.string() + right.string());
		if (left.kind == reals and right.kind == reals) return Node(left.value.real + right.value.real);
		if (left.kind == reals and right.kind == longs) return Node(left.value.real + right.value.longy);
		if (left.kind == longs and right.kind == reals) return Node(left.value.longy + right.value.real);
		if (left.kind == longs and right.kind == longs) return Node(left.value.longy + right.value.longy);
		todo(op + " operator NOT defined for types %s and %s "s % typeName(left.kind) % typeName(right.kind));
	}



	// todo: 2 * -x
	if (op == "-" or op == "minus" or op == "subtract") {
		if (left.kind == reals and right.kind == reals) return Node(left.value.real - right.value.real);
		if (left.kind == longs and right.kind == reals) return Node(left.value.longy - right.value.real);
		if (left.kind == reals and right.kind == longs) return Node(left.value.real - right.value.longy);
		if (left.kind == longs and right.kind == longs) return Node(left.value.longy - right.value.longy);
	}


	if (op == "*" or op == "⋆" or op == "×" or op == "∗" or op == "times") {// ⊗
		if (left.kind == strings) return Node(left.string().times(right.value.longy));
		if (right.kind == strings) return Node(right.string().times(left.value.longy));
		if (left.kind == reals and right.kind == reals) return Node(left.value.real * right.value.real);
		if (left.kind == longs and right.kind == reals) return Node(left.value.longy * right.value.real);
		if (left.kind == reals and right.kind == longs) return Node(left.value.real * right.value.longy);
		if (left.kind == longs and right.kind == longs) return Node(left.value.longy * right.value.longy);
	}

	if (op == "%" or op == "rem" or op == "modulo") {
		if (left.kind == longs and right.kind == longs) return Node(left.value.longy % right.value.longy);
	}

	if (op == "/" or op == "÷" or op == "div" or op == "divide") { // "by"
		if (left.kind == reals and right.kind == reals) return Node(left.value.real / right.value.real);
		if (left.kind == longs and right.kind == reals) return Node(left.value.longy / right.value.real);
		if (left.kind == reals and right.kind == longs) return Node(left.value.real / right.value.longy);
		if (left.kind == longs and right.kind == longs) return Node(left.value.longy / right.value.longy);
	}

	if (op == "=" or op == ":=" or op == ":" or op == "⇒" or op == "=>") {
		warn("proper '=' operator");
		left.kind = reference;
		if (right.value.data) {// and ...
			left.kind = right.kind; // there are certainly things lost here!?!
			left.value.data = right.value.data;// todo failed copy assignment: length=0!!!
			if (right.kind == strings)left.value.string.length = right.value.string.length;// DONT WORKAROUND BUGS!!
		} else
			left.value.node = &right;
		return left;
	}
	if (op == "else" or op == "then")return right;// consume by "if"! todo COULD be used as or if there is no 'if'
	if (op == "if") return If(right);
	if (op.in(function_list) or op.in(functor_list)) {
//		kind=Type::function; // functor same concept, different arguments
		// careful, functions take arguments, functors take bodies if(1,2,3)!=if{1}{2}{3}
	}
	todo("operator “%s” NOT defined for types %s and %s "s % op % typeName(left.kind) % typeName(right.kind));
	return NIL;
//	log("NO builtin operator "+op0+" calling…")
//	return call(left, op0, right);
}


Node Angle::analyze(Node data) {
	if (data.kind == reference and data.length == 0)return data;
	if (data.kind == expressions or data.kind == declaration) {
		return groupOperators(data);
	} else // or data.kind==groups or
		warn("REPLACE with their ast?");
	Node grouped=*data.clone();
	grouped.children = 0;
	grouped.length = 0;
	for (Node &child: data) {
		child = analyze(child);// REPLACE with their ast? NO! todo
		grouped.addRaw(child);
	}
	return grouped;
}

Node analyze(Node data) {
	return Angle::analyze(data);
}

Node emit(String code) {
	Node data = parse(code);
	Node charged = Angle::analyze(data);
	Node node = emit(charged).run();
	return node;
}


float precedence(String name) {
	// like c++ here HIGHER up == lower value == more important
//	switch (node.name) nope
//		name = operater.value.string;// NO strings are not automatic operators lol WTF
	if (eq(name, "."))return 0.5;
	if (eq(name, "of"))return 0.6;
	if (eq(name, "in"))return 0.7;
	if (eq(name, "from"))return 0.8;

	if (eq(name, "not"))return 1;
	if (eq(name, "¬"))return 1;
	if (eq(name, "!"))return 1;


	if (eq(name, "√"))return 3;
	if (eq(name, "#"))return 3;// count
	if (eq(name, "++"))return 3;
//	if (eq(node.name, "+"))return 3;//
	if (eq(name, "--"))return 3;
	if (eq(name, "-"))return 3;// 1 + -x

	if (eq(name, "/"))return 4.9;
	if (eq(name, "÷"))return 4.9;


	if (eq(name, "times"))return 5;
	if (eq(name, "*"))return 5;
	if (eq(name, "×"))return 5;
	if (eq(name, "add"))return 6;
	if (eq(name, "plus"))return 6;
	if (eq(name, "+"))return 6;
	if (eq(name, "minus"))return 6;
	if (eq(name, "-"))return 6;
	if (eq(name, "%"))return 6.1;
	if (eq(name, "rem"))return 6.1;
	if (eq(name, "modulo"))return 6.1;

	if (eq(name, "<"))return 6.5;
	if (eq(name, "<="))return 6.5;
	if (eq(name, ">="))return 6.5;
	if (eq(name, ">"))return 6.5;
	if (eq(name, "≥"))return 6.5;
	if (eq(name, "≤"))return 6.5;
	if (eq(name, "≈"))return 6.5;
	if (eq(name, "=="))return 6.6;

	if (eq(name, "and"))return 7.1;
	if (eq(name, "&&"))return 7.1;
	if (eq(name, "&"))return 7.1;
	if (eq(name, "xor"))return 7.2;
	if (eq(name, "or"))return 7.2;
	if (eq(name, "||"))return 7.2;

	if (eq(name, ":"))return 7.5;// todo:

	if (name.in(function_list))// f 1 > f 2
		return 8;// 1000;// function calls outmost operation todo? add 3*square 4+1

	if (eq(name, "⇒"))return 9;// todo
	if (eq(name, "=>"))return 9;// todo:
	if (eq(name, "="))return 10;
	if (eq(name, "≠"))return 10;
	if (eq(name, "!="))return 10;
	if (eq(name, ":="))return 11;
	if (eq(name, "equals"))return 10;
	if (eq(name, "equal"))return 10;
	if (eq(name, "else"))return 11.09;
	if (eq(name, "then"))return 11.15;
	if (eq(name, "if"))return 100;
	if (name.in(functor_list))// f 1 > f 2
		return 1000;// if, while, ... statements calls outmost operation todo? add 3*square 4+1
	return 0;// no precedence
}

float precedence(Node &operater) {
	String &name = operater.name;
//	if (operater == NIL)return 0; error prone
	if (name.empty())return 0;// no precedence
	if (operater.kind == reals)return 0;//;1000;// implicit multiplication HAS to be done elsewhere!
	if (operater.kind == longs)return 0;//;1000;// implicit multiplication HAS to be done elsewhere!
	if (operater.kind == strings)return 0;// and name.empty()
	if (operater.kind == groups or operater.kind == patterns)
		return precedence("if") * 0.999;// needs to be smaller than functor/function calls
	if (operater.name.in(function_list))return 999;// function call
	return precedence(name);
}
