#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 64

typedef struct {
    uint8_t *rd_ptr;
    uint8_t *wr_ptr;
    uint8_t arr[BUFFER_SIZE];
    bool full;
} ringBuffer_t;

/*
Example usage
ringBuffer_t* rb = ringBufferInit();
ringBufferWrite(rb, 'h');
ringBufferWrite(rb, 'e');
.....
*/

static inline ringBuffer_t* ringBufferInit(void)
{
    ringBuffer_t* rb = malloc(sizeof(ringBuffer_t));
    
    //initialize pointers
    rb->wr_ptr = &(rb->arr[0]); //write and read pointers both start at first index
    rb->rd_ptr = &(rb->arr[0]);
    rb->full = false;
    memset(rb->arr, 0, BUFFER_SIZE);
    return rb;
}

static inline bool ringBufferIsEmpty(ringBuffer_t *buffer)
{
    
    return (buffer->wr_ptr == buffer->rd_ptr && buffer->full == false);
}

static inline void ringBufferWrite(ringBuffer_t *buffer, uint8_t byteToWrite)
{
    ringBuffer_t *rb = buffer;

    if (rb->full == true)
    {
        // Drop the oldest byte to make room for the new one.
        rb->rd_ptr += 1;

        //wrap the read pointer index around the circle if it's reached end
        if (rb->rd_ptr > &(rb->arr[BUFFER_SIZE - 1]))
        {
            rb->rd_ptr = &(rb->arr[0]);
        }
    }

    *(rb->wr_ptr) = byteToWrite; //write the byte into write index

    rb->wr_ptr += 1;//increase write index

    //make sure write pointer isn't out of bounds,and wrap around if it is. 
    if (rb->wr_ptr > &(rb->arr[BUFFER_SIZE - 1]))
    {
        rb->wr_ptr = &(rb->arr[0]);
    }

    // If the read and write pointers are the same set full by default //
    if (rb->wr_ptr == rb->rd_ptr)
    {
        rb->full = true; //we're setting this after advancing the read pointer, so we know that we've written at least one byte, can't be empty.
    }
}

/*
 * NOTE:
 * This function returns 0 when the buffer is empty.
 *
 * For the current UART implementation, this is acceptable because the ring
 * buffer is used exclusively for ASCII command reception. Commands are
 * expected to consist of printable ASCII characters and are terminated by
 * a newline character ('\n'). Therefore, 0x00 is not expected to appear as
 * valid command data.
 *
 * As a result, a return value of 0 can safely be interpreted as "no data
 * available" for the current application.
 *
 * If this ring buffer is later reused for arbitrary binary data (DMA
 * transfers, sensor data, binary protocols, etc.), 0x00 may become a valid
 * payload byte. In that case, the API should be redesigned to return a
 * success/failure status (e.g. bool) and provide the read byte through an
 * output parameter.
 */

static inline uint8_t ringBufferRead(ringBuffer_t *buffer)
{
    ringBuffer_t *rb = buffer;

    //if the buffer is empty, no read is possible. we know that an empty buffer is characterized by the read and write pointers pointing to the same element of the array

    if (rb->wr_ptr != rb->rd_ptr || rb->full == true)
    {
        //first make space for the data we are reading from the buffer
        uint8_t myByte = *(rb->rd_ptr); //read at the read buffer. This may be different than the write ptr, or, if it is the same as the write pointer, we want to make
        //sure that we are at the buffer full condition because there must be data that needs to be read to actually read anything. otherwise we are reading from an empty buffer

        rb->rd_ptr += 1; //now advance for next element

        rb->full = false; //if it was full before, it isnt anymore because we just read

        //now just make sure that the rd pointer isn't out of bounds, because we need to wrap it around the circle if it is
        if (rb->rd_ptr > &(rb->arr[BUFFER_SIZE - 1]))
        {
            rb->rd_ptr = &(rb->arr[0]); //wrap back around to first element.
        }

        return myByte;
    }

    return 0;
}

static inline void RingBufferClear(ringBuffer_t* rb)
{
    rb->wr_ptr = &(rb->arr[0]);
    rb->rd_ptr = &(rb->arr[0]);
    rb->full = false;
    memset(rb->arr, 0, BUFFER_SIZE);
}

static inline void RingBufferDispose(ringBuffer_t* rb)
{
    free(rb);
}

#endif
