#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"

#include "ptest.h"

//=================================================================

// Even though this is a non-test function, an assertion exception can still be passed up.
// 
void printStr(char* c) {
    P_ASSERT(c != NULL);
    printf("%s", c);
}

P_TEST(SomeStuff) {
    printStr("Hello world");
    printStr(NULL);
    
    printf("I shouldn't ever print.\n");
    
    return TEST_SUCCESS;
}

//=================================================================

// A failure message can be set so that if the next assertion fails,
// the message will be displayed.

// If the assertion passes, the fail message will be reset.

P_TEST(FailWithoutMessage) {
    P_SetFailMessage("This string shouldn't be seen because it's cleared by the next assert.");
    P_ASSERT(5==5);
    return TEST_FAIL;
}

//=================================================================

P_TEST(FailWithMessage) {
    P_SetFailMessage("This test is just built to fail.");
    return TEST_FAIL;
}

//=================================================================

P_TEST(FailInNestedFunction) {
    printf("Do less\n");
    int r = 5;
    
    P_SetFailMessage("Could not set yes().");
    P_ASSERT(5==5);
    yes();
    
    printf("If successful, this should print.");
    
    return TEST_SUCCESS;
}

int yes() {
    P_ASSERT(5 == 7);
}

//=================================================================

typedef struct {
    int r;
    char* str;
} TestFixtureData;

TestFixtureData* create_testdata() {
    TestFixtureData* data = (TestFixtureData*)malloc(sizeof(TestFixtureData));
    data->r = 42;
    data->str = "Hello I'm set from a test fixture!";
    printf("Hello from inside test fixture init.\n");
    return data;
}
void free_testdata(TestFixtureData* data) {
    printf("Destroying...\n");
    if(data) {
        free(data);
    }
}

// You could pass free for the destroy function here, but the truth is, these functions don't even need to really allocate anything
// but can be used for set-up and teardown purposes rather uniformly for tests that will require the same starting and ending
// behavior, creating a fixture of sorts in C.
P_TEST_F(TestFixtureTest, create_testdata, free_testdata, NULL) {
    
    TestFixtureData *fixture_data = (TestFixtureData*)data;
    printf("This test should pass!\n");
    printf("%s\n",fixture_data->str);
    P_ASSERT(fixture_data->r == 42);
    return TEST_SUCCESS;
}

//=================================================================

TestFixtureData stack_allocdata = { 42, "I'm on the stack!" };

P_TEST_F(TestFixtureTestPassInData, NULL, NULL, &stack_allocdata) {
    
    TestFixtureData *fixture_data = (TestFixtureData*)data;
    printf("%s\n", fixture_data->str);
    
    P_ASSERT(fixture_data->r == 42);
    
    return TEST_SUCCESS;
}

void* file_create() {
	printf("[Setup:] Creating testfile...\n");
    FILE *fd = fopen("testfile","wb");
    return fd;
}

void file_cleanup(void *data) {
	printf("[Cleanup:] Removing testfile...\n");
    remove("testfile");
}

P_TEST_F(FileIO, file_create, file_cleanup, NULL) {
    FILE* fd = (FILE*)data;
    P_ASSERT(fd != NULL);
    P_ASSERT(fputs("This is a test", fd) >= 0);
}

int main(int argc, char **argv) {
    printf("==============\n[TEST BATTERY]\n==============\n\n");
    
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        printf("Usage: ptest_test [wildcard]\n\nwildcard: Run only those tests whose names contain this partial string.\n\n");
        return 1;
    }
    
    if(argc >= 2) {
        printf("Running all tests containing \"%s\"...\n\n", argv[1]);
        return P_RunTestsWith(argv[1]);
    }
    else
        return P_RunTests();
    
    // Only run tests with the substring "Fail" somewhere in the test name.
    //return P_RunTestsWith("Fail");
}