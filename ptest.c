
#include "ptest.h"

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "setjmp.h"

#include "string.h"

extern const char *__progname;

typedef struct {
    const char* test_name;
    bool has_fixture;
    void* test_func;
    CREATE_FUNC create_func;
    TEARDOWN_FUNC teardown_func;
    void* data;
} PTestInfo;
static PTestInfo *test_funcs = NULL;
static size_t num_tests = 0;

static jmp_buf fail_jump;
static const char* failure_message = NULL;

void P_Fail() {
#ifdef PTEST_ENABLE_TRACE
    void *trace[PTEST_TRACE_DEPTH+1];
    backtrace(&trace, PTEST_TRACE_DEPTH+1);
    printf("Trace:\n");
    static const int buflen = 1024;   
    int i = 1;
    for(; trace[i] && i < PTEST_TRACE_DEPTH+1; ++i) {
        char syscom[buflen];
        snprintf(syscom, buflen,"addr2line -e %s -f -s -C -p %p", __progname, trace[i]);
        FILE *f = popen(syscom, "r");
        if (f != NULL)
        {
            char buffer[buflen];
            memset(buffer, 0, buflen*sizeof(char));
            while(fgets(buffer, sizeof(buffer), f) != NULL)
            {
                printf("\t%s", buffer);
            }
            pclose(f);
            
            if(strstr(buffer,"main at") == buffer) {
                break;
            }
        }
    }
#endif
    longjmp(fail_jump,1);
}

void P_ClearFailMessage() {
    failure_message = NULL;
}

void P_SetFailMessage(const char* fail_msg) {
    failure_message = fail_msg;
}

void P_AddTest(const char* test_name, TEST_FUNC testfunc) {
    num_tests++;
    test_funcs = realloc(test_funcs, num_tests*sizeof(PTestInfo));
    
    PTestInfo *info = &test_funcs[num_tests-1];
    info->has_fixture = false;
    info->test_func = testfunc;
    info->test_name = test_name;
}

void P_AddTestWithFixture(const char* test_name, TEST_FIXTURE_FUNC test_func, CREATE_FUNC create_func, TEARDOWN_FUNC teardown_func, void* data) {
    num_tests++;
    test_funcs = realloc(test_funcs, num_tests*sizeof(PTestInfo));
    
    PTestInfo *info = &test_funcs[num_tests-1];
    info->has_fixture = true;
    info->test_func = test_func;
    info->test_name = test_name;
    info->create_func = create_func;
    info->teardown_func = teardown_func;
    info->data = data;
}

static bool P_RunTest(PTestInfo* info) {
    bool success = true;
    
    failure_message = NULL;
    printf("\n=====================================\n[Test: %s]\n=====================================\n", info->test_name);
    
    if(!setjmp(fail_jump)) {    
        if (info->create_func && info->data == NULL) {
            info->data = info->create_func();
        }
    
        int result = 0;
        if (info->has_fixture) {
            TEST_FIXTURE_FUNC test_func = (TEST_FIXTURE_FUNC)(info->test_func);
            result = test_func(info->data);
        } else {
            TEST_FUNC test_func = (TEST_FUNC)(info->test_func);
            result = test_func();
        }
        if(!result) {
            printf("\n[OK]\n");
        } else {
            printf("\n[FAILURE] Test returned TEST_FAIL\n");
            if (failure_message) {
                printf("%s\n", failure_message);
                success = false;
            }
        }
        
    } else {
        if (failure_message) {
            printf("%s\n", failure_message);
        }
        printf("\n[FAILURE]\n");
        success = false;
    }
    
    // Allow teardown to assert if necessary.  Probably shouldn't allow this tbh.
    if(!setjmp(fail_jump)) {
        if (info->teardown_func) {
            info->teardown_func(info->data);
        }
    }
    
    return success;
}

int P_RunTests() {
    size_t tests_succeeded = 0;
    
    putchar('\n');
    
    size_t i = 0;
    for(; i < num_tests; ++i) {
        if (P_RunTest(&test_funcs[i])) {
            tests_succeeded++;
        }
    }
    
    printf("\n=====================================\n\nTests run: %zu\tTests Succeeded: %zu\tTests Failed: %zu\n\n", i, tests_succeeded, i - tests_succeeded);
    
    return (tests_succeeded == i) ? 0 : -1;
}

int P_RunTestsWith(const char *partial_match) {
    size_t tests_run = 0;
    size_t tests_succeeded = 0;
    
    putchar('\n');
    
    size_t i = 0;
    for(; i < num_tests; ++i) {
        if (strstr(test_funcs[i].test_name, partial_match)) {
            if (P_RunTest(&test_funcs[i])) {
                tests_succeeded++;
            }
            tests_run++;
        }
    }
    
    printf("=====================================\n\nTests run: %zu\tTests Succeeded: %zu\tTests Failed: %zu\n\n", tests_run, tests_succeeded, tests_run - tests_succeeded);
    
    return (tests_succeeded == i) ? 0 : -1;
}