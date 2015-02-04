char*** func1();
int**** func2();
long**** func3();

char*** func1(char*** a, int**** b, long**** c)
{
char d, *e;
int f, *g, **h;
long i, *j, **k;

d = '0';
e = &d;
a[10] = &e;

f = 5555;
g = &f;
h = &g;
b[10] = &h;

i = 10;
j = &i;
k = &j;
c[15] = &k;

return &a[10];
}