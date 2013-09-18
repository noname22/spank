#include <dep_a.h>
#include <dep_b.h>

int main()
{
	int a = dep_a();
	int b = dep_b();
	return a + b;
}
