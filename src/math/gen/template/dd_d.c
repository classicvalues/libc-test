#include <stdint.h>
#include <stdio.h>
#include "util.h"

static struct dd_d t[] = {
HEADERS
};

int main(void)
{
	double y;
	float d;
	int e, i, err = 0;
	struct dd_d *p;

	for (i = 0; i < sizeof t/sizeof *t; i++) {
		p = t + i;
		setupfenv(p->r);
		y = ___(p->x, p->x2);
		e = getexcept();
		if (!checkexcept(e, p->e, p->r)) {
			printf("%s:%d: bad fp exception: %s ___(%a,%a)=%a, want %s",
				p->file, p->line, rstr(p->r), p->x, p->x2, p->y, estr(p->e));
			printf(" got %s\n", estr(e));
			err++;
		}
		d = ulperr(y, p->y, p->dy);
		if (!checkulp(d, p->r)) {
			printf("%s:%d: %s ___(%a,%a) want %a got %a ulperr %.3f = %a + %a\n",
				p->file, p->line, rstr(p->r), p->x, p->x2, p->y, y, d, d-p->dy, p->dy);
			err++;
		}
	}
	return !!err;
}
