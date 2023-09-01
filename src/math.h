#ifndef MATH_H_
#define MATH_H_

static inline int mod(int n, int m) {
	// return ((n % m) + m) % m;
	// return (n/m)*m + n%m;
	return ((n % m) + m) % m;
	//      -1 %10  +10  % 10
	//        -1  +10  % 10
	//        9  % 10
}

static inline int div_rd(int n, int m) {
	if (n >= 0) return n/m;
	else return (n-m+1)/m;
}

#endif // MATH_H_

