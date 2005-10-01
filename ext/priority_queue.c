#include "ferret.h"

ID less_than;
/****************************************************************************
 *
 * PriorityQueue Methods
 *
 ****************************************************************************/

void
frt_priq_free(void *p)
{
  free(p);
}

static VALUE
frt_priq_alloc(VALUE klass)
{
	PriorityQueue *priq;
	priq = (PriorityQueue *)ALLOC(PriorityQueue);
	priq->len = 0;
	priq->size = 0;
	VALUE rpriq = Data_Wrap_Struct(klass, NULL, frt_priq_free, priq);
	return rpriq;
}

static VALUE
frt_priq_init(VALUE self, VALUE rsize)
{
	VALUE heap;
	PriorityQueue *priq;
	
	int size = FIX2INT(rsize);
	heap = rb_ary_new2(size+1);
	Data_Get_Struct(self, PriorityQueue, priq);
	
	priq->heap = RARRAY(heap)->ptr;
	priq->size = size;
	rb_iv_set(self, "@heap", heap);
	return self;
}

void
priq_up(PriorityQueue *priq, VALUE self, VALUE rary)
{
	int i,j;
	VALUE *heap, node;
	
	i = priq->len;
	heap = priq->heap;
	node = heap[i];
	j = i >> 1;
	while((j > 0) && rb_funcall(self, less_than, 2, node, heap[j])){
		heap[i] = heap[j];
		i = j;
		j = j >> 1;
	}
	rb_ary_store(rary, i, node);
}

void
priq_down(PriorityQueue *priq, VALUE self, VALUE rary)
{
	int i, j, k, len;
	VALUE *heap, node;
	
	i = 1;
	heap = priq->heap;
	len = priq->len;
	node = heap[i];
	j = i << 1;
	k = j + 1;

	if ((k <= len) && rb_funcall(self, less_than, 2, heap[k], heap[j]))
		j = k;

	while((j <= len) && rb_funcall(self, less_than, 2, heap[j], node)){
		heap[i] = heap[j];
		i = j;
		j = i << 1;
		k = j + 1;
		if((k <= len) && rb_funcall(self, less_than, 2, heap[k], heap[j]))
			j = k;
	}
	rb_ary_store(rary,i, node);
}

static VALUE
frt_priq_push(VALUE self, VALUE e)
{
	PriorityQueue *priq;
	Data_Get_Struct(self, PriorityQueue, priq);
	int len = priq->len;
	VALUE rary = rb_iv_get(self, "@heap");
	
	len++;
	rb_ary_store(rary, len, e);
	priq->len = len;
	priq_up(priq, self, rary);
	
	return Qnil;
}

static VALUE
frt_priq_insert(VALUE self, VALUE e)
{
	PriorityQueue *priq;
	VALUE *heap, rary;
	int len, size;
	rary = rb_iv_get(self, "@heap");
	
	Data_Get_Struct(self, PriorityQueue, priq);
	len = priq->len;
	size = priq->size;
	heap = priq->heap;
	
	if(len > size){
		frt_priq_push(self, e);
		return 1;
	} else if((len > 0) && !rb_funcall(self, less_than, 2, e, heap[1])){
		heap[1] = e;
		priq_down(priq, self, rary);
		return 1;
	} else
		return 0;
}

static VALUE
frt_priq_top(VALUE self)
{
	PriorityQueue *priq;
	VALUE *heap;
	int len;
	
	Data_Get_Struct(self, PriorityQueue, priq);
	len = priq->len;
	heap = priq->heap;

	if(len > 0)
		return heap[1];
	else
		return Qnil;
}

static VALUE
frt_priq_pop(VALUE self)
{
	PriorityQueue *priq;
	VALUE *heap, rary;
	int len;
	
	Data_Get_Struct(self, PriorityQueue, priq);
	rary = rb_iv_get(self, "@heap");
	len = priq->len;
	heap = priq->heap;

	if(len > 0){
		VALUE res = heap[1];
		heap[1] = heap[len];
		heap[len] = Qnil;
		len--;
		priq->len = len;
		priq_down(priq, self, rary);
		return res;
	} else
		return Qnil;
}

static VALUE
frt_priq_clear(VALUE self)
{
	PriorityQueue *priq;
	Data_Get_Struct(self, PriorityQueue, priq);
	VALUE heap = rb_ary_new2(priq->size);
	rb_iv_set(self, "@heap", heap);
	priq->heap = RARRAY(heap)->ptr;
	priq->len = 0;
	return Qnil;
	
}

static VALUE
frt_priq_size(VALUE self)
{
	PriorityQueue *priq;
	Data_Get_Struct(self, PriorityQueue, priq);
	return INT2FIX(priq->len);
}

static VALUE
frt_priq_adjust_top(VALUE self)
{
	PriorityQueue *priq;
	Data_Get_Struct(self, PriorityQueue, priq);
	VALUE rary = rb_iv_get(self, "@heap");
	
	priq_up(priq, self, rary);
	
	return Qnil;
}


/****************************************************************************
 *
 * Init Function
 *
 ****************************************************************************/

void
Init_priority_queue(void)
{
	less_than = rb_intern("less_than");
	
	cPriorityQueue = rb_define_class_under(mUtils, "PriorityQueue", rb_cObject);
	rb_define_alloc_func(cPriorityQueue, frt_priq_alloc);

	rb_define_method(cPriorityQueue, "initialize", frt_priq_init, 1);
	rb_define_method(cPriorityQueue, "pop", frt_priq_pop, 0);
	rb_define_method(cPriorityQueue, "top", frt_priq_top, 0);
	rb_define_method(cPriorityQueue, "clear", frt_priq_clear, 0);
	rb_define_method(cPriorityQueue, "insert", frt_priq_insert, 1);
	rb_define_method(cPriorityQueue, "push", frt_priq_push, 1);
	rb_define_method(cPriorityQueue, "size", frt_priq_size, 0);
	rb_define_method(cPriorityQueue, "adjust_top", frt_priq_adjust_top, 0);
}
