#pragma once
#ifndef _P_TEST_H_
#define _P_TEST_H_

#define STRINGIFY(x) #x

#define CAT_HELPER(x, y) x##y
#define CAT(val1, val2) CAT_HELPER(val1, val2)

#define MAKE_UNIQUE_TESTNAME(name) CAT(name, __LINE__)

#define TEST_FAIL -1
#define TEST_SUCCESS 0

#if defined(_MSC_VER)
#pragma section(".CRT$XCU",read)
#define INITIALIZER2_(f,p) \
        static void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        static void f(void)
#ifdef _WIN64
#define INITIALIZER(f) INITIALIZER2_(f,"")
#else
#define INITIALIZER(f) INITIALIZER2_(f,"_")
#endif
#else
#define INITIALIZER(f) \
        static void f(void) __attribute__((constructor)); \
        static void f(void)
#endif

#define P_TEST(TEST_NAME) \
    static int TEST_NAME(void);\
    INITIALIZER(MAKE_UNIQUE_TESTNAME(TEST_NAME)){\
    P_AddTest(STRINGIFY(TEST_NAME), TEST_NAME);\
    }\
    static int TEST_NAME(void)

#define P_TEST_F(TEST_NAME, CREATE_FUNCTION, TEARDOWN_FUNCTION, DATA) \
    static int TEST_NAME(void*);\
    INITIALIZER(MAKE_UNIQUE_TESTNAME(TEST_NAME)){\
    P_AddTestWithFixture(STRINGIFY(TEST_NAME), TEST_NAME, CREATE_FUNCTION, TEARDOWN_FUNCTION, DATA);\
    }\
    static int TEST_NAME(void* data)

static const char *ASSERT_FAIL_STR = "\nFailure in %s() %s:%u:\n\tAssertion failed (\"%s\")\n";
static const char *ASSERT_FAIL_MSG_STR = "Message: %s\n";

#define P_ASSERT(expr) {if (!(expr)) { \
printf(ASSERT_FAIL_STR, __FUNCTION__, __FILE__, __LINE__, STRINGIFY(expr)); \
P_Fail(); } else { P_ClearFailMessage();}}

#define P_ASSERT_S(expr, msg) {if (!(expr)) { \
printf(ASSERT_FAIL_STR, __FUNCTION__, __FILE__, __LINE__, STRINGIFY(expr)); \
printf(ASSERT_FAIL_MSG_STR, msg); \
P_Fail(); } else { P_ClearFailMessage();}}

typedef void* (*CREATE_FUNC)(void);
typedef void  (*TEARDOWN_FUNC)(void*);

typedef int  (*TEST_FUNC)();
typedef int  (*TEST_FIXTURE_FUNC)(void*);

void P_AddTest(const char* test_name, TEST_FUNC testfunc);
void P_AddTestWithFixture(const char* test_name, TEST_FIXTURE_FUNC test_func, CREATE_FUNC create_func, TEARDOWN_FUNC teardown_func, void* data);
int  P_RunTests();
int  P_RunTestsWith(const char *partial_match);
void P_SetFailMessage(const char *failure_message);
void P_ClearFailMessage();

void P_Fail();

#endif