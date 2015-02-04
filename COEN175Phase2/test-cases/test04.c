int *f(int *s, int *t)
{
    int *p;

    p = s;

    while (*t != 0) {
	*p = *t;
	p = p + 1;
	t = t + 1;
    }

    *p = 0;
    return s;
}
