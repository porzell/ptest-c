#!/bin/bash
rm ptest_test; gcc ptest.c example.c -o ptest_test -ggdb -DPTEST_ENABLE_TRACE -DPTEST_TRACE_DEPTH=15
