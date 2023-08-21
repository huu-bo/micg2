#ifndef TEST_H_
#define TEST_H_

#include <stdio.h>

#define TEST_EQ(a, b) if ((a) != (b)) {fprintf(stderr, "%s != %s, %d != %d\n", (#a), (#b), (a), (b)); return 1;} else (void)0

#endif // TEST_H_
