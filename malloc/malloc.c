#define NALLOC 1024

#define NULL ((void*) 0)

typedef struct header
{
	struct header* ptr;
	unsigned int   size;
} header;

static header  base;
static header* freep = NULL;

void*   malloc(unsigned int);
header* morecore(unsigned int);
char*   sbrk(int);
void    free(void*);

void* malloc(unsigned int nbytes)
{
	unsigned int nunits = (nbytes + sizeof(header) - 1) / sizeof(header) + 1;
	
	header* prevp = freep;
	if (!prevp)
	{
		base.ptr  = &base;
		freep     = &base;
		prevp     = &base;
		base.size = 0;
	}
	
	header* p = prevp->ptr;
	while (42)
	{
		if (p->size >= nunits)
		{
			if (p->size == nunits)
				prevp->ptr = p->ptr;
			else
			{
				p->size -= nunits;
				p       += p->size;
				p->size  = nunits;
			}
			
			freep = prevp;
			return (void*) (p + 1);
		}
		
		if (p == freep)
		{
			p = morecore(nunits);
			if (!p)
				return NULL;
		}
		
		prevp = p;
		p = p->ptr;
	}
}

header* morecore(unsigned int nu)
{
	if (nu < NALLOC)
		nu = NALLOC;
	
	register header* up;
	register int n = nu * sizeof(header);
	asm("\tmove    $4, %1\n"
	    "\tsyscall 9\n"
	    "\tmove    %1, $2\n"
	    : "=r"(up)
	    : "r"(n)
	    : "$2", "$4"
	);
	
	if (!up)
		return NULL;
	
	up->size = nu;
	free((void*) (up + 1));
	return freep;
}

void free(void* ap)
{
	header* bp = (header*) ap - 1;
	
	header* p;
	for (p = freep; !(bp > p && bp < p->ptr); p = p->ptr)
		if (p >= p->ptr && (bp > p || bp < p->ptr))
			break;
	
	if (bp + bp->size == p->ptr)
	{
		bp->size += p->ptr->size;
		bp->ptr   = p->ptr->ptr;
	}
	else
		bp->ptr = p->ptr;
	
	if (p + p->size == bp)
	{
		p->size += bp->size;
		p->ptr   = bp->ptr;
	}
	else
		p->ptr = bp;
	
	freep = p;
}
