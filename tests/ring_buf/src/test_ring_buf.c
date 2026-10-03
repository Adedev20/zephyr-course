/*
 * Ring Buffer Module - Homework Test Skeleton
 *
 * test_fresh_state is provided as a worked example. Fill in the remaining
 * 7 ZTEST bodies according to TEST_SPEC.md. Stubs call ztest_test_skip()
 * so the binary builds and runs cleanly before each test is implemented.
 *
 * Run:
 *   west twister -T tests/ring_buf -p native_sim
 */

#include <zephyr/ztest.h>
#include <errno.h>

#include "ring_buf.h"

/*
 * Shared before hook: every suite reinitialises the ring buffer with a
 * capacity of 4 so tests start from a clean, known state. Capacity 4 is
 * enough to exercise FIFO order (push 1, 2, 3) and overflow (full at 4).
 */
static void before(void *f)
{
	ARG_UNUSED(f);
	rb_init(4);
}

/*
 * ============================================================================
 * Test Suite: ring_buf_init
 *
 * Initial state and re-initialization behaviour.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_init, NULL, NULL, before, NULL, NULL);

/* PROVIDED — study this test before writing the rest. */
ZTEST(ring_buf_init, test_fresh_state)
{
	zassert_true(rb_is_empty(), "Fresh buffer must be empty");
	zassert_equal(rb_count(), 0, "Fresh buffer count must be 0");
}

ZTEST(ring_buf_init, test_reinit_clears_state)
{
	int val;
	
	/* Push an initial value into the buffer to alter internal indexes */
	int ret = rb_push(10);
	zassert_equal(ret, 0, "Initial push failed during reinitialization setup");
	zassert_false(rb_is_empty(), "Buffer should report not empty after single push");
	zassert_equal(rb_count(), 1, "Buffer count should be 1 after single push");

	/* Trigger reinitialization to clear state out cleanly */
	rb_init(4);

	/* Verify the buffer has dropped back down to initial empty states */
	zassert_true(rb_is_empty(), "Reinitialization failed to mark the buffer empty");
	zassert_equal(rb_count(), 0, "Reinitialization failed to drop count bounds to 0");
	
	/* Attempting to pop must now fail since it should be completely blank */
	ret = rb_pop(&val);
	zassert_not_equal(ret, 0, "Pop allowed processing reading from a freshly reset context");
}

/*
 * ============================================================================
 * Test Suite: ring_buf_push_pop
 *
 * Single push/pop round-trip, FIFO order, full error path.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_push_pop, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_push_pop, test_single_push_pop)
{
	int v = 0;
	
	/* Perform single value assignment checks */
	int ret = rb_push(42);
	zassert_equal(ret, 0, "Push operation failed to store value 42 inside allocation bounds");

	/* Extract value and assert data integration metrics stay intact */
	ret = rb_pop(&v);
	zassert_equal(ret, 0, "Pop operation failed unexpectedly while extracting valid index item");
	zassert_equal(v, 42, "Extracted data variable contents corrupt! Expected 42, got %d", v);
	
	/* Check boundary metrics are cleared */
	zassert_true(rb_is_empty(), "Buffer state tracks residual items remaining after matching pop calls");
}

ZTEST(ring_buf_push_pop, test_fifo_order)
{
	int v1 = 0, v2 = 0, v3 = 0;

	/* Sequential waterfall push array simulation mapping 1, 2, 3 */
	zassert_equal(rb_push(1), 0, "Failed to push item 1");
	zassert_equal(rb_push(2), 0, "Failed to push item 2");
	zassert_equal(rb_push(3), 0, "Failed to push item 3");

	/* Extract and verify First-In, First-Out sequence preservation */
	zassert_equal(rb_pop(&v1), 0, "Failed to extract index 0 element");
	zassert_equal(v1, 1, "First structural layout queue extraction broken. Expected 1, got %d", v1);

	zassert_equal(rb_pop(&v2), 0, "Failed to extract index 1 element");
	zassert_equal(v2, 2, "Second structural layout queue extraction broken. Expected 2, got %d", v2);

	zassert_equal(rb_pop(&v3), 0, "Failed to extract index 2 element");
	zassert_equal(v3, 3, "Third structural layout queue extraction broken. Expected 3, got %d", v3);
}

ZTEST(ring_buf_push_pop, test_push_full_returns_enospc)
{
	/* Pack the buffer elements up to the explicit structural capacity boundary (4) */
	zassert_equal(rb_push(100), 0, "Failed to load block 1");
	zassert_equal(rb_push(200), 0, "Failed to load block 2");
	zassert_equal(rb_push(300), 0, "Failed to load block 3");
	zassert_equal(rb_push(400), 0, "Failed to load block 4");

	/* Attempt illegal extra item write operation across maximum boundary line */
	int ret = rb_push(500);
	
	/* Assert error code matches standard No Space allocation rules */
	zassert_equal(ret, -ENOSPC, "Buffer accepted write operations over its fixed size parameters. Code: %d", ret);
}

/*
 * ============================================================================
 * Test Suite: ring_buf_boundaries
 *
 * Peek semantics and NULL-pointer boundary conditions.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_boundaries, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_boundaries, test_peek_does_not_consume)
{
	int v = 0;
	
	zassert_equal(rb_push(7), 0, "Failed to push initial data parameter 7 into tracking block");

	/* First look without extraction index advancement */
	int ret = rb_peek(&v);
	zassert_equal(ret, 0, "First execution pass profile for peek macro failed");
	zassert_equal(v, 7, "First check corrupt. Expected data variable contents 7, got %d", v);

	/* Second non-destructive verification look */
	v = 0; // Clear locally to isolate confirmation passes
	ret = rb_peek(&v);
	zassert_equal(ret, 0, "Second execution pass profile for peek macro failed");
	zassert_equal(v, 7, "Second check corrupt. Expected data variable contents 7, got %d", v);

	/* Prove structural tracking counters were completely unimpacted by peek look routines */
	zassert_equal(rb_count(), 1, "Internal layout indicators advanced or dropped state metrics during peek reads");
}

ZTEST(ring_buf_boundaries, test_pop_null_returns_einval)
{
	/* Passing NULL must activate validation guards returning Invalid Parameter Error */
	int ret = rb_pop(NULL);
	zassert_equal(ret, -EINVAL, "Buffer parsed NULL context reference handle without throwing an error code exception. Code: %d", ret);
}

ZTEST(ring_buf_boundaries, test_is_full_after_fill)
{
	zassert_false(rb_is_full(), "Empty structural tracker incorrectly marked full at initial test block boot");

	/* Load maximum size boundary metrics */
	rb_push(1);
	rb_push(2);
	rb_push(3);
	rb_push(4);

	/* Assert parameters validate completely packed states accurately */
	zassert_true(rb_is_full(), "Internal flag verification evaluation did not register completely full capacities");
	zassert_equal(rb_count(), 4, "Total registered storage indicators mismatch fixed allocation space tracking lengths");
}
