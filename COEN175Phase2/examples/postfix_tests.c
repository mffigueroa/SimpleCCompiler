int foo();

int main (void)
{
	char a, *p, t[5];
	int b, *q, ***s;
	long c, r[5];

	t[0];
	t[0l];
	t['a'];
	t[0][0];			/* invalid E4 */
	s[0];
	s[0][0][0];
	s[0][0][0][0];		/* invalid E4 */
	a[0];				/* invalid E4 */
	c[0];				/* invalid E4 */
	r[0];

	return 0;
}
