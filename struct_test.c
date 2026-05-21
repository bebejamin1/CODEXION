#include <stdio.h>
#include <stdalign.h>

struct test {
    void *p;
    long long ll;
    long double ld;
};


int main(void)
{
	struct test t = {.p = "ggrgsgsgesgsgsegrsgsgsgsgsgsgsgesgesgsgsegesgsgsegss", .ll = 56666, .ld = 15.1261};
	printf("%zu\n", _Alignof(char));
	printf("%zu\n", sizeof(t));
	printf("%zu\n", _Alignof(t));
	// pk ca a la meme taille que une structure non instancier ?
}