; ModuleID = 'my cool jit'
source_filename = "my cool jit"

%ty_array = type { i64*, i64 }

@0 = private unnamed_addr constant [27 x i8] c"n should be greater than 1\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

declare i64 @extern_input()

declare void @extern_print_string(i8*)

declare void @extern_print_int(i64)

declare i64 @extern_pow(i64, i64)

declare void @extern_abort()

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #0

define i64 @main() {
entry:
  %retval = alloca i64
  store i64 0, i64* %retval
  %n = alloca i64
  store i64 0, i64* %n
  %0 = call i64 @extern_input()
  store i64 %0, i64* %n
  %n1 = load i64, i64* %n
  %1 = alloca i64, i64 %n1
  %2 = bitcast i64* %1 to i8*
  %3 = mul i64 %n1, 8
  call void @llvm.memset.p0i8.i64(i8* %2, i8 0, i64 %3, i1 false)
  %arr = alloca %ty_array
  %a1 = getelementptr inbounds %ty_array, %ty_array* %arr, i64 0, i32 0
  %arraydecay = getelementptr inbounds i64, i64* %1, i64 0
  store i64* %arraydecay, i64** %a1
  %b2 = getelementptr inbounds %ty_array, %ty_array* %arr, i64 0, i32 1
  store i64 %n1, i64* %b2
  %n2 = load i64, i64* %n
  %4 = icmp slt i64 %n2, 1
  br i1 %4, label %then, label %else

then:                                             ; preds = %entry
  call void @extern_print_string(i8* getelementptr inbounds ([27 x i8], [27 x i8]* @0, i32 0, i32 0))
  store i64 -1, i64* %retval
  br label %return

else:                                             ; preds = %entry
  br label %ifcont

ifcont:                                           ; preds = %else
  %a13 = getelementptr inbounds %ty_array, %ty_array* %arr, i32 0, i32 0
  %5 = load i64*, i64** %a13
  %arrayidx = getelementptr inbounds i64, i64* %5, i64 0
  store i64 2, i64* %arrayidx
  %i = alloca i64
  store i64 0, i64* %i
  %x = alloca i64
  store i64 0, i64* %x
  store i64 1, i64* %i
  store i64 3, i64* %x
  br label %while.cond

while.cond:                                       ; preds = %ifcont28, %ifcont
  %i4 = load i64, i64* %i
  %n5 = load i64, i64* %n
  %6 = icmp slt i64 %i4, %n5
  br i1 %6, label %loop, label %afterloop

loop:                                             ; preds = %while.cond
  %j = alloca i64
  store i64 0, i64* %j
  %flag = alloca i64
  store i64 0, i64* %flag
  store i64 0, i64* %j
  store i64 0, i64* %flag
  br label %while.cond6

afterloop:                                        ; preds = %while.cond
  %n31 = load i64, i64* %n
  %subtmp = sub i64 %n31, 1
  %a132 = getelementptr inbounds %ty_array, %ty_array* %arr, i32 0, i32 0
  %7 = load i64*, i64** %a132
  %arrayidx33 = getelementptr inbounds i64, i64* %7, i64 %subtmp
  %8 = load i64, i64* %arrayidx33
  call void @extern_print_int(i64 %8)
  call void @extern_print_string(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0))
  store i64 0, i64* %retval
  br label %return

while.cond6:                                      ; preds = %ifcont17, %loop
  %j9 = load i64, i64* %j
  %i10 = load i64, i64* %i
  %9 = icmp slt i64 %j9, %i10
  br i1 %9, label %loop7, label %afterloop8

loop7:                                            ; preds = %while.cond6
  %x11 = load i64, i64* %x
  %j12 = load i64, i64* %j
  %a113 = getelementptr inbounds %ty_array, %ty_array* %arr, i32 0, i32 0
  %10 = load i64*, i64** %a113
  %arrayidx14 = getelementptr inbounds i64, i64* %10, i64 %j12
  %11 = load i64, i64* %arrayidx14
  %12 = srem i64 %x11, %11
  %13 = icmp eq i64 %12, 0
  br i1 %13, label %then15, label %else16

afterloop8:                                       ; preds = %while.cond6
  %flag19 = load i64, i64* %flag
  %14 = icmp eq i64 %flag19, 0
  br i1 %14, label %then20, label %else27

then15:                                           ; preds = %loop7
  store i64 1, i64* %flag
  br label %ifcont17

else16:                                           ; preds = %loop7
  br label %ifcont17

ifcont17:                                         ; preds = %else16, %then15
  %j18 = load i64, i64* %j
  %addtmp = add i64 %j18, 1
  store i64 %addtmp, i64* %j
  br label %while.cond6

then20:                                           ; preds = %afterloop8
  %x21 = load i64, i64* %x
  %i22 = load i64, i64* %i
  %a123 = getelementptr inbounds %ty_array, %ty_array* %arr, i32 0, i32 0
  %15 = load i64*, i64** %a123
  %arrayidx24 = getelementptr inbounds i64, i64* %15, i64 %i22
  store i64 %x21, i64* %arrayidx24
  %i25 = load i64, i64* %i
  %addtmp26 = add i64 %i25, 1
  store i64 %addtmp26, i64* %i
  br label %ifcont28

else27:                                           ; preds = %afterloop8
  br label %ifcont28

ifcont28:                                         ; preds = %else27, %then20
  %x29 = load i64, i64* %x
  %addtmp30 = add i64 %x29, 1
  store i64 %addtmp30, i64* %x
  br label %while.cond

return:                                           ; preds = %afterloop, %then
  %16 = load i64, i64* %retval
  ret i64 %16
}

attributes #0 = { argmemonly nounwind }
