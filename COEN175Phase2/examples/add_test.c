int main (void)
{
	int x, y, *p, *q, r[5], **s;

	x+y;    
	x+p;
	p+x;
	p+q;    /* illegal */
	
	x-y;
	x-p;    /* illegal */
	p-x;
	p-q;

	p-r;
	p-s;    /* illegal */



}
