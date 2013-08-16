#include "VMEStream.h"

#include <stdio.h>
#include <assert.h>

int main(int argc, char* argv[])
{
    CircularBuffer *a1 = cbuffer_new();
    CircularBuffer *b1 = cbuffer_new();
    CircularBuffer *a2 = cbuffer_new();
    CircularBuffer *b2 = cbuffer_new();

    VMEStream *test1 = vmestream_initialize(a1, b1);
    VMEStream *test2 = vmestream_initialize(a2, b2);

    test2->rx_size = test1->tx_size;
    test2->tx_size = test1->rx_size;
    test2->rx_data = test1->tx_data;
    test2->tx_data = test1->rx_data;

    cbuffer_push_back(a1, 0xDEADBEEF);
    vmestream_transfer_data(test1);
    vmestream_transfer_data(test2);

    assert(0xDEADBEEF == cbuffer_pop_front(a1));
    assert(0 == cbuffer_size(a1));

    return 0;
}
