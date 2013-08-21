#include "VMEStream.h"

#include <stdio.h>
#include <assert.h>

int main(int argc, char* argv[])
{
    // local application buffers
    CircularBuffer *tx1 = cbuffer_new();
    CircularBuffer *rx1 = cbuffer_new();
    CircularBuffer *tx2 = cbuffer_new();
    CircularBuffer *rx2 = cbuffer_new();

    VMEStream *test1 = vmestream_initialize(tx1, rx1, 1);
    VMEStream *test2 = malloc(sizeof(VMEStream));
    test2->input = tx2;
    test2->output = rx2;

    test2->rx_size = test1->tx_size;
    test2->tx_size = test1->rx_size;
    test2->rx_data = test1->tx_data;
    test2->tx_data = test1->rx_data;
    test2->MAXRAM  = test1->MAXRAM;

    for (unsigned int i = 0; i < 20; ++i) {
        // put some output data on host #1
        cbuffer_push_back(tx1, 0xDEADBEEF + i);
        // put some output data on host #2
        cbuffer_push_back(tx2, 0xBEEFCAFE + i);
    }

    // do a transfer
    vmestream_transfer_data(test1); // step 1 
    vmestream_transfer_data(test2); // step 2

    // host #2 has received data, since host #1 filled it's TX buffer in step 1
    // and host #2 can read it out in step 2
    assert(0xDEADBEEF == cbuffer_pop_front(rx2));

    vmestream_transfer_data(test1); // step 3
    // now host #1 can read the data loaded by host #2 in step 2
    assert(0xBEEFCAFE == cbuffer_pop_front(rx1));

    // do another transfer
    vmestream_transfer_data(test2);
    vmestream_transfer_data(test1);

    assert(0xBEEFCAFE + 1 == cbuffer_pop_front(rx1));
    assert(0xDEADBEEF + 1 == cbuffer_pop_front(rx2));

    // We have consumed all received data (via pop).  There is a word of 
    // data in limbo for host #1
    assert(0 == cbuffer_size(rx1));
    assert(0 == cbuffer_size(rx2));
    assert(17 == cbuffer_size(tx1));
    assert(18 == cbuffer_size(tx2));

    // call transfer on #1 twice in a row.  Since it's still waiting for
    // #2 to read the data, nothing happens.
    vmestream_transfer_data(test1);
    assert(0 == cbuffer_size(rx1));
    assert(0 == cbuffer_size(rx2));
    assert(17 == cbuffer_size(tx1));
    assert(18 == cbuffer_size(tx2));

    // #2 receives limbo data, puts one of it's words in limbo.
    vmestream_transfer_data(test2);
    assert(0 == cbuffer_size(rx1));
    assert(1 == cbuffer_size(rx2));
    assert(17 == cbuffer_size(tx1));
    assert(17 == cbuffer_size(tx2));

    return 0;
}
