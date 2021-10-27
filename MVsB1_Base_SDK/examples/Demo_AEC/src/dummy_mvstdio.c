#include "type.h"
#include "mvstdio.h"

static mv_mread_callback_t mv_mread_callback = 0;

typedef struct
{
    uint8_t* address;     /**< memory base address      */
    uint32_t capacity;    /**< memory capacity in bytes */
    uint32_t wp;          /**< write point in bytes     */
    uint32_t rp;          /**< read point in bytes      */
}CircleBufferContext;

void mv_mopen(MemHandle* mp, uint8_t* addr, uint32_t capacity, mv_mread_callback_t callback)
{
    mp->addr              = addr;
    mp->mem_capacity      = capacity;
    mp->mem_len           = 0;
    mp->p                 = 0;
    mv_mread_callback     = callback;
}

void mv_mclose(MemHandle* mp)
{
    mp->addr              = 0;
    mp->mem_capacity      = 0;
    mp->mem_len           = 0;
    mp->p                 = 0; 
    mv_mread_callback     = 0;
}

void mv_mread_callback_set(mv_mread_callback_t callback)
{
    mv_mread_callback = callback;
}

void mv_mread_callback_unset(void)
{
    mv_mread_callback = 0;
}

uint32_t mv_mread(void* _buffer, uint32_t size, uint32_t count, void *mp)
{
    CircleBufferContext* cbc = (CircleBufferContext*)mp;

    uint8_t* buffer = (uint8_t*)_buffer;
    uint32_t required_bytes = size * count;
    uint32_t read_bytes;
    uint32_t remain_bytes;
    uint32_t wp = cbc->wp;

    if(required_bytes == 0) return 0;

    if(wp >= cbc->rp)
    {
        remain_bytes = wp - cbc->rp;

        if(required_bytes > remain_bytes)
        {
            read_bytes = remain_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);
            cbc->rp += read_bytes;

            if(mv_mread_callback)
            {
                read_bytes += mv_mread_callback(((uint8_t*)buffer) + read_bytes, required_bytes - read_bytes);
            }
        }
        else
        {
            read_bytes = required_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);
            cbc->rp += read_bytes;
        }
    }
    else
    {
        remain_bytes = cbc->capacity - cbc->rp;

        if(required_bytes > remain_bytes)
        {
            read_bytes = remain_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);

            if(required_bytes - read_bytes > wp)
            {
                memcpy(buffer + read_bytes, &cbc->address[0], wp);
                cbc->rp = wp;
                read_bytes += wp;

                if(mv_mread_callback)
                {
                    read_bytes += mv_mread_callback(((uint8_t*)buffer) + read_bytes, required_bytes - read_bytes);
                }
            }
            else
            {
                memcpy(buffer + read_bytes, &cbc->address[0], required_bytes - read_bytes);
                cbc->rp = required_bytes - read_bytes;
                read_bytes = required_bytes;
            }
        }
        else
        {
            read_bytes = required_bytes;
            memcpy(buffer, &cbc->address[cbc->rp], read_bytes);
            cbc->rp += read_bytes;
        }
    }

    return read_bytes;
}

#define RWP_SAFE_INTERVAL   (1)

uint32_t mv_mwrite(void* _buffer, uint32_t size, uint32_t count, void* mp)
{
    CircleBufferContext* cbc = (CircleBufferContext*)mp;

    uint8_t* buffer = (uint8_t*)_buffer;
    uint32_t remain_bytes;
    uint32_t write_bytes = size * count;
    uint32_t rp = cbc->rp;

    if(cbc->wp >= rp)
    {
        remain_bytes = cbc->capacity - cbc->wp + rp;

        if(remain_bytes >= write_bytes + RWP_SAFE_INTERVAL)
        {
            remain_bytes = cbc->capacity - cbc->wp;

            if(remain_bytes >= write_bytes)
            {
                memcpy(&cbc->address[cbc->wp], buffer, write_bytes);
                cbc->wp += write_bytes;
            }
            else
            {
                memcpy(&cbc->address[cbc->wp], buffer, remain_bytes);
                cbc->wp = write_bytes - remain_bytes;
                memcpy(&cbc->address[0], &buffer[remain_bytes], cbc->wp);
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        remain_bytes = rp - cbc->wp;

        if(remain_bytes >= write_bytes + RWP_SAFE_INTERVAL)
        {
            memcpy(&cbc->address[cbc->wp], buffer, write_bytes);
            cbc->wp += write_bytes;
        }
        else
        {
            return 0;
        }
    }

    if(cbc->wp >= cbc->capacity && cbc->rp)
    {
        cbc->wp = 0;
    }

    return write_bytes;
}

int32_t mv_meom(void *mp)
{
    return 0;//((MemHandle*)mp)->p>=((MemHandle*)mp)->mem_len ? 1 : 0;
}

int32_t mv_msize(void *mp)
{
    CircleBufferContext* cbc = (CircleBufferContext*)mp;

    return cbc->wp >= cbc->rp ? cbc->wp - cbc->rp : cbc->capacity - cbc->rp + cbc->wp;
}

int32_t mv_mremain(void* mp)
{
    int32_t remain = ((CircleBufferContext*)mp)->capacity - mv_msize(mp);

    return remain > RWP_SAFE_INTERVAL ? remain - RWP_SAFE_INTERVAL : 0;
}


