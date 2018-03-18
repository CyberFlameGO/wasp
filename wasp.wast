;; wasmx foo.wast
;; https://github.com/sunfishcode/wasm-reference-manual/blob/master/WebAssembly.md
;; http://webassembly.org/docs/js/
(module

 (table 1 1 anyfunc)
 (memory $0 10)
 (start $test) ;; start must have 0 params

	(export "memory" (memory $0))
	(export "main" (func $main))

 ;; If you try to call a exported wasm function that takes or returns an i64 type value, it currently throws
 ;; This may change — a new int64 type is being considered for future standards, which could then be used by wasm.
 (import "console" "log" (func $logp (param i32 i32))) ;;  pointer to anything / string!
 (import "console" "logs" (func $logs (param i32))) ;;  pointer to string!
 (import "console" "logc" (func $logc (param i32))) ;;  int as char
 (import "console" "logi" (func $logi (param i32)))
 (import "console" "logx" (func $logx (param i32)))
 (import "console" "logi" (func $logf (param f64))) 
 (import "console" "logi" (func $logn (param f64))) 
 (import "console" "raise" (func $imports.raise)) ;; (param i64)

 ;;(global $a-mutable-global (mut f32) (f32.const 7.5))
 ;;(elem (i32.const 0) "$kitchen()sinker")

 ;;(global $data i32 (i32.const 1))
	;;(export "data" (global $data))

 (global $offset i32 (i32.const 100))
 (export "offset" (global $offset))

 (global $OK i32 (i32.const 1))
 (data (i32.const 1) "OK\00")

 (global $Hello i32 (i32.const 70))
 (data (i32.const 70) "hello wasm!\00") ;; no identifier here :(
 ;;(data (get_global $Hello) "Hello global!\00")
 ;;(data (global $Hello) "Hello global!\00")


 (data (i32.const 20) "Hello!\00")


  (global $type.error i32 (i32.const -1))
  (global $type.null 	i32 (i32.const 0))
  (global $type.bool 	i32 (i32.const 0))
  (global $type.int 	i32 (i32.const 0))
  (global $type.byte 	i32 (i32.const 1))
  (global $type.short i32 (i32.const 2))
  (global $type.type 	i32 (i32.const 3))
  (global $type.word 	i32 (i32.const 4)) ;; vs int32 float32 !
  (global $type.json5 i32 (i32.const 5))
  (global $type.json 	i32 (i32.const 6))
  (global $type.data 	i32 (i32.const 7)) ;; serialized
  (global $type.pointer i32 (i32.const 8));; vs int64, float64
  (global $type.string i32 (i32.const 9))
  (global $type.array  i32 (i32.const 0xA))
  (global $type.binary i32 (i32.const 0xB)) ;; ??
  (global $type.code   i32 (i32.const 0xC)) ;; ??
  (global $type.data	 i32 (i32.const 0xD)) ;; ??
  (global $type.object i32 (i32.const 0xE)) ;; entity
  (global $type.char i32 (i32.const 20))

	(global $null  i64 (i64.const 0))
	(global $false i32 (i32.const 0))
	(global $true  i32 (i32.const 1))
	(global $value.error32 i32 (i64.const 0xFFFFFFFF))
	(global $value.error64 i64 (i64.const 0xFFFFFFFFFFFFFFFF))
	(global $value.error   i64 (i64.const 0xFFFFFFFFFFFFFFFF))


(func $ok 
	(call $print (get_global $OK) ) 
) 

(func $assert_eq32 (param i32) (param i32) 
 (if (i32.eq (get_local $0) (get_local $1)) (return))
 (call $logi (get_local $0))
 (call $logi (get_local $1))
 (call $raise)
) 

(func $log (param i64) 
 (local $type i32) 
 (local $pointer i32) 
 (set_local $type (call $type.ofi (get_local $0) ) )
 (set_local $pointer (call $type.raw (get_local $0) )) 
 (call $logx (i32.const 0xFFFFFFFF))
 (call $logx (get_local $type) ) 
 (call $logx (get_local $pointer) ) 
 (call $logp (get_local $type) (get_local $pointer)  ) 
) 

(func $type.of (param i64)  (result i64) 
 (i64.and (get_local $0) (i64.const 0xFFFFFFFF00000000))
) 

(func $type.ofi (param i64)  (result i32)
 (i32.wrap/i64 (i64.shr_u (get_local $0) (i64.const 32) ))
 ;;(i64.and (get_local $0) (i64.const 0xFFFFFFFF00000000))
) 

(func $type.to (param i32) (param i64) (result i64)
 (i64.or (i64.extend_u (get_local $0)) (get_global $type.string))
)
;;(func $type.to (param i32) (param i32) (result i64)
;; (i64.or (i64.extend_u (get_local $0)) (get_global $type.string))
;;)

(func $string.is (param i64) (result i32)
 (i32.wrap/i64 (i64.and (get_local $0) (get_global $type.string)))
)

(func $raise
 (call $imports.raise)
 (call $logf (f64.div (f64.const 0) (f64.const 0)))
)

(func $assert (param i32) ;;(result i32)
	(if (get_local $0) (nop) (call $raise))
	;;(i32.const 1)
)

(func $string.pointer (param i32) (result i64) ;; stringify
 (return (call $type.to (get_local $0) (get_global $type.string)))
)
;;(func $string.const (param i32) (result i64)
;;)

;;(func $type.of (param i64) (result i32)
;; (return )
;;)

(func $print (param i32)
   (local $char i32)
  (loop $while
  	 (set_local $char (i32.load8 (get_local $0)))
     (set_local $0 (i32.add (get_local $0) (i32.const 1) ) )     
   	 (call $logc (get_local $char))
     (if (get_local $char) (br $while))
  )
)

(func $is (param i64) (param i64) (result i32)
 (i32.wrap/i64 (i64.and (get_local $0) (get_local $1)))
)
(func $is.string (param i64) (result i32)
 (i32.wrap/i64 (i64.and (get_local $0) (get_global $type.string)))
)
(func $todo
 (call $raise)
)

(func $atoi (param i32) (result i32)
 (local $i i32)
 (local $t i32)
 (local $char i32)
 (loop $while
	 (set_local $0 (i32.add (get_local $0) (i32.const 1)))
	 (set_local $char (i32.load8 offset=0 align=1 (get_local $0)))
	 (if (get_local $char) (br $while))
	 (set_local $t 	(i32.sub (get_local $char) (i32.const 48))) 
	 (if (i32.gt_u (get_local $t) (i32.const 9)) (br $while))
	 (if (i32.lt_u (get_local $t) (i32.const 0)) (br $while))
	 (set_local $i (i32.add (get_local $i) (get_local $t)))
 )
 (return (get_local $i))
)
(func $type.raw (param i64) (result i32)
 (i32.wrap/i64 (i64.and (get_local $0) (i64.const 0x00000000FFFFFFFF) ) )
)
(func $string.raw (param i64) (result i32)
 (i32.wrap/i64 (i64.and (get_local $0) (i64.const 0x00000000FFFFFFFF) ) )
)

(func $int.parse (param i64) (result i32)
 ;;(call $assert (i32.not (i32.const 0)))
 (call $logi (call $is (get_local $0) (get_global $type.string)))
 (call $assert (call $is (get_local $0) (get_global $type.string)))
 (call $atoi (i32.wrap/i64 (get_local $0)))
 ;;(call $todo);;
 ;;(i32.const -1)
)


(func $int (param i64) (result i32)
 (if (call $is.string (get_local $0)) (return (call $int.parse (get_local $0))))
 ;;(if (call $string.is (get_local $0) (call $atoi (get_local $0))))
 (call $raise) ;; cannot cast type to int
 (i32.const -1)
)

(func $str_cpy (param i32) (param i32) ;; from to !
  (local $char i32)
  ;;(call $logs (get_global $from))
  ;;(call $logs (get_local $0))
  ;;(call $logs (get_global $to))
  ;;(call $logs (get_local $1))
  (loop $while
  	 (set_local $char (i32.load8 (get_local $0)))
   	 (i32.store8 (get_local $1) (get_local $char))
     (set_local $0 (i32.add (get_local $0) (i32.const 1) ) )     
     (set_local $1 (i32.add (get_local $1) (i32.const 1) ) )     
   	 ;;(call $logc (get_local $char))
     (if (get_local $char) (br $while))
  )
)
	

(func $string.len (param i32) (result i32)
  (local $offset i32)
  (local $char i32)
  (set_local $offset (get_local $0))
;; 	(return (i32.load8 offset=0 align=1 (get_local $0))) safe len in first byte
  (loop $while
  	 (set_local $char (i32.load8 (get_local $offset)))
     (set_local $offset (i32.add (get_local $offset) (i32.const 1) ) )     
   	 ;;(call $logc (get_local $char))
     (if (get_local $char) (br $while))
  )
  (set_local $offset (i32.sub (get_local $offset) (get_local $0)))
  (i32.sub (get_local $offset) (i32.const 1))
)
	
(func $main (param i32) (result i32) 
 (call $logi (get_local $0))
 (call $logs (get_local $0))
  (call $str_cpy (get_local $0) (i32.const 20))
  ;;(call $str_cpy (i32.const 20) (get_local $0))
  (drop (call $string.len (i32.const 20)))
  (return (i32.const 20))
)


(func $test
 ;;(call $assert (i32.eq (i32.const 1) (i32.const 0))) throws, ok
	(local $s i64)  
 ;;(call $log (i64.const 0xFFFFDDDD11112222))
 (call $log (i64.const 0x1111222255556666))
 (call $ok) 
 (call $assert (i32.eq (i32.const 1) (i32.const 1)))
	(set_local $s (call $string.pointer (get_global $OK))) 
 (call $log (get_local $s)) 
	(call $assert (i64.eq (call $type.of (get_local $s)) (get_global $type.string) ) )
	(call $assert_eq32 (call $type.ofi (get_local $s)) (i32.const  0x10000000 ))
	(call $logi (call $type.raw (get_local $s))) 
	(call $assert (i32.eq (call $type.raw (get_local $s)) (get_global $OK) ) )
	(call $assert (call $is.string (get_local $s)))
	(call $assert (call $string.is (get_local $s)))
	(call $assert (call $string.raw (get_local $s)))
 (call $ok) 
	(call $assert (call $type.raw (get_local $s)))
 (call $ok) 	
	;;(call $assert (i32.eq (call $type.pointer (get_local $s) (get_global $OK) ) ) )
 (call $logi (i32.const 42)  ) 
 (call $ok) 	
 ;;(call $print (get_global $DONE))
)

)
