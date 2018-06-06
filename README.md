# PTest-C
## What is PTest-C?
Glad you asked!  PTest-C is short for PicoTest-C.

### PicoTest-C is a very lightweight but easy-to-use unit testing library for C.

PTest-C follows a structure very familiar to users of GTest, but in a C environment.
There is no need to create and manage a list of tests as is needed in many other C frameworks.

## So, how do I use this thing?
Easy!  First off you've got to make a test case.  That'll look like this:
```C
P_TEST(MyTestName) {
    ...
}
```

Inside your P_TEST body, you'll want to add some assertions to test.  Basically,
any expression that would work in an if statement will work inside an assertion.

Here's how simple an example test file can be!

```C
#include "ptest.h"

P_TEST(MyFirstTest) {
	P_ASSERT(2 + 2 != 4);
	
	return TEST_SUCCESS;
}

int main() {
	return P_RunTests();
}
```

And here's what you'll see in your console:

```
=====================================
[Test: MyFirstTest]
=====================================

Failure in MyFirstTest() example.c:4:  Assertion failed ("2 + 2 != 4")
Trace:
        MyFirstTest at example.c:6
        P_RunTest at ptest.c:106
        P_RunTests at ptest.c:143
        main at example.c:11

[FAILURE]

=====================================

Tests run: 1    Tests Succeeded: 0      Tests Failed: 1
```

Nifty, huh?

## Other Features

### Test Fixtures

PTest-C provides fixture-like support in the form of allowing the programmer to provide his/her own create and destroy functions (and even data pointers) to run alongside tests.
It works like this:

```C
#include "stdio.h"
#include "stdlib.h"
#include "ptest.h"

typedef struct {
	int myInt;
	char* myStr;
} Foo;

void* foo_setup() {
	printf("In setup...");
	Foo* foo = malloc(sizeof(Foo));
	foo->myInt = 42;
	foo->myStr = "So long, and thanks for all the fish!";
	return (void*)foo;
}

void foo_cleanup(void* foo) {
	printf("In cleanup...");
	free(foo);
}

// The NULL can be a preallocated object you can pass.
// All of the setup, cleanup, and data parameters are nullable and optional.
// The data param will be set automatically by the return value of the setup func
// if it is passed in as NULL.
P_TEST_F(MyTestFixtureTest, foo_setup, foo_cleanup, NULL) {
	Foo* foo = (Foo*)data;
    P_ASSERT(foo->myInt == 42);
	printf("%s\n", foo->myStr);
	
	return TEST_SUCCESS;
}

// You can even pass free() as a cleanup function for PoD objects if you just need
// to make sure they get destroyed, or you can use the setup and cleanup functions
// to do external things, like setup networking functionality, and not even use them
// for allocation.
P_TEST_F(MyTestFixtureTest2, foo_setup, free, NULL) {
	Foo* foo = (Foo*)data;
    P_ASSERT(foo->myInt == 42);
	printf("%s\n", foo->myStr);
	
	return TEST_SUCCESS;
}

int main() {
    return P_RunTests();
}
```

And again the output will be:

```
=====================================
[Test: MyTestFixtureTest]
=====================================
In setup...So long, and thanks for all the fish!

[OK]
In cleanup...
=====================================
[Test: MyTestFixtureTest2]
=====================================
In setup...So long, and thanks for all the fish!

[OK]

=====================================

Tests run: 2    Tests Succeeded: 2      Tests Failed: 0
```

## Test Filtering

PTest-C will automatically gather all ```P_TEST()``` and ```P_TEST_F()``` tests to
run, so you don't need to worry about it.

If you would like to run only certain tests, you can run all tests that partially match a given string like so:
```C
int retval = P_RunTestsWith("Integration");
```
That will run all test cases that contain the string "Integration" as a substring in the order they are declared.

## Nested functions with assertions

PTest-C will allow you to make assertions in a nested function and still have things behave nicely, like so:

```C
void mySubroutine() {
	P_ASSERT(2 + 2 != 4);
}

P_TEST(TestSubroutine) {
	mySubroutine();
	return TEST_SUCCESS;
}
```

Doing so will not trip PTest-C up.  PTest-C even supports assertions in your test fixture init, and cleanup (though I'm not sure why you'd want to run an assertion in cleanup :D)

## Optional human-readable stack tracing on assertion failure

On a failure, you might want some more information on why it failed.
PTest-C is configurable to display a human-readable stack trace with line numbers and function names on an assertion failure.
This works best if GCC/Clang is configured to output GDB debugging info, though PTest-C will still be able to provide some information, such as the function, line number, and assertion expression that triggered the failure on a test.

To enable stack trace simply add the following to your GCC flags:
```
-ggdb -DPTEST_ENABLE_TRACE -DPTEST_TRACE_DEPTH=15
```

The maximum depth of the stack trace is configurable through PTEST_TRACE_DEPTH.

## Failure Text

If all else fails, PTest-C allows you to set a text to be displayed if the next assertion fails in order to provide some context in your output logs.  If the assertion succeeds, the text will be cleared and will not reappear with a later assertion unless you add it in again.

This can be set via ```P_SetFailMessage()```.

## Planned Features
I plan on adding new features to PTest-C as I go along.  Right now it is quite small, however, it seems to be easier and more intuitive to use than a lot of other testing suites I've seen out there for C.

Currently, I will work on making PTest-C able to support multi-threaded testing.  This shouldn't be difficult, but in the interest of getting something up, I neglected to include this at the moment.

## License
PicoTest is licensed under the Apache License, so it's easy to integrate into any project, big or small.

Copyright (c) 2018 Peter Orzell

## Compatibility
PTest-C is currently set up to work with GCC/Clang on and requires constructor attribute support to function properly.
The addr2line application is necessary for stack trace to function properly if enabled.  Unfortunately, there doesn't seem to be a better way to get this working than piping to addr2line, as backtrace_symbols() does not provide anything near as useful.

## Have fun!
Let me know if you end up using this!  I'll be working on it regularly.