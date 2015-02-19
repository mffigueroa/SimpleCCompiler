int foo();
int bar(char z){ }
int spam(void){ }
int x;

int main (void)
{
	char z;
	foo();
	bar();
	spam();
	foo(z);
	bar(z);
	spam(z);
	x();

	return 0;
}
