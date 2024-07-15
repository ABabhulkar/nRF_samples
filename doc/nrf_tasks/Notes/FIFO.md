It enables threads and Interrupts Service Routines (ISRs) to add or remove **any number of data items** of **varying sizes** from the queue structure. You don’t need to specify the number of items in a FIFO or their size statically. Instead, you typically need to use the heap memory (`k_malloc()` and `k_free()`) to allocate space for the items on the fly.![[Pasted image 20240715155507.png]]

#### How to use?
1. **Specify the size of the heap memory pool to store the items.**
   By default, `CONFIG_HEAP_MEM_POOL_SIZE` is set to zero. Therefore you need to set it to a value that represents the maximum number of elements you can have in the FIFO at a given time.

2. **Define the FIFO.**
   You can use the [K_FIFO_DEFINE()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/data_passing/fifos.html#c.K_FIFO_DEFINE) macro to statically define a FIFO.

3. **Specify the data type of the items.**
   It is important to define the data item as a struct where the first member is always a reserved void pointer. This is needed because the FIFO is implemented internally by the kernel as a simple linked list and the first word of an item should be reserved for use as a pointer to the next data item in the FIFO. The first example below shows a data item struct where the data has a defined size.
   ```c
struct data_item_t {
	void *fifo_reserved;
	uint8_t  data[256];
	uint16_t len;
};
```

4. **Add a data item to the FIFO.**
   Add the item with the [k_fifo_put()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/data_passing/fifos.html#c.k_fifo_put) function. The API is shown below: 
   ```c
/* create data item to send */
struct data_item_t *buf = k_malloc(sizeof(struct data_item_t));
if (buf == NULL){
/* Unable to locate memory from the heap */
  return ;
}
/* Populate the data item. This is usually done using memcpy() */

/* send data to consumers */
k_fifo_put(&my_fifo,buf);
```

5. **Read a data item from a FIFO.**
   Read the item with the [k_fifo_get()](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/services/data_passing/fifos.html#c.k_fifo_get) function. Calling `k_fifo_get()` removes the data from the FIFO. However, **the developer must handle removing allocated memory** from the heap memory by calling `k_free()` afterwards.
   When a data item is added, it is given to the highest priority thread that has waited the longest.
   ```c
struct data_item_t *rec_item = k_fifo_get(&my_fifo, K_FOREVER);
/* process FIFO data item */
/*Free */
k_free(rec_item);
```

> [!Important]
> You don’t have to use the heap with FIFO if you have reservations about using dynamic memory allocation in your code. You can choose to allocate the memory needed for the items statically.  
> 
> Also, it’s important to remember that you can NOT add the same data item twice in a FIFO. This is likely to break the linked list used by the FIFO internally, and results in undefined behavior.

