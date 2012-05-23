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
	    "\tmove    %0, $2\n"
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

/*
   typedef long Align;    // for alignment to long boundary 

   union header {         // block header 
       struct {
           union header *ptr; // next block if on free list 
           unsigned size;     // size of this block 
       } s;
       Align x;           // force alignment of blocks 
   };

   typedef union header Header;


static Header base;       // empty list to get started 
   static Header *freep = NULL;     // start of free list 

   // malloc:  general-purpose storage allocator 
   void *malloc(unsigned nbytes)
   {
       Header *p, *prevp;
       Header *moreroce(unsigned);
       unsigned nunits;

       nunits = (nbytes+sizeof(Header)-1)/sizeof(header) + 1;
       if ((prevp = freep) == NULL) {   // no free list yet  
           base.s.ptr = freeptr = prevptr = &base;
           base.s.size = 0;
       }
       for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
           if (p->s.size >= nunits) {  // big enough 
               if (p->s.size == nunits)  // exactly 
                   prevp->s.ptr = p->s.ptr;
               else {              // allocate tail end 
                   p->s.size -= nunits;
                   p += p->s.size;
                   p->s.size = nunits;
               }
               freep = prevp;
               return (void *)(p+1);
           }
           if (p == freep)  // wrapped around free list 
               if ((p = morecore(nunits)) == NULL)
                   return NULL;    // none left 
       }
   }

   #define NALLOC  1024   // minimum #units to request 

   // morecore:  ask system for more memory 
   static Header *morecore(unsigned nu)
   {
       char *cp, *sbrk(int);
       Header *up;

       if (nu < NALLOC)
           nu = NALLOC;
       cp = sbrk(nu * sizeof(Header));
       if (cp == (char *) -1)   // no space at all 
           return NULL;
       up = (Header *) cp;
       up->s.size = nu;
       free((void *)(up+1));
       return freep;
   }

 // free:  put block ap in free list 
   void free(void *ap)
   {
       Header *bp, *p;

       bp = (Header *)ap - 1;    // point to  block header 
       for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
            if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
                break;  // freed block at start or end of arena 

       if (bp + bp->size == p->s.ptr) {    // join to upper nbr 
           bp->s.size += p->s.ptr->s.size;
           bp->s.ptr = p->s.ptr->s.ptr;
       } else
           bp->s.ptr = p->s.ptr;
       if (p + p->size == bp) {            // join to lower nbr 
           p->s.size += bp->s.size;
           p->s.ptr = bp->s.ptr;
       } else
           p->s.ptr = bp;
       freep = p;
   }
*/
