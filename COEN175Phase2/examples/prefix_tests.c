int foo();

int main (void)
{
	char a, *p, t[5];
	int b, *q, ***s;
	long c, *r;

	/* Prefix tests for * operator */
	*a;			/* invalid E5 */
	*b;			/* invalid E5 */
	*c;			/* invalid E5 */
	*foo();		/* invalid E5 */
	*p;
	*q;
	*r;
	*t;
	*s;
	**s;
	***s;

	/* Prefix tests for & operator */
	&foo(); 	/* invalid E3 */	
	&t; 		/* invalid E3 */	
	&10; 		/* invalid E3 */	
	&q;
	&a;

	/* Prefix tests for ! operator */
	!(a||b);
	!foo();
	!foo;		/* invalid E5 */
	!t;
	!c;
	!s;

	/* Prefix tests for - operator */
	-a;
	-b;
	-c;
	-p;			/* invalid E5 */
	-t;			/* invalid E5 */
	-foo();
	-foo;		/* invalid E5 */

	/* Prefix tests for Sizeof operator */
	sizeof a;
	sizeof (a);
	sizeof b;
	sizeof c;
	sizeof p;
	sizeof (p);
	sizeof r;
	sizeof s;
	sizeof (foo); /* invalid */
	sizeof foo;	  /* invalid */
	sizeof (int **);
	

}
