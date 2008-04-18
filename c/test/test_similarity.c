#include "similarity.h"
#include "test.h"

void test_default_similarity(TestCase *tc, void *data)
{
  Similarity *dsim = sim_create_default();
  (void)data;

  Afequal(1.0/4, sim_length_norm(dsim, 0, 16));
  Afequal(1.0/4, sim_query_norm(dsim, 16));
  Afequal(3.0, sim_tf(dsim, 9));
  Afequal(1.0/10, sim_sloppy_freq(dsim, 9));
  Afequal(1.0, sim_idf(dsim, 9, 10));
  Afequal(4.0, sim_coord(dsim, 12, 3));
}

TestSuite *ts_similarity(TestSuite *suite)
{
    suite = ADD_SUITE(suite);

    tst_run_test(suite, test_default_similarity, NULL);

    return suite;
}
